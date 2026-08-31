//! 包管理器（M9）：px pkg 子命令
//!
//! 支持：
//! - `px pkg init [--name NAME]`         初始化项目（生成 px.toml）
//! - `px pkg add <path> [--name ALIAS]`  添加本地依赖（复制到 .px_modules/ 并记录）
//! - `px pkg install`                    按 px.toml 安装依赖（本地路径复制，幂等）
//! - `px pkg list`                       列出已安装依赖
//! - `px pkg remove <name>`              移除依赖
//!
//! px.toml 格式（零依赖手写解析，不引入 toml crate）：
//! ```toml
//! [package]
//! name = "myapp"
//! version = "0.1.0"
//!
//! [dependencies]
//! mylib = "../mylib"
//! ```

use std::collections::BTreeMap;
use std::fs;
use std::io::{Read, Write};
use std::net::TcpStream;
use std::path::{Path, PathBuf};

pub const PKG_FILE: &str = "px.toml";
pub const MODULES_DIR: &str = ".px_modules";

// ==================== M26 远程包 registry ====================
// px.toml 依赖值支持 URL（http:// 或 https://）：
//   mylib = "https://registry.example.com/mylib.px"          —— 远程单文件
//   mylib = "https://registry.example.com/mylib.zip"         —— 远程 zip 包（解压到 .px_modules/mylib/）
//   mylib = "https://.../mylib.px#sha256=HEX"                —— 可选 sha256 校验（下载后比对）
// 缓存：~/.px/cache/<sha256(url)>.bin（同 URL 只下载一次；install 幂等复用）。
// 双模式无关（CLI 层功能，纯 Rust）。

/// URL 依赖判断
fn is_url(s: &str) -> bool {
    s.starts_with("http://") || s.starts_with("https://")
}

/// 统一连接：明文或 TLS（rustls，webpki-roots + PX_TLS_CA_FILE 追加信任）
enum Conn {
    Plain(TcpStream),
    Tls(rustls::StreamOwned<rustls::ClientConnection, TcpStream>),
}
impl Read for Conn {
    fn read(&mut self, buf: &mut [u8]) -> std::io::Result<usize> {
        match self {
            Conn::Plain(s) => s.read(buf),
            Conn::Tls(t) => t.read(buf),
        }
    }
}
impl Write for Conn {
    fn write(&mut self, buf: &[u8]) -> std::io::Result<usize> {
        match self {
            Conn::Plain(s) => s.write(buf),
            Conn::Tls(t) => t.write(buf),
        }
    }
    fn flush(&mut self) -> std::io::Result<()> {
        match self {
            Conn::Plain(s) => s.flush(),
            Conn::Tls(t) => t.flush(),
        }
    }
}

fn tls_config() -> std::sync::Arc<rustls::ClientConfig> {
    let mut roots = rustls::RootCertStore::empty();
    roots.extend(webpki_roots::TLS_SERVER_ROOTS.iter().cloned());
    if let Ok(p) = std::env::var("PX_TLS_CA_FILE") {
        if let Ok(certs) = crate::builtin::load_ca_der(&p) {
            for der in certs {
                let _ = roots.add(der);
            }
        }
    }
    std::sync::Arc::new(
        rustls::ClientConfig::builder()
            .with_root_certificates(roots)
            .with_no_client_auth(),
    )
}

fn find_header_end(buf: &[u8]) -> Option<usize> {
    buf.windows(4).position(|w| w == b"\r\n\r\n")
}

/// HTTP GET 下载整个响应体（支持 Content-Length / chunked / 读到 EOF）。
fn http_get(url: &str) -> Result<Vec<u8>, String> {
    let (scheme, rest) = if let Some(r) = url.strip_prefix("https://") {
        ("https", r)
    } else if let Some(r) = url.strip_prefix("http://") {
        ("http", r)
    } else {
        return Err(format!("不支持的协议: {}", url));
    };
    let (hostport, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
    if hostport.is_empty() {
        return Err("URL 缺少主机名".into());
    }
    let (host, port) = match hostport.find(':') {
        Some(i) => (
            hostport[..i].to_string(),
            hostport[i + 1..]
                .parse::<u16>()
                .map_err(|_| format!("端口非法: {}", hostport))?,
        ),
        None => (
            hostport.to_string(),
            if scheme == "https" { 443u16 } else { 80u16 },
        ),
    };
    let mut conn = if scheme == "https" {
        let cfg = tls_config();
        let sn = rustls::pki_types::ServerName::try_from(host.clone())
            .map_err(|_| format!("非法主机名: {}", host))?;
        let sock = TcpStream::connect((host.as_str(), port))
            .map_err(|e| format!("连接 {}:{} 失败: {}", host, port, e))?;
        let c = rustls::ClientConnection::new(cfg, sn)
            .map_err(|e| format!("TLS 初始化失败: {}", e))?;
        Conn::Tls(rustls::StreamOwned::new(c, sock))
    } else {
        Conn::Plain(
            TcpStream::connect((host.as_str(), port))
                .map_err(|e| format!("连接 {}:{} 失败: {}", host, port, e))?,
        )
    };
    let host_header = if hostport.contains(':') {
        hostport.to_string()
    } else {
        format!("{}:{}", host, port)
    };
    let req = format!(
        "GET {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: PuXian-Pkg/0.1\r\nAccept: */*\r\nConnection: close\r\n\r\n",
        path, host_header
    );
    conn.write_all(req.as_bytes())
        .map_err(|e| format!("发送请求失败: {}", e))?;
    // 读响应头（直到 \r\n\r\n，上限 64KB）
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 8192];
    let mut header_end = None;
    while buf.len() < 65536 {
        let n = conn.read(&mut tmp).map_err(|e| format!("读响应头失败: {}", e))?;
        if n == 0 {
            break;
        }
        buf.extend_from_slice(&tmp[..n]);
        if let Some(idx) = find_header_end(&buf) {
            header_end = Some(idx);
            break;
        }
    }
    let idx = header_end.ok_or("响应头不完整")?;
    let head = String::from_utf8_lossy(&buf[..idx]).to_string();
    let status = head
        .lines()
        .next()
        .and_then(|l| l.split_whitespace().nth(1))
        .and_then(|s| s.parse::<u16>().ok())
        .unwrap_or(0);
    if status != 200 {
        return Err(format!(
            "HTTP {}: {}",
            status,
            head.lines().next().unwrap_or("")
        ));
    }
    let mut cl: Option<usize> = None;
    let mut chunked = false;
    for line in head.lines().skip(1) {
        let l = line.trim();
        if let Some(v) = l
            .strip_prefix("Content-Length:")
            .or_else(|| l.strip_prefix("content-length:"))
        {
            cl = v.trim().parse::<usize>().ok();
        }
        if let Some(v) = l
            .strip_prefix("Transfer-Encoding:")
            .or_else(|| l.strip_prefix("transfer-encoding:"))
        {
            chunked = v.trim().to_lowercase().contains("chunked");
        }
    }
    let mut body = buf[idx + 4..].to_vec();
    if chunked {
        let mut out: Vec<u8> = Vec::new();
        let mut rest: &[u8] = &body;
        loop {
            let eol = rest
                .windows(2)
                .position(|w| w == b"\r\n")
                .ok_or("chunked 格式错误")?;
            let size_str = String::from_utf8_lossy(&rest[..eol]).to_string();
            let sz = usize::from_str_radix(size_str.trim(), 16).map_err(|_| "chunk 大小非法")?;
            rest = &rest[eol + 2..];
            if sz == 0 {
                break;
            }
            if rest.len() < sz + 2 {
                return Err("chunk 数据不完整".into());
            }
            out.extend_from_slice(&rest[..sz]);
            rest = &rest[sz + 2..];
        }
        body = out;
    } else if let Some(cl) = cl {
        while body.len() < cl {
            let n = conn
                .read(&mut tmp)
                .map_err(|e| format!("读 body 失败: {}", e))?;
            if n == 0 {
                break;
            }
            body.extend_from_slice(&tmp[..n]);
        }
        body.truncate(cl.min(body.len()));
    } else {
        loop {
            let n = conn
                .read(&mut tmp)
                .map_err(|e| format!("读 body 失败: {}", e))?;
            if n == 0 {
                break;
            }
            body.extend_from_slice(&tmp[..n]);
        }
    }
    Ok(body)
}

fn sha256_hex(data: &[u8]) -> String {
    crate::crypto::sha256_hex(data)
}

/// 缓存路径：~/.px/cache/<sha256(url)>.bin
fn cache_path(url_no_frag: &str) -> PathBuf {
    let home = std::env::var("HOME").unwrap_or_else(|_| ".".into());
    PathBuf::from(home)
        .join(".px")
        .join("cache")
        .join(format!("{}.bin", sha256_hex(url_no_frag.as_bytes())))
}

/// 安装一个 URL 依赖到 .px_modules/<name>/（带缓存与可选 sha256 校验）
fn install_url_dep(name: &str, url: &str, modules_dir: &Path) -> Result<String, String> {
    // 支持 #sha256=HEX 片段
    let (base_url, expect_sha) = match url.find("#sha256=") {
        Some(i) => (&url[..i], Some(url[i + 8..].to_string())),
        None => (url, None),
    };
    let cache = cache_path(base_url);
    let data = if cache.exists() {
        fs::read(&cache).map_err(|e| format!("读缓存失败: {}", e))?
    } else {
        let d = http_get(base_url)?;
        if let Some(parent) = cache.parent() {
            let _ = fs::create_dir_all(parent);
        }
        let _ = fs::write(&cache, &d);
        d
    };
    let got = sha256_hex(&data);
    if let Some(exp) = expect_sha {
        if !got.eq_ignore_ascii_case(&exp) {
            return Err(format!(
                "{} sha256 校验失败: 期望 {} 实际 {}",
                name, exp, got
            ));
        }
    }
    let dst = modules_dir.join(name);
    fs::create_dir_all(&dst).map_err(|e| format!("创建 {} 目录失败: {}", name, e))?;
    let is_zip = base_url.ends_with(".zip") || data.starts_with(b"PK\x03\x04");
    if is_zip {
        let tmp = std::env::temp_dir().join(format!("px_pkg_{}_{}.zip", name, std::process::id()));
        fs::write(&tmp, &data).map_err(|e| format!("写临时 zip 失败: {}", e))?;
        let r = crate::zip::unpack(tmp.to_str().unwrap(), dst.to_str().unwrap());
        let _ = fs::remove_file(&tmp);
        r.map_err(|e| format!("解压 {} 失败: {}", name, e))?;
    } else {
        let fname = match base_url.rsplit('/').next() {
            Some(f) if f.ends_with(".px") && !f.is_empty() => f.to_string(),
            _ => format!("{}.px", name),
        };
        fs::write(dst.join(fname), &data).map_err(|e| format!("写入 {} 失败: {}", name, e))?;
    }
    Ok(format!("{}（sha256={}…）", name, &got[..16]))
}

/// 解析 px.toml → (package_name, version, dependencies)
/// 返回值：name, version, deps(name → path)
pub fn parse_manifest(src: &str) -> (String, String, BTreeMap<String, String>) {
    let mut name = String::new();
    let mut version = String::new();
    let mut deps: BTreeMap<String, String> = BTreeMap::new();
    let mut section = String::new();
    for line in src.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        if line.starts_with('[') && line.ends_with(']') {
            section = line[1..line.len() - 1].trim().to_string();
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            let k = k.trim().to_string();
            let v = v.trim().trim_matches('"').to_string();
            match section.as_str() {
                "package" => {
                    if k == "name" {
                        name = v;
                    } else if k == "version" {
                        version = v;
                    }
                }
                "dependencies" => {
                    deps.insert(k, v);
                }
                _ => {}
            }
        }
    }
    (name, version, deps)
}

/// 序列化 px.toml
pub fn serialize_manifest(name: &str, version: &str, deps: &BTreeMap<String, String>) -> String {
    let mut s = String::new();
    s.push_str("[package]\n");
    s.push_str(&format!("name = \"{}\"\n", name));
    s.push_str(&format!("version = \"{}\"\n", version));
    if !deps.is_empty() {
        s.push_str("\n[dependencies]\n");
        for (k, v) in deps {
            s.push_str(&format!("{} = \"{}\"\n", k, v));
        }
    }
    s
}

/// 读取项目清单（不存在返回 None）
fn read_manifest(dir: &Path) -> Option<(String, String, BTreeMap<String, String>)> {
    let path = dir.join(PKG_FILE);
    let src = fs::read_to_string(&path).ok()?;
    Some(parse_manifest(&src))
}

/// 复制目录（递归）
fn copy_dir(src: &Path, dst: &Path) -> std::io::Result<()> {
    fs::create_dir_all(dst)?;
    for entry in fs::read_dir(src)? {
        let entry = entry?;
        let ft = entry.file_type()?;
        let from = entry.path();
        let to = dst.join(entry.file_name());
        if ft.is_dir() {
            copy_dir(&from, &to)?;
        } else {
            fs::copy(&from, &to)?;
        }
    }
    Ok(())
}

/// 从源路径计算依赖名：目录名 / 文件名去 .px
fn dep_name_from_path(src: &Path, alias: Option<&str>) -> String {
    if let Some(a) = alias {
        return a.to_string();
    }
    let base = if src.is_dir() {
        src.file_name().map(|s| s.to_string_lossy().to_string())
    } else {
        src.file_stem().map(|s| s.to_string_lossy().to_string())
    };
    base.unwrap_or_else(|| "dep".to_string())
}

/// px pkg init：生成 px.toml
pub fn pkg_init(dir: &Path, name: Option<&str>) -> Result<String, String> {
    let manifest_path = dir.join(PKG_FILE);
    if manifest_path.exists() {
        return Err(format!("{} 已存在，跳过初始化", PKG_FILE));
    }
    let pkg_name = name
        .map(|s| s.to_string())
        .unwrap_or_else(|| {
            dir.file_name()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_else(|| "px_app".to_string())
        });
    let content = serialize_manifest(&pkg_name, "0.1.0", &BTreeMap::new());
    fs::write(&manifest_path, content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    Ok(format!("已初始化 {}（包名: {}）", PKG_FILE, pkg_name))
}

/// px pkg add <path|url>：添加依赖到 .px_modules/<name> 并写入 px.toml。
/// 本地路径复制；URL（http/https）下载（zip 解压 / .px 单文件）。
pub fn pkg_add(dir: &Path, src_path: &str, alias: Option<&str>) -> Result<String, String> {
    let (pkg_name, version, mut deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    let modules_dir = dir.join(MODULES_DIR);
    fs::create_dir_all(&modules_dir).map_err(|e| format!("创建 {} 失败: {}", MODULES_DIR, e))?;

    // 远程 URL 依赖
    if is_url(src_path) {
        let name = alias.map(|s| s.to_string()).unwrap_or_else(|| {
            src_path
                .rsplit('/')
                .next()
                .and_then(|f| f.split('#').next())
                .and_then(|f| f.split('.').next())
                .filter(|s| !s.is_empty())
                .unwrap_or("remote_pkg")
                .to_string()
        });
        let msg = install_url_dep(&name, src_path, &modules_dir)?;
        deps.insert(name.clone(), src_path.to_string());
        let content = serialize_manifest(&pkg_name, &version, &deps);
        fs::write(dir.join(PKG_FILE), content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
        return Ok(format!("已添加远程依赖 {} <- {}\n{}", name, src_path, msg));
    }

    let src = PathBuf::from(src_path);
    if !src.exists() {
        return Err(format!("依赖路径不存在: {}", src_path));
    }
    let name = dep_name_from_path(&src, alias);

    // 复制到 .px_modules/<name>
    let dst = modules_dir.join(&name);
    if src.is_dir() {
        copy_dir(&src, &dst).map_err(|e| format!("复制目录失败: {}", e))?;
    } else {
        fs::create_dir_all(&dst).map_err(|e| format!("创建目录失败: {}", e))?;
        fs::copy(&src, dst.join(src.file_name().unwrap())).map_err(|e| format!("复制文件失败: {}", e))?;
    }

    // 写入 px.toml
    deps.insert(name.clone(), src_path.to_string());
    let content = serialize_manifest(&pkg_name, &version, &deps);
    fs::write(dir.join(PKG_FILE), content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    Ok(format!("已添加依赖 {} <- {}", name, src_path))
}

/// px pkg install：按 px.toml 安装依赖（幂等：已存在则跳过）
pub fn pkg_install(dir: &Path) -> Result<String, String> {
    let (pkg_name, version, deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    if deps.is_empty() {
        return Ok("没有依赖需要安装".to_string());
    }
    let modules_dir = dir.join(MODULES_DIR);
    fs::create_dir_all(&modules_dir).map_err(|e| format!("创建 {} 失败: {}", MODULES_DIR, e))?;
    let mut installed = Vec::new();
    for (name, path_str) in &deps {
        let dst = modules_dir.join(name);
        if dst.exists() {
            installed.push(format!("{}（已存在，跳过）", name));
            continue;
        }
        // 远程 URL 依赖（下载 + 缓存 + 可选 sha256 校验）
        if is_url(path_str) {
            let msg = install_url_dep(name, path_str, &modules_dir)?;
            installed.push(msg);
            continue;
        }
        let src = PathBuf::from(path_str);
        if !src.exists() {
            return Err(format!("依赖 {} 的源路径不存在: {}", name, path_str));
        }
        if src.is_dir() {
            copy_dir(&src, &dst).map_err(|e| format!("安装 {} 失败: {}", name, e))?;
        } else {
            fs::create_dir_all(&dst).map_err(|e| format!("创建目录失败: {}", e))?;
            fs::copy(&src, dst.join(src.file_name().unwrap())).map_err(|e| format!("复制失败: {}", e))?;
        }
        installed.push(name.clone());
    }
    Ok(format!(
        "已按 {}（包名 {} v{}）安装 {} 个依赖：{}",
        PKG_FILE,
        pkg_name,
        version,
        installed.len(),
        installed.join(", ")
    ))
}

/// px pkg list：列出依赖
pub fn pkg_list(dir: &Path) -> Result<String, String> {
    let (pkg_name, version, deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    let modules_dir = dir.join(MODULES_DIR);
    let mut out = String::new();
    out.push_str(&format!("项目: {} v{}\n", pkg_name, version));
    out.push_str(&format!("清单: {} 个依赖\n", deps.len()));
    for (name, path_str) in &deps {
        let installed = modules_dir.join(name).exists();
        out.push_str(&format!(
            "  - {} <- {} [{}]\n",
            name,
            path_str,
            if installed { "已安装" } else { "未安装" }
        ));
    }
    if deps.is_empty() {
        out.push_str("  （空）\n");
    }
    // 列出 .px_modules 内容
    if modules_dir.is_dir() {
        out.push_str(&format!("{} 目录:\n", MODULES_DIR));
        if let Ok(entries) = fs::read_dir(&modules_dir) {
            for e in entries.flatten() {
                let name = e.file_name().to_string_lossy().to_string();
                let t = if e.path().is_dir() { "dir" } else { "file" };
                out.push_str(&format!("  - {} [{}]\n", name, t));
            }
        }
    } else {
        out.push_str(&format!("{} 目录不存在（尚未安装）\n", MODULES_DIR));
    }
    Ok(out)
}

/// px pkg remove <name>：移除依赖
pub fn pkg_remove(dir: &Path, name: &str) -> Result<String, String> {
    let (pkg_name, version, mut deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    if !deps.contains_key(name) {
        return Err(format!("依赖 {} 不在清单中", name));
    }
    deps.remove(name);
    let content = serialize_manifest(&pkg_name, &version, &deps);
    fs::write(dir.join(PKG_FILE), content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    let dst = dir.join(MODULES_DIR).join(name);
    if dst.exists() {
        fs::remove_dir_all(&dst).map_err(|e| format!("删除 {} 失败: {}", name, e))?;
    }
    Ok(format!("已移除依赖 {}", name))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tmpdir() -> PathBuf {
        let d = std::env::temp_dir().join(format!("px_pkg_test_{}", std::process::id()));
        let _ = fs::remove_dir_all(&d);
        fs::create_dir_all(&d).unwrap();
        d
    }

    #[test]
    fn test_manifest_roundtrip() {
        let src = "[package]\nname = \"demo\"\nversion = \"0.2.0\"\n\n[dependencies]\nmylib = \"../mylib\"\nfoo = \"./foo\"\n";
        let (n, v, deps) = parse_manifest(src);
        assert_eq!(n, "demo");
        assert_eq!(v, "0.2.0");
        assert_eq!(deps.get("mylib").unwrap(), "../mylib");
        assert_eq!(deps.get("foo").unwrap(), "./foo");
        let s2 = serialize_manifest(&n, &v, &deps);
        let (n2, v2, deps2) = parse_manifest(&s2);
        assert_eq!(n2, n);
        assert_eq!(v2, v);
        assert_eq!(deps2, deps);
    }

    #[test]
    fn test_pkg_init_and_add_list_remove() {
        let d = tmpdir();
        // 源依赖
        let srcdir = d.join("_src_lib");
        fs::create_dir_all(srcdir.join("sub")).unwrap();
        fs::write(srcdir.join("core.px"), "def core():\n    return 1\n").unwrap();
        fs::write(srcdir.join("sub/extra.px"), "def extra():\n    return 2\n").unwrap();

        let pkg = pkg_init(&d, Some("demo")).unwrap();
        assert!(pkg.contains("demo"));

        let add = pkg_add(&d, srcdir.to_str().unwrap(), Some("mylib")).unwrap();
        assert!(add.contains("mylib"));

        // 校验复制
        assert!(d.join(MODULES_DIR).join("mylib/core.px").exists());
        assert!(d.join(MODULES_DIR).join("mylib/sub/extra.px").exists());

        let list = pkg_list(&d).unwrap();
        assert!(list.contains("mylib"));

        let rm = pkg_remove(&d, "mylib").unwrap();
        assert!(rm.contains("已移除"));
        assert!(!d.join(MODULES_DIR).join("mylib").exists());
        let _ = fs::remove_dir_all(&d);
    }
}
