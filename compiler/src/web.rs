//! M17 P1：.px 脚本执行机制（PHP / OpenResty 式应用平台）
//! 目标（docs/PROGRESS.md C 场景）："写个 .px 扔进目录访问 URL 就跑，不用编译、不用重启、不用单独部署"。
//!
//! 提供两个语言层能力：
//! - `px_exec(path, params?) -> str|null`：**内嵌解释器执行 API**（路线 A）
//!   在当前解释器内新开隔离解释器执行 .px 文件，捕获 print 输出为返回值；
//!   文件不存在返回 null；params dict 注入为脚本全局变量。
//! - `px_serve(port, docroot[, timeout_ms])`：**PHP 式应用服务器**（开发模式 A）
//!   静态文件直接返回（MIME 表）；.px 脚本内嵌解释器执行（注入 REQUEST/GET/POST/SERVER 全局变量，
//!   print 输出即响应体；脚本可设全局 RESPONSE = {status, headers, body} 精确控制响应）；
//!   目录 → index.px / index.html；路径穿越防护（403）；404 / 500 / 504（超时）。
//!
//! 安全（PROGRESS 第一版架构要求）：
//! - 目录隔离：URL 分段拒绝 ".."，且 canonicalize 前缀校验（防符号链接逃逸）✅
//! - 超时控制：脚本死循环不阻塞服务器（超时放弃；编译模式 C 端为子进程 + kill）✅
//! - 危险函数禁/限：语言暂无 exec 内置，天然安全 ✅
//! - 每请求独立线程：单脚本卡死不影响其他请求 ✅

use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::path::{Path, PathBuf};
use std::sync::{mpsc, Arc, Mutex};
use std::time::Duration;

use crate::builtin::{
    content_type_of, multipart_boundary, parse_form, parse_http_request, parse_multipart,
};
use crate::interp::Interpreter;
use crate::value::Value;
use crate::{lexer, module, parser};

// ==================== px_exec：内嵌解释器执行 API ====================

/// px_exec(path, params?)：内嵌执行 .px 脚本，返回 print 输出字符串。
/// 文件不存在 → Ok(Value::Null)；解析/运行出错 → Err。
/// params dict 的每个键注入为脚本全局变量；同时提供默认的 REQUEST/GET/POST/SERVER
/// （空值），保证 Web 风格脚本（读 GET["x"] 等）在非 Web 语境下也能安全运行。
pub fn px_exec(path: &str, params: &HashMap<String, Value>) -> Result<Value, String> {
    if !Path::new(path).is_file() {
        return Ok(Value::Null);
    }
    let mut p = params.clone();
    let empty = || Value::Dict(Arc::new(Mutex::new(HashMap::new())));
    p.entry("REQUEST".to_string()).or_insert_with(empty);
    p.entry("GET".to_string()).or_insert_with(empty);
    p.entry("POST".to_string()).or_insert_with(empty);
    p.entry("SERVER".to_string()).or_insert(Value::Dict(Arc::new(Mutex::new({
        let mut m = HashMap::new();
        m.insert(
            "px".to_string(),
            Value::Str(env!("CARGO_PKG_VERSION").to_string()),
        );
        m
    }))));
    let (_code, out) = run_script_capture(path, &p, 0)?; // 0 = 不超时（同步嵌入语义）
    Ok(Value::Str(String::from_utf8_lossy(&out).to_string()))
}

/// 内嵌执行 .px 脚本并捕获 print 输出。timeout_ms=0 表示不超时。
/// 返回 (退出码, 捕获的 print 输出字节)。
fn run_script_capture(
    path: &str,
    params: &HashMap<String, Value>,
    timeout_ms: i64,
) -> Result<(i32, Vec<u8>), String> {
    let src =
        std::fs::read_to_string(path).map_err(|e| format!("无法读取脚本 {}: {}", path, e))?;
    let tokens = lexer::Lexer::new(&src).tokenize().map_err(|e| e.to_string())?;
    let mut parser = parser::Parser::new(tokens);
    let prog = parser.parse_program().map_err(|e| e.to_string())?;
    let base_dir = Path::new(path)
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = module::ModuleResolver::new(Path::new(&base_dir)).resolve(prog);

    let mut interp = Interpreter::new();
    interp.output = Some(Arc::new(Mutex::new(Vec::new())));
    if !params.is_empty() {
        let mut g = interp.globals.lock().unwrap();
        for (k, v) in params {
            g.define(k, v.clone());
        }
    }
    if timeout_ms <= 0 {
        let code = interp.run_program(&prog).map_err(|e| e.to_string())?;
        let out = interp
            .output
            .take()
            .map(|o| o.lock().unwrap().clone())
            .unwrap_or_default();
        Ok((code, out))
    } else {
        // 超时执行：独立线程 + channel 限时等待（脚本死循环不阻塞服务器）
        let (tx, rx) = mpsc::channel();
        let mut worker = interp;
        std::thread::spawn(move || {
            let r = worker.run_program(&prog);
            let out = worker
                .output
                .take()
                .map(|o| o.lock().unwrap().clone())
                .unwrap_or_default();
            let _ = tx.send((r, out));
        });
        match rx.recv_timeout(Duration::from_millis(timeout_ms.max(1) as u64)) {
            Ok((r, out)) => Ok((r.map_err(|e| e.to_string())?, out)),
            Err(_) => Err(format!("脚本执行超时（>{}ms）", timeout_ms)),
        }
    }
}

// ==================== px_serve：PHP 式应用服务器 ====================

/// px_serve(port, docroot[, timeout_ms])：阻塞 accept 循环，每请求独立线程。
pub fn px_serve(port: i64, docroot: &str, timeout_ms: i64) -> Result<(), String> {
    let root = std::fs::canonicalize(docroot).map_err(|e| format!("docroot 无效 {}: {}", docroot, e))?;
    if !root.is_dir() {
        return Err(format!("docroot 不是目录: {}", docroot));
    }
    let addr = format!("0.0.0.0:{}", port);
    let listener = TcpListener::bind(&addr)
        .map_err(|e| format!("监听端口失败 {}: {}", addr, e))?;
    eprintln!(
        "[px-serve] 普贤应用服务器 docroot={} 端口={} 超时={}ms",
        root.display(),
        port,
        timeout_ms
    );
    for stream in listener.incoming() {
        let mut stream = match stream {
            Ok(s) => s,
            Err(_) => continue,
        };
        let root = root.clone();
        std::thread::spawn(move || {
            if let Err(e) = handle_px_conn(&mut stream, &root, port, timeout_ms) {
                eprintln!("[px-serve] {}", e);
            }
        });
    }
    Ok(())
}

/// 处理单个 HTTP 连接：读请求 → 路径映射/穿越防护 → 静态文件或 .px 脚本 → 响应。
fn handle_px_conn(
    stream: &mut TcpStream,
    root: &Path,
    port: i64,
    timeout_ms: i64,
) -> Result<(), String> {
    let (req, method, _body) = read_http_conn(stream)?;
    let path = req_str(&req, "path").unwrap_or_else(|| "/".to_string());
    let start = std::time::Instant::now();

    // 1. 路径解析 + 目录隔离（穿越防护）
    let fs_path = match resolve_path(root, &path) {
        Ok(p) => p,
        Err(e) => {
            let (status, msg) = if e == "FORBIDDEN" {
                (403, "403 Forbidden: 路径穿越被拒绝")
            } else {
                (404, "404 Not Found")
            };
            let _ = send_response(stream, status, &[], msg.as_bytes(), method == "HEAD");
            return Ok(());
        }
    };

    // 2. 目录 → index.px / index.html
    let target = if fs_path.is_dir() {
        let idx_px = fs_path.join("index.px");
        let idx_html = fs_path.join("index.html");
        if idx_px.is_file() {
            Some(idx_px)
        } else if idx_html.is_file() {
            Some(idx_html)
        } else {
            None
        }
    } else {
        Some(fs_path.clone())
    };
    let target = match target {
        Some(t) if t.is_file() => t,
        _ => {
            let _ = send_response(stream, 404, &[], b"404 Not Found", method == "HEAD");
            return Ok(());
        }
    };

    // 3. .px 脚本 vs 静态文件
    let is_px = target
        .extension()
        .and_then(|e| e.to_str())
        .map(|e| e.eq_ignore_ascii_case("px"))
        .unwrap_or(false);
    let (status, headers, body) = if is_px {
        match exec_script(&target, &req, root, port, timeout_ms) {
            Ok((st, hd, bd)) => (st, hd, bd),
            Err(e) if e == "TIMEOUT" => {
                eprintln!("[px-serve] 脚本超时 {} (>{}ms)", target.display(), timeout_ms);
                (
                    504,
                    vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
                    format!("504 Gateway Timeout: 脚本执行超时（>{}ms）", timeout_ms).into_bytes(),
                )
            }
            Err(e) => {
                eprintln!("[px-serve] 脚本错误 {}: {}", target.display(), e);
                (
                    500,
                    vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
                    format!("500 Internal Server Error\n\n{}", e).into_bytes(),
                )
            }
        }
    } else {
        match std::fs::read(&target) {
            Ok(data) => {
                let ct = mime_type(&target.to_string_lossy());
                (200, vec![("Content-Type".into(), ct.into())], data)
            }
            Err(e) => (
                404,
                vec![],
                format!("404 Not Found: {}", e).into_bytes(),
            ),
        }
    };

    let ms = start.elapsed().as_millis();
    eprintln!("[px-serve] {} {} -> {} ({}ms)", method, path, status, ms);
    let _ = send_response(stream, status, &headers, &body, method == "HEAD");
    Ok(())
}

/// 执行 .px 脚本（内嵌解释器 + 超时控制），返回 (status, headers, body)。
fn exec_script(
    script_path: &Path,
    req: &Value,
    root: &Path,
    port: i64,
    timeout_ms: i64,
) -> Result<(i64, Vec<(String, String)>, Vec<u8>), String> {
    let src = std::fs::read_to_string(script_path)
        .map_err(|e| format!("读取脚本失败: {}", e))?;
    let tokens = lexer::Lexer::new(&src).tokenize().map_err(|e| e.to_string())?;
    let mut parser = parser::Parser::new(tokens);
    let prog = parser.parse_program().map_err(|e| e.to_string())?;
    let base_dir = script_path
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = module::ModuleResolver::new(Path::new(&base_dir)).resolve(prog);

    let mut interp = Interpreter::new();
    interp.output = Some(Arc::new(Mutex::new(Vec::new())));
    {
        let mut g = interp.globals.lock().unwrap();
        g.define("REQUEST", req.clone());
        g.define("GET", extract_get(req));
        g.define("POST", extract_post(req));
        g.define("SERVER", make_server_dict(root, port, script_path));
    }

    let (tx, rx) = mpsc::channel();
    let mut worker = interp;
    std::thread::spawn(move || {
        let r = worker.run_program(&prog);
        let out = worker
            .output
            .take()
            .map(|o| o.lock().unwrap().clone())
            .unwrap_or_default();
        let resp = worker.globals.lock().unwrap().get("RESPONSE");
        let _ = tx.send((r, out, resp));
    });

    match rx.recv_timeout(Duration::from_millis(timeout_ms.max(1) as u64)) {
        Ok((Ok(code), out, resp)) => {
            // 脚本 exit 非 0（含显式 exit(n)）→ 500，与编译模式子进程退出码语义一致
            if code != 0 {
                return Err(format!("脚本退出码非零: {}", code));
            }
            // 脚本可设全局 RESPONSE = {status, headers, body} 精确控制响应；否则 print 输出即 body
            if let Some(Value::Dict(d)) = resp {
                let d = d.lock().unwrap();
                let status = match d.get("status") {
                    Some(Value::Int(s)) => *s,
                    Some(Value::Float(f)) => *f as i64,
                    _ => 200,
                };
                let body = match d.get("body") {
                    Some(Value::Str(s)) => s.clone().into_bytes(),
                    Some(Value::Null) | None => out,
                    Some(o) => o.to_string().into_bytes(),
                };
                let headers = match d.get("headers") {
                    Some(Value::Dict(h)) => {
                        let h = h.lock().unwrap();
                        h.iter().map(|(k, v)| (k.clone(), v.to_string())).collect()
                    }
                    _ => vec![("Content-Type".into(), "text/html; charset=utf-8".into())],
                };
                Ok((status, headers, body))
            } else {
                Ok((
                    200,
                    vec![("Content-Type".into(), "text/html; charset=utf-8".into())],
                    out,
                ))
            }
        }
        Ok((Err(e), _, _)) => Err(format!("脚本执行出错: {}", e)),
        Err(_) => Err("TIMEOUT".into()),
    }
}

// ==================== 辅助 ====================

/// 读取 HTTP 请求（头 + body），返回 (req dict, method, body)。
fn read_http_conn(stream: &mut TcpStream) -> Result<(Value, String, String), String> {
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 4096];
    let header_end;
    loop {
        let n = stream
            .read(&mut tmp)
            .map_err(|e| format!("读请求失败: {}", e))?;
        if n == 0 {
            return Err("连接已关闭".into());
        }
        buf.extend_from_slice(&tmp[..n]);
        if let Some(idx) = find_http_header_end(&buf) {
            header_end = idx;
            break;
        }
        if buf.len() > 65536 {
            return Err("请求头超过 64KB".into());
        }
    }
    let head = String::from_utf8_lossy(&buf[..header_end]).to_string();
    let remote = stream
        .peer_addr()
        .map(|a| a.to_string())
        .unwrap_or_default();
    let (req, content_length) = parse_http_request(&head, &remote)?;
    let body_off = header_end + 4;
    let body = if content_length > 0 {
        let mut rest = Vec::with_capacity(content_length);
        let have = buf.len().saturating_sub(body_off);
        if have >= content_length {
            String::from_utf8_lossy(&buf[body_off..body_off + content_length]).to_string()
        } else {
            rest.extend_from_slice(&buf[body_off..]);
            let mut tmp2 = [0u8; 4096];
            while rest.len() < content_length {
                let n = stream
                    .read(&mut tmp2)
                    .map_err(|e| format!("读 body 失败: {}", e))?;
                if n == 0 {
                    break;
                }
                rest.extend_from_slice(&tmp2[..n]);
            }
            rest.truncate(content_length);
            String::from_utf8_lossy(&rest).to_string()
        }
    } else {
        String::new()
    };
    if let Value::Dict(d) = &req {
        let mut g = d.lock().unwrap();
        g.insert("body".into(), Value::Str(body.clone()));
        if !body.is_empty() {
            if let Some(ct) = content_type_of(&g) {
                let lct = ct.to_lowercase();
                if lct.contains("application/x-www-form-urlencoded") {
                    g.insert("form".into(), parse_form(&body));
                } else if lct.contains("multipart/form-data") {
                    let (form, files) = parse_multipart(&body, &multipart_boundary(&ct));
                    g.insert("form".into(), form);
                    g.insert("files".into(), files);
                }
            }
        }
    }
    let method = req_str(&req, "method").unwrap_or_default();
    Ok((req, method, body))
}

/// 查找 \r\n\r\n 在缓冲区中的起始下标
fn find_http_header_end(buf: &[u8]) -> Option<usize> {
    if buf.len() >= 4 {
        for i in 0..=buf.len() - 4 {
            if &buf[i..i + 4] == b"\r\n\r\n" {
                return Some(i);
            }
        }
    }
    None
}

/// 路径映射 + 目录隔离：URL path → 站点根内文件路径。
/// 分段含 ".." → FORBIDDEN；不存在 → NOT_FOUND；canonicalize 前缀校验防符号链接逃逸。
fn resolve_path(root: &Path, path: &str) -> Result<PathBuf, String> {
    let clean = path.trim_start_matches('/');
    let mut parts = Vec::new();
    for seg in clean.split('/') {
        match seg {
            "" | "." => {}
            ".." => return Err("FORBIDDEN".into()),
            s => parts.push(s),
        }
    }
    let mut fs = root.to_path_buf();
    for p in parts {
        fs.push(p);
    }
    match std::fs::canonicalize(&fs) {
        Ok(c) => {
            if c.starts_with(root) {
                Ok(c)
            } else {
                Err("FORBIDDEN".into())
            }
        }
        Err(_) => Err("NOT_FOUND".into()),
    }
}

/// 静态文件 MIME 类型表
fn mime_type(name: &str) -> &'static str {
    let ext = Path::new(name)
        .extension()
        .and_then(|e| e.to_str())
        .unwrap_or("")
        .to_lowercase();
    match ext.as_str() {
        "html" | "htm" => "text/html; charset=utf-8",
        "css" => "text/css; charset=utf-8",
        "js" | "mjs" => "application/javascript; charset=utf-8",
        "json" => "application/json; charset=utf-8",
        "txt" | "md" => "text/plain; charset=utf-8",
        "xml" | "svg" => "text/xml; charset=utf-8",
        "csv" => "text/csv; charset=utf-8",
        "png" => "image/png",
        "jpg" | "jpeg" => "image/jpeg",
        "gif" => "image/gif",
        "webp" => "image/webp",
        "ico" => "image/x-icon",
        "bmp" => "image/bmp",
        "pdf" => "application/pdf",
        "woff" => "font/woff",
        "woff2" => "font/woff2",
        "ttf" => "font/ttf",
        "eot" => "application/vnd.ms-fontobject",
        "wasm" => "application/wasm",
        "zip" => "application/zip",
        "mp3" => "audio/mpeg",
        "mp4" => "video/mp4",
        "webm" => "video/webm",
        _ => "application/octet-stream",
    }
}

/// 发送 HTTP 响应（二进制安全；HEAD 只发响应头）
fn send_response(
    stream: &mut TcpStream,
    status: i64,
    headers: &[(String, String)],
    body: &[u8],
    head_only: bool,
) -> Result<(), String> {
    let mut resp = Vec::new();
    let reason = status_reason(status);
    resp.extend_from_slice(
        format!(
            "HTTP/1.1 {} {}\r\nContent-Length: {}\r\nConnection: close\r\n",
            status,
            reason,
            body.len()
        )
        .as_bytes(),
    );
    let mut has_ct = false;
    for (k, v) in headers {
        if k.eq_ignore_ascii_case("content-type") {
            has_ct = true;
        }
        resp.extend_from_slice(format!("{}: {}\r\n", k, v).as_bytes());
    }
    if !has_ct {
        resp.extend_from_slice(b"Content-Type: text/plain; charset=utf-8\r\n");
    }
    resp.extend_from_slice(b"\r\n");
    if !head_only {
        resp.extend_from_slice(body);
    }
    stream
        .write_all(&resp)
        .map_err(|e| format!("发送响应失败: {}", e))?;
    stream.flush().map_err(|e| format!("flush 失败: {}", e))?;
    Ok(())
}

fn status_reason(code: i64) -> &'static str {
    match code {
        200 => "OK",
        201 => "Created",
        202 => "Accepted",
        204 => "No Content",
        301 => "Moved Permanently",
        302 => "Found",
        304 => "Not Modified",
        400 => "Bad Request",
        401 => "Unauthorized",
        403 => "Forbidden",
        404 => "Not Found",
        405 => "Method Not Allowed",
        409 => "Conflict",
        413 => "Payload Too Large",
        500 => "Internal Server Error",
        501 => "Not Implemented",
        502 => "Bad Gateway",
        503 => "Service Unavailable",
        504 => "Gateway Timeout",
        _ => "OK",
    }
}

/// 从 req dict 取字符串字段
fn req_str(req: &Value, key: &str) -> Option<String> {
    if let Value::Dict(d) = req {
        if let Some(Value::Str(s)) = d.lock().unwrap().get(key) {
            return Some(s.clone());
        }
    }
    None
}

/// GET 全局变量：query 串 → dict（urlencoded 解析）
fn extract_get(req: &Value) -> Value {
    let q = req_str(req, "query").unwrap_or_default();
    parse_form(&q)
}

/// POST 全局变量：form dict（POST 表单）；无则空 dict
fn extract_post(req: &Value) -> Value {
    if let Value::Dict(d) = req {
        if let Some(v) = d.lock().unwrap().get("form") {
            return v.clone();
        }
    }
    Value::Dict(Arc::new(Mutex::new(HashMap::new())))
}

/// SERVER 全局变量
fn make_server_dict(root: &Path, port: i64, script: &Path) -> Value {
    let mut m = HashMap::new();
    m.insert("port".into(), Value::Int(port));
    m.insert("docroot".into(), Value::Str(root.to_string_lossy().to_string()));
    m.insert("script".into(), Value::Str(script.to_string_lossy().to_string()));
    m.insert("px".into(), Value::Str(env!("CARGO_PKG_VERSION").to_string()));
    Value::Dict(Arc::new(Mutex::new(m)))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_resolve_path_rejects_traversal() {
        let root = Path::new("/tmp");
        assert_eq!(resolve_path(root, "/a/../b"), Err("FORBIDDEN".into()));
        assert_eq!(resolve_path(root, "/.."), Err("FORBIDDEN".into()));
        assert_eq!(resolve_path(root, "/a/./b/.."), Err("FORBIDDEN".into()));
        // 无穿越 → NOT_FOUND（路径不存在）
        assert_eq!(resolve_path(root, "/no/such/file.px"), Err("NOT_FOUND".into()));
    }

    #[test]
    fn test_mime_type() {
        assert!(mime_type("a.html").contains("text/html"));
        assert!(mime_type("x.js").contains("javascript"));
        assert!(mime_type("i.png").contains("image/png"));
        assert!(mime_type("noext").contains("octet-stream"));
        assert!(mime_type("a.HTML").contains("text/html")); // 大小写不敏感
    }

    #[test]
    fn test_status_reason() {
        assert_eq!(status_reason(404), "Not Found");
        assert_eq!(status_reason(504), "Gateway Timeout");
        assert_eq!(status_reason(999), "OK");
    }
}
