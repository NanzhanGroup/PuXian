//! M27 P0-1：服务端 TLS（HTTPS / WSS / SSE-over-TLS）
//!
//! 语言层 API（builtin.rs 分派）：`tls_server(cert_pem, key_pem) -> bool`
//! - cert_pem / key_pem 为 PEM 文件路径，或直接 PEM 内容（含 "-----BEGIN" 前缀）
//! - 注册后 px_serve / ws_serve / sse_serve 自动接受 TLS 连接（双模式一致）
//!
//! 连接抽象：`SConn`（Plain / Tls 统一 Read+Write），服务端 accept 后若已注册
//! 服务端 TLS 配置则先做 TLS 握手，否则透传明文。

use std::net::{TcpListener, TcpStream};
use std::sync::{Arc, Mutex, OnceLock};
use std::time::Duration;

/// 服务端 TLS 连接：明文 TCP 或 rustls 服务端会话。
/// 内部为 Arc<Mutex>：try_clone 可安全共享（sse_send / ws 读写独立句柄场景）。
pub struct SConn {
    inner: Arc<Mutex<SConnInner>>,
}

enum SConnInner {
    Plain(TcpStream),
    Tls(Box<rustls::StreamOwned<rustls::ServerConnection, TcpStream>>),
}

impl SConn {
    pub fn plain(s: TcpStream) -> Self {
        SConn {
            inner: Arc::new(Mutex::new(SConnInner::Plain(s))),
        }
    }
    /// 明文连接克隆（客户端 ws_connect：注册读写独立句柄）
    pub fn plain_try_clone(s: &TcpStream) -> std::io::Result<Self> {
        Ok(SConn::plain(s.try_clone()?))
    }
    fn tls(t: rustls::StreamOwned<rustls::ServerConnection, TcpStream>) -> Self {
        SConn {
            inner: Arc::new(Mutex::new(SConnInner::Tls(Box::new(t)))),
        }
    }
    /// 克隆句柄（共享同一底层连接；sse_send 等跨线程写场景）
    pub fn try_clone(&self) -> std::io::Result<Self> {
        Ok(SConn {
            inner: self.inner.clone(),
        })
    }
    pub fn shutdown(&mut self) {
        let mut g = self.inner.lock().unwrap();
        match &mut *g {
            SConnInner::Plain(s) => {
                let _ = s.shutdown(std::net::Shutdown::Both);
            }
            SConnInner::Tls(t) => {
                let _ = t.sock.shutdown(std::net::Shutdown::Both);
            }
        }
    }
    /// 对端地址字符串
    pub fn peer_addr_str(&self) -> String {
        let g = self.inner.lock().unwrap();
        match &*g {
            SConnInner::Plain(s) => s.peer_addr().map(|a| a.to_string()).unwrap_or_default(),
            SConnInner::Tls(t) => t.sock.peer_addr().map(|a| a.to_string()).unwrap_or_default(),
        }
    }
    /// 读超时（ws_recv 帧边界超时；TLS 转发到底层 sock）
    pub fn set_read_timeout(&self, dur: Option<Duration>) -> std::io::Result<()> {
        let g = self.inner.lock().unwrap();
        match &*g {
            SConnInner::Plain(s) => s.set_read_timeout(dur),
            SConnInner::Tls(t) => t.sock.set_read_timeout(dur),
        }
    }
}

impl std::io::Read for SConn {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        let mut g = self.inner.lock().unwrap();
        match &mut *g {
            SConnInner::Plain(s) => s.read(buf),
            SConnInner::Tls(t) => t.read(buf),
        }
    }
}
impl std::io::Write for SConn {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        let mut g = self.inner.lock().unwrap();
        match &mut *g {
            SConnInner::Plain(s) => s.write(buf),
            SConnInner::Tls(t) => t.write(buf),
        }
    }
    fn flush(&mut self) -> std::io::Result<()> {
        let mut g = self.inner.lock().unwrap();
        match &mut *g {
            SConnInner::Plain(s) => s.flush(),
            SConnInner::Tls(t) => t.flush(),
        }
    }
}

/// 进程级服务端 TLS 配置（可重复注册覆盖）
static TLS_SERVER: OnceLock<Mutex<Option<Arc<rustls::ServerConfig>>>> = OnceLock::new();

fn tls_server_lock() -> &'static Mutex<Option<Arc<rustls::ServerConfig>>> {
    TLS_SERVER.get_or_init(|| Mutex::new(None))
}

/// 是否已注册服务端 TLS 配置
pub fn tls_server_configured() -> bool {
    tls_server_lock().lock().unwrap().is_some()
}

fn tls_server_config() -> Option<Arc<rustls::ServerConfig>> {
    tls_server_lock().lock().unwrap().clone()
}

// ==================== PEM 解析（不引入 rustls-pemfile） ====================

/// 宽松 base64 解码（忽略空白/换行；非法字符返回 None）
fn pem_b64_decode(s: &str) -> Option<Vec<u8>> {
    let mut out = Vec::with_capacity(s.len() / 4 * 3 + 3);
    let mut acc: u32 = 0;
    let mut nbits = 0u32;
    for c in s.bytes() {
        let v = match c {
            b'A'..=b'Z' => c - b'A',
            b'a'..=b'z' => c - b'a' + 26,
            b'0'..=b'9' => c - b'0' + 52,
            b'+' => 62,
            b'/' => 63,
            b'=' | b'\r' | b'\n' | b' ' | b'\t' => continue,
            _ => return None,
        };
        acc = (acc << 6) | v as u32;
        nbits += 6;
        if nbits >= 8 {
            nbits -= 8;
            out.push((acc >> nbits) as u8);
            acc &= (1u32 << nbits) - 1;
        }
    }
    Some(out)
}

/// 从 PEM 文本提取所有指定标签块（如 "CERTIFICATE" / "PRIVATE KEY" / "RSA PRIVATE KEY"）
fn pem_blocks(text: &str, tag: &str) -> Vec<Vec<u8>> {
    let begin = format!("-----BEGIN {}-----", tag);
    let end = format!("-----END {}-----", tag);
    let mut blocks = Vec::new();
    let mut rest = text;
    while let Some(b) = rest.find(&begin) {
        let after = &rest[b + begin.len()..];
        if let Some(e) = after.find(&end) {
            if let Some(der) = pem_b64_decode(&after[..e]) {
                blocks.push(der);
            }
            rest = &after[e + end.len()..];
        } else {
            break;
        }
    }
    blocks
}

/// 解析证书链（PEM 可多块；单 DER 也接受）
fn parse_certs(input: &str) -> Result<Vec<rustls::pki_types::CertificateDer<'static>>, String> {
    let data = input.as_bytes();
    let text = if std::str::from_utf8(data).is_ok() {
        input.to_string()
    } else {
        // 原始 DER 文件：按路径读入的二进制
        return Ok(vec![rustls::pki_types::CertificateDer::from(data.to_vec())]);
    };
    let blocks = pem_blocks(&text, "CERTIFICATE");
    if blocks.is_empty() {
        return Err("未找到 PEM 证书块（-----BEGIN CERTIFICATE-----）".into());
    }
    Ok(blocks.into_iter().map(rustls::pki_types::CertificateDer::from).collect())
}

/// 解析私钥（PKCS#8 "PRIVATE KEY" 或 PKCS#1 "RSA PRIVATE KEY"）
fn parse_private_key(input: &str) -> Result<rustls::pki_types::PrivateKeyDer<'static>, String> {
    let text = if input.contains("-----BEGIN") {
        input.to_string()
    } else {
        std::fs::read_to_string(input)
            .map_err(|e| format!("读取私钥失败: {}", e))?
    };
    // PKCS#8
    let pkcs8 = pem_blocks(&text, "PRIVATE KEY");
    if let Some(der) = pkcs8.into_iter().next() {
        return Ok(rustls::pki_types::PrivateKeyDer::Pkcs8(
            rustls::pki_types::PrivatePkcs8KeyDer::from(der),
        ));
    }
    // PKCS#1 RSA
    let pkcs1 = pem_blocks(&text, "RSA PRIVATE KEY");
    if let Some(der) = pkcs1.into_iter().next() {
        return Ok(rustls::pki_types::PrivateKeyDer::Pkcs1(
            rustls::pki_types::PrivatePkcs1KeyDer::from(der),
        ));
    }
    Err("未找到私钥（-----BEGIN PRIVATE KEY----- 或 -----BEGIN RSA PRIVATE KEY-----）".into())
}

/// 注册服务端 TLS 配置（cert/key 为 PEM 路径或 PEM 内容）
pub fn tls_server_register(cert_pem: &str, key_pem: &str) -> Result<(), String> {
    let cert_text = if cert_pem.contains("-----BEGIN") {
        cert_pem.to_string()
    } else {
        std::fs::read_to_string(cert_pem)
            .map_err(|e| format!("读取证书失败 {}: {}", cert_pem, e))?
    };
    let certs = parse_certs(&cert_text)?;
    let key = parse_private_key(key_pem)?;
    let mut cfg = rustls::ServerConfig::builder()
        .with_no_client_auth()
        .with_single_cert(certs, key)
        .map_err(|e| format!("TLS 配置失败: {}", e))?;
    // M30：服务端 https 连接池——TLS 会话缓存（Session ID 缓存 + 票据）
    // 新 HTTPS 连接可凭 Session ID / NewSessionTicket 快速恢复握手，省一次往返。
    cfg.session_storage = rustls::server::ServerSessionMemoryCache::new(256);
    cfg.ticketer = rustls::crypto::ring::Ticketer::new()
        .map_err(|e| format!("TLS 票据生成失败: {}", e))?;
    *tls_server_lock().lock().unwrap() = Some(Arc::new(cfg));
    Ok(())
}

/// accept 一个已建立的 TCP 连接：若已注册服务端 TLS 则做 TLS 握手，否则明文。
/// 握手失败返回 None（连接已关闭）。
pub fn accept_stream(stream: TcpStream) -> Option<SConn> {
    let _ = stream.set_nodelay(true);
    let cfg = tls_server_config();
    match cfg {
        None => Some(SConn::plain(stream)),
        Some(cfg) => {
            let conn = match rustls::ServerConnection::new(cfg) {
                Ok(c) => c,
                Err(_) => return None,
            };
            let mut tls = rustls::StreamOwned::new(conn, stream);
            let mut stalls = 0;
            while tls.conn.is_handshaking() {
                match tls.conn.complete_io(&mut tls.sock) {
                    Ok((r, w)) if r == 0 && w == 0 => {
                        stalls += 1;
                        if stalls > 5 {
                            return None;
                        }
                    }
                    Ok(_) => stalls = 0,
                    Err(_) => return None,
                }
            }
            Some(SConn::tls(tls))
        }
    }
}

/// accept 一个客户端连接（listener 阻塞模式）：TLS 或明文
pub fn accept_conn(listener: &TcpListener) -> Option<SConn> {
    let (stream, _peer) = listener.accept().ok()?;
    accept_stream(stream)
}

// ==================== 服务端 TLS 单测 ====================

#[cfg(test)]
mod tests {
    use super::*;
    use std::io::{Read, Write};

    /// 生成自签证书 + PKCS#8 私钥（Rust 侧测试用：调用系统 openssl）
    fn gen_self_signed(dir: &std::path::Path) -> (String, String) {
        let cert = dir.join("cert.pem");
        let key = dir.join("key.pem");
        let st = std::process::Command::new("openssl")
            .args([
                "req", "-x509", "-newkey", "rsa:2048", "-keyout",
                key.to_str().unwrap(), "-out", cert.to_str().unwrap(),
                "-days", "1", "-nodes", "-subj", "/CN=localhost",
                "-addext", "basicConstraints=critical,CA:FALSE",
                "-addext", "subjectAltName=DNS:localhost",
            ])
            .output()
            .expect("openssl 不可用");
        assert!(st.status.success(), "openssl 生成证书失败: {:?}", st);
        (
            std::fs::read_to_string(&cert).unwrap(),
            std::fs::read_to_string(&key).unwrap(),
        )
    }

    #[test]
    fn test_pem_block_parse() {
        let dir = std::env::temp_dir().join(format!("px_tls_test_{}", std::process::id()));
        std::fs::create_dir_all(&dir).unwrap();
        let (cert, key) = gen_self_signed(&dir);
        // 证书块解析
        let certs = parse_certs(&cert).unwrap();
        assert_eq!(certs.len(), 1);
        assert!(certs[0].as_ref().len() > 100);
        // 私钥解析（openssl req -newkey 生成 PKCS#8）
        let pk = parse_private_key(&key).unwrap();
        assert!(matches!(
            pk,
            rustls::pki_types::PrivateKeyDer::Pkcs8(_)
        ));
        // ServerConfig 构建
        let _cfg = rustls::ServerConfig::builder()
            .with_no_client_auth()
            .with_single_cert(certs.clone(), pk)
            .expect("ServerConfig 构建失败");
        // 清理
        let _ = std::fs::remove_dir_all(&dir);
    }

    #[test]
    fn test_tls_server_roundtrip() {
        let dir = std::env::temp_dir().join(format!("px_tls_rt_{}", std::process::id()));
        std::fs::create_dir_all(&dir).unwrap();
        let (cert, key) = gen_self_signed(&dir);
        // 注册服务端 TLS（用文件路径）
        tls_server_register(
            cert.as_bytes().is_empty().then(|| "").unwrap_or(""),
            "",
        )
        .unwrap_err(); // 空参数应报错
        let cert_path = dir.join("cert.pem");
        let key_path = dir.join("key.pem");
        tls_server_register(cert_path.to_str().unwrap(), key_path.to_str().unwrap()).unwrap();
        assert!(tls_server_configured());

        // 客户端用 rustls + 不校验证书（测试目的）连接
        let listener = TcpListener::bind("127.0.0.1:0").unwrap();
        let addr = listener.local_addr().unwrap();
        let server = std::thread::spawn(move || {
            let mut conn = accept_conn(&listener).expect("accept_conn 失败");
            let mut buf = [0u8; 64];
            let n = conn.read(&mut buf).unwrap();
            assert!(n > 0);
            conn.write_all(b"pong").unwrap();
            conn.flush().unwrap();
        });

        // 客户端：rustls 客户端（不安全校验，测试用）
        let mut roots = rustls::RootCertStore::empty();
        let certs = parse_certs(&std::fs::read_to_string(&cert_path).unwrap()).unwrap();
        let _ = roots.add(certs[0].clone());
        let cfg = rustls::ClientConfig::builder()
            .with_root_certificates(roots)
            .with_no_client_auth();
        let sn = rustls::pki_types::ServerName::try_from("localhost").unwrap();
        let sock = std::net::TcpStream::connect(addr).unwrap();
        let conn = rustls::ClientConnection::new(Arc::new(cfg), sn).unwrap();
        let mut tls = rustls::StreamOwned::new(conn, sock);
        tls.write_all(b"ping").unwrap();
        tls.flush().unwrap();
        let mut buf = [0u8; 16];
        let n = tls.read(&mut buf).unwrap();
        assert_eq!(&buf[..n], b"pong");
        drop(tls);
        server.join().unwrap();
        let _ = std::fs::remove_dir_all(&dir);
    }
}
