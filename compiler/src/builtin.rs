//! 普贤 (PuXian) 内置函数实现（spec.md §10.2）
//! 自由函数（非方法）分派

use std::collections::HashMap;
use std::io::{Read, Write};
use std::net::{TcpListener, TcpStream};
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{mpsc, Arc, Mutex, OnceLock};
use std::thread;
use std::time::Duration;

use crate::interp::{Interpreter, LxError};
use crate::token::Pos;
use crate::value::{Builtin, Value};

fn err(msg: impl Into<String>, pos: Pos) -> LxError {
    LxError::new("R1002", msg, Some(pos))
}

// ==================== M23 进程 / 信号 ====================
// os_pid() / os_spawn(cmd, args) / os_wait(pid) / os_kill(pid, sig) / signal(sig, handler)
// 双模式一致：退出码约定 正常退出=exit code，信号终止=128+信号号，失败=-1。
// signal 用 self-pipe 模式：C 信号处理器只写 1 字节到管道（async-signal-safe），
// 专用线程读管道 → 新线程 fork 解释器执行注册的普贤 handler(sig)。

use std::os::unix::process::ExitStatusExt;

unsafe extern "C" {
    fn pipe(fds: *mut i32) -> i32;
    fn write(fd: i32, buf: *const u8, n: usize) -> isize;
    fn read(fd: i32, buf: *mut u8, n: usize) -> isize;
    fn kill(pid: i32, sig: i32) -> i32;
    fn signal(signum: i32, handler: usize) -> usize;
}

static SIG_PIPE: std::sync::OnceLock<(i32, i32)> = std::sync::OnceLock::new();
static SIG_HANDLERS: std::sync::OnceLock<std::sync::Mutex<std::collections::HashMap<i32, Value>>> =
    std::sync::OnceLock::new();
static SIG_INTERP: std::sync::OnceLock<Interpreter> = std::sync::OnceLock::new();
static SIG_POS: std::sync::OnceLock<Pos> = std::sync::OnceLock::new();

/// os_spawn 启动的子进程表（os_wait 取回并等待）
fn proc_children() -> &'static std::sync::Mutex<std::collections::HashMap<u32, std::process::Child>> {
    static M: std::sync::OnceLock<std::sync::Mutex<std::collections::HashMap<u32, std::process::Child>>> =
        std::sync::OnceLock::new();
    M.get_or_init(|| std::sync::Mutex::new(std::collections::HashMap::new()))
}

extern "C" fn sig_bridge(sig: i32) {
    if let Some((_, w)) = SIG_PIPE.get() {
        let b = [sig as u8];
        unsafe {
            let _ = write(*w, b.as_ptr(), 1);
        }
    }
}

fn sig_pipe() -> (i32, i32) {
    *SIG_PIPE.get_or_init(|| {
        let mut fds = [0i32; 2];
        unsafe {
            pipe(fds.as_mut_ptr());
        }
        (fds[0], fds[1])
    })
}

fn sig_handlers() -> &'static std::sync::Mutex<std::collections::HashMap<i32, Value>> {
    SIG_HANDLERS.get_or_init(|| std::sync::Mutex::new(std::collections::HashMap::new()))
}

/// 启动信号分发线程（首次 signal 调用时启动一次）
fn ensure_signal_thread(interp: &mut Interpreter, pos: Pos) {
    let (r, _w) = sig_pipe();
    if SIG_INTERP.get().is_none() {
        let _ = SIG_INTERP.set(interp.fork());
        let _ = SIG_POS.set(pos);
    }
    static STARTED: std::sync::atomic::AtomicBool = std::sync::atomic::AtomicBool::new(false);
    if STARTED.swap(true, std::sync::atomic::Ordering::SeqCst) {
        return;
    }
    let pos_c = pos;
    thread::spawn(move || {
        // 专用线程持有 fork 的解释器执行回调
        let mut i = SIG_INTERP.get().unwrap().fork();
        let mut buf = [0u8; 64];
        loop {
            let n = unsafe { read(r, buf.as_mut_ptr(), buf.len()) };
            if n <= 0 {
                continue;
            }
            for &s in &buf[..n as usize] {
                let sig = s as i32;
                let h = sig_handlers().lock().unwrap().get(&sig).cloned();
                if let Some(h) = h {
                    let arg = Value::Int(sig as i64);
                    if let Err(e) = i.call_value(&h, &[arg], pos_c) {
                        eprintln!("[signal {}] {}", sig, e);
                    }
                }
            }
        }
    });
}

// ==================== M21 SSE 连接注册表 ====================
// sse_serve(port, handler)：handler(req) 内/任意线程可 sse_send(conn, data) 推送，
// handler 返回后连接保持打开，直到 sse_close(conn) 或对端断开（写失败自动清理）。

struct SseConn {
    stream: Arc<Mutex<std::net::TcpStream>>,
    close_tx: mpsc::Sender<()>,
}

fn sse_conns() -> &'static Mutex<HashMap<i64, SseConn>> {
    static M: OnceLock<Mutex<HashMap<i64, SseConn>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

static SSE_NEXT_ID: AtomicI64 = AtomicI64::new(1);

// ==================== M23 SSE 客户端 ====================
// sse_connect(url) → int conn | null：HTTP GET 连接 SSE 服务端（校验 200 + text/event-stream）
// sse_read(conn) → dict{event?, data, id?, retry?} | null：阻塞读一条事件（断开/超时 → null）
// sse_close(conn) → bool：关闭客户端连接
// 说明：sse_connect 仅支持 http://（长连接 + 流式读取需要原始 TCP；https 需 TLS 不在此列）。

struct SseClient {
    reader: Mutex<TcpStream>,
    pending: Mutex<Vec<u8>>,
}

fn sse_clients() -> &'static Mutex<HashMap<i64, SseClient>> {
    static M: OnceLock<Mutex<HashMap<i64, SseClient>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

static SSE_CLIENT_NEXT_ID: AtomicI64 = AtomicI64::new(1);

/// 解析一条 SSE 事件文本（field: value 行），返回 dict
fn sse_parse_event(text: &str) -> Value {
    let mut event = String::from("message");
    let mut data: Vec<String> = Vec::new();
    let mut id: Option<String> = None;
    let mut retry: Option<i64> = None;
    for line in text.lines() {
        let line = line.strip_suffix('\r').unwrap_or(line);
        let (field, value) = match line.find(':') {
            Some(i) => (
                &line[..i],
                line[i + 1..].strip_prefix(' ').unwrap_or(&line[i + 1..]),
            ),
            None => (line, ""),
        };
        match field {
            "event" => event = value.to_string(),
            "data" => data.push(value.to_string()),
            "id" => id = Some(value.to_string()),
            "retry" => retry = value.trim().parse::<i64>().ok(),
            _ => {}
        }
    }
    let mut m = HashMap::new();
    m.insert("event".into(), Value::Str(event));
    m.insert("data".into(), Value::Str(data.join("\n")));
    if let Some(i) = id {
        m.insert("id".into(), Value::Str(i));
    }
    if let Some(r) = retry {
        m.insert("retry".into(), Value::Int(r));
    }
    Value::Dict(Arc::new(Mutex::new(m)))
}

/// 从 pending 缓冲中取出一条完整事件（按空行分隔）；不足返回 None（缓冲保留）
fn sse_take_event(pending: &mut Vec<u8>) -> Option<String> {
    for i in 0..pending.len() {
        if pending[i] == b'\n' {
            if i + 1 < pending.len() && pending[i + 1] == b'\n' {
                let ev = String::from_utf8_lossy(&pending[..i]).to_string();
                pending.drain(..i + 2);
                return Some(ev);
            }
            if i + 2 < pending.len() && pending[i + 1] == b'\r' && pending[i + 2] == b'\n' {
                let ev = String::from_utf8_lossy(&pending[..i]).to_string();
                pending.drain(..i + 3);
                return Some(ev);
            }
        }
    }
    None
}

/// sse_connect(url) → conn id 或 null（仅 http://）
fn sse_client_connect(url: &str) -> Option<i64> {
    let rest = url.strip_prefix("http://")?;
    let (hostport, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
    if hostport.is_empty() {
        return None;
    }
    let (host, port) = match hostport.find(':') {
        Some(i) => (
            hostport[..i].to_string(),
            hostport[i + 1..].parse::<u16>().ok()?,
        ),
        None => (hostport.to_string(), 80u16),
    };
    let host_header = if hostport.contains(':') {
        hostport.to_string()
    } else {
        format!("{}:{}", host, port)
    };
    let mut stream = TcpStream::connect(format!("{}:{}", host, port)).ok()?;
    let req = format!(
        "GET {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: PuXian/0.1\r\nAccept: text/event-stream\r\nConnection: close\r\nCache-Control: no-cache\r\n\r\n",
        path, host_header
    );
    stream.write_all(req.as_bytes()).ok()?;
    // 读响应头（直到 \r\n\r\n，上限 64KB）
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 4096];
    let mut header_end = None;
    while buf.len() < 65536 {
        let n = stream.read(&mut tmp).ok()?;
        if n == 0 {
            break;
        }
        buf.extend_from_slice(&tmp[..n]);
        if let Some(idx) = find_http_header_end(&buf) {
            header_end = Some(idx);
            break;
        }
    }
    let idx = header_end?;
    let head = String::from_utf8_lossy(&buf[..idx]).to_string();
    let status = head
        .lines()
        .next()
        .and_then(|l| l.split_whitespace().nth(1))
        .and_then(|s| s.parse::<u16>().ok())
        .unwrap_or(0);
    if status != 200 {
        return None;
    }
    // Content-Type 校验：text/event-stream 放行；其他明确 Content-Type 拒绝
    let mut ct_ok = true;
    for line in head.lines().skip(1) {
        let l = line.trim();
        if l.to_lowercase().starts_with("content-type:") {
            ct_ok = l.to_lowercase().contains("text/event-stream");
        }
    }
    if !ct_ok {
        return None;
    }
    // 剩余字节（头之后）作为初始 pending
    let mut pending: Vec<u8> = buf[idx + 4..].to_vec();
    let id = SSE_CLIENT_NEXT_ID.fetch_add(1, Ordering::SeqCst);
    {
        let mut m = sse_clients().lock().unwrap();
        m.insert(
            id,
            SseClient {
                reader: Mutex::new(stream),
                pending: Mutex::new(std::mem::take(&mut pending)),
            },
        );
    }
    Some(id)
}

/// sse_read(conn) → 事件 dict 或 null（阻塞读一条；断开 → null）
fn sse_client_read(conn: i64) -> Option<Value> {
    let mut m = sse_clients().lock().unwrap();
    let c = m.get_mut(&conn)?;
    loop {
        {
            let mut p = c.pending.lock().unwrap();
            if let Some(ev) = sse_take_event(&mut p) {
                return Some(sse_parse_event(&ev));
            }
        }
        let mut tmp = [0u8; 8192];
        let n = {
            let mut r = c.reader.lock().unwrap();
            match r.read(&mut tmp) {
                Ok(0) => return None, // 对端关闭
                Ok(n) => n,
                Err(_) => return None,
            }
        };
        c.pending.lock().unwrap().extend_from_slice(&tmp[..n]);
    }
}

/// sse_close(conn) → bool：关闭客户端 SSE 连接
fn sse_client_close(conn: i64) -> bool {
    sse_clients().lock().unwrap().remove(&conn).is_some()
}

/// 编码 SSE 帧：str → `data: xxx\n\n`；dict 支持 {event, data, id, retry}
fn sse_frame(data: &Value) -> String {
    match data {
        Value::Str(s) => {
            let mut f = String::new();
            for line in s.split('\n') {
                f.push_str(&format!("data: {}\n", line));
            }
            f.push('\n');
            f
        }
        Value::Dict(d) => {
            let d = d.lock().unwrap();
            let mut f = String::new();
            if let Some(Value::Str(id)) = d.get("id") {
                f.push_str(&format!("id: {}\n", id));
            }
            if let Some(Value::Str(ev)) = d.get("event") {
                f.push_str(&format!("event: {}\n", ev));
            }
            if let Some(Value::Int(r)) = d.get("retry") {
                f.push_str(&format!("retry: {}\n", r));
            }
            match d.get("data") {
                Some(Value::Str(s)) => {
                    for line in s.split('\n') {
                        f.push_str(&format!("data: {}\n", line));
                    }
                }
                Some(Value::Null) | None => {}
                Some(other) => f.push_str(&format!("data: {}\n", other)),
            }
            f.push('\n');
            f
        }
        other => format!("data: {}\n\n", other),
    }
}

/// 处理单个 SSE 连接：读请求 → 解析 → 发响应头 → 注册连接 → 调 handler → 保持到关闭
fn handle_sse_conn(
    i: &mut Interpreter,
    stream: &mut TcpStream,
    handler: &Value,
    pos: Pos,
) -> Result<(), String> {
    // 1. 读请求头（直到 \r\n\r\n，上限 64KB）
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 4096];
    let header_end;
    loop {
        let n = stream.read(&mut tmp).map_err(|e| format!("读请求失败: {}", e))?;
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
    // 2. 解析请求
    let head = String::from_utf8_lossy(&buf[..header_end]).to_string();
    let remote = stream
        .peer_addr()
        .map(|a| a.to_string())
        .unwrap_or_default();
    let (req, _cl) = parse_http_request(&head, &remote)?;
    // 3. 分配连接 ID 并注入 req["conn"]
    let conn_id = SSE_NEXT_ID.fetch_add(1, Ordering::SeqCst);
    if let Value::Dict(d) = &req {
        d.lock().unwrap().insert("conn".into(), Value::Int(conn_id));
    }
    // 4. 发送 SSE 响应头（连接保持，直到 sse_close）
    let hdr = "HTTP/1.1 200 OK\r\nContent-Type: text/event-stream; charset=utf-8\r\nCache-Control: no-cache\r\nConnection: close\r\n\r\n";
    stream.write_all(hdr.as_bytes()).map_err(|e| format!("发送响应头失败: {}", e))?;
    stream.flush().ok();
    // 5. 注册连接（共享写端；sse_send 任意线程可写）
    let (close_tx, close_rx) = mpsc::channel::<()>();
    {
        let mut m = sse_conns().lock().unwrap();
        m.insert(
            conn_id,
            SseConn {
                stream: Arc::new(Mutex::new(stream.try_clone().map_err(|e| e.to_string())?)),
                close_tx,
            },
        );
    }
    // 6. 调 handler（handler 内/后台线程可 sse_send；出错不中断连接）
    if let Err(e) = i.call_value(handler, &[req], pos) {
        eprintln!("[sse] handler 出错: {}", e);
    }
    // 7. 保持连接：等待 sse_close 信号（对端断开时 sse_send 写失败自动唤醒）
    let _ = close_rx.recv();
    sse_conns().lock().unwrap().remove(&conn_id);
    Ok(())
}

/// 调用内置函数
pub fn call_builtin(interp: &mut Interpreter, b: Builtin, args: &[Value], pos: Pos) -> Result<Value, LxError> {
    match b {
        Builtin::Print => {
            let parts: Vec<String> = args.iter().map(|v| v.to_string()).collect();
            let line = parts.join(" ");
            // M17：px_exec 内嵌执行时捕获 print 输出（output 缓冲区），否则写 stdout
            if let Some(out) = &interp.output {
                let mut buf = out.lock().unwrap();
                buf.extend_from_slice(line.as_bytes());
                buf.push(b'\n');
            } else {
                println!("{}", line);
            }
            Ok(Value::Null)
        }
        Builtin::Len => {
            if args.len() != 1 {
                return Err(err("len 需要一个参数", pos));
            }
            let n = match &args[0] {
                Value::Str(s) => s.chars().count() as i64,
                Value::List(l) => l.lock().unwrap().len() as i64,
                Value::Tuple(t) => t.len() as i64,
                Value::Dict(d) => d.lock().unwrap().len() as i64,
                Value::Range { start, end, step } => {
                    if *step == 0 {
                        return Err(LxError::new("R1006", "range step 不能为 0", Some(pos)));
                    }
                    if (*step > 0 && start >= end) || (*step < 0 && start <= end) {
                        0
                    } else {
                        ((end - start).abs() as f64 / step.abs() as f64).ceil() as i64
                    }
                }
                Value::Chan(c) => c.inner.lock().unwrap().buf.len() as i64,
                _ => return Err(err(format!("len 不支持类型 {}", interp.type_name(&args[0])), pos)),
            };
            Ok(Value::Int(n))
        }
        Builtin::Range => {
            // range(n) / range(a, b) / range(a, b, step)
            let (start, end, step) = match args.len() {
                1 => (0, expect_int(&args[0], "range", pos)?, 1),
                2 => (
                    expect_int(&args[0], "range", pos)?,
                    expect_int(&args[1], "range", pos)?,
                    1,
                ),
                3 => (
                    expect_int(&args[0], "range", pos)?,
                    expect_int(&args[1], "range", pos)?,
                    expect_int(&args[2], "range", pos)?,
                ),
                _ => return Err(err("range 需要 1-3 个参数", pos)),
            };
            if step == 0 {
                return Err(LxError::new("R1006", "range step 不能为 0", Some(pos)));
            }
            Ok(Value::Range { start, end, step })
        }
        Builtin::Type => {
            if args.len() != 1 {
                return Err(err("type 需要一个参数", pos));
            }
            Ok(Value::Str(interp.type_name(&args[0]).to_string()))
        }
        Builtin::Str => {
            if args.len() != 1 {
                return Err(err("str 需要一个参数", pos));
            }
            Ok(Value::Str(args[0].to_string()))
        }
        Builtin::Int => {
            if args.len() != 1 {
                return Err(err("int 需要一个参数", pos));
            }
            match &args[0] {
                Value::Int(i) => Ok(Value::Int(*i)),
                Value::Float(f) => Ok(Value::Int(*f as i64)),
                Value::Bool(b) => Ok(Value::Int(if *b { 1 } else { 0 })),
                Value::Str(s) => s
                    .trim()
                    .parse::<i64>()
                    .map(Value::Int)
                    .map_err(|_| err(format!("无法将 '{}' 转为 int", s), pos)),
                _ => Err(err("int 不支持此类型", pos)),
            }
        }
        Builtin::Float => {
            if args.len() != 1 {
                return Err(err("float 需要一个参数", pos));
            }
            match &args[0] {
                Value::Int(i) => Ok(Value::Float(*i as f64)),
                Value::Float(f) => Ok(Value::Float(*f)),
                Value::Str(s) => s
                    .trim()
                    .parse::<f64>()
                    .map(Value::Float)
                    .map_err(|_| err(format!("无法将 '{}' 转为 float", s), pos)),
                _ => Err(err("float 不支持此类型", pos)),
            }
        }
        Builtin::Bool => {
            if args.len() != 1 {
                return Err(err("bool 需要一个参数", pos));
            }
            Ok(Value::Bool(interp.is_truthy(&args[0], pos)?))
        }
        Builtin::Assert => {
            if args.len() < 1 || args.len() > 2 {
                return Err(err("assert 需要 1-2 个参数", pos));
            }
            let ok = interp.is_truthy(&args[0], pos)?;
            if !ok {
                let msg = args
                    .get(1)
                    .map(|v| v.to_string())
                    .unwrap_or_else(|| "断言失败".to_string());
                return Err(LxError::new("R1100", format!("断言失败: {}", msg), Some(pos)));
            }
            Ok(Value::Null)
        }
        Builtin::Panic => {
            let msg = args
                .first()
                .map(|v| v.to_string())
                .unwrap_or_else(|| "panic".to_string());
            Err(LxError::new("R1100", msg, Some(pos)))
        }
        Builtin::Input => {
            use std::io::{self, BufRead};
            let prompt = args.first().map(|v| v.to_string()).unwrap_or_default();
            if !prompt.is_empty() {
                print!("{}", prompt);
                use std::io::Write;
                let _ = io::stdout().flush();
            }
            let mut line = String::new();
            match io::stdin().lock().read_line(&mut line) {
                Ok(_) => Ok(Value::Str(line.trim_end().to_string())),
                Err(e) => Err(err(format!("读取输入失败: {}", e), pos)),
            }
        }
        Builtin::Exit => {
            let code = args
                .first()
                .map(|v| match v {
                    Value::Int(i) => *i as i32,
                    _ => 0,
                })
                .unwrap_or(0);
            interp.exit_code = code;
            Ok(Value::Null)
        }
        Builtin::Sleep => {
            let ms = expect_int(args.first().unwrap_or(&Value::Int(0)), "sleep", pos)?;
            std::thread::sleep(std::time::Duration::from_millis(ms.max(0) as u64));
            Ok(Value::Null)
        }
        Builtin::ToUpper => one_str(args, pos, |s| Value::Str(s.to_uppercase())),
        Builtin::ToLower => one_str(args, pos, |s| Value::Str(s.to_lowercase())),
        Builtin::Trim => one_str(args, pos, |s| Value::Str(s.trim().to_string())),
        Builtin::Split => {
            if args.len() < 1 || args.len() > 2 {
                return Err(err("split 需要 1-2 个参数", pos));
            }
            let s = expect_str(&args[0], "split", pos)?;
            let sep = args
                .get(1)
                .map(|v| expect_str(v, "split", pos))
                .transpose()?
                .unwrap_or(" ");
            let parts: Vec<Value> = s
                .split(sep)
                .filter(|x| !x.is_empty())
                .map(|x| Value::Str(x.to_string()))
                .collect();
            Ok(Value::new_list(parts))
        }
        Builtin::Join => {
            if args.len() != 2 {
                return Err(err("join 需要 2 个参数", pos));
            }
            let sep = expect_str(&args[0], "join", pos)?;
            let items = match &args[1] {
                Value::List(l) => l.lock().unwrap().clone(),
                Value::Tuple(t) => t.clone(),
                _ => return Err(err("join 第二参数需要 list/tuple", pos)),
            };
            let parts: Vec<String> = items.iter().map(|v| v.to_string()).collect();
            Ok(Value::Str(parts.join(sep)))
        }
        Builtin::Contains => {
            if args.len() != 2 {
                return Err(err("contains 需要 2 个参数", pos));
            }
            match (&args[0], &args[1]) {
                (Value::Str(s), Value::Str(sub)) => Ok(Value::Bool(s.contains(sub))),
                (Value::List(l), v) => Ok(Value::Bool(l.lock().unwrap().contains(v))),
                _ => Err(err("contains 参数类型不支持", pos)),
            }
        }
        Builtin::Replace => {
            if args.len() != 3 {
                return Err(err("replace 需要 3 个参数", pos));
            }
            let s = expect_str(&args[0], "replace", pos)?;
            let old = expect_str(&args[1], "replace", pos)?;
            let new = expect_str(&args[2], "replace", pos)?;
            Ok(Value::Str(s.replace(old, new)))
        }
        Builtin::StartsWith => {
            two_str(args, pos, |s, p| Value::Bool(s.starts_with(p)))
        }
        Builtin::EndsWith => {
            two_str(args, pos, |s, p| Value::Bool(s.ends_with(p)))
        }
        Builtin::Abs => one_num(args, pos, |i| Value::Int(i.abs()), |f| Value::Float(f.abs())),
        Builtin::Sqrt => one_num(args, pos, |i| Value::Float((i as f64).sqrt()), |f| Value::Float(f.sqrt())),
        Builtin::Min => {
            if args.is_empty() {
                return Err(err("min 需要参数", pos));
            }
            let mut best = args[0].clone();
            for a in &args[1..] {
                if a.to_string() < best.to_string() {
                    best = a.clone();
                }
            }
            Ok(best)
        }
        Builtin::Max => {
            if args.is_empty() {
                return Err(err("max 需要参数", pos));
            }
            let mut best = args[0].clone();
            for a in &args[1..] {
                if a.to_string() > best.to_string() {
                    best = a.clone();
                }
            }
            Ok(best)
        }
        Builtin::Pow => {
            if args.len() != 2 {
                return Err(err("pow 需要 2 个参数", pos));
            }
            match (&args[0], &args[1]) {
                (Value::Int(a), Value::Int(b)) => Ok(Value::Int(a.pow(*b as u32))),
                _ => {
                    let a = to_f64(&args[0], pos)?;
                    let b = to_f64(&args[1], pos)?;
                    Ok(Value::Float(a.powf(b)))
                }
            }
        }
        Builtin::Sorted => {
            if args.len() != 1 {
                return Err(err("sorted 需要一个参数", pos));
            }
            let items = match &args[0] {
                Value::List(l) => l.lock().unwrap().clone(),
                Value::Tuple(t) => t.clone(),
                _ => return Err(err("sorted 参数需要 list/tuple", pos)),
            };
            let mut sorted = items;
            sorted.sort_by(|a, b| a.to_string().cmp(&b.to_string()));
            Ok(Value::new_list(sorted))
        }
        Builtin::Reversed => {
            if args.len() != 1 {
                return Err(err("reversed 需要一个参数", pos));
            }
            let mut items = match &args[0] {
                Value::List(l) => l.lock().unwrap().clone(),
                Value::Tuple(t) => t.clone(),
                Value::Str(s) => s.chars().rev().map(|c| Value::Str(c.to_string())).collect(),
                _ => return Err(err("reversed 参数类型不支持", pos)),
            };
            items.reverse();
            Ok(Value::new_list(items))
        }
        Builtin::Sum => {
            if args.len() != 1 {
                return Err(err("sum 需要一个参数", pos));
            }
            let items = match &args[0] {
                Value::List(l) => l.lock().unwrap().clone(),
                Value::Tuple(t) => t.clone(),
                _ => return Err(err("sum 参数需要 list/tuple", pos)),
            };
            let mut acc = Value::Int(0);
            for it in items {
                acc = interp.eval_binary(crate::ast::BinaryOp::Add, acc, it, pos)?;
            }
            Ok(acc)
        }
        Builtin::NowMs => {
            use std::time::{SystemTime, UNIX_EPOCH};
            let ms = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_millis() as i64)
                .unwrap_or(0);
            Ok(Value::Int(ms))
        }
        // ==================== M5 标准库 ====================
        // ---- std.io / std.fs ----
        Builtin::ReadFile => {
            if args.len() != 1 {
                return Err(err("read_file 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "read_file", pos)?;
            match std::fs::read_to_string(p) {
                Ok(s) => Ok(Value::Str(s)),
                Err(e) => Err(LxError::new(
                    "R2001",
                    format!("io: 读取文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::WriteFile => {
            if args.len() != 2 {
                return Err(err("write_file 需要 (路径, 内容) 两个参数", pos));
            }
            let p = expect_str(&args[0], "write_file", pos)?;
            let content = match &args[1] {
                Value::Str(s) => s.clone(),
                v => v.to_string(),
            };
            match std::fs::write(p, content) {
                Ok(_) => Ok(Value::Null),
                Err(e) => Err(LxError::new(
                    "R2002",
                    format!("io: 写入文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::AppendFile => {
            if args.len() != 2 {
                return Err(err("append_file 需要 (路径, 内容) 两个参数", pos));
            }
            let p = expect_str(&args[0], "append_file", pos)?;
            let content = match &args[1] {
                Value::Str(s) => s.clone(),
                v => v.to_string(),
            };
            use std::io::Write;
            match std::fs::OpenOptions::new().create(true).append(true).open(p) {
                Ok(mut f) => match f.write_all(content.as_bytes()) {
                    Ok(_) => Ok(Value::Null),
                    Err(e) => Err(LxError::new(
                        "R2003",
                        format!("io: 追加写入失败 {}: {}", p, e),
                        Some(pos),
                    )),
                },
                Err(e) => Err(LxError::new(
                    "R2003",
                    format!("io: 打开文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::Exists => {
            if args.len() != 1 {
                return Err(err("exists 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "exists", pos)?;
            Ok(Value::Bool(std::path::Path::new(p).exists()))
        }
        // ---- M12 P0：文件随机读写 + fsync（WAL / 增量日志基石）----
        // read_at(path, offset, length) → 字符串：从 offset 偏移读 length 字节
        Builtin::ReadAt => {
            if args.len() != 3 {
                return Err(err("read_at 需要 (路径, 偏移, 长度) 三个参数", pos));
            }
            let p = expect_str(&args[0], "read_at", pos)?;
            let offset = expect_int(&args[1], "read_at", pos)?;
            let length = expect_int(&args[2], "read_at", pos)?;
            if length < 0 {
                return Err(err("read_at 长度不能为负", pos));
            }
            use std::os::unix::fs::FileExt;
            match std::fs::File::open(p) {
                Ok(f) => {
                    let mut buf = vec![0u8; length as usize];
                    match f.read_at(&mut buf, offset as u64) {
                        Ok(n) => {
                            buf.truncate(n);
                            Ok(Value::Str(String::from_utf8_lossy(&buf).to_string()))
                        }
                        Err(e) => Err(LxError::new(
                            "R2004",
                            format!("io: 随机读失败 {}: {}", p, e),
                            Some(pos),
                        )),
                    }
                }
                Err(e) => Err(LxError::new(
                    "R2004",
                    format!("io: 打开文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // write_at(path, offset, content) → 实际写入字节数
        Builtin::WriteAt => {
            if args.len() != 3 {
                return Err(err("write_at 需要 (路径, 偏移, 内容) 三个参数", pos));
            }
            let p = expect_str(&args[0], "write_at", pos)?;
            let offset = expect_int(&args[1], "write_at", pos)?;
            let content = match &args[2] {
                Value::Str(s) => s.clone(),
                v => v.to_string(),
            };
            use std::os::unix::fs::FileExt;
            match std::fs::OpenOptions::new().create(true).write(true).open(p) {
                Ok(f) => match f.write_at(content.as_bytes(), offset as u64) {
                    Ok(n) => Ok(Value::Int(n as i64)),
                    Err(e) => Err(LxError::new(
                        "R2005",
                        format!("io: 随机写失败 {}: {}", p, e),
                        Some(pos),
                    )),
                },
                Err(e) => Err(LxError::new(
                    "R2005",
                    format!("io: 打开文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // file_size(path) → int
        Builtin::FileSize => {
            if args.len() != 1 {
                return Err(err("file_size 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "file_size", pos)?;
            match std::fs::metadata(p) {
                Ok(m) => Ok(Value::Int(m.len() as i64)),
                Err(e) => Err(LxError::new(
                    "R2006",
                    format!("io: 获取文件大小失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // fsync_file(path) → null：将文件数据刷入磁盘
        Builtin::FsyncFile => {
            if args.len() != 1 {
                return Err(err("fsync_file 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "fsync_file", pos)?;
            match std::fs::OpenOptions::new().read(true).write(true).open(p) {
                Ok(f) => match f.sync_all() {
                    Ok(_) => Ok(Value::Null),
                    Err(e) => Err(LxError::new(
                        "R2007",
                        format!("io: fsync 失败 {}: {}", p, e),
                        Some(pos),
                    )),
                },
                Err(e) => Err(LxError::new(
                    "R2007",
                    format!("io: 打开文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // truncate_file(path, size) → null：截断/扩展文件
        Builtin::TruncateFile => {
            if args.len() != 2 {
                return Err(err("truncate_file 需要 (路径, 大小) 两个参数", pos));
            }
            let p = expect_str(&args[0], "truncate_file", pos)?;
            let size = expect_int(&args[1], "truncate_file", pos)?;
            if size < 0 {
                return Err(err("truncate_file 大小不能为负", pos));
            }
            match std::fs::OpenOptions::new().write(true).open(p) {
                Ok(f) => match f.set_len(size as u64) {
                    Ok(_) => Ok(Value::Null),
                    Err(e) => Err(LxError::new(
                        "R2008",
                        format!("io: 截断文件失败 {}: {}", p, e),
                        Some(pos),
                    )),
                },
                Err(e) => Err(LxError::new(
                    "R2008",
                    format!("io: 打开文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // ---- M14 P1：crypto 哈希（签名校验 / 缓存 key / 数据指纹）----
        // sha256(data) → 64 字符小写 hex 字符串
        Builtin::Sha256 => {
            if args.len() != 1 {
                return Err(err("sha256 需要一个参数", pos));
            }
            let data = match &args[0] {
                Value::Str(s) => s.clone().into_bytes(),
                v => v.to_string().into_bytes(),
            };
            Ok(Value::Str(crate::crypto::sha256_hex(&data)))
        }
        // xxhash(data) → int（XXH64, seed=0；高速指纹/取模分片）
        Builtin::Xxhash => {
            if args.len() != 1 {
                return Err(err("xxhash 需要一个参数", pos));
            }
            let data = match &args[0] {
                Value::Str(s) => s.clone().into_bytes(),
                v => v.to_string().into_bytes(),
            };
            Ok(Value::Int(crate::crypto::xxh64(&data) as i64))
        }
        // ---- M15 P1：正则表达式（文本解析 / 日志分析 / 参数抽取）----
        // regex_find(pattern, text) → 第一个匹配串或 null
        Builtin::RegexFind => {
            if args.len() != 2 {
                return Err(err("regex_find 需要 2 个参数: (pattern, text)", pos));
            }
            let pat = expect_str(&args[0], "regex_find", pos)?;
            let text = expect_str(&args[1], "regex_find", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            Ok(match re.search(text) {
                Some(m) => Value::Str(text[m.start..m.end].to_string()),
                None => Value::Null,
            })
        }
        // regex_match(pattern, text) → bool（整体匹配）
        Builtin::RegexMatch => {
            if args.len() != 2 {
                return Err(err("regex_match 需要 2 个参数: (pattern, text)", pos));
            }
            let pat = expect_str(&args[0], "regex_match", pos)?;
            let text = expect_str(&args[1], "regex_match", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            Ok(Value::Bool(re.full_match(text).is_some()))
        }
        // regex_search(pattern, text) → dict{match,start,end,groups} 或 null
        Builtin::RegexSearch => {
            if args.len() != 2 {
                return Err(err("regex_search 需要 2 个参数: (pattern, text)", pos));
            }
            let pat = expect_str(&args[0], "regex_search", pos)?;
            let text = expect_str(&args[1], "regex_search", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            Ok(match re.search(text) {
                Some(m) => {
                    let groups: Vec<Value> = m
                        .groups
                        .iter()
                        .skip(1)
                        .map(|g| match g {
                            Some(s) => Value::Str(text[s.0..s.1].to_string()),
                            None => Value::Null,
                        })
                        .collect();
                    let mut map = std::collections::HashMap::new();
                    map.insert("match".to_string(), Value::Str(text[m.start..m.end].to_string()));
                    map.insert("start".to_string(), Value::Int(m.start as i64));
                    map.insert("end".to_string(), Value::Int(m.end as i64));
                    map.insert("groups".to_string(), Value::new_list(groups));
                    Value::new_dict(map)
                }
                None => Value::Null,
            })
        }
        // regex_find_all(pattern, text) → [匹配串...]
        Builtin::RegexFindAll => {
            if args.len() != 2 {
                return Err(err("regex_find_all 需要 2 个参数: (pattern, text)", pos));
            }
            let pat = expect_str(&args[0], "regex_find_all", pos)?;
            let text = expect_str(&args[1], "regex_find_all", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            let items: Vec<Value> = re
                .find_all(text)
                .into_iter()
                .map(|m| Value::Str(text[m.start..m.end].to_string()))
                .collect();
            Ok(Value::new_list(items))
        }
        // regex_replace(pattern, text, repl) → str（$1-$9 捕获组、$$ 字面 $）
        Builtin::RegexReplace => {
            if args.len() != 3 {
                return Err(err("regex_replace 需要 3 个参数: (pattern, text, repl)", pos));
            }
            let pat = expect_str(&args[0], "regex_replace", pos)?;
            let text = expect_str(&args[1], "regex_replace", pos)?;
            let repl = expect_str(&args[2], "regex_replace", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            Ok(Value::Str(re.replace(text, repl)))
        }
        // regex_split(pattern, text) → [片段...]（分隔符丢弃）
        Builtin::RegexSplit => {
            if args.len() != 2 {
                return Err(err("regex_split 需要 2 个参数: (pattern, text)", pos));
            }
            let pat = expect_str(&args[0], "regex_split", pos)?;
            let text = expect_str(&args[1], "regex_split", pos)?;
            let re = crate::regex::Regex::new(pat)
                .map_err(|e| LxError::new("R1007", format!("regex: {}", e), Some(pos)))?;
            let items: Vec<Value> = re.split(text).into_iter().map(Value::Str).collect();
            Ok(Value::new_list(items))
        }
        Builtin::ListDir => {
            if args.len() != 1 {
                return Err(err("list_dir 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "list_dir", pos)?;
            match std::fs::read_dir(p) {
                Ok(rd) => {
                    let mut names = Vec::new();
                    for e in rd.flatten() {
                        names.push(Value::Str(e.file_name().to_string_lossy().to_string()));
                    }
                    names.sort_by(|a, b| a.to_string().cmp(&b.to_string()));
                    Ok(Value::new_list(names))
                }
                Err(e) => Err(LxError::new(
                    "R2004",
                    format!("fs: 读取目录失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::Mkdir => {
            if args.len() != 1 {
                return Err(err("mkdir 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "mkdir", pos)?;
            match std::fs::create_dir_all(p) {
                Ok(_) => Ok(Value::Null),
                Err(e) => Err(LxError::new(
                    "R2005",
                    format!("fs: 创建目录失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::Remove => {
            if args.len() != 1 {
                return Err(err("remove 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "remove", pos)?;
            match std::fs::remove_file(p).or_else(|_| std::fs::remove_dir(p)) {
                Ok(_) => Ok(Value::Null),
                Err(e) => Err(LxError::new(
                    "R2006",
                    format!("fs: 删除失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        // ---- std.json ----
        Builtin::JsonParse => {
            if args.len() != 1 {
                return Err(err("json_parse 需要一个字符串参数", pos));
            }
            let s = expect_str(&args[0], "json_parse", pos)?;
            let mut p = JsonParser { bytes: s.as_bytes(), idx: 0 };
            p.skip_ws();
            match p.parse_value() {
                Ok(v) => Ok(v),
                Err(e) => Err(LxError::new("R2010", format!("json: {}", e), Some(pos))),
            }
        }
        Builtin::JsonStringify => {
            if args.len() != 1 {
                return Err(err("json_stringify 需要一个参数", pos));
            }
            match json_stringify(&args[0]) {
                Ok(s) => Ok(Value::Str(s)),
                Err(e) => Err(LxError::new("R2011", format!("json: {}", e), Some(pos))),
            }
        }
        // ---- std.time ----
        Builtin::Now => {
            use std::time::{SystemTime, UNIX_EPOCH};
            let secs = SystemTime::now()
                .duration_since(UNIX_EPOCH)
                .map(|d| d.as_secs() as i64)
                .unwrap_or(0);
            Ok(Value::Str(format_unix(secs)))
        }
        // ---- std.os ----
        Builtin::Env => {
            if args.len() != 1 {
                return Err(err("env 需要一个变量名", pos));
            }
            let k = expect_str(&args[0], "env", pos)?;
            match std::env::var(k) {
                Ok(v) => Ok(Value::Str(v)),
                Err(_) => Ok(Value::Null),
            }
        }
        Builtin::Args => {
            let a: Vec<Value> = std::env::args().skip(1).map(Value::Str).collect();
            Ok(Value::new_list(a))
        }
        // ---- std.collections（高阶函数） ----
        Builtin::Map => {
            if args.len() != 2 {
                return Err(err("map 需要 (list, fn) 两个参数", pos));
            }
            let items = list_of(&args[0], "map", pos)?;
            let f = args[1].clone();
            let mut out = Vec::with_capacity(items.len());
            for it in &items {
                out.push(interp.call_value(&f, &[it.clone()], pos)?);
            }
            Ok(Value::new_list(out))
        }
        Builtin::Filter => {
            if args.len() != 2 {
                return Err(err("filter 需要 (list, fn) 两个参数", pos));
            }
            let items = list_of(&args[0], "filter", pos)?;
            let f = args[1].clone();
            let mut out = Vec::new();
            for it in &items {
                let r = interp.call_value(&f, &[it.clone()], pos)?;
                if interp.is_truthy(&r, pos)? {
                    out.push(it.clone());
                }
            }
            Ok(Value::new_list(out))
        }
        Builtin::Reduce => {
            if args.len() < 2 || args.len() > 3 {
                return Err(err("reduce 需要 (list, fn, [init]) 参数", pos));
            }
            let items = list_of(&args[0], "reduce", pos)?;
            let f = args[1].clone();
            let mut acc = args.get(2).cloned().unwrap_or(Value::Int(0));
            for it in &items {
                acc = interp.call_value(&f, &[acc.clone(), it.clone()], pos)?;
            }
            Ok(acc)
        }
        // ---- std.net（M5.2）：TCP + HTTP 客户端 ----
        Builtin::TcpListen => {
            if args.len() != 1 {
                return Err(err("tcp_listen 需要 (port) 参数", pos));
            }
            let port = expect_int(&args[0], "tcp_listen", pos)?;
            let addr = format!("0.0.0.0:{}", port);
            match std::net::TcpListener::bind(&addr) {
                Ok(l) => {
                    let id = net_alloc_listener(l);
                    Ok(Value::Int(id))
                }
                Err(e) => Err(LxError::new(
                    "R3001",
                    format!("net: 监听端口失败 {}: {}", addr, e),
                    Some(pos),
                )),
            }
        }
        Builtin::TcpAccept => {
            if args.len() != 1 {
                return Err(err("tcp_accept 需要 (listener_id) 参数", pos));
            }
            let id = expect_int(&args[0], "tcp_accept", pos)?;
            // try_clone 副本：释放锁后再阻塞 accept，避免持锁阻塞导致死锁
            let listener = {
                let listeners = net_listeners().lock().unwrap();
                match listeners.get(&id) {
                    Some(l) => l
                        .try_clone()
                        .map_err(|e| LxError::new("R3002", format!("net: 复制 listener 失败: {}", e), Some(pos)))?,
                    None => {
                        return Err(LxError::new("R3002", format!("net: 无效的 listener {}", id), Some(pos)))
                    }
                }
            };
            match listener.accept() {
                Ok((stream, _addr)) => {
                    let sid = net_alloc_stream(stream);
                    Ok(Value::Int(sid))
                }
                Err(e) => Err(LxError::new("R3003", format!("net: accept 失败: {}", e), Some(pos))),
            }
        }
        Builtin::TcpConnect => {
            if args.len() != 2 {
                return Err(err("tcp_connect 需要 (host, port) 参数", pos));
            }
            let host = expect_str(&args[0], "tcp_connect", pos)?;
            let port = expect_int(&args[1], "tcp_connect", pos)?;
            let addr = format!("{}:{}", host, port);
            match std::net::TcpStream::connect(&addr) {
                Ok(stream) => {
                    let sid = net_alloc_stream(stream);
                    Ok(Value::Int(sid))
                }
                Err(e) => Err(LxError::new(
                    "R3004",
                    format!("net: 连接失败 {}: {}", addr, e),
                    Some(pos),
                )),
            }
        }
        Builtin::TcpSend => {
            if args.len() != 2 {
                return Err(err("tcp_send 需要 (conn_id, data) 参数", pos));
            }
            let id = expect_int(&args[0], "tcp_send", pos)?;
            let data = match &args[1] {
                Value::Str(s) => s.clone(),
                v => v.to_string(),
            };
            use std::io::Write;
            // try_clone 副本：释放锁后再写，避免持锁阻塞
            let mut stream = {
                let streams = net_streams().lock().unwrap();
                match streams.get(&id) {
                    Some(s) => s
                        .try_clone()
                        .map_err(|e| LxError::new("R3005", format!("net: 复制连接失败: {}", e), Some(pos)))?,
                    None => return Err(LxError::new("R3005", format!("net: 无效的连接 {}", id), Some(pos))),
                }
            };
            match stream.write_all(data.as_bytes()) {
                Ok(_) => Ok(Value::Int(data.len() as i64)),
                Err(e) => Err(LxError::new("R3006", format!("net: 发送失败: {}", e), Some(pos))),
            }
        }
        Builtin::TcpRecv => {
            if args.len() != 2 {
                return Err(err("tcp_recv 需要 (conn_id, maxlen) 参数", pos));
            }
            let id = expect_int(&args[0], "tcp_recv", pos)?;
            let maxlen = expect_int(&args[1], "tcp_recv", pos)?;
            use std::io::Read;
            // try_clone 副本：释放锁后再读，避免持锁阻塞
            let mut stream = {
                let streams = net_streams().lock().unwrap();
                match streams.get(&id) {
                    Some(s) => s
                        .try_clone()
                        .map_err(|e| LxError::new("R3007", format!("net: 复制连接失败: {}", e), Some(pos)))?,
                    None => return Err(LxError::new("R3007", format!("net: 无效的连接 {}", id), Some(pos))),
                }
            };
            let mut buf = vec![0u8; maxlen.max(1) as usize];
            match stream.read(&mut buf) {
                Ok(0) => Ok(Value::Str(String::new())),
                Ok(n) => Ok(Value::Str(String::from_utf8_lossy(&buf[..n]).to_string())),
                Err(e) => Err(LxError::new("R3008", format!("net: 接收失败: {}", e), Some(pos))),
            }
        }
        Builtin::TcpClose => {
            if args.len() != 1 {
                return Err(err("tcp_close 需要 (id) 参数", pos));
            }
            let id = expect_int(&args[0], "tcp_close", pos)?;
            net_close(id);
            Ok(Value::Null)
        }
        Builtin::HttpGet => {
            if args.len() != 1 {
                return Err(err("http_get 需要 (url) 参数", pos));
            }
            let url = expect_str(&args[0], "http_get", pos)?;
            match http_request(&url, "GET", None) {
                Ok(body) => Ok(Value::Str(body)),
                Err(e) => Err(LxError::new("R3009", format!("net: http_get 失败: {}", e), Some(pos))),
            }
        }
        Builtin::HttpPost => {
            if args.len() != 2 {
                return Err(err("http_post 需要 (url, body) 参数", pos));
            }
            let url = expect_str(&args[0], "http_post", pos)?;
            let body = expect_str(&args[1], "http_post", pos)?;
            match http_request(&url, "POST", Some(&body)) {
                Ok(b) => Ok(Value::Str(b)),
                Err(e) => Err(LxError::new("R3009", format!("net: http_post 失败: {}", e), Some(pos))),
            }
        }

        // ==================== M23c HTTP 生产化 ====================
        // http_request(url, method, body?, headers?) → dict {status, headers, body}
        //（keep-alive 连接池：同 host 连接复用；body/headers 可选）
        Builtin::HttpRequest => {
            if args.len() < 2 || args.len() > 4 {
                return Err(err("http_request 需要 (url, method[, body[, headers]]) 参数", pos));
            }
            let url = expect_str(&args[0], "http_request", pos)?;
            let method = expect_str(&args[1], "http_request", pos)?;
            let body = match args.get(2) {
                Some(Value::Str(s)) => Some(s.clone()),
                Some(Value::Null) | None => None,
                Some(v) => {
                    return Err(err(format!("http_request 的 body 需要字符串，实际是 {}", interp.type_name(v)), pos))
                }
            };
            let mut headers: HashMap<String, String> = HashMap::new();
            if let Some(Value::Dict(d)) = args.get(3) {
                for (k, v) in d.lock().unwrap().iter() {
                    match v {
                        Value::Str(sv) => {
                            headers.insert(k.clone(), sv.clone());
                        }
                        _ => return Err(err(format!("http_request 的 headers 值需要字符串: {}", k), pos)),
                    }
                }
            }
            match http_request_full(&url, &method, body.as_deref(), &headers) {
                Ok(d) => Ok(Value::new_dict(d)),
                Err(e) => Err(LxError::new("R3009", format!("net: http_request 失败: {}", e), Some(pos))),
            }
        }
        // http_get_stream(url, chunk_handler) → bool：流式下载，每块调 chunk_handler(块文本)；
        // handler 返回 false 中止（返回 true=完整下载，false=被中止）
        Builtin::HttpGetStream => {
            if args.len() != 2 {
                return Err(err("http_get_stream 需要 (url, chunk_handler) 参数", pos));
            }
            let url = expect_str(&args[0], "http_get_stream", pos)?;
            let handler = args[1].clone();
            if !matches!(handler, Value::Func(_)) {
                return Err(err("http_get_stream 的 chunk_handler 必须是函数", pos));
            }
            match http_get_stream_impl(&url, interp, &handler, pos) {
                Ok(complete) => Ok(Value::Bool(complete)),
                Err(e) => Err(LxError::new("R3009", format!("net: http_get_stream 失败: {}", e), Some(pos))),
            }
        }

        Builtin::HttpServe => {
            if args.len() != 2 {
                return Err(err("http_serve 需要 (port, handler) 参数", pos));
            }
            let port = expect_int(&args[0], "http_serve", pos)?;
            let handler = args[1].clone();
            if !matches!(handler, Value::Func(_)) {
                return Err(err("http_serve 的 handler 必须是函数", pos));
            }
            let addr = format!("0.0.0.0:{}", port);
            let listener = TcpListener::bind(&addr)
                .map_err(|e| LxError::new("R3010", format!("net: 监听端口失败 {}: {}", addr, e), Some(pos)))?;
            // M16：HTTP 服务端框架——阻塞 accept 循环（Go 风格 ListenAndServe），每连接一个处理线程
            let srv = interp.fork();
            let mut i = srv;
            for stream in listener.incoming() {
                let mut stream = match stream {
                    Ok(s) => s,
                    Err(_) => continue,
                };
                let h = handler.clone();
                let mut ci = i.fork();
                std::thread::spawn(move || {
                    if let Err(e) = handle_http_conn(&mut ci, &mut stream, &h, pos) {
                        eprintln!("[http] {}", e);
                    }
                });
            }
            Ok(Value::Null) // 不可达：incoming() 无限迭代
        }

        Builtin::PxExec => {
            // px_exec(path, params?)：内嵌解释器执行 .px 脚本，捕获 print 输出返回
            if args.len() < 1 || args.len() > 2 {
                return Err(err("px_exec 需要 (path[, params]) 参数", pos));
            }
            let path = expect_str(&args[0], "px_exec", pos)?;
            let params = match args.get(1) {
                Some(Value::Dict(d)) => d.lock().unwrap().clone(),
                Some(Value::Null) | None => HashMap::new(),
                Some(other) => {
                    return Err(err(
                        format!("px_exec 的 params 需要 dict，实际为 {}", interp.type_name(other)),
                        pos,
                    ))
                }
            };
            match crate::web::px_exec(path, &params) {
                Ok(v) => Ok(v),
                Err(e) => Err(LxError::new("R3011", format!("px_exec: {}", e), Some(pos))),
            }
        }
        Builtin::PxServe => {
            // px_serve(port, docroot[, timeout_ms])：PHP 式应用服务器（静态文件 + .px 脚本）
            if args.len() < 2 || args.len() > 3 {
                return Err(err("px_serve 需要 (port, docroot[, timeout_ms]) 参数", pos));
            }
            let port = expect_int(&args[0], "px_serve", pos)?;
            let docroot = expect_str(&args[1], "px_serve", pos)?;
            let timeout_ms = match args.get(2) {
                Some(Value::Int(t)) => *t,
                _ => 10000,
            };
            crate::web::px_serve(port, docroot, timeout_ms.max(1)).map_err(|e| {
                LxError::new("R3012", format!("px_serve: {}", e), Some(pos))
            })?;
            Ok(Value::Null) // 不可达：阻塞 accept
        }

        // ==================== M18 P1：后台定时任务 / 定时器原语 ====================
        // set_timeout(fn, ms, ...args) → int：一次性定时器，ms 毫秒后调用 fn(...args)
        // set_interval(fn, ms, ...args) → int：周期定时器，每 ms 毫秒调用一次 fn(...args)
        // clear_timer(id) → bool：取消定时器（已执行/已取消返回 false）
        // 语义：回调在独立线程执行（与主线程并发），执行前检查取消标记；
        //       set_interval 固定节奏（执行耗时不计入间隔，不堆积）；回调内可安全使用
        //       全局变量 / chan / 锁 / spawn（共享 globals 与类型表）。
        Builtin::SetTimeout | Builtin::SetInterval => {
            if args.len() < 2 {
                return Err(err(
                    "set_timeout/set_interval 需要 (fn, ms[, ...args]) 参数",
                    pos,
                ));
            }
            let fv = args[0].clone();
            if !matches!(fv, Value::Func(_)) {
                return Err(err("定时器第一个参数必须是函数", pos));
            }
            let ms = expect_int(&args[1], "定时器", pos)?;
            if ms < 0 {
                return Err(err("定时器间隔不能为负数", pos));
            }
            let extra = args[2..].to_vec();
            let periodic = matches!(b, Builtin::SetInterval);
            let timers = interp.timers.clone();
            let id = timers.next_id.fetch_add(1, Ordering::SeqCst) + 1;
            let interp = interp.fork();
            thread::spawn(move || {
                let mut i = interp;
                loop {
                    std::thread::sleep(Duration::from_millis(ms as u64));
                    // 取消检查（执行前）
                    if timers.canceled.lock().unwrap().contains(&id) {
                        timers.canceled.lock().unwrap().remove(&id);
                        return;
                    }
                    if let Err(e) = i.call_value(&fv, &extra, pos) {
                        eprintln!("[定时器 {}] {}", id, e);
                    }
                    if !periodic {
                        // 一次性：执行完毕，清理 id（后续 clear_timer 返回 false）
                        timers.canceled.lock().unwrap().remove(&id);
                        return;
                    }
                }
            });
            Ok(Value::Int(id))
        }
        Builtin::ClearTimer => {
            if args.len() != 1 {
                return Err(err("clear_timer 需要 1 个参数", pos));
            }
            let id = expect_int(&args[0], "clear_timer", pos)?;
            let mut c = interp.timers.canceled.lock().unwrap();
            if c.contains(&id) {
                return Ok(Value::Bool(false));
            }
            c.insert(id);
            Ok(Value::Bool(true))
        }
        // ---- M19 P1：AES 加密（企微回调加解密 / 数据落盘加密 / Cookie 签名）----
        // aes_encrypt(data, key, iv) → hex（AES-CBC-PKCS7；key 16/24/32 字节 → 128/192/256 位）
        Builtin::AesEncrypt => {
            if args.len() != 3 {
                return Err(err("aes_encrypt 需要 3 个参数: (data, key, iv)", pos));
            }
            let data = bytes_of(&args[0]);
            let key = bytes_of(&args[1]);
            let iv = bytes_of(&args[2]);
            let ct = crate::aes::cbc_encrypt(&data, &key, &iv).map_err(|e| err(e, pos))?;
            Ok(Value::Str(hex_encode(&ct)))
        }
        // aes_decrypt(hex, key, iv) → str 或 null（padding 非法 / 非 UTF-8 → null）
        Builtin::AesDecrypt => {
            if args.len() != 3 {
                return Err(err("aes_decrypt 需要 3 个参数: (hex, key, iv)", pos));
            }
            let ct = match hex_decode(expect_str(&args[0], "aes_decrypt", pos)?) {
                Ok(c) => c,
                Err(_) => return Ok(Value::Null), // 非法 hex → null（与编译模式一致）
            };
            let key = bytes_of(&args[1]);
            let iv = bytes_of(&args[2]);
            match crate::aes::cbc_decrypt(&ct, &key, &iv).map_err(|e| err(e, pos))? {
                Some(pt) => match String::from_utf8(pt) {
                    Ok(s) => Ok(Value::Str(s)),
                    Err(_) => Ok(Value::Null),
                },
                None => Ok(Value::Null),
            }
        }
        // aes_gcm_encrypt(data, key, iv) → hex（密文 + 16 字节 tag）
        Builtin::AesGcmEncrypt => {
            if args.len() != 3 {
                return Err(err("aes_gcm_encrypt 需要 3 个参数: (data, key, iv)", pos));
            }
            let data = bytes_of(&args[0]);
            let key = bytes_of(&args[1]);
            let iv = bytes_of(&args[2]);
            let (ct, tag) = crate::aes::gcm_encrypt(&data, &key, &iv).map_err(|e| err(e, pos))?;
            let mut out = ct;
            out.extend_from_slice(&tag);
            Ok(Value::Str(hex_encode(&out)))
        }
        // aes_gcm_decrypt(hex, key, iv) → str 或 null（tag 校验失败 → null）
        Builtin::AesGcmDecrypt => {
            if args.len() != 3 {
                return Err(err("aes_gcm_decrypt 需要 3 个参数: (hex, key, iv)", pos));
            }
            let all = match hex_decode(expect_str(&args[0], "aes_gcm_decrypt", pos)?) {
                Ok(c) => c,
                Err(_) => return Ok(Value::Null), // 非法 hex → null（与编译模式一致）
            };
            if all.len() < 16 {
                return Ok(Value::Null);
            }
            let (ct, tag) = all.split_at(all.len() - 16);
            let key = bytes_of(&args[1]);
            let iv = bytes_of(&args[2]);
            match crate::aes::gcm_decrypt(ct, &key, &iv, tag).map_err(|e| err(e, pos))? {
                Some(pt) => match String::from_utf8(pt) {
                    Ok(s) => Ok(Value::Str(s)),
                    Err(_) => Ok(Value::Null),
                },
                None => Ok(Value::Null),
            }
        }
        // ---- M19 P1：XML 解析（企微回调 Encrypt 报文 / 配置文件 / 文档）----
        // xml_parse(xml) → dict{name, attrs, children, text} 或 null
        Builtin::XmlParse => {
            if args.len() != 1 {
                return Err(err("xml_parse 需要 1 个参数", pos));
            }
            let s = expect_str(&args[0], "xml_parse", pos)?;
            match crate::xml::parse(s) {
                Ok(root) => Ok(xml_node_to_value(&root)),
                Err(e) => Err(err(e, pos)),
            }
        }
        // xml_escape(text) → str
        Builtin::XmlEscape => {
            if args.len() != 1 {
                return Err(err("xml_escape 需要 1 个参数", pos));
            }
            Ok(Value::Str(crate::xml::escape(expect_str(&args[0], "xml_escape", pos)?)))
        }
        // xml_unescape(text) → str
        Builtin::XmlUnescape => {
            if args.len() != 1 {
                return Err(err("xml_unescape 需要 1 个参数", pos));
            }
            Ok(Value::Str(crate::xml::unescape(expect_str(&args[0], "xml_unescape", pos)?)))
        }
        // ---- M19 P1：zip 打包/解压（docx/xlsx/pptx 是 zip+xml，文档工具基石）----
        // zip_pack(files, out_path) → bool（files: dict{路径→内容}，deflate 压缩）
        Builtin::ZipPack => {
            if args.len() != 2 {
                return Err(err("zip_pack 需要 2 个参数: (files, out_path)", pos));
            }
            let d = match &args[0] {
                Value::Dict(m) => m.clone(),
                _ => return Err(err("zip_pack 第一个参数须为 dict{路径→内容}", pos)),
            };
            let out_path = expect_str(&args[1], "zip_pack", pos)?.to_string();
            let files: Vec<(String, Vec<u8>)> = {
                let map = d.lock().unwrap();
                map.iter()
                    .map(|(k, v)| (k.clone(), bytes_of(v)))
                    .collect()
            };
            crate::zip::pack(&files, &out_path).map_err(|e| err(e, pos))?;
            Ok(Value::Bool(true))
        }
        // zip_unpack(zip_path, out_dir) → int（解压文件数，防路径穿越）
        Builtin::ZipUnpack => {
            if args.len() != 2 {
                return Err(err("zip_unpack 需要 2 个参数: (zip_path, out_dir)", pos));
            }
            let zp = expect_str(&args[0], "zip_unpack", pos)?.to_string();
            let od = expect_str(&args[1], "zip_unpack", pos)?.to_string();
            let names = crate::zip::unpack(&zp, &od).map_err(|e| err(e, pos))?;
            Ok(Value::Int(names.len() as i64))
        }

        // ==================== M21 P1：base64 编解码 ====================
        Builtin::Base64Encode => {
            if args.len() != 1 {
                return Err(err("base64_encode 需要一个参数", pos));
            }
            let data = bytes_of(&args[0]);
            Ok(Value::Str(base64_encode_bytes(&data)))
        }
        Builtin::Base64Decode => {
            if args.len() != 1 {
                return Err(err("base64_decode 需要一个参数", pos));
            }
            let s = expect_str(&args[0], "base64_decode", pos)?;
            match base64_decode_bytes(s) {
                Some(bytes) => Ok(Value::Str(String::from_utf8_lossy(&bytes).to_string())),
                None => Ok(Value::Null),
            }
        }

        // ==================== M21 P1：SSE 服务端 ====================
        Builtin::SseServe => {
            if args.len() != 2 {
                return Err(err("sse_serve 需要 (port, handler) 参数", pos));
            }
            let port = expect_int(&args[0], "sse_serve", pos)?;
            let handler = args[1].clone();
            if !matches!(handler, Value::Func(_)) {
                return Err(err("sse_serve 的 handler 必须是函数", pos));
            }
            let addr = format!("0.0.0.0:{}", port);
            let listener = TcpListener::bind(&addr)
                .map_err(|e| LxError::new("R3010", format!("net: 监听端口失败 {}: {}", addr, e), Some(pos)))?;
            let srv = interp.fork();
            let mut i = srv;
            for stream in listener.incoming() {
                let mut stream = match stream {
                    Ok(s) => s,
                    Err(_) => continue,
                };
                let h = handler.clone();
                let mut ci = i.fork();
                std::thread::spawn(move || {
                    if let Err(e) = handle_sse_conn(&mut ci, &mut stream, &h, pos) {
                        eprintln!("[sse] {}", e);
                    }
                });
            }
            Ok(Value::Null) // 不可达：incoming() 无限迭代
        }
        Builtin::SseSend => {
            if args.len() != 2 {
                return Err(err("sse_send 需要 (conn, data) 参数", pos));
            }
            let conn = expect_int(&args[0], "sse_send", pos)?;
            let frame = sse_frame(&args[1]);
            let mut m = sse_conns().lock().unwrap();
            if !m.contains_key(&conn) {
                return Ok(Value::Bool(false)); // 连接不存在/已关闭
            }
            let mut write_ok = false;
            {
                let c = m.get(&conn).unwrap();
                let mut st = c.stream.lock().unwrap();
                write_ok = st.write_all(frame.as_bytes()).is_ok();
                if write_ok {
                    let _ = st.flush();
                }
            }
            if !write_ok {
                // 对端断开：唤醒连接线程清理并移除
                if let Some(c) = m.get(&conn) {
                    let _ = c.close_tx.send(());
                }
                m.remove(&conn);
                Ok(Value::Bool(false))
            } else {
                Ok(Value::Bool(true))
            }
        }
        Builtin::SseClose => {
            if args.len() != 1 {
                return Err(err("sse_close 需要 (conn) 参数", pos));
            }
            let conn = expect_int(&args[0], "sse_close", pos)?;
            // 服务端连接
            {
                let mut m = sse_conns().lock().unwrap();
                if let Some(c) = m.remove(&conn) {
                    let _ = c.close_tx.send(());
                    return Ok(Value::Bool(true));
                }
            }
            // 客户端连接
            Ok(Value::Bool(sse_client_close(conn)))
        }
        // ==================== M23 SSE 客户端（流式消费 / 事件订阅） ====================
        Builtin::SseConnect => {
            if args.len() != 1 {
                return Err(err("sse_connect 需要 (url) 参数", pos));
            }
            let url = expect_str(&args[0], "sse_connect", pos)?;
            match sse_client_connect(&url) {
                Some(id) => Ok(Value::Int(id)),
                None => Ok(Value::Null),
            }
        }
        Builtin::SseRead => {
            if args.len() != 1 {
                return Err(err("sse_read 需要 (conn) 参数", pos));
            }
            let conn = expect_int(&args[0], "sse_read", pos)?;
            match sse_client_read(conn) {
                Some(v) => Ok(v),
                None => Ok(Value::Null),
            }
        }

        // ==================== M22 P1：位运算 / 二进制数据视图 ====================
        Builtin::IntToHex => {
            if args.len() != 2 {
                return Err(err("int_to_hex 需要 (n, width) 参数", pos));
            }
            let n = expect_int(&args[0], "int_to_hex", pos)?;
            let w = expect_int(&args[1], "int_to_hex", pos)?;
            if w < 1 || w > 16 {
                return Err(err("int_to_hex 的 width 必须在 1..16", pos));
            }
            // 负数按补码：取低 4*width 位
            let mask = if w >= 16 { u64::MAX } else { (1u64 << (4 * w)) - 1 };
            let v = (n as u64) & mask;
            Ok(Value::Str(format!("{:0width$x}", v, width = w as usize)))
        }
        Builtin::HexToInt => {
            if args.len() != 1 {
                return Err(err("hex_to_int 需要一个参数", pos));
            }
            let s = expect_str(&args[0], "hex_to_int", pos)?;
            let clean: String = s.chars().filter(|c| !c.is_whitespace()).collect();
            if clean.is_empty() || clean.len() > 16 {
                return Ok(Value::Null);
            }
            match i64::from_str_radix(&clean, 16) {
                Ok(v) => Ok(Value::Int(v)),
                Err(_) => Ok(Value::Null),
            }
        }
        Builtin::BytesToHex => {
            if args.len() != 1 {
                return Err(err("bytes_to_hex 需要一个参数", pos));
            }
            let data = bytes_of(&args[0]);
            Ok(Value::Str(hex_encode(&data)))
        }
        Builtin::HexToBytes => {
            if args.len() != 1 {
                return Err(err("hex_to_bytes 需要一个参数", pos));
            }
            let s = expect_str(&args[0], "hex_to_bytes", pos)?;
            match hex_decode(s) {
                Ok(bytes) => Ok(Value::Str(String::from_utf8_lossy(&bytes).to_string())),
                Err(_) => Ok(Value::Null),
            }
        }
        Builtin::BitCount => {
            if args.len() != 1 {
                return Err(err("bit_count 需要一个参数", pos));
            }
            let n = expect_int(&args[0], "bit_count", pos)?;
            Ok(Value::Int(n.count_ones() as i64))
        }
        Builtin::BitLength => {
            if args.len() != 1 {
                return Err(err("bit_length 需要一个参数", pos));
            }
            let n = expect_int(&args[0], "bit_length", pos)?;
            if n <= 0 {
                Ok(Value::Int(0))
            } else {
                Ok(Value::Int((64 - n.leading_zeros()) as i64))
            }
        }

        // ==================== M22 P1：WebSocket（RFC 6455） ====================
        Builtin::WsServe => {
            if args.len() != 2 {
                return Err(err("ws_serve 需要 (port, handler) 参数", pos));
            }
            let port = expect_int(&args[0], "ws_serve", pos)?;
            let handler = args[1].clone();
            if !matches!(handler, Value::Func(_)) {
                return Err(err("ws_serve 的 handler 必须是函数", pos));
            }
            let addr = format!("0.0.0.0:{}", port);
            let listener = TcpListener::bind(&addr)
                .map_err(|e| LxError::new("R3010", format!("net: 监听端口失败 {}: {}", addr, e), Some(pos)))?;
            let srv = interp.fork();
            for stream in listener.incoming() {
                let mut stream = match stream {
                    Ok(s) => s,
                    Err(_) => continue,
                };
                // 握手失败直接关闭该连接
                if crate::ws::server_handshake(&mut stream).is_err() {
                    continue;
                }
                let h = handler.clone();
                let mut ci = srv.fork();
                std::thread::spawn(move || {
                    // 注册连接（读写独立句柄）
                    let id = match stream.try_clone() {
                        Ok(w) => crate::ws::ws_register(stream, w),
                        Err(_) => return,
                    };
                    // 调 handler(conn)：handler 内 ws_recv 阻塞读 / ws_send 推送
                    let arg = Value::Int(id);
                    if let Err(e) = ci.call_value(&h, &[arg], pos) {
                        eprintln!("[ws] handler 出错: {}", e);
                    }
                    // handler 返回后保持连接（后台线程可 ws_send），直到 ws_close/对端断开
                    std::thread::sleep(std::time::Duration::from_millis(100));
                    // 连接断开时 ws_recv 已自动清理；这里兜底清理已关闭连接
                    let closed = {
                        let m = crate::ws::ws_conns().lock().unwrap();
                        m.get(&id).map(|c| c.closed.load(std::sync::atomic::Ordering::SeqCst)).unwrap_or(true)
                    };
                    if closed {
                        crate::ws::ws_unregister(id);
                    }
                });
            }
            Ok(Value::Null) // 不可达
        }
        Builtin::WsConnect => {
            if args.len() != 3 {
                return Err(err("ws_connect 需要 (host, port, path) 参数", pos));
            }
            let host = expect_str(&args[0], "ws_connect", pos)?.to_string();
            let port = expect_int(&args[1], "ws_connect", pos)?;
            let path = expect_str(&args[2], "ws_connect", pos)?.to_string();
            let addr = format!("{}:{}", host, port);
            let mut stream = TcpStream::connect(&addr)
                .map_err(|e| LxError::new("R3010", format!("net: 连接 {} 失败: {}", addr, e), Some(pos)))?;
            if let Err(_e) = crate::ws::client_handshake(&mut stream, &host, port as u16, &path) {
                return Ok(Value::Null); // 握手失败 → null（与编译模式一致）
            }
            let (id, _rx) = match stream.try_clone() {
                Ok(w) => crate::ws::ws_register_client(stream, w),
                Err(_) => return Ok(Value::Null),
            };
            Ok(Value::Int(id))
        }
        Builtin::WsSend => {
            if args.len() != 2 {
                return Err(err("ws_send 需要 (conn, data) 参数", pos));
            }
            let conn = expect_int(&args[0], "ws_send", pos)?;
            let data = match &args[1] {
                Value::Str(s) => s.clone(),
                other => other.to_string(),
            };
            Ok(Value::Bool(crate::ws::ws_send(conn, &data)))
        }
        Builtin::WsRecv => {
            let timeout_ms = match args.len() {
                1 => None,
                2 => {
                    let ms = expect_int(&args[1], "ws_recv", pos)?;
                    Some(ms)
                }
                _ => {
                    return Err(err("ws_recv 需要 (conn) 或 (conn, timeout_ms) 参数", pos));
                }
            };
            let conn = expect_int(&args[0], "ws_recv", pos)?;
            match crate::ws::ws_recv(conn, timeout_ms) {
                Some(msg) => Ok(Value::Str(msg)),
                None => Ok(Value::Null),
            }
        }
        Builtin::WsClose => {
            if args.len() != 1 {
                return Err(err("ws_close 需要 (conn) 参数", pos));
            }
            let conn = expect_int(&args[0], "ws_close", pos)?;
            Ok(Value::Bool(crate::ws::ws_close(conn)))
        }
        Builtin::WsPing => {
            if args.len() != 1 {
                return Err(err("ws_ping 需要 (conn) 参数", pos));
            }
            let conn = expect_int(&args[0], "ws_ping", pos)?;
            Ok(Value::Bool(crate::ws::ws_ping(conn)))
        }

        // ==================== M23 进程 / 信号 ====================
        Builtin::OsPid => Ok(Value::Int(std::process::id() as i64)),
        Builtin::OsSpawn => {
            if args.len() != 2 {
                return Err(err("os_spawn 需要 (cmd, args) 参数", pos));
            }
            let cmd = expect_str(&args[0], "os_spawn", pos)?;
            let list = match &args[1] {
                Value::List(l) => l.lock().unwrap().clone(),
                _ => return Err(err("os_spawn 的 args 必须是字符串列表", pos)),
            };
            let mut cmd_args: Vec<String> = Vec::new();
            for a in list {
                match a {
                    Value::Str(s) => cmd_args.push(s.clone()),
                    _ => return Err(err("os_spawn 的 args 必须是字符串列表", pos)),
                }
            }
            match std::process::Command::new(&cmd).args(&cmd_args).spawn() {
                Ok(child) => {
                    let pid = child.id();
                    proc_children().lock().unwrap().insert(pid, child);
                    Ok(Value::Int(pid as i64))
                }
                Err(_) => Ok(Value::Null), // 启动失败
            }
        }
        Builtin::OsWait => {
            if args.len() != 1 {
                return Err(err("os_wait 需要 (pid) 参数", pos));
            }
            let pid = expect_int(&args[0], "os_wait", pos)?;
            let mut m = proc_children().lock().unwrap();
            match m.remove(&(pid as u32)) {
                Some(mut child) => match child.wait() {
                    Ok(st) => {
                        if let Some(c) = st.code() {
                            Ok(Value::Int(c as i64))
                        } else if let Some(sig) = st.signal() {
                            Ok(Value::Int(128 + sig as i64))
                        } else {
                            Ok(Value::Int(-1))
                        }
                    }
                    Err(_) => Ok(Value::Int(-1)),
                },
                None => Ok(Value::Int(-1)), // 非 os_spawn 子进程无法 wait
            }
        }
        Builtin::OsKill => {
            if args.len() != 2 {
                return Err(err("os_kill 需要 (pid, sig) 参数", pos));
            }
            let pid = expect_int(&args[0], "os_kill", pos)?;
            let sig = expect_int(&args[1], "os_kill", pos)?;
            let r = unsafe { kill(pid as i32, sig as i32) };
            Ok(Value::Bool(r == 0))
        }
        Builtin::Signal => {
            if args.len() != 2 {
                return Err(err("signal 需要 (sig, handler) 参数", pos));
            }
            let sig = expect_int(&args[0], "signal", pos)?;
            if sig < 1 || sig > 64 {
                return Err(err("signal 的 sig 必须在 1..64", pos));
            }
            let handler = args[1].clone();
            if !matches!(handler, Value::Func(_)) {
                return Err(err("signal 的 handler 必须是函数", pos));
            }
            sig_handlers().lock().unwrap().insert(sig as i32, handler);
            ensure_signal_thread(interp, pos);
            unsafe {
                signal(sig as i32, sig_bridge as usize);
            }
            Ok(Value::Bool(true))
        }

        // ==================== M23d RSA（PKCS#1 v1.5，密钥/密文/签名均 hex） ====================
        Builtin::RsaGenKey => {
            if args.len() != 1 {
                return Err(err("rsa_gen_key 需要 (bits) 参数", pos));
            }
            let bits = expect_int(&args[0], "rsa_gen_key", pos)?;
            if bits < 512 || bits > 4096 {
                return Err(err("rsa_gen_key 的 bits 必须在 512..4096", pos));
            }
            match crate::rsa::rsa_gen_key(bits as usize) {
                Some(items) => {
                    let mut m = HashMap::new();
                    for (k, v) in items {
                        m.insert(k, Value::Str(v));
                    }
                    Ok(Value::new_dict(m))
                }
                None => Err(err("rsa_gen_key 生成密钥失败", pos)),
            }
        }
        Builtin::RsaEncrypt => {
            if args.len() != 3 {
                return Err(err("rsa_encrypt 需要 (data, n_hex, e_hex) 参数", pos));
            }
            let data = bytes_of(&args[0]);
            let n = expect_str(&args[1], "rsa_encrypt", pos)?;
            let e = expect_str(&args[2], "rsa_encrypt", pos)?;
            match crate::rsa::rsa_encrypt(&data, &n, &e) {
                Some(ct) => Ok(Value::Str(ct)),
                None => Ok(Value::Null), // 数据过长/密钥非法
            }
        }
        Builtin::RsaDecrypt => {
            if args.len() != 3 {
                return Err(err("rsa_decrypt 需要 (ct_hex, n_hex, d_hex) 参数", pos));
            }
            let ct = expect_str(&args[0], "rsa_decrypt", pos)?;
            let n = expect_str(&args[1], "rsa_decrypt", pos)?;
            let d = expect_str(&args[2], "rsa_decrypt", pos)?;
            match crate::rsa::rsa_decrypt(&ct, &n, &d) {
                Some(pt) => Ok(Value::Str(String::from_utf8_lossy(&pt).to_string())),
                None => Ok(Value::Null),
            }
        }
        Builtin::RsaSign => {
            if args.len() != 3 {
                return Err(err("rsa_sign 需要 (data, n_hex, d_hex) 参数", pos));
            }
            let data = bytes_of(&args[0]);
            let n = expect_str(&args[1], "rsa_sign", pos)?;
            let d = expect_str(&args[2], "rsa_sign", pos)?;
            match crate::rsa::rsa_sign(&data, &n, &d) {
                Some(sig) => Ok(Value::Str(sig)),
                None => Ok(Value::Null),
            }
        }
        Builtin::RsaVerify => {
            if args.len() != 4 {
                return Err(err("rsa_verify 需要 (data, sig_hex, n_hex, e_hex) 参数", pos));
            }
            let data = bytes_of(&args[0]);
            let sig = expect_str(&args[1], "rsa_verify", pos)?;
            let n = expect_str(&args[2], "rsa_verify", pos)?;
            let e = expect_str(&args[3], "rsa_verify", pos)?;
            Ok(Value::Bool(crate::rsa::rsa_verify(&data, &sig, &n, &e)))
        }

        // ==================== M23b 二进制安全字节串 ====================
        // bytes(s) → bytes（字符串 UTF-8 字节原样；Str/Bytes 均可）
        Builtin::Bytes => {
            if args.len() != 1 {
                return Err(err("bytes 需要一个参数", pos));
            }
            Ok(Value::Bytes(bytes_of(&args[0])))
        }
        Builtin::BytesLen => {
            if args.len() != 1 {
                return Err(err("bytes_len 需要一个参数", pos));
            }
            match &args[0] {
                Value::Bytes(b) => Ok(Value::Int(b.len() as i64)),
                v => Err(err(
                    format!("bytes_len 需要 bytes，实际是 {}", interp.type_name(v)),
                    pos,
                )),
            }
        }
        Builtin::BytesGet => {
            if args.len() != 2 {
                return Err(err("bytes_get 需要 (bytes, index) 参数", pos));
            }
            let (b, i) = match (&args[0], &args[1]) {
                (Value::Bytes(b), Value::Int(i)) => (b, *i),
                (v, _) => {
                    return Err(err(
                        format!("bytes_get 需要 bytes 和 int，实际是 {}", interp.type_name(v)),
                        pos,
                    ))
                }
            };
            let mut idx = i;
            if idx < 0 {
                idx += b.len() as i64;
            }
            if idx < 0 || idx >= b.len() as i64 {
                return Ok(Value::Null);
            }
            Ok(Value::Int(b[idx as usize] as i64))
        }
        Builtin::BytesSet => {
            // bytes_set(b, i, v) → bytes（函数式：返回修改后的新 bytes，原对象不变）
            if args.len() != 3 {
                return Err(err("bytes_set 需要 (bytes, index, value) 参数", pos));
            }
            let mut b = match &args[0] {
                Value::Bytes(b) => b.clone(),
                v => {
                    return Err(err(
                        format!("bytes_set 需要 bytes，实际是 {}", interp.type_name(v)),
                        pos,
                    ))
                }
            };
            let i = expect_int(&args[1], "bytes_set", pos)?;
            let v = expect_int(&args[2], "bytes_set", pos)?;
            if v < 0 || v > 255 {
                return Err(err("bytes_set 的值必须在 0..255", pos));
            }
            let mut idx = i;
            if idx < 0 {
                idx += b.len() as i64;
            }
            if idx < 0 || idx >= b.len() as i64 {
                return Err(err("bytes_set 下标越界", pos));
            }
            b[idx as usize] = v as u8;
            Ok(Value::Bytes(b))
        }
        Builtin::BytesSlice => {
            // bytes_slice(b, start, end) → bytes；start/end 可为 null（省略）；负索引/越界 clamp
            if args.len() < 1 || args.len() > 3 {
                return Err(err("bytes_slice 需要 (bytes[, start[, end]]) 参数", pos));
            }
            let b = match &args[0] {
                Value::Bytes(b) => b.clone(),
                v => {
                    return Err(err(
                        format!("bytes_slice 需要 bytes，实际是 {}", interp.type_name(v)),
                        pos,
                    ))
                }
            };
            let start = match args.get(1) {
                None | Some(Value::Null) => None,
                Some(Value::Int(i)) => Some(*i),
                Some(_) => return Err(err("bytes_slice 的边界必须是整数或 null", pos)),
            };
            let end = match args.get(2) {
                None | Some(Value::Null) => None,
                Some(Value::Int(i)) => Some(*i),
                Some(_) => return Err(err("bytes_slice 的边界必须是整数或 null", pos)),
            };
            let (a, bb) = bytes_slice_bounds(start, end, b.len() as i64);
            Ok(Value::Bytes(b[a..bb].to_vec()))
        }
        Builtin::BytesConcat => {
            // bytes_concat(a, b, ...) → bytes（Str/Bytes 混合均可，UTF-8 字节原样）
            if args.len() < 1 {
                return Err(err("bytes_concat 至少需要一个参数", pos));
            }
            let mut out = Vec::new();
            for a in args {
                out.extend_from_slice(&bytes_of(a));
            }
            Ok(Value::Bytes(out))
        }
        Builtin::BytesToStr => {
            if args.len() != 1 {
                return Err(err("bytes_to_str 需要一个参数", pos));
            }
            match &args[0] {
                Value::Bytes(b) => Ok(Value::Str(String::from_utf8_lossy(b).to_string())),
                v => Err(err(
                    format!("bytes_to_str 需要 bytes，实际是 {}", interp.type_name(v)),
                    pos,
                )),
            }
        }
        Builtin::BytesBase64 => {
            if args.len() != 1 {
                return Err(err("bytes_base64 需要一个参数", pos));
            }
            match &args[0] {
                Value::Bytes(b) => Ok(Value::Str(base64_encode_bytes(b))),
                v => Err(err(
                    format!("bytes_base64 需要 bytes，实际是 {}", interp.type_name(v)),
                    pos,
                )),
            }
        }
        Builtin::Base64ToBytes => {
            if args.len() != 1 {
                return Err(err("base64_to_bytes 需要一个参数", pos));
            }
            let s = expect_str(&args[0], "base64_to_bytes", pos)?;
            match base64_decode_bytes(s) {
                Some(b) => Ok(Value::Bytes(b)),
                None => Ok(Value::Null), // 严格：非法 base64 → null
            }
        }
        Builtin::BytesFind => {
            // bytes_find(b, sub) → int|null（子串字节下标；sub 可为 bytes 或 str）
            if args.len() != 2 {
                return Err(err("bytes_find 需要 (bytes, sub) 参数", pos));
            }
            let b = match &args[0] {
                Value::Bytes(b) => b.clone(),
                v => {
                    return Err(err(
                        format!("bytes_find 需要 bytes，实际是 {}", interp.type_name(v)),
                        pos,
                    ))
                }
            };
            let sub = bytes_of(&args[1]);
            if sub.is_empty() {
                return Ok(Value::Int(0));
            }
            let n = b.len();
            let m = sub.len();
            if m > n {
                return Ok(Value::Null);
            }
            let mut i = 0;
            while i + m <= n {
                if &b[i..i + m] == &sub[..] {
                    return Ok(Value::Int(i as i64));
                }
                i += 1;
            }
            Ok(Value::Null)
        }
        Builtin::ReadBytes => {
            if args.len() != 1 {
                return Err(err("read_bytes 需要一个路径参数", pos));
            }
            let p = expect_str(&args[0], "read_bytes", pos)?;
            match std::fs::read(p) {
                Ok(b) => Ok(Value::Bytes(b)),
                Err(e) => Err(LxError::new(
                    "R2001",
                    format!("io: 读取文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }
        Builtin::WriteBytes => {
            if args.len() != 2 {
                return Err(err("write_bytes 需要 (路径, bytes) 参数", pos));
            }
            let p = expect_str(&args[0], "write_bytes", pos)?;
            let data = bytes_of(&args[1]);
            match std::fs::write(p, &data) {
                Ok(_) => Ok(Value::Bool(true)),
                Err(e) => Err(LxError::new(
                    "R2002",
                    format!("io: 写入文件失败 {}: {}", p, e),
                    Some(pos),
                )),
            }
        }

        // ==================== M22 P1：解释器循环引用回收 ====================
        Builtin::Gc => {
            if !args.is_empty() {
                return Err(err("gc 不需要参数", pos));
            }
            let ran = crate::gc::collect();
            Ok(Value::Int(if ran { 1 } else { 0 }))
        }
    }
}

// ==================== M21 辅助：base64 ====================

/// RFC 4648 标准 Base64 编码（带 padding）
pub(crate) fn base64_encode_bytes(data: &[u8]) -> String {
    const T: &[u8; 64] = b"ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    let mut out = String::with_capacity((data.len() + 2) / 3 * 4);
    let mut i = 0;
    while i + 3 <= data.len() {
        let n = ((data[i] as u32) << 16) | ((data[i + 1] as u32) << 8) | (data[i + 2] as u32);
        out.push(T[((n >> 18) & 63) as usize] as char);
        out.push(T[((n >> 12) & 63) as usize] as char);
        out.push(T[((n >> 6) & 63) as usize] as char);
        out.push(T[(n & 63) as usize] as char);
        i += 3;
    }
    let rem = data.len() - i;
    if rem == 1 {
        let n = (data[i] as u32) << 16;
        out.push(T[((n >> 18) & 63) as usize] as char);
        out.push(T[((n >> 12) & 63) as usize] as char);
        out.push('=');
        out.push('=');
    } else if rem == 2 {
        let n = ((data[i] as u32) << 16) | ((data[i + 1] as u32) << 8);
        out.push(T[((n >> 18) & 63) as usize] as char);
        out.push(T[((n >> 12) & 63) as usize] as char);
        out.push(T[((n >> 6) & 63) as usize] as char);
        out.push('=');
    }
    out
}

/// RFC 4648 标准 Base64 解码（严格：非法字符 / 长度非法 → None；容忍缺失/正确 padding）
pub(crate) fn base64_decode_bytes(s: &str) -> Option<Vec<u8>> {
    fn val(c: u8) -> Option<u32> {
        match c {
            b'A'..=b'Z' => Some((c - b'A') as u32),
            b'a'..=b'z' => Some((c - b'a' + 26) as u32),
            b'0'..=b'9' => Some((c - b'0' + 52) as u32),
            b'+' => Some(62),
            b'/' => Some(63),
            _ => None,
        }
    }
    let bytes = s.as_bytes();
    if bytes.is_empty() {
        return Some(Vec::new());
    }
    // 去尾部 padding（最多 2 个）
    let mut n = bytes.len();
    let mut pad = 0;
    while n > 0 && bytes[n - 1] == b'=' {
        pad += 1;
        n -= 1;
    }
    if pad > 2 || (n % 4) == 1 {
        return None;
    }
    let mut out = Vec::with_capacity(n / 4 * 3 + 2);
    let mut q = [0u32; 4];
    let mut qi = 0usize;
    for &c in &bytes[..n] {
        q[qi] = val(c)?;
        qi += 1;
        if qi == 4 {
            let v = (q[0] << 18) | (q[1] << 12) | (q[2] << 6) | q[3];
            out.push((v >> 16) as u8);
            out.push((v >> 8) as u8);
            out.push(v as u8);
            qi = 0;
        }
    }
    if qi == 2 {
        let v = (q[0] << 18) | (q[1] << 12);
        out.push((v >> 16) as u8);
    } else if qi == 3 {
        let v = (q[0] << 18) | (q[1] << 12) | (q[2] << 6);
        out.push((v >> 16) as u8);
        out.push((v >> 8) as u8);
    } else if qi != 0 {
        return None;
    }
    Some(out)
}

// ==================== M19 辅助（hex / 字节） ====================

/// 任意值 → 字节（字符串取 UTF-8 字节，其余按 to_string）
fn bytes_of(v: &Value) -> Vec<u8> {
    match v {
        Value::Str(s) => s.clone().into_bytes(),
        Value::Bytes(b) => b.clone(),
        other => other.to_string().into_bytes(),
    }
}

/// bytes_slice 边界归一化（负索引 + len；越界 clamp；start>end 空切片）——与 interp 切片语义一致
fn bytes_slice_bounds(start: Option<i64>, end: Option<i64>, len: i64) -> (usize, usize) {
    let a = match start {
        None => 0,
        Some(i) => {
            if i < 0 {
                (i + len).max(0)
            } else {
                i.min(len)
            }
        }
    };
    let b = match end {
        None => len,
        Some(i) => {
            if i < 0 {
                (i + len).max(0)
            } else {
                i.min(len)
            }
        }
    };
    let (a, b) = if a > b { (a, a) } else { (a, b) };
    (a as usize, b as usize)
}

/// 字节 → 小写 hex
fn hex_encode(data: &[u8]) -> String {
    const HEX: &[u8; 16] = b"0123456789abcdef";
    let mut s = String::with_capacity(data.len() * 2);
    for b in data {
        s.push(HEX[(b >> 4) as usize] as char);
        s.push(HEX[(b & 0x0f) as usize] as char);
    }
    s
}

/// hex → 字节（允许空白，非法返回 Err）
fn hex_decode(s: &str) -> Result<Vec<u8>, String> {
    let clean: String = s.chars().filter(|c| !c.is_whitespace()).collect();
    if clean.len() % 2 != 0 {
        return Err("hex 字符串长度必须为偶数".to_string());
    }
    let mut out = Vec::with_capacity(clean.len() / 2);
    let b = clean.as_bytes();
    for i in (0..b.len()).step_by(2) {
        let hi = (b[i] as char).to_digit(16).ok_or("hex 含非法字符")? as u8;
        let lo = (b[i + 1] as char).to_digit(16).ok_or("hex 含非法字符")? as u8;
        out.push((hi << 4) | lo);
    }
    Ok(out)
}

/// xml_parse 结果 → PuXian Value（dict 树）
fn xml_node_to_value(n: &crate::xml::XmlNode) -> Value {
    let map = std::collections::HashMap::new();
    let d = Value::Dict(std::sync::Arc::new(std::sync::Mutex::new(map)));
    let attrs_map = std::collections::HashMap::new();
    let ad = Value::Dict(std::sync::Arc::new(std::sync::Mutex::new(attrs_map)));
    if let Value::Dict(am) = &ad {
        let mut am = am.lock().unwrap();
        for (k, v) in &n.attrs {
            am.insert(k.clone(), Value::Str(v.clone()));
        }
    }
    let children_list = Value::List(std::sync::Arc::new(std::sync::Mutex::new(Vec::new())));
    let mut text = String::new();
    if let Value::List(cl) = &children_list {
        let mut cl = cl.lock().unwrap();
        for child in &n.children {
            match child {
                crate::xml::XmlChild::Elem(e) => {
                    cl.push(xml_node_to_value(e));
                }
                crate::xml::XmlChild::Text(t) => {
                    cl.push(Value::Str(t.clone()));
                    text.push_str(t);
                }
            }
        }
    }
    if let Value::Dict(dm) = &d {
        let mut dm = dm.lock().unwrap();
        dm.insert("name".to_string(), Value::Str(n.name.clone()));
        dm.insert("attrs".to_string(), ad);
        dm.insert("children".to_string(), children_list);
        dm.insert("text".to_string(), Value::Str(text));
    }
    d
}

// ==================== std.net 辅助（Rust 实现） ====================

static NET_LISTENERS: OnceLock<Mutex<HashMap<i64, TcpListener>>> = OnceLock::new();
static NET_STREAMS: OnceLock<Mutex<HashMap<i64, TcpStream>>> = OnceLock::new();
static NET_NEXT: AtomicI64 = AtomicI64::new(1);

fn net_listeners() -> &'static Mutex<HashMap<i64, TcpListener>> {
    NET_LISTENERS.get_or_init(|| Mutex::new(HashMap::new()))
}
fn net_streams() -> &'static Mutex<HashMap<i64, TcpStream>> {
    NET_STREAMS.get_or_init(|| Mutex::new(HashMap::new()))
}
fn net_alloc_listener(l: TcpListener) -> i64 {
    let id = NET_NEXT.fetch_add(1, Ordering::SeqCst);
    net_listeners().lock().unwrap().insert(id, l);
    id
}
fn net_alloc_stream(s: TcpStream) -> i64 {
    let id = NET_NEXT.fetch_add(1, Ordering::SeqCst);
    net_streams().lock().unwrap().insert(id, s);
    id
}
fn net_close(id: i64) {
    net_streams().lock().unwrap().remove(&id);
    net_listeners().lock().unwrap().remove(&id);
}

// ==================== M16 HTTP 服务端框架（解释器模式） ====================
// http_serve(port, handler)：阻塞 accept 循环，每连接一个处理线程。
// handler(req_dict) -> 响应 dict{status, headers, body} / str / int。

/// 处理单个 HTTP 连接：读请求 -> 解析 -> 调 handler -> 构造响应 -> 发送
fn handle_http_conn(i: &mut Interpreter, stream: &mut TcpStream, handler: &Value, pos: Pos) -> Result<(), String> {
    use std::io::{Read, Write};
    // M23c：HTTP/1.1 keep-alive——同一连接循环处理多个请求，直到客户端关闭 / 请求带
    // Connection: close / handler 返回 keep_alive:false / 空闲超时。
    loop {
        // 1. 读请求头（直到 \r\n\r\n），上限 64KB；keep-alive 空闲读超时 15s
        stream
            .set_read_timeout(Some(std::time::Duration::from_secs(15)))
            .ok();
        let mut buf: Vec<u8> = Vec::new();
        let mut tmp = [0u8; 4096];
        let header_end;
        loop {
            let n = match stream.read(&mut tmp) {
                Ok(n) => n,
                Err(e)
                    if e.kind() == std::io::ErrorKind::WouldBlock
                        || e.kind() == std::io::ErrorKind::TimedOut =>
                {
                    // keep-alive 空闲超时：正常关闭连接
                    return Ok(());
                }
                Err(e) => return Err(format!("读请求失败: {}", e)),
            };
            if n == 0 {
                // 客户端关闭：keep-alive 连接自然结束
                return Ok(());
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
        // 2. 解析请求行 + 头部
        let head = String::from_utf8_lossy(&buf[..header_end]).to_string();
        let remote = stream
            .peer_addr()
            .map(|a| a.to_string())
            .unwrap_or_default();
        let (req, content_length) = parse_http_request(&head, &remote)?;
        // 3. 读 body（按 Content-Length；keep-alive 时 body 后还有下个请求，只读 body_len）
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
                    let n = stream.read(&mut tmp2).map_err(|e| format!("读 body 失败: {}", e))?;
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
        // 4. 填充 body / form（Content-Type 驱动：urlencoded / multipart）
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
        // 5. 调 handler
        let method = req_method(&req);
        let resp = i
            .call_value(handler, &[req], pos)
            .map_err(|e| format!("handler 出错: {}", e))?;
        // 6. 发送响应：file 流式（大文件不占内存）或普通
        if let Some(path) = resp_file_path(&resp) {
            send_file_response(stream, &resp, &path)?;
        } else {
            let mut resp_bytes = build_http_response(&resp);
            if method == "HEAD" {
                if let Some(idx) = find_http_header_end(&resp_bytes) {
                    resp_bytes.truncate(idx + 4);
                }
            }
            stream
                .write_all(&resp_bytes)
                .map_err(|e| format!("发送响应失败: {}", e))?;
            stream.flush().map_err(|e| format!("flush 失败: {}", e))?;
        }
        // 7. keep-alive 判定：客户端 Connection: close / handler keep_alive:false → 关闭
        let client_keep = !request_wants_close(&head);
        let server_keep = response_keep_alive(&resp);
        if !client_keep || !server_keep {
            break;
        }
    }
    Ok(())
}

/// 请求头是否带 Connection: close
fn request_wants_close(head: &str) -> bool {
    for line in head.split("\r\n").skip(1) {
        if let Some(v) = line
            .strip_prefix("Connection:")
            .or_else(|| line.strip_prefix("connection:"))
        {
            return v.trim().eq_ignore_ascii_case("close");
        }
    }
    false
}

/// 响应 dict 是否要求 keep-alive（默认 true；keep_alive:false 强制关闭）
fn response_keep_alive(resp: &Value) -> bool {
    if let Value::Dict(d) = resp {
        let d = d.lock().unwrap();
        return !matches!(d.get("keep_alive"), Some(Value::Bool(false)));
    }
    true
}

/// 响应 dict 的 "file" 路径（流式文件响应）
fn resp_file_path(resp: &Value) -> Option<String> {
    if let Value::Dict(d) = resp {
        let d = d.lock().unwrap();
        if let Some(Value::Str(p)) = d.get("file") {
            return Some(p.clone());
        }
    }
    None
}

/// 流式发送文件响应（Content-Length + 64KB 块，不整读进内存）
fn send_file_response(stream: &mut TcpStream, resp: &Value, path: &str) -> Result<(), String> {
    use std::io::Write;
    let meta = std::fs::metadata(path).map_err(|e| format!("file 响应失败 {}: {}", path, e))?;
    let len = meta.len();
    // 状态码 + Content-Type（简单按扩展名）
    let (status, content_type) = if let Value::Dict(d) = resp {
        let d = d.lock().unwrap();
        let st = match d.get("status") {
            Some(Value::Int(s)) => *s,
            _ => 200,
        };
        let ct = match d.get("content_type") {
            Some(Value::Str(s)) => s.clone(),
            _ => file_content_type(path),
        };
        (st, ct)
    } else {
        (200, file_content_type(path))
    };
    let reason = status_reason(status);
    let mut hdr = format!(
        "HTTP/1.1 {} {}\r\nContent-Length: {}\r\nContent-Type: {}\r\nConnection: close\r\n\r\n",
        status, reason, len, content_type
    );
    stream
        .write_all(hdr.as_bytes())
        .map_err(|e| format!("发送文件响应头失败: {}", e))?;
    let mut f = std::fs::File::open(path).map_err(|e| format!("打开文件失败 {}: {}", path, e))?;
    use std::io::Read;
    let mut chunk = vec![0u8; 65536];
    loop {
        let n = f.read(&mut chunk).map_err(|e| format!("读文件失败: {}", e))?;
        if n == 0 {
            break;
        }
        stream
            .write_all(&chunk[..n])
            .map_err(|e| format!("发送文件块失败: {}", e))?;
    }
    stream.flush().map_err(|e| format!("flush 失败: {}", e))
}

/// 简单 Content-Type 推断（按扩展名）
fn file_content_type(path: &str) -> String {
    let ext = path.rsplit('.').next().unwrap_or("").to_lowercase();
    match ext.as_str() {
        "html" | "htm" => "text/html; charset=utf-8",
        "css" => "text/css; charset=utf-8",
        "js" => "application/javascript; charset=utf-8",
        "json" => "application/json; charset=utf-8",
        "png" => "image/png",
        "jpg" | "jpeg" => "image/jpeg",
        "gif" => "image/gif",
        "svg" => "image/svg+xml",
        "ico" => "image/x-icon",
        "txt" => "text/plain; charset=utf-8",
        "xml" => "application/xml; charset=utf-8",
        "zip" => "application/zip",
        "pdf" => "application/pdf",
        _ => "application/octet-stream",
    }
    .to_string()
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

/// 解析 HTTP 请求头：返回 (请求 dict, Content-Length)
pub(crate) fn parse_http_request(head: &str, remote: &str) -> Result<(Value, usize), String> {
    let mut lines = head.split("\r\n");
    let req_line = lines.next().ok_or("空请求")?;
    let mut parts = req_line.split_whitespace();
    let method = parts.next().ok_or("请求行缺少方法")?.to_string();
    let target = parts.next().ok_or("请求行缺少路径")?.to_string();
    let version = parts.next().unwrap_or("HTTP/1.1").to_string();
    let (path, query) = match target.find('?') {
        Some(idx) => (url_decode(&target[..idx]), url_decode(&target[idx + 1..])),
        None => (url_decode(&target), String::new()),
    };
    let mut headers = HashMap::new();
    let mut content_length = 0usize;
    for line in lines {
        if line.is_empty() {
            continue;
        }
        if let Some(ci) = line.find(':') {
            let k = line[..ci].trim().to_string();
            let v = line[ci + 1..].trim().to_string();
            if k.eq_ignore_ascii_case("content-length") {
                content_length = v.parse().unwrap_or(0);
            }
            headers.insert(k, Value::Str(v));
        }
    }
    let mut d = HashMap::new();
    d.insert("method".into(), Value::Str(method));
    d.insert("target".into(), Value::Str(target));
    d.insert("path".into(), Value::Str(path));
    d.insert("query".into(), Value::Str(query));
    d.insert("version".into(), Value::Str(version));
    d.insert("headers".into(), Value::new_dict(headers));
    d.insert("form".into(), Value::new_dict(HashMap::new()));
    d.insert("remote".into(), Value::Str(remote.to_string()));
    Ok((Value::new_dict(d), content_length))
}

/// 解析 application/x-www-form-urlencoded 表单 -> dict
pub(crate) fn parse_form(body: &str) -> Value {
    let mut m = HashMap::new();
    for pair in body.split('&') {
        if pair.is_empty() {
            continue;
        }
        let (k, v) = match pair.find('=') {
            Some(idx) => (&pair[..idx], &pair[idx + 1..]),
            None => (pair, ""),
        };
        m.insert(url_decode(k), Value::Str(url_decode(v)));
    }
    Value::new_dict(m)
}

/// 从 req dict 的 headers 中取 Content-Type 头
pub(crate) fn content_type_of(g: &HashMap<String, Value>) -> Option<String> {
    if let Some(Value::Dict(h)) = g.get("headers") {
        let h = h.lock().unwrap();
        for (k, v) in h.iter() {
            if k.eq_ignore_ascii_case("content-type") {
                if let Value::Str(s) = v {
                    return Some(s.clone());
                }
            }
        }
    }
    None
}

/// 取请求方法（HEAD/GET/POST...）
fn req_method(req: &Value) -> String {
    if let Value::Dict(d) = req {
        if let Some(Value::Str(s)) = d.lock().unwrap().get("method") {
            return s.clone();
        }
    }
    String::new()
}

/// 从 multipart Content-Type 中提取 boundary
pub(crate) fn multipart_boundary(ct: &str) -> String {
    for part in ct.split(';') {
        let p = part.trim();
        if let Some(rest) = p.strip_prefix("boundary=") {
            return rest.trim().trim_matches('"').to_string();
        }
    }
    "----WebKitFormBoundary".to_string() // 兜底，正常请求必有 boundary
}

/// 解析 multipart/form-data -> (form dict, files dict)
/// files: filename -> 文件内容；form: 普通字段（name -> value）
pub(crate) fn parse_multipart(body: &str, boundary: &str) -> (Value, Value) {
    let mut form: HashMap<String, Value> = HashMap::new();
    let mut files: HashMap<String, Value> = HashMap::new();
    let delim = format!("--{}", boundary);
    for part in body.split(&delim) {
        let part = part.strip_prefix("\r\n").unwrap_or(part);
        let part = part.strip_suffix("\r\n").unwrap_or(part);
        if part.is_empty() || part == "--" {
            continue;
        }
        if let Some(idx) = part.find("\r\n\r\n") {
            let head = &part[..idx];
            let content = &part[idx + 4..];
            let mut name: Option<String> = None;
            let mut filename: Option<String> = None;
            for line in head.split("\r\n") {
                if line.to_lowercase().starts_with("content-disposition:") {
                    name = extract_mime_attr(line, "name=");
                    filename = extract_mime_attr(line, "filename=");
                }
            }
            match (filename, name) {
                (Some(f), _) => {
                    files.insert(f, Value::Str(content.to_string()));
                }
                (None, Some(n)) => {
                    form.insert(n, Value::Str(content.to_string()));
                }
                _ => {}
            }
        }
    }
    (
        Value::new_dict(form),
        Value::new_dict(files),
    )
}

/// 从 Content-Disposition 行提取 name="..." / filename="..." 属性值
fn extract_mime_attr(line: &str, key: &str) -> Option<String> {
    let needle = format!("{key}\"");
    let start = line.find(&needle)? + needle.len();
    let end = line[start..].find('"')? + start;
    Some(line[start..end].to_string())
}

/// URL 解码：+ -> 空格，%XX -> 字节
pub(crate) fn url_decode(s: &str) -> String {
    let bytes = s.as_bytes();
    let mut out = Vec::with_capacity(bytes.len());
    let mut i = 0;
    while i < bytes.len() {
        match bytes[i] {
            b'+' => {
                out.push(b' ');
                i += 1;
            }
            b'%' if i + 2 < bytes.len() => {
                if let (Some(h), Some(l)) = (hex_val(bytes[i + 1]), hex_val(bytes[i + 2])) {
                    out.push(h * 16 + l);
                    i += 3;
                } else {
                    out.push(b'%');
                    i += 1;
                }
            }
            b => {
                out.push(b);
                i += 1;
            }
        }
    }
    String::from_utf8_lossy(&out).to_string()
}

fn hex_val(b: u8) -> Option<u8> {
    match b {
        b'0'..=b'9' => Some(b - b'0'),
        b'a'..=b'f' => Some(b - b'a' + 10),
        b'A'..=b'F' => Some(b - b'A' + 10),
        _ => None,
    }
}

/// 根据 handler 返回值构造 HTTP 响应报文
/// M21：构造 HTTP 响应字节流。
/// dict 响应支持 `gzip: true`（body gzip 压缩 + Content-Encoding: gzip）与
/// `chunked: true`（chunked 传输编码，无 Content-Length）。
fn build_http_response(v: &Value) -> Vec<u8> {
    let (status, headers, body, gzip, chunked, keep_alive) = match v {
        Value::Dict(d) => {
            let d = d.lock().unwrap();
            let status = match d.get("status") {
                Some(Value::Int(s)) => *s,
                Some(Value::Float(f)) => *f as i64,
                _ => 200,
            };
            let body = match d.get("body") {
                Some(Value::Str(s)) => s.clone(),
                Some(Value::Null) | None => String::new(),
                Some(other) => other.to_string(),
            };
            let headers = match d.get("headers") {
                Some(Value::Dict(h)) => {
                    let h = h.lock().unwrap();
                    h.iter()
                        .map(|(k, v)| format!("{}: {}\r\n", k, v))
                        .collect::<String>()
                }
                _ => String::new(),
            };
            let gzip = matches!(d.get("gzip"), Some(Value::Bool(true)));
            let chunked = matches!(d.get("chunked"), Some(Value::Bool(true)));
            let keep_alive = !matches!(d.get("keep_alive"), Some(Value::Bool(false)));
            (status, headers, body, gzip, chunked, keep_alive)
        }
        Value::Str(s) => (200, String::new(), s.clone(), false, false, true),
        Value::Int(st) => (*st, String::new(), String::new(), false, false, true),
        Value::Null => (204, String::new(), String::new(), false, false, true),
        other => (200, String::new(), other.to_string(), false, false, true),
    };
    let reason = status_reason(status);
    // body 预处理：gzip 压缩 / chunked 编码
    let mut body_bytes = body.into_bytes();
    let mut extra_header = String::new();
    if gzip {
        use std::io::Write;
        let mut enc = flate2::write::GzEncoder::new(Vec::new(), flate2::Compression::new(6));
        let _ = enc.write_all(&body_bytes);
        body_bytes = enc.finish().unwrap_or_default();
        extra_header.push_str("Content-Encoding: gzip\r\n");
    }
    if chunked {
        body_bytes = encode_chunked(&body_bytes);
        extra_header.push_str("Transfer-Encoding: chunked\r\n");
    }
    let mut resp = Vec::with_capacity(body_bytes.len() + 512);
    resp.extend_from_slice(format!("HTTP/1.1 {} {}\r\n", status, reason).as_bytes());
    if !chunked {
        resp.extend_from_slice(format!("Content-Length: {}\r\n", body_bytes.len()).as_bytes());
    }
    // M23c：HTTP/1.1 默认 keep-alive；dict "keep_alive": false 强制关闭
    resp.extend_from_slice(if keep_alive { b"Connection: keep-alive\r\n" } else { b"Connection: close\r\n" });
    if !headers.is_empty() {
        resp.extend_from_slice(headers.as_bytes());
    }
    resp.extend_from_slice(extra_header.as_bytes());
    if !headers.to_lowercase().contains("content-type") {
        resp.extend_from_slice(b"Content-Type: text/plain; charset=utf-8\r\n");
    }
    resp.extend_from_slice(b"\r\n");
    resp.extend_from_slice(&body_bytes);
    resp
}

/// M21：chunked 传输编码（固定 4096 块）
fn encode_chunked(data: &[u8]) -> Vec<u8> {
    let mut out = Vec::with_capacity(data.len() + data.len() / 16 + 64);
    if data.is_empty() {
        out.extend_from_slice(b"0\r\n\r\n");
        return out;
    }
    let mut i = 0;
    while i < data.len() {
        let end = (i + 4096).min(data.len());
        out.extend_from_slice(format!("{:x}\r\n", end - i).as_bytes());
        out.extend_from_slice(&data[i..end]);
        out.extend_from_slice(b"\r\n");
        i = end;
    }
    out.extend_from_slice(b"0\r\n\r\n");
    out
}

/// M21：解码 chunked 传输编码（字节级）
fn decode_chunked(data: &[u8]) -> Result<Vec<u8>, String> {
    let mut out = Vec::new();
    let mut i = 0;
    let n = data.len();
    loop {
        let start = i;
        while i < n && data[i] != b'\r' {
            i += 1;
        }
        if i >= n {
            return Err("chunked: 缺少 chunk 大小行".into());
        }
        let size_line = String::from_utf8_lossy(&data[start..i]).to_string();
        i += 2; // 跳过 \r\n
        let size_hex = size_line.split(';').next().unwrap_or("").trim();
        let size = usize::from_str_radix(size_hex, 16)
            .map_err(|_| format!("chunked: 非法块大小 '{}'", size_hex))?;
        if size == 0 {
            break; // 终止块，忽略 trailer
        }
        if i + size > n {
            return Err("chunked: 数据不完整".into());
        }
        out.extend_from_slice(&data[i..i + size]);
        i += size;
        if i + 2 <= n && data[i] == b'\r' && data[i + 1] == b'\n' {
            i += 2;
        } else {
            return Err("chunked: 缺少块尾 CRLF".into());
        }
    }
    Ok(out)
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
        500 => "Internal Server Error",
        501 => "Not Implemented",
        502 => "Bad Gateway",
        503 => "Service Unavailable",
        _ => "OK",
    }
}

/// HTTP 请求（M10：支持 http:// 与 https://，自动跟随重定向最多 5 次）
/// 返回响应体。http/https 统一入口，GET/POST 均可。
// ==================== M23c HTTP keep-alive 连接池（明文 http 同 host 复用） ====================
use std::collections::HashMap as _HashMap;
use std::net::TcpStream as _TcpStream;
use std::sync::{Mutex as _Mutex, OnceLock as _OnceLock};

type HttpPool = std::collections::HashMap<String, Vec<std::net::TcpStream>>;
static HTTP_POOL: _OnceLock<_Mutex<HttpPool>> = _OnceLock::new();
const POOL_PER_HOST: usize = 4;
const POOL_TOTAL: usize = 64;

fn http_pool() -> &'static _Mutex<HttpPool> {
    HTTP_POOL.get_or_init(|| _Mutex::new(std::collections::HashMap::new()))
}

/// 从池取空闲连接（无则 None）
fn pool_take(key: &str) -> Option<std::net::TcpStream> {
    let mut pool = http_pool().lock().unwrap();
    if let Some(v) = pool.get_mut(key) {
        v.pop()
    } else {
        None
    }
}

/// 归还连接（池容量限制；超限直接丢弃）
fn pool_put(key: &str, stream: std::net::TcpStream) {
    let mut pool = http_pool().lock().unwrap();
    let total: usize = pool.values().map(|v| v.len()).sum();
    if total >= POOL_TOTAL {
        return;
    }
    let v = pool.entry(key.to_string()).or_default();
    if v.len() < POOL_PER_HOST {
        v.push(stream);
    }
}

/// 解析 URL → (scheme, host, port, path, host_header)
fn parse_url(url: &str) -> Result<(String, String, u16, String, String), String> {
    let (scheme, rest) = if let Some(r) = url.strip_prefix("https://") {
        ("https", r)
    } else if let Some(r) = url.strip_prefix("http://") {
        ("http", r)
    } else {
        return Err(format!("不支持的协议: {}（支持 http:// 与 https://）", url));
    };
    let (hostport, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
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
    if host.is_empty() {
        return Err("主机名为空".to_string());
    }
    let host_header = if hostport.contains(':') {
        hostport.to_string()
    } else {
        format!("{}:{}", host, port)
    };
    Ok((scheme.to_string(), host, port, path.to_string(), host_header))
}

/// 池化客户端：http_request(url, method, body?, headers?) → dict{status, headers, body}
/// 明文 http 使用 keep-alive 连接池（同 host 复用）；https 每次新建（TLS 会话不复用）。
fn http_request_full(
    url: &str,
    method: &str,
    body: Option<&str>,
    headers: &HashMap<String, String>,
) -> Result<std::collections::HashMap<String, Value>, String> {
    let (scheme, host, port, path, host_header) = parse_url(url)?;
    let key = format!("{}:{}", host, port);
    let mut req = format!(
        "{} {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: PuXian/0.1\r\nConnection: keep-alive\r\n",
        method, path, host_header
    );
    for (k, v) in headers {
        req.push_str(&format!("{}: {}\r\n", k, v));
    }
    if let Some(b) = body {
        if !headers.keys().any(|k| k.eq_ignore_ascii_case("content-type")) {
            req.push_str("Content-Type: application/x-www-form-urlencoded\r\n");
        }
        req.push_str(&format!("Content-Length: {}\r\n", b.len()));
    } else if headers.keys().any(|k| k.eq_ignore_ascii_case("content-length")) {
        // 用户自定义头已含 Content-Length
    }
    req.push_str("\r\n");
    if let Some(b) = body {
        req.push_str(b);
    }
    // 尝试：池连接（失败丢弃重连 1 次）
    let mut attempt = 0;
    loop {
        attempt += 1;
        let pooled = scheme == "http";
        let mut st = if pooled {
            match pool_take(&key) {
                Some(s) => s,
                None => {
                    let addr = format!("{}:{}", host, port);
                    std::net::TcpStream::connect(&addr)
                        .map_err(|e| format!("连接 {} 失败: {}", addr, e))?
                }
            }
        } else {
            let addr = format!("{}:{}", host, port);
            std::net::TcpStream::connect(&addr)
                .map_err(|e| format!("连接 {} 失败: {}", addr, e))?
        };
        match http_exchange(&mut st, &req) {
            Ok((status, headers_map, resp_body, location, keep_alive)) => {
                // keep-alive 且可复用 → 归还连接池
                if keep_alive && pooled {
                    pool_put(&key, st);
                }
                let mut d = std::collections::HashMap::new();
                d.insert("status".into(), Value::Int(status as i64));
                let mut hm = std::collections::HashMap::new();
                for (k, v) in &headers_map {
                    hm.insert(k.clone(), Value::Str(v.clone()));
                }
                d.insert("headers".into(), Value::new_dict(hm));
                d.insert("body".into(), Value::Str(resp_body.clone()));
                if let Some(loc) = location {
                    d.insert("location".into(), Value::Str(loc));
                }
                return Ok(d);
            }
            Err(e) => {
                // 池连接可能已失效：重试一次新连接
                if attempt == 1 && pooled {
                    continue;
                }
                return Err(e);
            }
        }
    }
}

/// 在已建立的连接上完成一次 HTTP 往返（keep-alive 安全：按 Content-Length/chunked 精确读）
/// 返回 (status, headers, body, location, keep_alive)
fn http_exchange(
    stream: &mut std::net::TcpStream,
    req: &str,
) -> Result<(u16, std::collections::HashMap<String, String>, String, Option<String>, bool), String> {
    use std::io::{Read, Write};
    stream
        .write_all(req.as_bytes())
        .map_err(|e| format!("发送请求失败: {}", e))?;
    stream
        .set_read_timeout(Some(std::time::Duration::from_secs(30)))
        .ok();
    // 读响应头
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 4096];
    let header_end;
    loop {
        let n = stream.read(&mut tmp).map_err(|e| format!("读响应失败: {}", e))?;
        if n == 0 {
            return Err("响应连接已关闭".into());
        }
        buf.extend_from_slice(&tmp[..n]);
        if let Some(idx) = find_http_header_end(&buf) {
            header_end = idx;
            break;
        }
        if buf.len() > 65536 {
            return Err("响应头超过 64KB".into());
        }
    }
    let head_str = String::from_utf8_lossy(&buf[..header_end]).to_string();
    let status = head_str
        .lines()
        .next()
        .and_then(|l| l.split_whitespace().nth(1))
        .and_then(|s| s.parse::<u16>().ok())
        .unwrap_or(0);
    // 解析响应头
    let mut headers_map: std::collections::HashMap<String, String> = std::collections::HashMap::new();
    let mut location = None;
    let mut chunked = false;
    let mut gzip = false;
    let mut content_length: Option<usize> = None;
    let mut keep_alive = true;
    for line in head_str.lines().skip(1) {
        let l = line.trim();
        if let Some(ci) = l.find(':') {
            let k = l[..ci].trim().to_string();
            let v = l[ci + 1..].trim().to_string();
            headers_map.insert(k.clone(), v.clone());
            let lk = k.to_lowercase();
            if lk == "content-length" {
                content_length = v.parse().ok();
            } else if lk == "transfer-encoding" && v.to_lowercase().contains("chunked") {
                chunked = true;
            } else if lk == "content-encoding" && v.to_lowercase().contains("gzip") {
                gzip = true;
            } else if lk == "connection" && v.to_lowercase().contains("close") {
                keep_alive = false;
            }
        }
        if let Some(v) = l
            .strip_prefix("Location:")
            .or_else(|| l.strip_prefix("location:"))
        {
            location = Some(v.trim().to_string());
        }
    }
    // 读 body（精确：Content-Length 或 chunked）
    let mut body_bytes: Vec<u8> = buf[header_end + 4..].to_vec();
    if chunked {
        // 继续读剩余 chunked 数据
        loop {
            if let Some(end) = find_chunked_end(&body_bytes) {
                body_bytes.truncate(end);
                break;
            }
            let n = stream.read(&mut tmp).map_err(|e| format!("读 chunked 失败: {}", e))?;
            if n == 0 {
                break;
            }
            body_bytes.extend_from_slice(&tmp[..n]);
        }
        body_bytes = decode_chunked(&body_bytes)?;
    } else if let Some(cl) = content_length {
        while body_bytes.len() < cl {
            let n = stream.read(&mut tmp).map_err(|e| format!("读 body 失败: {}", e))?;
            if n == 0 {
                break;
            }
            body_bytes.extend_from_slice(&tmp[..n]);
        }
        body_bytes.truncate(cl);
    }
    // gzip 解压
    if gzip {
        use std::io::Read as _;
        let mut d = flate2::read::GzDecoder::new(&body_bytes[..]);
        let mut out = Vec::new();
        d.read_to_end(&mut out)
            .map_err(|e| format!("gzip 解压失败: {}", e))?;
        body_bytes = out;
    }
    let body_str = String::from_utf8_lossy(&body_bytes).to_string();
    Ok((status, headers_map, body_str, location, keep_alive))
}

/// 判断 chunked 数据是否已完整（结尾含 "0\r\n\r\n"）
fn find_chunked_end(buf: &[u8]) -> Option<usize> {
    if buf.len() >= 5 {
        for i in 0..=buf.len() - 5 {
            if &buf[i..i + 5] == b"0\r\n\r\n" {
                return Some(i + 5);
            }
        }
    }
    None
}

/// http_get_stream(url, chunk_handler)：流式下载（Content-Length / chunked 分块回调）
fn http_get_stream_impl(
    url: &str,
    interp: &mut Interpreter,
    handler: &Value,
    pos: Pos,
) -> Result<bool, String> {
    use std::io::{Read, Write};
    let (scheme, host, port, path, host_header) = parse_url(url)?;
    let mut req = format!(
        "GET {} HTTP/1.1\r\nHost: {}\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n\r\n",
        path, host_header
    );
    // 连接
    let mut st = if scheme == "https" {
        let raw = https_request(&host, port, &req)?; // https 一次性读（复用现有实现）
        req.clear();
        // 简化：https 流式走现有整包路径
        let head_end = find_http_header_end(&raw).unwrap_or(0);
        let mut body = raw[head_end + 4..].to_vec();
        if is_chunked_header(&raw) {
            body = decode_chunked(&body)?;
        }
        // 分块回调
        return http_stream_chunks(interp, handler, &body, pos);
    } else {
        let addr = format!("{}:{}", host, port);
        let mut s = std::net::TcpStream::connect(&addr)
            .map_err(|e| format!("连接 {} 失败: {}", addr, e))?;
        s.write_all(req.as_bytes())
            .map_err(|e| format!("发送请求失败: {}", e))?;
        s.set_read_timeout(Some(std::time::Duration::from_secs(60))).ok();
        s
    };
    // 读响应头
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 8192];
    let header_end;
    loop {
        let n = st.read(&mut tmp).map_err(|e| format!("读响应失败: {}", e))?;
        if n == 0 {
            return Err("响应连接已关闭".into());
        }
        buf.extend_from_slice(&tmp[..n]);
        if let Some(idx) = find_http_header_end(&buf) {
            header_end = idx;
            break;
        }
        if buf.len() > 65536 {
            return Err("响应头超过 64KB".into());
        }
    }
    let head_str = String::from_utf8_lossy(&buf[..header_end]).to_string();
    let chunked = head_str
        .lines()
        .skip(1)
        .any(|l| l.to_lowercase().contains("transfer-encoding:") && l.to_lowercase().contains("chunked"));
    let gzip = head_str
        .lines()
        .skip(1)
        .any(|l| l.to_lowercase().contains("content-encoding:") && l.to_lowercase().contains("gzip"));
    let content_length: Option<usize> = head_str
        .lines()
        .skip(1)
        .find_map(|l| {
            if let Some(v) = l.trim().strip_prefix("Content-Length:").or_else(|| l.trim().strip_prefix("content-length:")) {
                v.trim().parse().ok()
            } else {
                None
            }
        });
    // 流式读 body
    let mut pending: Vec<u8> = buf[header_end + 4..].to_vec();
    let mut complete = true;
    if chunked {
        // chunked 流式：按 chunk 边界切块
        loop {
            while !pending.is_empty() {
                // 找下一个 chunk 头
                if let Some(ci) = pending.windows(2).position(|w| w == b"\r\n") {
                    let size_str = String::from_utf8_lossy(&pending[..ci]).to_string();
                    let size = usize::from_str_radix(size_str.trim(), 16).unwrap_or(0);
                    let chunk_start = ci + 2;
                    if size == 0 {
                        pending.clear();
                        return Ok(complete);
                    }
                    if pending.len() >= chunk_start + size + 2 {
                        let chunk = pending[chunk_start..chunk_start + size].to_vec();
                        pending.drain(..chunk_start + size + 2);
                        if gzip {
                            // gzip 整体解压：收集后统一解压（流式 gzip 复杂，简化）
                        }
                        if !http_stream_chunks(interp, handler, &chunk, pos)? {
                            complete = false;
                            return Ok(false);
                        }
                    } else {
                        break; // 数据不完整，继续读
                    }
                } else {
                    break;
                }
            }
            let n = st.read(&mut tmp).map_err(|e| format!("读流失败: {}", e))?;
            if n == 0 {
                break;
            }
            pending.extend_from_slice(&tmp[..n]);
        }
    } else {
        // Content-Length / EOF 流式：64KB 块
        while let Some(cl) = content_length {
            if pending.len() >= cl {
                pending.truncate(cl);
                break;
            }
            let n = st.read(&mut tmp).map_err(|e| format!("读流失败: {}", e))?;
            if n == 0 {
                break;
            }
            pending.extend_from_slice(&tmp[..n]);
        }
        // 无 Content-Length：读到 EOF
        if content_length.is_none() {
            loop {
                let n = st.read(&mut tmp).map_err(|e| format!("读流失败: {}", e))?;
                if n == 0 {
                    break;
                }
                pending.extend_from_slice(&tmp[..n]);
            }
        }
        // 分块回调（64KB）
        let mut off = 0;
        while off < pending.len() {
            let end = (off + 65536).min(pending.len());
            let chunk = pending[off..end].to_vec();
            if gzip {
                // gzip 无法分块解压：整包解压后单块回调
                use std::io::Read as _;
                let mut d = flate2::read::GzDecoder::new(&pending[..]);
                let mut out = Vec::new();
                d.read_to_end(&mut out).map_err(|e| format!("gzip 解压失败: {}", e))?;
                if !http_stream_chunks(interp, handler, &out, pos)? {
                    return Ok(false);
                }
                return Ok(true);
            }
            if !http_stream_chunks(interp, handler, &chunk, pos)? {
                return Ok(false);
            }
            off = end;
        }
    }
    Ok(complete)
}

/// 分块回调 helper
fn http_stream_chunks(interp: &mut Interpreter, handler: &Value, data: &[u8], pos: Pos) -> Result<bool, String> {
    let chunk = String::from_utf8_lossy(data).to_string();
    let v = interp
        .call_value(handler, &[Value::Str(chunk)], pos)
        .map_err(|e| format!("chunk_handler 出错: {}", e))?;
    match v {
        Value::Bool(b) => Ok(b),
        Value::Null => Ok(true),
        _ => Ok(true),
    }
}

/// 响应头是否 chunked（https 流式路径用）
fn is_chunked_header(raw: &[u8]) -> bool {
    let head = String::from_utf8_lossy(&raw[..find_http_header_end(raw).unwrap_or(0)]);
    head.lines()
        .skip(1)
        .any(|l| l.to_lowercase().contains("transfer-encoding:") && l.to_lowercase().contains("chunked"))
}

fn http_request(url: &str, method: &str, body: Option<&str>) -> Result<String, String> {
    let mut cur = url.to_string();
    for _ in 0..5 {
        let (status, _head, resp_body, location) = http_once(&cur, method, body)?;
        if (300..400).contains(&status) {
            if let Some(loc) = location {
                cur = resolve_url(&cur, &loc)?;
                continue;
            }
        }
        return Ok(resp_body);
    }
    Err("重定向次数过多（>5）".to_string())
}

/// 解析相对 Location 为完整 URL（基于当前 URL）
fn resolve_url(base: &str, loc: &str) -> Result<String, String> {
    if loc.starts_with("http://") || loc.starts_with("https://") {
        return Ok(loc.to_string());
    }
    // 取 scheme://host[:port]
    let scheme_end = base.find("://").ok_or("非法 base URL")?;
    let scheme = &base[..scheme_end];
    let rest = &base[scheme_end + 3..];
    let hostport = match rest.find('/') {
        Some(i) => &rest[..i],
        None => rest,
    };
    if loc.starts_with('/') {
        Ok(format!("{}://{}{}", scheme, hostport, loc))
    } else {
        // 相对路径：取当前路径目录
        let dir = match rest.find('/') {
            Some(i) => {
                let p = &rest[i..];
                match p.rfind('/') {
                    Some(j) => &p[..j + 1],
                    None => "/",
                }
            }
            None => "/",
        };
        Ok(format!("{}://{}{}{}", scheme, hostport, dir, loc))
    }
}

/// 单次 HTTP 往返（不重定向）：返回 (状态码, 响应头, 响应体, Location)
fn http_once(url: &str, method: &str, body: Option<&str>) -> Result<(u16, String, String, Option<String>), String> {
    let (scheme, rest) = if let Some(r) = url.strip_prefix("https://") {
        ("https", r)
    } else if let Some(r) = url.strip_prefix("http://") {
        ("http", r)
    } else {
        return Err(format!("不支持的协议: {}（支持 http:// 与 https://）", url));
    };
    let (hostport, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
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
    if host.is_empty() {
        return Err("主机名为空".to_string());
    }
    use std::io::{Read, Write};
    let host_header = if hostport.contains(':') {
        hostport.to_string()
    } else {
        format!("{}:{}", host, port)
    };
    let mut req = format!(
        "{} {} HTTP/1.0\r\nHost: {}\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n",
        method, path, host_header
    );
    if let Some(b) = body {
        req.push_str(&format!(
            "Content-Type: application/x-www-form-urlencoded\r\nContent-Length: {}\r\n",
            b.len()
        ));
    }
    req.push_str("\r\n");
    if let Some(b) = body {
        req.push_str(b);
    }
    let resp = if scheme == "https" {
        https_request(&host, port, &req)?
    } else {
        let addr = format!("{}:{}", host, port);
        let mut stream = std::net::TcpStream::connect(&addr)
            .map_err(|e| format!("连接 {} 失败: {}", addr, e))?;
        stream
            .write_all(req.as_bytes())
            .map_err(|e| format!("发送请求失败: {}", e))?;
        let mut resp = Vec::new();
        stream
            .read_to_end(&mut resp)
            .map_err(|e| format!("读取响应失败: {}", e))?;
        resp
    };
    // M21：字节级解析响应头（gzip 二进制 body 不能被 UTF-8 lossy 破坏）
    let (head, mut body_bytes) = match find_http_header_end(&resp) {
        Some(i) => (&resp[..i], resp[i + 4..].to_vec()),
        None => (resp.as_slice(), Vec::new()),
    };
    let head_str = String::from_utf8_lossy(head).to_string();
    // 状态码
    let status = head_str
        .lines()
        .next()
        .and_then(|l| l.split_whitespace().nth(1))
        .and_then(|s| s.parse::<u16>().ok())
        .unwrap_or(0);
    // 头解析：Location / Transfer-Encoding / Content-Encoding
    let mut location = None;
    let mut chunked = false;
    let mut gzip = false;
    for line in head_str.lines().skip(1) {
        let l = line.trim();
        if let Some(v) = l
            .strip_prefix("Location:")
            .or_else(|| l.strip_prefix("location:"))
        {
            location = Some(v.trim().to_string());
        }
        let lc = l.to_lowercase();
        if lc.starts_with("transfer-encoding:") && lc.contains("chunked") {
            chunked = true;
        }
        if lc.starts_with("content-encoding:") && lc.contains("gzip") {
            gzip = true;
        }
    }
    // chunked 解码
    if chunked {
        body_bytes = decode_chunked(&body_bytes)?;
    }
    // gzip 解压
    if gzip {
        use std::io::Read;
        let mut d = flate2::read::GzDecoder::new(&body_bytes[..]);
        let mut out = Vec::new();
        d.read_to_end(&mut out)
            .map_err(|e| format!("gzip 解压失败: {}", e))?;
        body_bytes = out;
    }
    let body_str = String::from_utf8_lossy(&body_bytes).to_string();
    Ok((status, head_str, body_str, location))
}

/// HTTPS 请求（rustls TLS 1.2/1.3，内置 webpki-roots 根证书）
fn https_request(host: &str, port: u16, req: &str) -> Result<Vec<u8>, String> {
    use std::io::{Read, Write};
    let mut roots = rustls::RootCertStore::empty();
    roots.extend(webpki_roots::TLS_SERVER_ROOTS.iter().cloned());
    let cfg = rustls::ClientConfig::builder()
        .with_root_certificates(roots)
        .with_no_client_auth();
    let server_name = rustls::pki_types::ServerName::try_from(host.to_string())
        .map_err(|_| format!("非法主机名: {}", host))?;
    let mut sock = std::net::TcpStream::connect((host, port))
        .map_err(|e| format!("连接 {}:{} 失败: {}", host, port, e))?;
    let mut conn = rustls::ClientConnection::new(std::sync::Arc::new(cfg), server_name)
        .map_err(|e| format!("TLS 初始化失败: {}", e))?;
    let mut tls = rustls::Stream::new(&mut conn, &mut sock);
    tls.write_all(req.as_bytes())
        .map_err(|e| format!("TLS 发送失败: {}", e))?;
    let mut resp = Vec::new();
    tls.read_to_end(&mut resp)
        .map_err(|e| format!("TLS 读取失败: {}", e))?;
    Ok(resp)
}

// ==================== M5 JSON 工具 ====================

/// 极简 JSON 解析器（对象/数组/字符串/数字/布尔/null）
struct JsonParser<'a> {
    bytes: &'a [u8],
    idx: usize,
}

impl<'a> JsonParser<'a> {
    fn skip_ws(&mut self) {
        while self.idx < self.bytes.len() && matches!(self.bytes[self.idx], b' ' | b'\t' | b'\n' | b'\r') {
            self.idx += 1;
        }
    }
    fn peek(&self) -> Option<u8> {
        self.bytes.get(self.idx).copied()
    }
    fn expect(&mut self, b: u8) -> Result<(), String> {
        if self.peek() == Some(b) {
            self.idx += 1;
            Ok(())
        } else {
            Err(format!("期望 '{}'，位置 {}", b as char, self.idx))
        }
    }
    fn parse_value(&mut self) -> Result<Value, String> {
        self.skip_ws();
        match self.peek() {
            Some(b'{') => {
                self.idx += 1;
                let mut map = std::collections::HashMap::new();
                self.skip_ws();
                if self.peek() == Some(b'}') {
                    self.idx += 1;
                    return Ok(Value::new_dict(map));
                }
                loop {
                    self.skip_ws();
                    let k = self.parse_string()?;
                    self.skip_ws();
                    self.expect(b':')?;
                    let v = self.parse_value()?;
                    map.insert(k, v);
                    self.skip_ws();
                    match self.peek() {
                        Some(b',') => {
                            self.idx += 1;
                        }
                        Some(b'}') => {
                            self.idx += 1;
                            break;
                        }
                        _ => return Err(format!("对象解析失败，位置 {}", self.idx)),
                    }
                }
                Ok(Value::new_dict(map))
            }
            Some(b'[') => {
                self.idx += 1;
                let mut arr = Vec::new();
                self.skip_ws();
                if self.peek() == Some(b']') {
                    self.idx += 1;
                    return Ok(Value::new_list(arr));
                }
                loop {
                    let v = self.parse_value()?;
                    arr.push(v);
                    self.skip_ws();
                    match self.peek() {
                        Some(b',') => {
                            self.idx += 1;
                        }
                        Some(b']') => {
                            self.idx += 1;
                            break;
                        }
                        _ => return Err(format!("数组解析失败，位置 {}", self.idx)),
                    }
                }
                Ok(Value::new_list(arr))
            }
            Some(b'"') => Ok(Value::Str(self.parse_string()?)),
            Some(b't') => {
                self.expect_lit("true")?;
                Ok(Value::Bool(true))
            }
            Some(b'f') => {
                self.expect_lit("false")?;
                Ok(Value::Bool(false))
            }
            Some(b'n') => {
                self.expect_lit("null")?;
                Ok(Value::Null)
            }
            Some(c) if c == b'-' || c.is_ascii_digit() => self.parse_number(),
            _ => Err(format!("无法解析 JSON，位置 {}", self.idx)),
        }
    }
    fn expect_lit(&mut self, lit: &str) -> Result<(), String> {
        for ch in lit.bytes() {
            if self.peek() != Some(ch) {
                return Err(format!("期望 '{}'", lit));
            }
            self.idx += 1;
        }
        Ok(())
    }
    fn parse_string(&mut self) -> Result<String, String> {
        self.expect(b'"')?;
        // 按原始字节累积，最后统一 UTF-8 解码（修复多字节中文逐字节转 latin-1 乱码）
        let mut bytes: Vec<u8> = Vec::new();
        while let Some(c) = self.peek() {
            self.idx += 1;
            match c {
                b'"' => {
                    return String::from_utf8(bytes)
                        .map_err(|_| "字符串包含非法 UTF-8 字节".to_string());
                }
                b'\\' => {
                    let esc = self
                        .peek()
                        .ok_or_else(|| "字符串转义不完整".to_string())?;
                    self.idx += 1;
                    match esc {
                        b'"' => bytes.push(b'"'),
                        b'\\' => bytes.push(b'\\'),
                        b'/' => bytes.push(b'/'),
                        b'n' => bytes.push(b'\n'),
                        b't' => bytes.push(b'\t'),
                        b'r' => bytes.push(b'\r'),
                        b'b' => bytes.push(0x08),
                        b'f' => bytes.push(0x0c),
                        b'u' => {
                            if self.idx + 4 > self.bytes.len() {
                                return Err("\\u 转义不完整".to_string());
                            }
                            let hex = std::str::from_utf8(&self.bytes[self.idx..self.idx + 4])
                                .map_err(|_| "\\u 非法".to_string())?;
                            let code = u32::from_str_radix(hex, 16)
                                .map_err(|_| "\\u 非法".to_string())?;
                            self.idx += 4;
                            if let Some(ch) = char::from_u32(code) {
                                let mut tmp = [0u8; 4];
                                bytes.extend_from_slice(ch.encode_utf8(&mut tmp).as_bytes());
                            }
                        }
                        _ => return Err(format!("非法转义 \\{}", esc as char)),
                    }
                }
                _ => {
                    bytes.push(c);
                }
            }
        }
        Err("字符串未闭合".to_string())
    }
    fn parse_number(&mut self) -> Result<Value, String> {
        let start = self.idx;
        if self.peek() == Some(b'-') {
            self.idx += 1;
        }
        while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) {
            self.idx += 1;
        }
        let mut is_float = false;
        if self.peek() == Some(b'.') {
            is_float = true;
            self.idx += 1;
            while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) {
                self.idx += 1;
            }
        }
        if matches!(self.peek(), Some(b'e') | Some(b'E')) {
            is_float = true;
            self.idx += 1;
            if matches!(self.peek(), Some(b'+') | Some(b'-')) {
                self.idx += 1;
            }
            while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) {
                self.idx += 1;
            }
        }
        let txt = std::str::from_utf8(&self.bytes[start..self.idx]).map_err(|_| "数字非法".to_string())?;
        if is_float {
            txt.parse::<f64>()
                .map(Value::Float)
                .map_err(|_| format!("数字非法: {}", txt))
        } else if let Ok(i) = txt.parse::<i64>() {
            Ok(Value::Int(i))
        } else {
            txt.parse::<f64>()
                .map(Value::Float)
                .map_err(|_| format!("数字非法: {}", txt))
        }
    }
}

/// Value → JSON 字符串
pub(crate) fn json_stringify(v: &Value) -> Result<String, String> {
    match v {
        Value::Null => Ok("null".to_string()),
        Value::Bool(b) => Ok(if *b { "true".to_string() } else { "false".to_string() }),
        Value::Int(i) => Ok(i.to_string()),
        Value::Float(f) => {
            if f.fract() == 0.0 && f.is_finite() {
                Ok(format!("{:.1}", f))
            } else {
                Ok(f.to_string())
            }
        }
        Value::Str(s) => Ok(format!("\"{}\"", json_escape(s))),
        Value::List(l) => {
            let items: Vec<String> = l
                .lock()
                .unwrap()
                .iter()
                .map(json_stringify)
                .collect::<Result<_, _>>()?;
            Ok(format!("[{}]", items.join(",")))
        }
        Value::Tuple(t) => {
            let items: Vec<String> = t.iter().map(json_stringify).collect::<Result<_, _>>()?;
            Ok(format!("[{}]", items.join(",")))
        }
        Value::Dict(d) => {
            let b = d.lock().unwrap();
            let mut parts: Vec<String> = b
                .iter()
                .map(|(k, val)| Ok(format!("\"{}\":{}", json_escape(k), json_stringify(val)?)))
                .collect::<Result<_, String>>()?;
            parts.sort();
            Ok(format!("{{{}}}", parts.join(",")))
        }
        _ => Err(format!("无法序列化为 JSON: {}", v)),
    }
}

fn json_escape(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => out.push_str(&format!("\\u{:04x}", c as u32)),
            c => out.push(c),
        }
    }
    out
}

/// Unix 时间戳 → ISO 8601 本地时间字符串（YYYY-MM-DD HH:MM:SS）
fn format_unix(secs: i64) -> String {
    // 简单算法：公历日期转换（proleptic Gregorian）
    let days = secs.div_euclid(86400);
    let rem = secs.rem_euclid(86400);
    let hh = rem / 3600;
    let mm = (rem % 3600) / 60;
    let ss = rem % 60;
    // 1970-01-01 起算
    let (y, m, d) = civil_from_days(days);
    format!("{:04}-{:02}-{:02} {:02}:{:02}:{:02}", y, m, d, hh, mm, ss)
}

/// Howard Hinnant 算法：天数 → 公历 (y, m, d)
fn civil_from_days(z: i64) -> (i64, u32, u32) {
    let z = z + 719468;
    let era = z.div_euclid(146097);
    let doe = z.rem_euclid(146097);
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365;
    let y = yoe + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100);
    let mp = (5 * doy + 2) / 153;
    let d = (doy - (153 * mp + 2) / 5 + 1) as u32;
    let m = (if mp < 10 { mp + 3 } else { mp - 9 }) as u32;
    let y = if m <= 2 { y + 1 } else { y };
    (y, m, d)
}

// ==================== 参数辅助 ====================

fn expect_int(v: &Value, fname: &str, pos: Pos) -> Result<i64, LxError> {
    match v {
        Value::Int(i) => Ok(*i),
        _ => Err(err(format!("{} 参数需要整数", fname), pos)),
    }
}

fn expect_str<'a>(v: &'a Value, fname: &str, pos: Pos) -> Result<&'a str, LxError> {
    match v {
        Value::Str(s) => Ok(s.as_str()),
        _ => Err(err(format!("{} 参数需要字符串", fname), pos)),
    }
}

fn to_f64(v: &Value, pos: Pos) -> Result<f64, LxError> {
    match v {
        Value::Int(i) => Ok(*i as f64),
        Value::Float(f) => Ok(*f),
        _ => Err(err("需要数值", pos)),
    }
}

fn one_str<F>(args: &[Value], pos: Pos, f: F) -> Result<Value, LxError>
where
    F: FnOnce(&str) -> Value,
{
    if args.len() != 1 {
        return Err(err("需要一个字符串参数", pos));
    }
    let s = expect_str(&args[0], "字符串函数", pos)?;
    Ok(f(s))
}

fn two_str<F>(args: &[Value], pos: Pos, f: F) -> Result<Value, LxError>
where
    F: FnOnce(&str, &str) -> Value,
{
    if args.len() != 2 {
        return Err(err("需要两个字符串参数", pos));
    }
    let a = expect_str(&args[0], "字符串函数", pos)?;
    let b = expect_str(&args[1], "字符串函数", pos)?;
    Ok(f(a, b))
}

fn one_num<FI, FF>(args: &[Value], pos: Pos, fi: FI, ff: FF) -> Result<Value, LxError>
where
    FI: FnOnce(i64) -> Value,
    FF: FnOnce(f64) -> Value,
{
    if args.len() != 1 {
        return Err(err("需要一个数值参数", pos));
    }
    match &args[0] {
        Value::Int(i) => Ok(fi(*i)),
        Value::Float(f) => Ok(ff(*f)),
        _ => Err(err("需要数值", pos)),
    }
}

/// 提取 list/tuple 为 Vec<Value>（map/filter/reduce 用）
fn list_of(v: &Value, fname: &str, pos: Pos) -> Result<Vec<Value>, LxError> {
    match v {
        Value::List(l) => Ok(l.lock().unwrap().clone()),
        Value::Tuple(t) => Ok(t.clone()),
        _ => Err(err(format!("{} 第一个参数需要 list/tuple", fname), pos)),
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_resolve_url_absolute() {
        assert_eq!(
            resolve_url("https://a.com/x", "https://b.com/y").unwrap(),
            "https://b.com/y"
        );
        assert_eq!(
            resolve_url("http://a.com/x", "http://b.com/y").unwrap(),
            "http://b.com/y"
        );
    }

    #[test]
    fn test_resolve_url_root_relative() {
        assert_eq!(
            resolve_url("https://a.com/path/page", "/new").unwrap(),
            "https://a.com/new"
        );
        assert_eq!(
            resolve_url("http://a.com:8080/p", "/q").unwrap(),
            "http://a.com:8080/q"
        );
    }

    #[test]
    fn test_resolve_url_relative_dir() {
        assert_eq!(
            resolve_url("https://a.com/dir/page", "other").unwrap(),
            "https://a.com/dir/other"
        );
        assert_eq!(
            resolve_url("https://a.com/page", "sub").unwrap(),
            "https://a.com/sub"
        );
    }

    #[test]
    fn test_http_once_rejects_bad_scheme() {
        let e = http_once("ftp://x.com/", "GET", None).unwrap_err();
        assert!(e.contains("不支持的协议"));
    }

    #[test]
    fn test_http_parse_request() {
        let head = "GET /api/user?page=1&size=10 HTTP/1.1\r\nHost: localhost\r\nContent-Length: 0\r\n";
        let (req, cl) = parse_http_request(head, "127.0.0.1:1").unwrap();
        assert_eq!(cl, 0);
        let d = match &req {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("req 不是 dict"),
        };
        assert_eq!(d["method"], Value::Str("GET".into()));
        assert_eq!(d["target"], Value::Str("/api/user?page=1&size=10".into()));
        assert_eq!(d["path"], Value::Str("/api/user".into()));
        assert_eq!(d["query"], Value::Str("page=1&size=10".into()));
        assert_eq!(d["version"], Value::Str("HTTP/1.1".into()));
        assert_eq!(d["remote"], Value::Str("127.0.0.1:1".into()));
        let hs = match &d["headers"] {
            Value::Dict(h) => h.lock().unwrap().clone(),
            _ => panic!("headers 不是 dict"),
        };
        assert_eq!(hs["Host"], Value::Str("localhost".into()));
    }

    #[test]
    fn test_http_parse_request_content_length() {
        let head = "POST /submit HTTP/1.1\r\nContent-Length: 11\r\n";
        let (_, cl) = parse_http_request(head, "").unwrap();
        assert_eq!(cl, 11);
        // 无 Content-Length
        let (_, cl) = parse_http_request("GET / HTTP/1.1\r\n", "").unwrap();
        assert_eq!(cl, 0);
    }

    #[test]
    fn test_http_form_parse() {
        let form = parse_form("name=abc&msg=hello+world&tag=%E4%B8%AD");
        let d = match &form {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("form 不是 dict"),
        };
        assert_eq!(d["name"], Value::Str("abc".into()));
        assert_eq!(d["msg"], Value::Str("hello world".into()));
        assert_eq!(d["tag"], Value::Str("中".into()));
        assert_eq!(url_decode("a+b%20c"), "a b c");
    }

    #[test]
    fn test_http_build_response() {
        // dict 响应
        let mut h = HashMap::new();
        h.insert("Content-Type".into(), Value::Str("application/json".into()));
        let mut d = HashMap::new();
        d.insert("status".into(), Value::Int(201));
        d.insert("body".into(), Value::Str("{\"ok\":1}".into()));
        d.insert("headers".into(), Value::new_dict(h));
        let r = build_http_response(&Value::new_dict(d));
        let rs = String::from_utf8_lossy(&r).to_string();
        assert!(rs.starts_with("HTTP/1.1 201 Created\r\n"));
        assert!(rs.contains("Content-Length: 8\r\n"));
        assert!(rs.contains("Content-Type: application/json\r\n"));
        assert!(rs.ends_with("\r\n\r\n{\"ok\":1}"));

        // str 响应
        let r = build_http_response(&Value::Str("hi".into()));
        let rs = String::from_utf8_lossy(&r).to_string();
        assert!(rs.starts_with("HTTP/1.1 200 OK\r\n"));
        assert!(rs.contains("Content-Type: text/plain; charset=utf-8\r\n"));
        assert!(rs.ends_with("\r\n\r\nhi"));

        // int 响应
        let r = build_http_response(&Value::Int(404));
        let rs = String::from_utf8_lossy(&r).to_string();
        assert!(rs.starts_with("HTTP/1.1 404 Not Found\r\n"));

        // M21：gzip + chunked
        let mut d = HashMap::new();
        d.insert("body".into(), Value::Str("hello hello hello".into()));
        d.insert("gzip".into(), Value::Bool(true));
        d.insert("chunked".into(), Value::Bool(true));
        let r = build_http_response(&Value::new_dict(d));
        let rs = String::from_utf8_lossy(&r).to_string();
        assert!(rs.contains("Content-Encoding: gzip\r\n"), "{:?}", rs);
        assert!(rs.contains("Transfer-Encoding: chunked\r\n"), "{:?}", rs);
        assert!(!rs.contains("Content-Length:"), "{:?}", rs);
        assert!(rs.ends_with("0\r\n\r\n"), "{:?}", rs);
    }

    #[test]
    fn test_http_chunked_roundtrip() {
        // chunked 编码 → 解码 round-trip
        let data = b"hello world, chunked encoding test data here!";
        let enc = encode_chunked(data);
        let dec = decode_chunked(&enc).unwrap();
        assert_eq!(dec, data);
        // 空数据
        assert_eq!(decode_chunked(b"0\r\n\r\n").unwrap(), b"");
        // 非法
        assert!(decode_chunked(b"zz\r\nabc").is_err());
    }

    #[test]
    fn test_http_find_header_end() {
        assert_eq!(find_http_header_end(b"GET / HTTP/1.1\r\n\r\n"), Some(14));
        assert_eq!(find_http_header_end(b"GET / HTTP/1.1\r\nHost: a\r\n\r\nbody"), Some(23));
        assert_eq!(find_http_header_end(b"GET /"), None);
    }

    #[test]
    fn test_http_multipart_parse() {
        let body = "--TB\r\nContent-Disposition: form-data; name=\"title\"\r\n\r\nhello\r\n--TB\r\nContent-Disposition: form-data; name=\"file1\"; filename=\"a.txt\"\r\nContent-Type: text/plain\r\n\r\nfile content 123\r\n--TB--\r\n";
        let (form, files) = parse_multipart(body, "TB");
        let fd = match &form {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("form 不是 dict"),
        };
        let fsd = match &files {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("files 不是 dict"),
        };
        assert_eq!(fd["title"], Value::Str("hello".into()));
        assert_eq!(fsd["a.txt"], Value::Str("file content 123".into()));
        assert_eq!(multipart_boundary("multipart/form-data; boundary=TB"), "TB");
        assert_eq!(multipart_boundary("multipart/form-data; boundary=\"quoted\""), "quoted");
    }

    #[test]
    fn test_http_path_query_decode() {
        // path/query 百分号解码 + '+' → 空格
        let head = "GET /%E4%B8%AD%E6%96%87?kw=hello+world&n=%E4%B8%AD HTTP/1.1\r\n";
        let (req, _) = parse_http_request(head, "").unwrap();
        let d = match &req {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("req 不是 dict"),
        };
        assert_eq!(d["path"], Value::Str("/中文".into()));
        assert_eq!(d["query"], Value::Str("kw=hello world&n=中".into()));
        // 无 query 时
        let (req2, _) = parse_http_request("GET /plain HTTP/1.1\r\n", "").unwrap();
        let d2 = match &req2 {
            Value::Dict(d) => d.lock().unwrap().clone(),
            _ => panic!("req2 不是 dict"),
        };
        assert_eq!(d2["path"], Value::Str("/plain".into()));
        assert_eq!(d2["query"], Value::Str("".into()));
    }
}
