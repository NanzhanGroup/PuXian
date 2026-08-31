//! 普贤 (PuXian) WebSocket（RFC 6455）协议层 + 连接注册表（M22）
//! 语言层 API（builtin.rs 分派）：
//!   ws_serve(port, handler)  —— 服务端：每连接握手后调 handler(conn)
//!   ws_connect(host, port, path) → int conn | null（客户端握手）
//!   ws_send(conn, data) → bool / ws_recv(conn) → str|null / ws_close(conn) → bool
//! 本模块只做协议（握手/帧/掩码/分片/ping-pong/close）+ 连接注册表，
//! 不依赖解释器；服务器 accept 循环与 handler 调用在 builtin.rs。
//! 并发模型：注册表存 Arc<WsConn>，操作先取 Arc 再释放注册表锁 → 读写互不阻塞
//! （read/write 为独立 try_clone 句柄；ws_recv 阻塞读不影响 ws_send 推送）。

use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::sync::atomic::{AtomicBool, AtomicI64, Ordering};
use std::sync::{mpsc, Arc, Mutex, OnceLock};
use std::thread;
use std::time::Duration;

use crate::tls::SConn;

/// WebSocket 连接：读写用独立 try_clone 句柄（ws_recv 阻塞读不阻塞 ws_send）
/// M27：SConn 支持明文与 TLS（wss）统一读写
pub struct WsConn {
    pub read: Mutex<SConn>,
    pub write: Mutex<SConn>,
    pub closed: AtomicBool,
    pub close_tx: mpsc::Sender<()>,
    /// 最近一次读到任何帧的时间（毫秒时间戳，0=未初始化）；ws_heartbeat 超时检测用
    pub last_activity: AtomicI64,
}

type ConnMap = HashMap<i64, Arc<WsConn>>;

pub fn ws_conns() -> &'static Mutex<ConnMap> {
    static M: OnceLock<Mutex<ConnMap>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

static WS_NEXT_ID: AtomicI64 = AtomicI64::new(1);

/// 当前毫秒时间戳
fn now_millis() -> i64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_millis() as i64)
        .unwrap_or(0)
}

/// 注册连接，返回 conn id
pub fn ws_register(read: SConn, write: SConn) -> i64 {
    let id = WS_NEXT_ID.fetch_add(1, Ordering::SeqCst);
    let (close_tx, _rx) = mpsc::channel::<()>();
    ws_conns().lock().unwrap().insert(
        id,
        Arc::new(WsConn {
            read: Mutex::new(read),
            write: Mutex::new(write),
            closed: AtomicBool::new(false),
            close_tx,
            last_activity: AtomicI64::new(0),
        }),
    );
    id
}

/// 客户端 ws_connect 用：注册并返回 close_rx（供等待关闭）
pub fn ws_register_client(read: SConn, write: SConn) -> (i64, mpsc::Receiver<()>) {
    let id = WS_NEXT_ID.fetch_add(1, Ordering::SeqCst);
    let (close_tx, close_rx) = mpsc::channel::<()>();
    ws_conns().lock().unwrap().insert(
        id,
        Arc::new(WsConn {
            read: Mutex::new(read),
            write: Mutex::new(write),
            closed: AtomicBool::new(false),
            close_tx,
            last_activity: AtomicI64::new(0),
        }),
    );
    (id, close_rx)
}

pub fn ws_unregister(id: i64) {
    ws_conns().lock().unwrap().remove(&id);
}

/// 取连接 Arc（释放注册表锁后使用，避免阻塞其他 ws_* 操作）
fn ws_get(id: i64) -> Option<Arc<WsConn>> {
    ws_conns().lock().unwrap().get(&id).cloned()
}

/// 标记关闭并从注册表移除（返回 true 若确实移除）
fn ws_mark_closed(c: &Arc<WsConn>, id: i64) {
    c.closed.store(true, Ordering::SeqCst);
    let _ = c.close_tx.send(());
    ws_unregister(id);
}

// ==================== 握手（RFC 6455 §4.2） ====================
const WS_GUID: &str = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";

/// 读 HTTP 请求头直到 \r\n\r\n（上限 64KB），返回头部字节
fn read_http_header<R: Read>(stream: &mut R) -> Result<Vec<u8>, String> {
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 4096];
    loop {
        let n = stream.read(&mut tmp).map_err(|e| format!("读请求失败: {}", e))?;
        if n == 0 {
            return Err("连接已关闭".into());
        }
        buf.extend_from_slice(&tmp[..n]);
        if buf.windows(4).any(|w| w == b"\r\n\r\n") {
            return Ok(buf);
        }
        if buf.len() > 65536 {
            return Err("请求头超过 64KB".into());
        }
    }
}

/// 从请求头提取指定头字段（大小写不敏感）
fn extract_header_key(head: &[u8], name: &str) -> Option<String> {
    let text = String::from_utf8_lossy(head);
    for line in text.lines().skip(1) {
        if let Some(idx) = line.find(':') {
            let k = line[..idx].trim();
            if k.eq_ignore_ascii_case(name) {
                return Some(line[idx + 1..].trim().to_string());
            }
        }
    }
    None
}

/// 计算 Sec-WebSocket-Accept = base64(SHA1(key + GUID))
fn ws_accept(key: &str) -> String {
    let input = format!("{}{}", key, WS_GUID);
    let digest = sha1(input.as_bytes());
    crate::builtin::base64_encode_bytes(&digest)
}

/// 服务端握手：读客户端 Upgrade 请求，校验并回 101；失败返回 Err
pub fn server_handshake(stream: &mut SConn) -> Result<(), String> {
    let head = read_http_header(stream)?;
    let text = String::from_utf8_lossy(&head).to_string();
    let req_line = text.lines().next().unwrap_or("");
    if !req_line.starts_with("GET ") {
        return Err(format!("WebSocket 握手需要 GET 请求: {}", req_line));
    }
    let key = extract_header_key(&head, "Sec-WebSocket-Key").ok_or("缺少 Sec-WebSocket-Key")?;
    let upgrade = extract_header_key(&head, "Upgrade").unwrap_or_default();
    if !upgrade.eq_ignore_ascii_case("websocket") {
        return Err("Upgrade 头不是 websocket".into());
    }
    let accept = ws_accept(&key);
    let resp = format!(
        "HTTP/1.1 101 Switching Protocols\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Accept: {}\r\n\r\n",
        accept
    );
    stream
        .write_all(resp.as_bytes())
        .map_err(|e| format!("发送 101 失败: {}", e))?;
    stream.flush().ok();
    Ok(())
}

/// 客户端握手：发送 Upgrade 请求并校验 101 + Accept
pub fn client_handshake(stream: &mut TcpStream, host: &str, port: u16, path: &str) -> Result<(), String> {
    // 生成 16 字节随机 key（时间 + 计数器派生，仅需唯一性）
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_nanos() as u64)
        .unwrap_or(0);
    static SEQ: AtomicI64 = AtomicI64::new(0);
    let seq = SEQ.fetch_add(1, Ordering::Relaxed) as u64;
    let mut seed = now ^ (seq.wrapping_mul(0x9E3779B97F4A7C15));
    let mut key_bytes = [0u8; 16];
    for b in key_bytes.iter_mut() {
        seed = seed.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        *b = (seed >> 33) as u8;
    }
    let key = crate::builtin::base64_encode_bytes(&key_bytes);
    let req = format!(
        "GET {} HTTP/1.1\r\nHost: {}:{}\r\nUpgrade: websocket\r\nConnection: Upgrade\r\nSec-WebSocket-Key: {}\r\nSec-WebSocket-Version: 13\r\n\r\n",
        path, host, port, key
    );
    stream
        .write_all(req.as_bytes())
        .map_err(|e| format!("发送握手请求失败: {}", e))?;
    stream.flush().ok();
    let head = read_http_header(stream)?;
    let text = String::from_utf8_lossy(&head).to_string();
    let status = text.lines().next().unwrap_or("");
    if !status.contains(" 101 ") {
        return Err(format!("握手被拒绝: {}", status));
    }
    let expect = ws_accept(&key);
    let got = extract_header_key(&head, "Sec-WebSocket-Accept").unwrap_or_default();
    if got != expect {
        return Err("Sec-WebSocket-Accept 校验失败".into());
    }
    Ok(())
}

// ==================== 帧协议（RFC 6455 §5） ====================
const OP_CONT: u8 = 0x0;
const OP_TEXT: u8 = 0x1;
const OP_BINARY: u8 = 0x2;
const OP_CLOSE: u8 = 0x8;
const OP_PING: u8 = 0x9;
const OP_PONG: u8 = 0xA;

/// 精确读 n 字节（循环直到读满或 EOF/错误）
fn read_exact<R: Read>(stream: &mut R, buf: &mut [u8]) -> Result<(), WsErr> {
    let mut off = 0;
    while off < buf.len() {
        match stream.read(&mut buf[off..]) {
            Ok(0) => return Err(WsErr::Closed),
            Ok(n) => off += n,
            Err(e) if e.kind() == std::io::ErrorKind::Interrupted => continue,
            Err(e) if e.kind() == std::io::ErrorKind::WouldBlock
                || e.kind() == std::io::ErrorKind::TimedOut =>
            {
                return Err(WsErr::Timeout)
            }
            Err(e) => return Err(WsErr::Io(format!("读失败: {}", e))),
        }
    }
    Ok(())
}

/// 帧读取错误：Closed（对端关闭）/ Timeout（读超时，连接状态完好）/ Io（其他）
enum WsErr {
    Closed,
    Timeout,
    Io(String),
}

/// 读帧头（2 字节 + 扩展长度 + 掩码）。返回 (opcode, fin, 载荷长度, 是否掩码, 掩码键)。
/// M23 拆分为 head/body：head 读成功前超时 → 连接状态完好（帧边界安全超时）；
/// head 读到后清除超时再读 body，避免载荷中途超时破坏分片状态。
fn read_frame_head<R: Read>(stream: &mut R) -> Result<(u8, bool, u64, bool, [u8; 4]), WsErr> {
    let mut h = [0u8; 2];
    read_exact(stream, &mut h)?;
    let fin = (h[0] & 0x80) != 0;
    let opcode = h[0] & 0x0F;
    let masked = (h[1] & 0x80) != 0;
    let mut len = (h[1] & 0x7F) as u64;
    if len == 126 {
        let mut ext = [0u8; 2];
        read_exact(stream, &mut ext)?;
        len = u16::from_be_bytes(ext) as u64;
    } else if len == 127 {
        let mut ext = [0u8; 8];
        read_exact(stream, &mut ext)?;
        len = u64::from_be_bytes(ext);
    }
    if len > 64 * 1024 * 1024 {
        return Err(WsErr::Io("帧载荷超过 64MB".into()));
    }
    let mut mask = [0u8; 4];
    if masked {
        read_exact(stream, &mut mask)?;
    }
    Ok((opcode, fin, len, masked, mask))
}

fn read_frame_body<R: Read>(stream: &mut R, len: u64, masked: bool, mask: [u8; 4]) -> Result<Vec<u8>, WsErr> {
    let mut payload = vec![0u8; len as usize];
    if len > 0 {
        read_exact(stream, &mut payload)?;
        if masked {
            for (i, b) in payload.iter_mut().enumerate() {
                *b ^= mask[i & 3];
            }
        }
    }
    Ok(payload)
}

/// 读一帧（头 + 载荷）。返回 (opcode, fin, payload)
fn read_frame<R: Read>(stream: &mut R) -> Result<(u8, bool, Vec<u8>), WsErr> {
    let (opcode, fin, len, masked, mask) = read_frame_head(stream)?;
    let payload = read_frame_body(stream, len, masked, mask)?;
    Ok((opcode, fin, payload))
}

/// 编码一帧（服务端 → 客户端：不掩码）
fn encode_frame(opcode: u8, payload: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(payload.len() + 10);
    out.push(0x80 | opcode); // FIN=1
    let len = payload.len();
    if len < 126 {
        out.push(len as u8);
    } else if len <= 0xFFFF {
        out.push(126);
        out.extend_from_slice(&(len as u16).to_be_bytes());
    } else {
        out.push(127);
        out.extend_from_slice(&(len as u64).to_be_bytes());
    }
    out.extend_from_slice(payload);
    out
}

// ==================== 语言层 API ====================

/// ws_send(conn, data)：发送文本帧；失败标记关闭并清理
pub fn ws_send(conn: i64, data: &str) -> bool {
    let Some(c) = ws_get(conn) else { return false };
    if c.closed.load(Ordering::SeqCst) {
        return false;
    }
    let frame = encode_frame(OP_TEXT, data.as_bytes());
    let mut w = match c.write.lock() {
        Ok(w) => w,
        Err(_) => return false,
    };
    let r = w.write_all(&frame).and_then(|_| w.flush());
    match r {
        Ok(_) => true,
        Err(_) => {
            ws_mark_closed(&c, conn);
            false
        }
    }
}

/// ws_ping(conn)：发送 ping 帧（心跳保活；对端应回 pong，可从 ws_recv 路径自动应答）。
/// 失败（连接已关闭/写错误）→ false
pub fn ws_ping(conn: i64) -> bool {
    let Some(c) = ws_get(conn) else { return false };
    if c.closed.load(Ordering::SeqCst) {
        return false;
    }
    let frame = encode_frame(OP_PING, &[]);
    let mut w = match c.write.lock() {
        Ok(w) => w,
        Err(_) => return false,
    };
    let r = w.write_all(&frame).and_then(|_| w.flush());
    match r {
        Ok(_) => true,
        Err(_) => {
            ws_mark_closed(&c, conn);
            false
        }
    }
}

/// 心跳线程注册表（conn id → 运行标志），防止同连接重复启动
fn ws_heartbeats() -> &'static Mutex<HashMap<i64, Arc<AtomicBool>>> {
    static M: OnceLock<Mutex<HashMap<i64, Arc<AtomicBool>>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

/// ws_heartbeat(conn, interval_ms, timeout_ms)：为连接启动自动心跳（内置保活）。
/// - 每 interval_ms 发送一个 ping 帧（对端回 pong，应用层 ws_recv 自动应答/更新活动时间）；
/// - 若超过 timeout_ms 未读到任何帧（含 pong），判定死链：shutdown 连接 + 标记关闭
///   （阻塞中的 ws_recv 返回 null，应用层可感知断线）。
/// 连接不存在/已关闭 → false；同连接重复调用 → true（已启动，不重复起线程）。
pub fn ws_heartbeat(conn: i64, interval_ms: i64, timeout_ms: i64) -> bool {
    let Some(c) = ws_get(conn) else { return false };
    if c.closed.load(Ordering::SeqCst) {
        return false;
    }
    {
        let mut m = ws_heartbeats().lock().unwrap();
        if m.get(&conn).map(|f| f.load(Ordering::SeqCst)).unwrap_or(false) {
            return true; // 已启动
        }
        m.insert(conn, Arc::new(AtomicBool::new(true)));
    }
    let interval = if interval_ms > 0 { interval_ms as u64 } else { 10_000 };
    let timeout = if timeout_ms > 0 { timeout_ms as u64 } else { 60_000 };
    thread::spawn(move || {
        loop {
            thread::sleep(Duration::from_millis(interval));
            let c = match ws_get(conn) {
                Some(c) => c,
                None => break, // 连接已被移除（ws_close / 断开）
            };
            if c.closed.load(Ordering::SeqCst) {
                break;
            }
            // 1) 发 ping 保活
            let frame = encode_frame(OP_PING, &[]);
            let mut dead = false;
            {
                let mut w = match c.write.lock() {
                    Ok(w) => w,
                    Err(_) => {
                        dead = true;
                        return;
                    }
                };
                if w.write_all(&frame).and_then(|_| w.flush()).is_err() {
                    dead = true;
                }
            }
            if dead {
                ws_mark_closed(&c, conn);
                break;
            }
            // 2) 超时检测：超过 timeout 未读到任何帧 → 死链
            let last = c.last_activity.load(Ordering::SeqCst);
            if last > 0 && now_millis().saturating_sub(last) > timeout as i64 {
                let _ = c
                    .write
                    .lock()
                    .map(|mut w| w.shutdown());
                ws_mark_closed(&c, conn);
                break;
            }
        }
        ws_heartbeats().lock().unwrap().remove(&conn);
    });
    true
}

/// ws_recv(conn[, timeout_ms])：读一条完整消息（自动重组分片、回复 ping）。
/// timeout_ms 缺省 → 阻塞；指定 → 帧边界等待超时返回 None（连接状态完好，可继续使用）。
/// 关闭/错误 → None。
pub fn ws_recv(conn: i64, timeout_ms: Option<i64>) -> Option<String> {
    let Some(c) = ws_get(conn) else { return None };
    if c.closed.load(Ordering::SeqCst) {
        return None;
    }
    let mut msg: Vec<u8> = Vec::new();
    loop {
        // 仅在本轮循环起始（等待新帧头）时应用超时；读到帧头后清除，
        // 避免载荷传输中途超时破坏分片/连接状态。
        let timed = msg.is_empty() && timeout_ms.is_some();
        let frame = {
            let mut r = match c.read.lock() {
                Ok(r) => r,
                Err(_) => return None,
            };
            if timed {
                let ms = timeout_ms.unwrap();
                if ms <= 0 {
                    let _ = r.set_read_timeout(None);
                } else {
                    let _ = r.set_read_timeout(Some(Duration::from_millis(ms as u64)));
                }
            }
            match read_frame_head(&mut *r) {
                Ok((opcode, fin, len, masked, mask)) => {
                    let _ = r.set_read_timeout(None);
                    match read_frame_body(&mut *r, len, masked, mask) {
                        Ok(payload) => Ok((opcode, fin, payload)),
                        Err(e) => Err(e),
                    }
                }
                Err(WsErr::Timeout) => {
                    let _ = r.set_read_timeout(None);
                    return None; // 帧边界超时：连接完好，不标记关闭
                }
                Err(e) => Err(e),
            }
        };
        let (opcode, fin, payload) = match frame {
            Ok(f) => f,
            Err(_) => {
                ws_mark_closed(&c, conn);
                return None;
            }
        };
        // 心跳：读到任何帧（含 pong）即更新最近活动时间
        c.last_activity.store(now_millis(), Ordering::SeqCst);
        match opcode {
            OP_PING => {
                // 自动回 pong
                let pong = encode_frame(OP_PONG, &payload);
                let mut w = match c.write.lock() {
                    Ok(w) => w,
                    Err(_) => return None,
                };
                let _ = w.write_all(&pong);
                continue;
            }
            OP_CLOSE => {
                // 回 close 并关闭
                let close = encode_frame(OP_CLOSE, &[]);
                let mut w = match c.write.lock() {
                    Ok(w) => w,
                    Err(_) => return None,
                };
                let _ = w.write_all(&close);
                ws_mark_closed(&c, conn);
                return None;
            }
            OP_TEXT | OP_BINARY | OP_CONT => {
                msg.extend_from_slice(&payload);
                if fin {
                    return Some(String::from_utf8_lossy(&msg).to_string());
                }
            }
            _ => { /* 忽略未知 opcode（含 PONG） */ }
        }
    }
}

/// ws_close(conn)：发送 close 帧 + 关闭连接
pub fn ws_close(conn: i64) -> bool {
    let Some(c) = ws_get(conn) else { return false };
    if c.closed.load(Ordering::SeqCst) {
        ws_unregister(conn);
        return false;
    }
    let close = encode_frame(OP_CLOSE, &[0x03, 0xE8]); // 1000 正常关闭
    let mut w = match c.write.lock() {
        Ok(w) => w,
        Err(_) => return false,
    };
    let _ = w.write_all(&close);
    let _ = w.shutdown();
    ws_mark_closed(&c, conn);
    true
}

// ==================== SHA-1（FIPS 180-4，纯标准库） ====================
pub fn sha1(data: &[u8]) -> [u8; 20] {
    let mut h: [u32; 5] = [0x67452301, 0xEFCDAB89, 0x98BADCFE, 0x10325476, 0xC3D2E1F0];
    let mut msg = data.to_vec();
    let bit_len = (data.len() as u64).wrapping_mul(8);
    msg.push(0x80);
    while msg.len() % 64 != 56 {
        msg.push(0);
    }
    msg.extend_from_slice(&bit_len.to_be_bytes());
    let mut w = [0u32; 80];
    for chunk in msg.chunks_exact(64) {
        for i in 0..16 {
            w[i] = u32::from_be_bytes([chunk[i * 4], chunk[i * 4 + 1], chunk[i * 4 + 2], chunk[i * 4 + 3]]);
        }
        for i in 16..80 {
            w[i] = (w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16]).rotate_left(1);
        }
        let (mut a, mut b, mut c, mut d, mut e) = (h[0], h[1], h[2], h[3], h[4]);
        for i in 0..80 {
            let (f, k) = match i {
                0..=19 => ((b & c) | ((!b) & d), 0x5A827999u32),
                20..=39 => (b ^ c ^ d, 0x6ED9EBA1),
                40..=59 => ((b & c) | (b & d) | (c & d), 0x8F1BBCDC),
                _ => (b ^ c ^ d, 0xCA62C1D6),
            };
            let tmp = a
                .rotate_left(5)
                .wrapping_add(f)
                .wrapping_add(e)
                .wrapping_add(k)
                .wrapping_add(w[i]);
            e = d;
            d = c;
            c = b.rotate_left(30);
            b = a;
            a = tmp;
        }
        h[0] = h[0].wrapping_add(a);
        h[1] = h[1].wrapping_add(b);
        h[2] = h[2].wrapping_add(c);
        h[3] = h[3].wrapping_add(d);
        h[4] = h[4].wrapping_add(e);
    }
    let mut out = [0u8; 20];
    for (i, hv) in h.iter().enumerate() {
        out[i * 4..i * 4 + 4].copy_from_slice(&hv.to_be_bytes());
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_sha1_vectors() {
        // RFC 3174 官方向量
        assert_eq!(
            sha1(b""),
            *b"\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09"
        );
        assert_eq!(
            sha1(b"abc"),
            *b"\xa9\x99\x3e\x36\x47\x06\x81\x6a\xba\x3e\x25\x71\x78\x50\xc2\x6c\x9c\xd0\xd8\x9d"
        );
        assert_eq!(
            sha1(b"abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq"),
            *b"\x84\x98\x3e\x44\x1c\x3b\xd2\x6e\xba\xae\x4a\xa1\xf9\x51\x29\xe5\xe5\x46\x70\xf1"
        );
    }

    #[test]
    fn test_ws_accept() {
        // RFC 6455 §1.3 示例
        assert_eq!(
            ws_accept("dGhlIHNhbXBsZSBub25jZQ=="),
            "s3pPLMBiTxaQ9kYGzzhZRbK+xOo="
        );
    }

    #[test]
    fn test_frame_roundtrip() {
        let f = encode_frame(OP_TEXT, b"hello");
        assert_eq!(f, vec![0x81, 0x05, b'h', b'e', b'l', b'l', b'o']);
    }
}
