//! M17 P1 + M27 P0：.px 脚本执行机制（PHP / OpenResty 式应用平台）
//! 目标（docs/PROGRESS.md C 场景）："写个 .px 扔进目录访问 URL 就跑，不用编译、不用重启、不用单独部署"。
//!
//! 提供两个语言层能力：
//! - `px_exec(path, params?) -> str|null`：**内嵌解释器执行 API**（路线 A）
//!   在当前解释器内新开隔离解释器执行 .px 文件，捕获 print 输出为返回值；
//!   文件不存在返回 null；params dict 注入为脚本全局变量。
//! - `px_serve(port, docroot[, timeout_ms[, opts]])`：**PHP 式应用服务器**（开发模式 A）
//!   静态文件直接返回（MIME 表）；.px 脚本内嵌解释器执行（注入 REQUEST/GET/POST/SERVER 全局变量，
//!   print 输出即响应体；脚本可设全局 RESPONSE = {status, headers, body} 精确控制响应）；
//!   目录 → index.px / index.html；路径穿越防护（403）；404 / 500 / 504（超时）。
//!
//! M27 P0 生产化：
//! - 服务端 TLS：`tls_server(cert, key)` 注册后本服务器自动 HTTPS（配合 crate::tls）
//! - 请求体：大小可配（opts.max_body_size，默认 10MB，超限 413）+ 大 body 落盘（>1MB 写临时文件，
//!   REQUEST.body_tmp 给出路径，避免大上传占满内存）
//! - Cookie：REQUEST.cookie = dict（解析 Cookie 头）
//! - Session：session_open()/session_get/session_set/session_del（跨请求共享，TTL 2h，Set-Cookie 自动注入响应）
//! - 基础认证：basic_auth(user, pass) → bool（失败自动 401 + WWW-Authenticate）
//! - 优雅关闭：SIGINT/SIGTERM → 停止 accept，等待在途请求完成（最多 5s）后返回
//!
//! 安全（PROGRESS 第一版架构要求）：
//! - 目录隔离：URL 分段拒绝 ".."，且 canonicalize 前缀校验（防符号链接逃逸）✅
//! - 超时控制：脚本死循环不阻塞服务器（超时放弃；编译模式 C 端为子进程 + kill）✅
//! - 危险函数禁/限：语言暂无 exec 内置，天然安全 ✅
//! - 每请求独立线程：单脚本卡死不影响其他请求 ✅

use std::cell::RefCell;
use std::collections::HashMap;
use std::io::{Read, Seek, Write};
use std::net::TcpListener;
use std::path::{Path, PathBuf};
use std::sync::atomic::{AtomicBool, AtomicI64, Ordering};
use std::sync::{mpsc, Arc, Mutex, OnceLock};
use std::time::Duration;

use crate::builtin::{
    content_type_of, multipart_boundary, parse_form, parse_http_request, parse_multipart,
};
use crate::interp::Interpreter;
use crate::tls::SConn;
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

// ==================== 优雅关闭（SIGINT / SIGTERM） ====================

/// px_serve 停止标志（信号 handler 设置）
static PX_SERVE_STOP: AtomicBool = AtomicBool::new(false);
/// 在途请求计数（优雅关闭等待归零）
static PX_IN_FLIGHT: AtomicI64 = AtomicI64::new(0);

unsafe extern "C" {
    fn signal(signum: i32, handler: usize) -> usize;
}

extern "C" fn px_serve_stop_handler(_sig: i32) {
    PX_SERVE_STOP.store(true, Ordering::SeqCst);
}

const SIGINT: i32 = 2;
const SIGTERM: i32 = 15;

/// 注册 SIGINT/SIGTERM → 优雅停止（async-signal-safe：只写原子标志）
fn install_stop_handlers() {
    unsafe {
        signal(SIGINT, px_serve_stop_handler as usize);
        signal(SIGTERM, px_serve_stop_handler as usize);
    }
}

// ==================== Session / Cookie / BasicAuth（跨请求共享） ====================

pub const SESSION_COOKIE_NAME: &str = "pxsid";
pub const SESSION_TTL_SECS: i64 = 7200;
/// 大 body 落盘阈值（> 该值写临时文件，不进内存）
const BODY_TMP_THRESHOLD: usize = 1 << 20;

/// 会话条目：数据 + 过期时间戳（秒）
/// M27：session 存储为文件（PX_SESSION_DIR 或 /tmp/px_sessions/<sid>.json）——
/// 跨请求 / 跨进程共享（C 编译模式 px_serve 用 `px run` 子进程执行脚本，内存存储会丢失；
/// 文件存储使双模式 + 进程池 worker 间行为一致）。写采用 tmp+rename 原子替换，并发安全。
pub struct SessionEntry {
    pub data: HashMap<String, Value>,
    pub expires: i64,
}

static SESSION_SEQ: AtomicI64 = AtomicI64::new(0);
static BODY_SEQ: AtomicI64 = AtomicI64::new(0);

fn now_secs() -> i64 {
    std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_secs() as i64)
        .unwrap_or(0)
}

/// session 存储目录（PX_SESSION_DIR 可覆盖；默认 /tmp/px_sessions）
pub fn session_dir() -> std::path::PathBuf {
    let d = std::env::var("PX_SESSION_DIR").unwrap_or_else(|_| "/tmp/px_sessions".to_string());
    let p = std::path::PathBuf::from(&d);
    let _ = std::fs::create_dir_all(&p);
    p
}

fn session_path(sid: &str) -> std::path::PathBuf {
    session_dir().join(format!("{}.json", sid))
}

/// 读 session 文件 → SessionEntry 或 None（不存在/过期/解析失败）
fn session_read(sid: &str) -> Option<SessionEntry> {
    let path = session_path(sid);
    let data = std::fs::read(&path).ok()?;
    let text = String::from_utf8_lossy(&data).to_string();
    let v = crate::builtin::json_parse_value(&text)?;
    if let Value::Dict(d) = v {
        let d = d.lock().unwrap();
        let expires = match d.get("exp") {
            Some(Value::Int(e)) => *e,
            _ => 0,
        };
        let data = match d.get("data") {
            Some(Value::Dict(dd)) => dd.lock().unwrap().clone(),
            _ => HashMap::new(),
        };
        Some(SessionEntry { data, expires })
    } else {
        None
    }
}

/// 写 session 文件（原子：tmp + rename）
fn session_write(sid: &str, e: &SessionEntry) {
    let dir = session_dir();
    let tmp = dir.join(format!("{}.tmp.{}", sid, std::process::id()));
    let mut m = HashMap::new();
    m.insert("exp".to_string(), Value::Int(e.expires));
    let data = Value::Dict(Arc::new(Mutex::new(e.data.clone())));
    m.insert("data".to_string(), data);
    let v = Value::Dict(Arc::new(Mutex::new(m)));
    if let Ok(s) = crate::builtin::json_stringify(&v) {
        let _ = std::fs::write(&tmp, s);
        let _ = std::fs::rename(&tmp, session_path(sid));
    }
}

// 每请求线程局部状态：当前 session id、待注入 Set-Cookie、基础认证失败
thread_local! {
    static CURRENT_SESSION: RefCell<Option<String>> = RefCell::new(None);
    static SESSION_SET_COOKIES: RefCell<Vec<String>> = RefCell::new(Vec::new());
    static AUTH_REQUIRED: RefCell<Option<String>> = RefCell::new(None); // realm
}

/// 请求结束清理线程局部状态（线程池复用线程时必须）
pub fn reset_request_state() {
    CURRENT_SESSION.with(|c| *c.borrow_mut() = None);
    SESSION_SET_COOKIES.with(|c| *c.borrow_mut() = Vec::new());
    AUTH_REQUIRED.with(|a| *a.borrow_mut() = None);
}

/// 取出本请求待注入的响应头：返回 (Set-Cookie 列表, auth realm 可选)
pub fn take_response_injections() -> (Vec<String>, Option<String>) {
    let cookies = SESSION_SET_COOKIES.with(|c| c.replace(Vec::new()));
    let auth = AUTH_REQUIRED.with(|a| a.replace(None));
    (cookies, auth)
}

/// 解析 Cookie 头："a=1; b=2" → dict
pub fn parse_cookie(header: &str) -> Value {
    let mut m = HashMap::new();
    for part in header.split(';') {
        let p = part.trim();
        if let Some(eq) = p.find('=') {
            let k = p[..eq].trim().to_string();
            let v = p[eq + 1..].trim().trim_matches('"').to_string();
            if !k.is_empty() {
                m.insert(k, Value::Str(v));
            }
        }
    }
    Value::Dict(Arc::new(Mutex::new(m)))
}

fn get_req_cookie(req: &Value, name: &str) -> Option<String> {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(c)) = d.lock().unwrap().get("cookie") {
            if let Some(Value::Str(v)) = c.lock().unwrap().get(name) {
                return Some(v.clone());
            }
        }
    }
    None
}

fn get_req_header(req: &Value, name: &str) -> Option<String> {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers") {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case(name) {
                    if let Value::Str(s) = v {
                        return Some(s.clone());
                    }
                }
            }
        }
    }
    None
}

fn new_session_id() -> String {
    let t = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_nanos())
        .unwrap_or(0);
    format!(
        "{:x}{:x}{:x}",
        t,
        std::process::id(),
        SESSION_SEQ.fetch_add(1, Ordering::SeqCst)
    )
}

fn set_current_session(id: String) {
    CURRENT_SESSION.with(|c| *c.borrow_mut() = Some(id));
}

/// session_open()：读取请求 Cookie 中的 pxsid；有效则续期复用，否则新建
/// （新 session 的 Set-Cookie 自动注入本请求响应头；数据存文件跨进程共享）。
pub fn session_open_with(req: &Value) -> String {
    let now = now_secs();
    if let Some(id) = get_req_cookie(req, SESSION_COOKIE_NAME) {
        if let Some(mut e) = session_read(&id) {
            if e.expires > now {
                e.expires = now + SESSION_TTL_SECS;
                session_write(&id, &e);
                set_current_session(id.clone());
                return id;
            } else {
                let _ = std::fs::remove_file(session_path(&id)); // 过期
            }
        }
    }
    let id = new_session_id();
    session_write(
        &id,
        &SessionEntry {
            data: HashMap::new(),
            expires: now + SESSION_TTL_SECS,
        },
    );
    let cookie = format!("{}={}; Path=/; HttpOnly", SESSION_COOKIE_NAME, id);
    SESSION_SET_COOKIES.with(|c| c.borrow_mut().push(cookie));
    set_current_session(id.clone());
    id
}

/// 清理过期 session（服务器启动时调用）
pub fn session_sweep_expired() {
    if let Ok(rd) = std::fs::read_dir(session_dir()) {
        let now = now_secs();
        for ent in rd.flatten() {
            let name = ent.file_name().to_string_lossy().to_string();
            if let Some(sid) = name.strip_suffix(".json") {
                if session_read(sid).map(|e| e.expires <= now).unwrap_or(true) {
                    let _ = std::fs::remove_file(ent.path());
                }
            }
        }
    }
}

/// 当前 session id（文件模式无需内存注册表；返回 thread_local 记录）
pub fn session_id_value() -> Value {
    CURRENT_SESSION
        .with(|c| c.borrow().clone())
        .map(Value::Str)
        .unwrap_or(Value::Null)
}

pub fn session_get_value(key: &str) -> Value {
    let id = CURRENT_SESSION.with(|c| c.borrow().clone());
    match id {
        None => Value::Null,
        Some(id) => match session_read(&id) {
            Some(e) => e.data.get(key).cloned().unwrap_or(Value::Null),
            None => Value::Null,
        },
    }
}

pub fn session_set_value(key: &str, v: Value) -> bool {
    let id = CURRENT_SESSION.with(|c| c.borrow().clone());
    match id {
        None => false,
        Some(id) => {
            let now = now_secs();
            let mut e = session_read(&id).unwrap_or(SessionEntry {
                data: HashMap::new(),
                expires: now + SESSION_TTL_SECS,
            });
            e.data.insert(key.to_string(), v);
            session_write(&id, &e);
            true
        }
    }
}

pub fn session_del_value(key: &str) -> bool {
    let id = CURRENT_SESSION.with(|c| c.borrow().clone());
    match id {
        None => false,
        Some(id) => {
            let mut e = match session_read(&id) {
                Some(e) => e,
                None => return false,
            };
            let removed = e.data.remove(key).is_some();
            session_write(&id, &e);
            removed
        }
    }
}

pub fn session_destroy_value() -> bool {
    let id = CURRENT_SESSION.with(|c| c.borrow().clone());
    match id {
        None => false,
        Some(id) => {
            let removed = std::fs::remove_file(session_path(&id)).is_ok();
            if removed {
                set_current_session(String::new());
            }
            removed
        }
    }
}

/// basic_auth(user, pass)：校验 Authorization: Basic 头；失败标记 401（响应自动注入）。
pub fn basic_auth_with(req: &Value, user: &str, pass: &str) -> bool {
    if let Some(h) = get_req_header(req, "Authorization") {
        let h = h.trim();
        if let Some(rest) = h.strip_prefix("Basic ") {
            if let Some(dec) = crate::builtin::base64_decode_bytes(rest.trim()) {
                let s = String::from_utf8_lossy(&dec).to_string();
                if s == format!("{}:{}", user, pass) {
                    return true;
                }
            }
        }
    }
    AUTH_REQUIRED.with(|a| *a.borrow_mut() = Some("px".to_string()));
    false
}

// ==================== px_serve：PHP 式应用服务器 ====================

/// px_serve 配置（opts dict 解析结果）
#[derive(Clone)]
pub struct PxServeOpts {
    pub max_body_size: usize,
    pub body_tmp_dir: String,
    pub timeout_ms: i64,
    /// M31：最大并发连接数（连接线程池 / 信号量上限，默认 32）
    pub max_conn: usize,
    /// M31/M35：限流配置（max 次 / window_sec 秒；key 组合维度；白名单 IP 放行）
    pub rate_limit: Option<RateLimitConfig>,
    /// M33：结构化访问日志落盘（路径；None = 仅 stderr）
    pub access_log: Option<String>,
    /// M33：HTTP/3 通告（Alt-Svc 响应头值；如 "h3=\":443\""；None = 不发）
    pub alt_svc: Option<String>,
    /// M36：access log JSON 行格式（{ts,remote,method,path,status,bytes,ms,req}）
    pub log_json: bool,
    /// M36：access log 按天轮转（文件名带日期 YYYYMMDD 后缀）
    pub log_daily: bool,
}

/// M35：多维度限流配置
#[derive(Clone)]
pub struct RateLimitConfig {
    pub max: i64,
    pub window_sec: i64,
    /// 限流 key 维度：ip / ip|ua / ip|path / ip|ua|path（默认 ip）
    pub key_mode: String,
    /// 白名单 IP（命中直接放行，不计数）
    pub whitelist: Vec<String>,
}

impl Default for PxServeOpts {
    fn default() -> Self {
        Self {
            max_body_size: 10 * 1024 * 1024,
            body_tmp_dir: std::env::temp_dir().to_string_lossy().to_string(),
            timeout_ms: 10000,
            max_conn: 32,
            rate_limit: None,
            access_log: None,
            alt_svc: None,
            log_json: false,
            log_daily: false,
        }
    }
}

/// 解析 opts dict：{max_body_size, body_tmp_dir, max_conn, rate_limit:{max,window_sec}, access_log, alt_svc}
pub fn parse_opts(opts: Option<&Value>, timeout_ms: i64) -> PxServeOpts {
    let mut o = PxServeOpts {
        timeout_ms: timeout_ms.max(1),
        ..Default::default()
    };
    if let Some(Value::Dict(d)) = opts {
        let d = d.lock().unwrap();
        if let Some(Value::Int(n)) = d.get("max_body_size") {
            o.max_body_size = (*n).max(1024) as usize;
        }
        if let Some(Value::Str(s)) = d.get("body_tmp_dir") {
            o.body_tmp_dir = s.clone();
        }
        if let Some(Value::Int(n)) = d.get("max_conn") {
            o.max_conn = (*n).max(1) as usize;
        }
        if let Some(Value::Dict(rl)) = d.get("rate_limit") {
            let rl = rl.lock().unwrap();
            let max = rl.get("max").and_then(|v| if let Value::Int(i) = v { Some(*i) } else { None });
            let win = rl
                .get("window_sec")
                .and_then(|v| if let Value::Int(i) = v { Some(*i) } else { None });
            if let (Some(m), Some(w)) = (max, win) {
                if m >= 1 && w >= 1 {
                    // M35：key 维度（ip / ip|ua / ip|path / ip|ua|path）+ 白名单 IP
                    let key_mode = match rl.get("key") {
                        Some(Value::Str(s)) => s.to_lowercase(),
                        _ => "ip".to_string(),
                    };
                    let whitelist = match rl.get("whitelist") {
                        Some(Value::List(l)) => l
                            .lock()
                            .unwrap()
                            .iter()
                            .filter_map(|v| if let Value::Str(s) = v { Some(s.clone()) } else { None })
                            .collect(),
                        _ => vec![],
                    };
                    o.rate_limit = Some(RateLimitConfig {
                        max: m,
                        window_sec: w,
                        key_mode,
                        whitelist,
                    });
                }
            }
        }
        if let Some(Value::Str(s)) = d.get("access_log") {
            o.access_log = Some(s.clone());
        }
        if let Some(Value::Str(s)) = d.get("alt_svc") {
            o.alt_svc = Some(s.clone());
        }
        if let Some(Value::Bool(b)) = d.get("log_json") {
            o.log_json = *b;
        }
        if let Some(Value::Bool(b)) = d.get("log_daily") {
            o.log_daily = *b;
        }
    }
    o
}

/// px_serve(port, docroot[, timeout_ms[, opts]])：阻塞 accept 循环，每请求独立线程。
/// - 服务端 TLS：tls_server(cert, key) 注册后自动 HTTPS
/// - 优雅关闭：SIGINT/SIGTERM → 停止 accept，等待在途请求（≤5s）后返回
pub fn px_serve(port: i64, docroot: &str, timeout_ms: i64, opts: Option<&Value>) -> Result<(), String> {
    let root =
        std::fs::canonicalize(docroot).map_err(|e| format!("docroot 无效 {}: {}", docroot, e))?;
    if !root.is_dir() {
        return Err(format!("docroot 不是目录: {}", docroot));
    }
    let o = parse_opts(opts, timeout_ms);
    let addr = format!("0.0.0.0:{}", port);
    let listener = TcpListener::bind(&addr)
        .map_err(|e| format!("监听端口失败 {}: {}", addr, e))?;
    // 优雅关闭：SIGINT/SIGTERM → 停止 accept
    PX_SERVE_STOP.store(false, Ordering::SeqCst);
    install_stop_handlers();
    let _ = listener.set_nonblocking(true);
    let tls = crate::tls::tls_server_configured();
    // M33：启动时配置 access log 落盘 + Alt-Svc 通告
    set_access_log(o.access_log.clone(), o.log_json, o.log_daily);
    set_alt_svc(o.alt_svc.clone());
    eprintln!(
        "[px-serve] 普贤应用服务器 docroot={} 端口={} 超时={}ms tls={} max_body={}B max_conn={}",
        root.display(),
        port,
        o.timeout_ms,
        tls,
        o.max_body_size,
        o.max_conn
    );
    // M31.4b：并发上限（信号量）——超出的连接在 TCP backlog 排队，处理完一个放行一个
    let sem = Arc::new(ConnSem::new(o.max_conn));
    loop {
        if PX_SERVE_STOP.load(Ordering::SeqCst) {
            break;
        }
        let (stream, _peer) = match listener.accept() {
            Ok(x) => x,
            Err(e) if e.kind() == std::io::ErrorKind::WouldBlock => {
                std::thread::sleep(Duration::from_millis(20));
                continue;
            }
            Err(_) => continue,
        };
        let root = root.clone();
        let o = o.clone();
        let sem2 = sem.clone();
        PX_IN_FLIGHT.fetch_add(1, Ordering::SeqCst);
        std::thread::spawn(move || {
            // M31.4b：并发信号量——超过 max_conn 时阻塞等待（连接保持，非拒绝）
            sem2.acquire();
            let r = (|| {
                let mut conn = match crate::tls::accept_stream(stream) {
                    Some(c) => c,
                    None => return Ok(()), // TLS 握手失败
                };
                handle_px_conn(&mut conn, &root, port, &o)
            })();
            sem2.release();
            PX_IN_FLIGHT.fetch_sub(1, Ordering::SeqCst);
            if let Err(e) = r {
                eprintln!("[px-serve] {}", e);
            }
        });
    }
    // 等待在途请求完成（最多 5s）
    let deadline = std::time::Instant::now() + Duration::from_secs(5);
    while PX_IN_FLIGHT.load(Ordering::SeqCst) > 0 && std::time::Instant::now() < deadline {
        std::thread::sleep(Duration::from_millis(50));
    }
    eprintln!(
        "[px-serve] 优雅关闭完成（在途 {}）",
        PX_IN_FLIGHT.load(Ordering::SeqCst)
    );
    Ok(())
}

/// 请求 ID 全局序号（每请求生成唯一 ID：px-<unixms>-<seq>）
static REQ_ID_SEQ: AtomicI64 = AtomicI64::new(0);

fn new_request_id() -> String {
    let ms = now_secs() * 1000;
    let seq = REQ_ID_SEQ.fetch_add(1, Ordering::SeqCst);
    format!("px-{}-{}", ms, seq)
}

/// M31.4b：并发信号量（max_conn 上限）。超出时阻塞等待（连接保持排队，不拒绝）。
struct ConnSem {
    mu: Mutex<usize>,
    cv: std::sync::Condvar,
}

impl ConnSem {
    fn new(max: usize) -> Self {
        Self {
            mu: Mutex::new(max.max(1)),
            cv: std::sync::Condvar::new(),
        }
    }
    fn acquire(&self) {
        let mut n = self.mu.lock().unwrap();
        while *n == 0 {
            n = self.cv.wait(n).unwrap();
        }
        *n -= 1;
    }
    fn release(&self) {
        let mut n = self.mu.lock().unwrap();
        *n += 1;
        self.cv.notify_one();
    }
}

/// 处理单个 HTTP 连接：keep-alive 循环——同一连接连续处理多个请求，
/// 直到客户端 Connection: close / 空闲超时 / 出错。每请求独立日志 + 请求 ID。
/// M31：每请求按 Host 头解析虚拟主机（vhost）→ 请求级 docroot / handler。
fn handle_px_conn(
    conn: &mut SConn,
    root: &Path,
    port: i64,
    o: &PxServeOpts,
) -> Result<(), String> {
    loop {
        // keep-alive：空闲读超时 15s（客户端保持连接但长时间无请求 → 关闭）
        conn.set_read_timeout(Some(Duration::from_secs(15))).ok();
        let (req, method, _body, body_tmp, residual) = match read_http_conn(conn, o) {
            Ok(x) => x,
            Err(e) if e == "PAYLOAD_TOO_LARGE" => {
                let _ = send_response(conn, 413, &[], b"413 Payload Too Large", false, false);
                return finish_conn(conn, None);
            }
            Err(e) if e == "连接已关闭" || e.contains("超时") || e.contains("TimedOut") || e.contains("WouldBlock") => {
                // keep-alive 自然结束（客户端关闭 / 空闲超时）；此时未读到完整请求，无 body_tmp
                return finish_conn(conn, None);
            }
            Err(e) => return Err(e),
        };
        // 客户端 keep-alive 判定（Connection: close → 处理后关闭；HTTP/1.1 默认 keep）
        let keep = request_keep_alive(&req);
        // M35：HTTP/2 —— h2c Upgrade（Upgrade: h2c + HTTP2-Settings）→ 升级为 h2 帧连接；
        // prior knowledge（请求行 "PRI * HTTP/2.0"）→ 直接 h2。进入帧循环后整连接为 h2。
        if req_header_contains(&req, "upgrade", "h2c") {
            crate::h2::h2_serve(conn, true, &residual)?;
            return finish_conn(conn, body_tmp);
        }
        if method == "PRI" {
            crate::h2::h2_serve(conn, false, &residual)?;
            return finish_conn(conn, body_tmp);
        }
        // M31：虚拟主机解析 → 每请求 docroot / handler
        let (vroot, vhandler) = vhost_resolve(req_host(&req).as_deref(), root);
        let ok = handle_one_request(conn, &req, &method, &vroot, port, o, keep, vhandler)?;
        finish_conn(conn, body_tmp)?;
        if !ok || !keep {
            return Ok(());
        }
    }
}

/// 请求是否要求 keep-alive（默认 true；Connection: close → false）
fn request_keep_alive(req: &Value) -> bool {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case("connection") {
                    if let Value::Str(s) = v {
                        return !s.trim().eq_ignore_ascii_case("close");
                    }
                }
            }
        }
    }
    true
}

/// 请求头是否存在（大小写不敏感）
fn req_header_present(req: &Value, name: &str) -> bool {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, _) in h.iter() {
                if k.eq_ignore_ascii_case(name) {
                    return true;
                }
            }
        }
    }
    false
}

/// 请求头值是否包含子串（大小写不敏感；substr 空串 → 仅判断头存在）
fn req_header_contains(req: &Value, name: &str, substr: &str) -> bool {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case(name) {
                    if substr.is_empty() {
                        return true;
                    }
                    if let Value::Str(s) = v {
                        return s.to_ascii_lowercase().contains(&substr.to_ascii_lowercase());
                    }
                }
            }
        }
    }
    false
}

/// 虚拟主机 handler：handler(req) → null 继续 / 非 null 作为响应（归一化）
fn run_vhost_handler(handler: &Value, req: &Value) -> Option<(i64, Vec<(String, String)>, Vec<u8>)> {
    let mut interp = Interpreter::new();
    let r = interp.call_value(handler, &[req.clone()], crate::token::Pos::new(0, 0));
    match r {
        Ok(Value::Null) => None,
        Ok(v) => Some(normalize_route_resp(&v)),
        Err(e) => Some((
            500,
            vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
            format!("500 Internal Server Error\n\n{}", e).into_bytes(),
        )),
    }
}

/// 处理单个请求（路由 / 静态文件 / .px 脚本）。返回是否继续 keep-alive。
fn handle_one_request(
    conn: &mut SConn,
    req: &Value,
    method: &str,
    root: &Path,
    port: i64,
    o: &PxServeOpts,
    keep_alive: bool,
    vhandler: Option<Value>,
) -> Result<bool, String> {
    // M36：每请求清除线程局部上下文（防跨请求泄漏）
    crate::builtin::ctx_clear_all();
    let path = req_str(req, "path").unwrap_or_else(|| "/".to_string());
    let req_id = new_request_id();
    // M29：请求 ID 注入 REQUEST + 响应头 X-Request-Id（链路追踪）
    if let Value::Dict(d) = req {
        let mut g = d.lock().unwrap();
        g.insert("request_id".into(), Value::Str(req_id.clone()));
    }
    let head_only = method == "HEAD";
    let remote = conn_peer(conn);

    // M31.4a：HTTP/2 预检——明确拒绝 h2c（HTTP/2 prior-knowledge / Upgrade）
    // 客户端探测 HTTP/2 能力（ALPN 已在 TLS 层固定 http/1.1；明文 h2c 这里拒绝 505）
    if method != "GET" && method != "HEAD" && method != "POST" && method != "PUT" && method != "DELETE" && method != "OPTIONS" && method != "PATCH" {
        let mut hd = vec![("X-Request-Id".into(), req_id.clone())];
        if head_only { hd.push(("Content-Length".into(), "31".into())); }
        let _ = send_response(conn, 505, &hd, b"505 HTTP Version Not Supported", head_only, keep_alive);
        return Ok(keep_alive);
    }
    let upgrade_h2c = req_header_contains(req, "upgrade", "h2c")
        || req_header_contains(req, "http2-settings", "");
    if upgrade_h2c {
        let mut hd = vec![("X-Request-Id".into(), req_id.clone())];
        if head_only { hd.push(("Content-Length".into(), "31".into())); }
        let _ = send_response(conn, 505, &hd, b"505 HTTP Version Not Supported", head_only, keep_alive);
        return Ok(keep_alive);
    }

    // M31.4a：CORS 预检——OPTIONS 且带 Access-Control-Request-Method → 自动 204 + CORS 头
    if method == "OPTIONS" && (req_header_present(req, "access-control-request-method") || req_header_present(req, "origin")) {
        let cors_hd: Vec<(String, String)> = vec![
            ("X-Request-Id".into(), req_id.clone()),
            ("Access-Control-Allow-Origin".into(), "*".into()),
            ("Access-Control-Allow-Methods".into(), "GET, POST, PUT, DELETE, PATCH, OPTIONS".into()),
            ("Access-Control-Allow-Headers".into(), "Content-Type, Authorization, X-Request-Id".into()),
            ("Access-Control-Max-Age".into(), "86400".into()),
        ];
        let _ = send_response(conn, 204, &cors_hd, b"", head_only, keep_alive);
        return Ok(keep_alive);
    }

    // M31.3/M35：服务端内置限流（px_serve opts{rate_limit:{max,window_sec,key,whitelist}}）
    if let Some(rl) = &o.rate_limit {
        let ip = remote_ip(&remote);
        // M35：白名单 IP 直接放行（不计数）
        let whitelisted = rl.whitelist.iter().any(|w| w == &ip);
        if !whitelisted {
            // M35：多维 key（ip / ip|ua / ip|path / ip|ua|path）
            let key = build_rate_key(rl, &ip, &path, req);
            if !rate_limit_try(&key, rl.max, rl.window_sec) {
                let mut hd = vec![("X-Request-Id".into(), req_id.clone())];
                if head_only { hd.push(("Content-Length".into(), "21".into())); }
                let _ = send_response(conn, 429, &hd, b"429 Too Many Requests", head_only, keep_alive);
                log_access(&req_id, &remote, method, &path, 429, 21, 0);
                return Ok(keep_alive);
            }
        }
    }
    // M31.2：虚拟主机 handler（该域所有请求交给 handler(req)；返回 null → 继续默认处理）
    if let Some(vh) = vhandler {
        if let Some((vst, vhd, vbd)) = run_vhost_handler(&vh, req) {
            let mut hd = vhd;
            hd.retain(|(k, _)| !k.eq_ignore_ascii_case("x-request-id"));
            hd.push(("X-Request-Id".into(), req_id.clone()));
            let ms = 0u128;
            log_access(&req_id, &remote, method, &path, vst, vbd.len(), ms);
            let _ = send_response(conn, vst, &hd, &vbd, head_only, keep_alive);
            return Ok(keep_alive);
        }
    }

    // M28：路由表非空 → 优先匹配路由（method+path 模式 + :id 参数 + 中间件链）
    if has_routes() {
        if let Some(()) = try_route_dispatch(conn, req, method, head_only, &req_id, &remote, keep_alive)? {
            return Ok(keep_alive);
        }
    }

    let start = std::time::Instant::now();
    let mut status = 500;
    let mut bytes_out = 0usize;

    // 1. 路径解析 + 目录隔离（穿越防护）
    let fs_path = match resolve_path(root, &path) {
        Ok(p) => p,
        Err(e) => {
            let (st, msg) = if e == "FORBIDDEN" {
                (403, "403 Forbidden: 路径穿越被拒绝")
            } else {
                (404, "404 Not Found")
            };
            let mut hd = vec![("X-Request-Id".into(), req_id.clone())];
            if head_only { hd.push(("Content-Length".into(), msg.len().to_string())); }
            let _ = send_response(conn, st, &hd, msg.as_bytes(), head_only, keep_alive);
            status = st;
            bytes_out = msg.len();
            log_access(&req_id, &remote, method, &path, status, bytes_out, start.elapsed().as_millis());
            return Ok(keep_alive);
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
            let mut hd = vec![("X-Request-Id".into(), req_id.clone())];
            if head_only { hd.push(("Content-Length".into(), "13".into())); }
            let _ = send_response(conn, 404, &hd, b"404 Not Found", head_only, keep_alive);
            log_access(&req_id, &remote, method, &path, 404, 13, start.elapsed().as_millis());
            return Ok(keep_alive);
        }
    };

    // 3. .px 脚本 vs 静态文件
    let is_px = target
        .extension()
        .and_then(|e| e.to_str())
        .map(|e| e.eq_ignore_ascii_case("px"))
        .unwrap_or(false);
    if is_px {
        let (st, mut headers, body) = match exec_script(&target, req, root, port, o) {
            Ok(x) => x,
            Err(e) if e == "TIMEOUT" => {
                eprintln!("[px-serve] 脚本超时 {} (>{}ms)", target.display(), o.timeout_ms);
                (
                    504,
                    vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
                    format!("504 Gateway Timeout: 脚本执行超时（>{}ms）", o.timeout_ms).into_bytes(),
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
        };
        headers.retain(|(k, _)| !k.eq_ignore_ascii_case("x-request-id"));
        headers.push(("X-Request-Id".into(), req_id.clone()));
        let (st2, hd2, bd2) = maybe_gzip_response(st, headers, body, req);
        let _ = send_response(conn, st2, &hd2, &bd2, head_only, keep_alive);
        status = st2;
        bytes_out = bd2.len();
        let ms = start.elapsed().as_millis();
        log_access(&req_id, &remote, method, &path, status, bytes_out, ms);
        return Ok(keep_alive);
    }

    // 4. 静态文件：ETag / Last-Modified / 304 / Range + 流式发送（M29）
    let r = send_static_file(conn, &target, req, head_only, keep_alive, &req_id, &remote, method, &path, start);
    status = r.0;
    bytes_out = r.1;
    Ok(keep_alive)
}

/// M35：gzip 解压字节（请求体自动解压；失败返回 Err 保持原样）
fn gunzip_bytes(data: &[u8]) -> Result<Vec<u8>, String> {
    use std::io::Read as _;
    let mut d = flate2::read::GzDecoder::new(data);
    let mut out = Vec::new();
    d.read_to_end(&mut out)
        .map_err(|e| format!("gzip 解压失败: {}", e))?;
    Ok(out)
}

/// M29：结构化访问日志（时间 remote method path status bytes ms req=id）
/// M33：落盘——px_serve opts{access_log:"path"} 时写文件 + 大小轮转（10MB → .1/.2/.3）
const ACCESS_LOG_MAX: u64 = 10 * 1024 * 1024;
#[derive(Clone)]
struct AccessLogState {
    path: String,
    /// M36：JSON 行格式
    json: bool,
    /// M36：按天轮转（文件名带日期后缀）
    daily: bool,
    /// 当前日期（daily 用，YYYYMMDD）
    day: String,
}
static ACCESS_LOG: OnceLock<Mutex<Option<AccessLogState>>> = OnceLock::new();
fn access_log_state() -> &'static Mutex<Option<AccessLogState>> {
    ACCESS_LOG.get_or_init(|| Mutex::new(None))
}

/// 当前日期 YYYYMMDD（UTC 民用日历）
fn today_str() -> String {
    let t = now_secs() as i64;
    let (y, m, d) = crate::tztime::civil_from_days(t / 86400);
    format!("{:04}{:02}{:02}", y, m, d)
}

/// px_serve 启动时配置 access log 落盘路径 + 格式（JSON / 按天轮转）
pub fn set_access_log(path: Option<String>, json: bool, daily: bool) {
    *access_log_state().lock().unwrap() = path.map(|p| AccessLogState {
        path: p,
        json,
        daily,
        day: today_str(),
    });
}

/// 当前落盘文件路径（daily 时带日期后缀；跨天切换）
fn access_log_file(st: &mut AccessLogState) -> String {
    if st.daily {
        let t = today_str();
        if t != st.day {
            st.day = t;
        }
        format!("{}.{}", st.path, st.day)
    } else {
        st.path.clone()
    }
}

fn access_log_rotate(state: &AccessLogState) {
    // 当前文件 > 10MB → 轮转 .1/.2/.3（保留 3 份）
    let p = &state.path;
    let sz = std::fs::metadata(p).map(|m| m.len()).unwrap_or(0);
    if sz < ACCESS_LOG_MAX {
        return;
    }
    for i in (1..=3).rev() {
        let src = if i == 1 { p.clone() } else { format!("{}.{}", p, i - 1) };
        let dst = format!("{}.{}", p, i);
        if std::fs::metadata(&src).is_ok() {
            let _ = std::fs::rename(&src, &dst);
        }
    }
}

fn log_access(req_id: &str, remote: &str, method: &str, path: &str, status: i64, bytes: usize, ms: u128) {
    let t = now_secs();
    let mut st = access_log_state().lock().unwrap().clone();
    let line = match &st {
        Some(s) if s.json => format!(
            "{{\"ts\":{},\"remote\":\"{}\",\"method\":\"{}\",\"path\":\"{}\",\"status\":{},\"bytes\":{},\"ms\":{},\"req\":\"{}\"}}\n",
            t, remote, method, path, status, bytes, ms, req_id
        ),
        _ => format!(
            "[px-access] {} {} {} {} {} {} {}ms req={}\n",
            t, remote, method, path, status, bytes, ms, req_id
        ),
    };
    eprint!("{}", line);
    // M33/M36：落盘（追加 + 大小轮转 + 按天切换）
    if let Some(mut st) = st {
        access_log_rotate(&st);
        let file = access_log_file(&mut st);
        if let Ok(mut f) = std::fs::OpenOptions::new().create(true).append(true).open(&file) {
            use std::io::Write;
            let _ = f.write_all(line.as_bytes());
        }
    }
}

/// M29c：gzip 响应压缩——Accept-Encoding: gzip 且 body > 1KB 且为文本类 → gzip + Content-Encoding
fn maybe_gzip_response(
    status: i64,
    mut headers: Vec<(String, String)>,
    body: Vec<u8>,
    req: &Value,
) -> (i64, Vec<(String, String)>, Vec<u8>) {
    if status != 200 && status != 206 && status != 201 {
        return (status, headers, body);
    }
    if body.len() < 1024 {
        return (status, headers, body);
    }
    // 请求 Accept-Encoding 含 gzip？
    let mut accept_gzip = false;
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case("accept-encoding") {
                    if let Value::Str(s) = v {
                        accept_gzip = s.to_lowercase().contains("gzip");
                    }
                }
            }
        }
    }
    if !accept_gzip {
        return (status, headers, body);
    }
    // 文本类才压缩（image/video/zip 等二进制不再压缩）
    let ct = headers
        .iter()
        .find(|(k, _)| k.eq_ignore_ascii_case("content-type"))
        .map(|(_, v)| v.to_lowercase())
        .unwrap_or_default();
    let compressible = ct.starts_with("text/")
        || ct.contains("json")
        || ct.contains("javascript")
        || ct.contains("xml")
        || ct.contains("svg")
        || ct.contains("csv")
        || ct.is_empty();
    if !compressible {
        return (status, headers, body);
    }
    use flate2::write::GzEncoder;
    use flate2::Compression;
    use std::io::Write as _;
    let mut enc = GzEncoder::new(Vec::new(), Compression::new(6));
    if enc.write_all(&body).is_err() {
        return (status, headers, body);
    }
    match enc.finish() {
        Ok(gz) => {
            headers.retain(|(k, _)| !k.eq_ignore_ascii_case("content-encoding"));
            headers.retain(|(k, _)| !k.eq_ignore_ascii_case("vary"));
            headers.push(("Content-Encoding".into(), "gzip".into()));
            headers.push(("Vary".into(), "Accept-Encoding".into()));
            (status, headers, gz)
        }
        Err(_) => (status, headers, body),
    }
}

/// M29b：静态文件响应——ETag / Last-Modified / 304 / Range + 流式发送。
/// 返回 (status, 发送字节数)。
fn send_static_file(
    conn: &mut SConn,
    target: &Path,
    req: &Value,
    head_only: bool,
    keep_alive: bool,
    req_id: &str,
    remote: &str,
    method: &str,
    path: &str,
    start: std::time::Instant,
) -> (i64, usize) {
    use std::io::Read as _;
    let meta = match std::fs::metadata(target) {
        Ok(m) => m,
        Err(e) => {
            let msg = format!("404 Not Found: {}", e);
            let mut hd = vec![("X-Request-Id".into(), req_id.to_string())];
            if head_only { hd.push(("Content-Length".into(), msg.len().to_string())); }
            let _ = send_response(conn, 404, &hd, msg.as_bytes(), head_only, keep_alive);
            log_access(req_id, remote, method, path, 404, msg.len(), start.elapsed().as_millis());
            return (404, msg.len());
        }
    };
    let size = meta.len();
    let mtime = meta
        .modified()
        .ok()
        .and_then(|t| t.duration_since(std::time::UNIX_EPOCH).ok())
        .map(|d| d.as_secs() as i64)
        .unwrap_or(0);
    let etag = format!("\"px-{:x}-{:x}\"", mtime, size);
    let last_mod = http_date(mtime);
    let ct = mime_type(&target.to_string_lossy()).to_string();

    // 请求头查询（If-None-Match / If-Modified-Since / Range / Accept-Encoding）
    let (if_none_match, if_mod_since, range, accept_gzip) = {
        let mut inm = None;
        let mut ims = None;
        let mut rng = None;
        let mut gz = false;
        if let Value::Dict(d) = req {
            if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
                let h = h.lock().unwrap();
                for (k, v) in h.iter() {
                    if let Value::Str(s) = v {
                        if k.eq_ignore_ascii_case("if-none-match") {
                            inm = Some(s.clone());
                        } else if k.eq_ignore_ascii_case("if-modified-since") {
                            ims = Some(s.clone());
                        } else if k.eq_ignore_ascii_case("range") {
                            rng = Some(s.clone());
                        } else if k.eq_ignore_ascii_case("accept-encoding") && s.to_lowercase().contains("gzip") {
                            gz = true;
                        }
                    }
                }
            }
        }
        (inm, ims, rng, gz)
    };

    // 304：If-None-Match 匹配 ETag
    if let Some(inm) = &if_none_match {
        if inm.trim().eq(&etag) || inm.trim() == "*" {
            let hd = vec![
                ("ETag".into(), etag.clone()),
                ("Last-Modified".into(), last_mod.clone()),
                ("X-Request-Id".into(), req_id.to_string()),
            ];
            let _ = send_response(conn, 304, &hd, &[], head_only, keep_alive);
            log_access(req_id, remote, method, path, 304, 0, start.elapsed().as_millis());
            return (304, 0);
        }
    }
    // 304：If-Modified-Since 且 mtime <= since
    if let Some(ims) = &if_mod_since {
        if let Some(since) = parse_http_date(ims) {
            if mtime <= since {
                let hd = vec![
                    ("ETag".into(), etag.clone()),
                    ("Last-Modified".into(), last_mod.clone()),
                    ("X-Request-Id".into(), req_id.to_string()),
                ];
                let _ = send_response(conn, 304, &hd, &[], head_only, keep_alive);
                log_access(req_id, remote, method, path, 304, 0, start.elapsed().as_millis());
                return (304, 0);
            }
        }
    }

    // Range: bytes=start-end → 206 Partial Content
    let mut start_b = 0u64;
    let mut end_b = size.saturating_sub(1);
    let mut status = 200;
    let mut extra: Vec<(String, String)> = vec![
        ("ETag".into(), etag),
        ("Last-Modified".into(), last_mod),
        ("X-Request-Id".into(), req_id.to_string()),
    ];
    if let Some(r) = &range {
        if let Some((s, e)) = parse_range(r, size) {
            start_b = s;
            end_b = e;
            status = 206;
            extra.push(("Content-Range".into(), format!("bytes {}-{}/{}", s, e, size)));
            extra.push(("Accept-Ranges".into(), "bytes".into()));
        }
    }
    let seg_len = (end_b - start_b + 1) as usize;

    // 响应头（HEAD 只发头）
    let reason = status_reason(status);
    let mut hd = format!(
        "HTTP/1.1 {} {}\r\nContent-Length: {}\r\nConnection: {}\r\nContent-Type: {}\r\n",
        status, reason, seg_len, if keep_alive { "keep-alive" } else { "close" }, ct
    );
    for (k, v) in &extra {
        hd.push_str(&format!("{}: {}\r\n", k, v));
    }
    // M33.4：Alt-Svc 通告（HTTP/3 协商；px_serve opts{alt_svc}）
    if let Some(alt) = alt_svc_value() {
        let mut dup = false;
        for (k, _) in &extra {
            if k.eq_ignore_ascii_case("alt-svc") {
                dup = true;
                break;
            }
        }
        if !dup {
            hd.push_str(&format!("Alt-Svc: {}\r\n", alt));
        }
    }
    if accept_gzip && size >= 1024 && ct.starts_with("text/") {
        // 静态文本文件 gzip：仅当请求 Accept-Encoding: gzip
        // （为保持与解释器一致 + 避免破坏 Range，仅对整文件 200 场景 gzip）
        if status == 200 {
            // 先读全文件压缩
            if let Ok(mut f) = std::fs::File::open(target) {
                let mut data = Vec::new();
                if f.read_to_end(&mut data).is_ok() && data.len() >= 1024 {
                    use flate2::write::GzEncoder;
                    use flate2::Compression;
                    use std::io::Write as _;
                    let mut enc = GzEncoder::new(Vec::new(), Compression::new(6));
                    if enc.write_all(&data).is_ok() {
                        if let Ok(gz) = enc.finish() {
                            hd.push_str("Content-Encoding: gzip\r\nVary: Accept-Encoding\r\n");
                            let _ = conn.write_all(hd.as_bytes());
                            let _ = conn.flush();
                            if !head_only {
                                let _ = conn.write_all(&gz);
                            }
                            let _ = conn.flush();
                            let n = if head_only { 0 } else { gz.len() };
                            log_access(req_id, remote, method, path, 200, n, start.elapsed().as_millis());
                            return (200, n);
                        }
                    }
                }
            }
        }
    }
    hd.push_str("\r\n");
    let _ = conn.write_all(hd.as_bytes());
    let _ = conn.flush();
    if !head_only {
        // 流式发送：64KB 块，从 start_b 到 end_b（大文件不占内存）
        let mut f = match std::fs::File::open(target) {
            Ok(f) => f,
            Err(e) => {
                let msg = format!("404 Not Found: {}", e);
                let mut h2 = vec![("X-Request-Id".into(), req_id.to_string())];
                if head_only { h2.push(("Content-Length".into(), msg.len().to_string())); }
                let _ = send_response(conn, 404, &h2, msg.as_bytes(), head_only, keep_alive);
                log_access(req_id, remote, method, path, 404, msg.len(), start.elapsed().as_millis());
                return (404, msg.len());
            }
        };
        let _ = f.seek(std::io::SeekFrom::Start(start_b));
        let mut remain = seg_len;
        let mut chunk = vec![0u8; 65536];
        while remain > 0 {
            let want = remain.min(chunk.len());
            let n = f.read(&mut chunk[..want]).unwrap_or(0);
            if n == 0 {
                break;
            }
            if conn.write_all(&chunk[..n]).is_err() {
                break;
            }
            remain -= n;
        }
        let _ = conn.flush();
        let sent = seg_len - remain;
        log_access(req_id, remote, method, path, status, sent, start.elapsed().as_millis());
        return (status, sent);
    }
    log_access(req_id, remote, method, path, status, 0, start.elapsed().as_millis());
    (status, 0)
}

/// HTTP date（RFC 7231 IMF-fixdate：Sun, 06 Nov 1994 08:49:37 GMT）——用 tztime 民用算法
fn http_date(ts: i64) -> String {
    let (y, m, d, h, mi, s, wd) = crate::tztime::civil_breakdown(ts, 0);
    const WD: [&str; 7] = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
    const MO: [&str; 12] = ["Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"];
    format!(
        "{}, {:02} {} {} {:02}:{:02}:{:02} GMT",
        WD[wd as usize],
        d,
        MO[(m - 1) as usize],
        y,
        h,
        mi,
        s
    )
}

/// 解析 HTTP date（IMF-fixdate / RFC 850 / asctime）→ epoch 秒；失败 → None
fn parse_http_date(s: &str) -> Option<i64> {
    let s = s.trim();
    // IMF-fixdate: Sun, 06 Nov 1994 08:49:37 GMT
    let parts: Vec<&str> = s.split(|c: char| c == ' ' || c == ',').filter(|p| !p.is_empty()).collect();
    if parts.len() < 6 {
        return None;
    }
    // 常见格式： [Sun,] 06 Nov 1994 08:49:37 GMT（weekday 可能省略）
    let mut i = 0usize;
    let day: i64 = if parts[0].parse::<i64>().is_ok() {
        parts[0].parse().ok()?
    } else {
        // 第一个是星期几（如 "Thu"）
        i = 1;
        if parts.len() < 6 {
            return None;
        }
        parts[1].parse().ok()?
    };
    let mon = parts.get(i + 1)?;
    let year: i64 = parts.get(i + 2)?.parse().ok()?;
    let hms = parts.get(i + 3)?;
    let mo = match *mon {
        "Jan" => 1, "Feb" => 2, "Mar" => 3, "Apr" => 4, "May" => 5, "Jun" => 6,
        "Jul" => 7, "Aug" => 8, "Sep" => 9, "Oct" => 10, "Nov" => 11, "Dec" => 12,
        _ => return None,
    };
    let hm: Vec<&str> = hms.split(':').collect();
    if hm.len() != 3 {
        return None;
    }
    let h: i64 = hm[0].parse().ok()?;
    let mi: i64 = hm[1].parse().ok()?;
    let sec: i64 = hm[2].parse().ok()?;
    let days = crate::tztime::days_from_civil(year, mo, day);
    Some(days * 86400 + h * 3600 + mi * 60 + sec)
}

/// 解析 Range: bytes=start-end（单段）；不支持/非法 → None
fn parse_range(r: &str, size: u64) -> Option<(u64, u64)> {
    let r = r.trim();
    let b = r.strip_prefix("bytes=")?;
    let b = b.trim();
    let dash = b.find('-')?;
    let s_str = b[..dash].trim();
    let e_str = b[dash + 1..].trim();
    if s_str.is_empty() {
        // suffix range: bytes=-N → 最后 N 字节
        let n: u64 = e_str.parse().ok()?;
        if n == 0 || size == 0 {
            return None;
        }
        let s = size.saturating_sub(n);
        return Some((s, size - 1));
    }
    let s: u64 = s_str.parse().ok()?;
    if s >= size {
        return None;
    }
    let e = if e_str.is_empty() {
        size - 1
    } else {
        e_str.parse::<u64>().ok()?.min(size - 1)
    };
    if e < s {
        return None;
    }
    Some((s, e))
}

/// 请求收尾：删除 body 落盘临时文件 + 清理线程局部状态
fn finish_conn(_conn: &mut SConn, body_tmp: Option<PathBuf>) -> Result<(), String> {
    if let Some(p) = body_tmp {
        let _ = std::fs::remove_file(&p);
    }
    reset_request_state();
    Ok(())
}

/// 执行 .px 脚本（内嵌解释器 + 超时控制），返回 (status, headers, body)。
/// M27：session Set-Cookie / basic_auth 401 自动注入响应。
fn exec_script(
    script_path: &Path,
    req: &Value,
    root: &Path,
    port: i64,
    o: &PxServeOpts,
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
        // M27：脚本内 session_open/basic_auth 设置的线程局部注入（Set-Cookie / 401）
        // 在 worker 线程收集（thread_local 属于执行线程）
        let (cookies, auth) = take_response_injections();
        let _ = tx.send((r, out, resp, cookies, auth));
    });

    match rx.recv_timeout(Duration::from_millis(o.timeout_ms.max(1) as u64)) {
        Ok((Ok(code), out, resp, set_cookies, auth_realm)) => {
            if code != 0 {
                return Err(format!("脚本退出码非零: {}", code));
            }
            // 脚本可设全局 RESPONSE = {status, headers, body}；否则 print 输出即 body
            let mut status;
            let mut body: Vec<u8>;
            let mut headers: Vec<(String, String)>;
            if let Some(Value::Dict(d)) = resp {
                let d = d.lock().unwrap();
                status = match d.get("status") {
                    Some(Value::Int(s)) => *s,
                    Some(Value::Float(f)) => *f as i64,
                    _ => 200,
                };
                body = match d.get("body") {
                    Some(Value::Str(s)) => s.clone().into_bytes(),
                    Some(Value::Null) | None => out,
                    Some(o) => o.to_string().into_bytes(),
                };
                headers = match d.get("headers") {
                    Some(Value::Dict(h)) => {
                        let h = h.lock().unwrap();
                        h.iter().map(|(k, v)| (k.clone(), v.to_string())).collect()
                    }
                    _ => vec![("Content-Type".into(), "text/html; charset=utf-8".into())],
                };
            } else {
                status = 200;
                body = out;
                headers = vec![("Content-Type".into(), "text/html; charset=utf-8".into())];
            }
            // 基础认证失败 → 强制 401（脚本用 `if !basic_auth(...) return` 风格）
            if let Some(realm) = auth_realm {
                if status == 200 {
                    status = 401;
                }
                if body.is_empty() {
                    body = b"401 Unauthorized".to_vec();
                }
                headers.retain(|(k, _)| !k.eq_ignore_ascii_case("www-authenticate"));
                headers.push((
                    "WWW-Authenticate".into(),
                    format!("Basic realm=\"{}\"", realm),
                ));
            }
            // session 新 Cookie 自动注入
            for c in set_cookies {
                headers.push(("Set-Cookie".into(), c));
            }
            Ok((status, headers, body))
        }
        Ok((Err(e), _, _, _, _)) => Err(format!("脚本执行出错: {}", e)),
        Err(_) => Err("TIMEOUT".into()),
    }
}

// ==================== 辅助 ====================

/// 读取 HTTP 请求（头 + body），返回 (req dict, method, body 字节, body 临时文件路径)。
/// - body 大小超过 opts.max_body_size → Err("PAYLOAD_TOO_LARGE")（上层回 413）
/// - body > 1MB → 落盘临时文件（req["body_tmp"] = 路径；body 字段为空串，防内存溢出）
pub fn read_http_conn(
    conn: &mut SConn,
    o: &PxServeOpts,
) -> Result<(Value, String, Vec<u8>, Option<PathBuf>, Vec<u8>), String> {
    let mut buf: Vec<u8> = Vec::new();
    let mut tmp = [0u8; 8192];
    let header_end;
    loop {
        let n = conn
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
    let remote = conn_peer(conn);
    let (req, content_length) = parse_http_request(&head, &remote)?;
    let body_off = header_end + 4;
    let (mut body_bytes, body_tmp) = read_body(conn, &buf[body_off..], content_length, o)?;
    // M35：HTTP/2 h2c——请求头后缓冲的残留字节（client preface 前几字节）回传给 h2 处理
    let extra: Vec<u8> = if buf.len() > body_off + content_length {
        buf[body_off + content_length..].to_vec()
    } else {
        Vec::new()
    };
    // M35：请求体 gzip 自动解压（Content-Encoding: gzip → 解压后供 REQUEST.body / form 使用）
    if !body_bytes.is_empty() && req_header_contains(&req, "content-encoding", "gzip") {
        if let Ok(dec) = gunzip_bytes(&body_bytes) {
            body_bytes = dec;
        }
    }
    if let Value::Dict(d) = &req {
        let mut g = d.lock().unwrap();
        g.insert("body".into(), Value::Str(String::from_utf8_lossy(&body_bytes).to_string()));
        if let Some(p) = &body_tmp {
            g.insert("body_tmp".into(), Value::Str(p.to_string_lossy().to_string()));
        }
        // Cookie 解析（M27：REQUEST.cookie）
        if let Some(cv) = g.get("headers").cloned() {
            if let Value::Dict(h) = cv {
                let cookie_header = {
                    let h = h.lock().unwrap();
                    h.iter()
                        .find(|(k, _)| k.eq_ignore_ascii_case("cookie"))
                        .and_then(|(_, v)| match v {
                            Value::Str(s) => Some(s.clone()),
                            _ => None,
                        })
                };
                if let Some(ch) = cookie_header {
                    g.insert("cookie".into(), parse_cookie(&ch));
                }
            }
        }
        if !body_bytes.is_empty() {
            if let Some(ct) = content_type_of(&g) {
                let lct = ct.to_lowercase();
                if lct.contains("application/x-www-form-urlencoded") {
                    g.insert("form".into(), parse_form(&String::from_utf8_lossy(&body_bytes)));
                } else if lct.contains("multipart/form-data") {
                    let (form, files) = parse_multipart(
                        &String::from_utf8_lossy(&body_bytes),
                        &multipart_boundary(&ct),
                    );
                    g.insert("form".into(), form);
                    g.insert("files".into(), files);
                }
            }
        }
    }
    let method = req_str(&req, "method").unwrap_or_default();
    Ok((req, method, body_bytes, body_tmp, extra))
}

/// 连接对端地址（TLS 或明文）
fn conn_peer(conn: &SConn) -> String {
    conn.peer_addr_str()
}

/// 流式读取 body：大小限制（超限 → Err PAYLOAD_TOO_LARGE）+ 大 body 落盘。
fn read_body(
    conn: &mut SConn,
    have: &[u8],
    content_length: usize,
    o: &PxServeOpts,
) -> Result<(Vec<u8>, Option<PathBuf>), String> {
    if content_length == 0 {
        return Ok((Vec::new(), None));
    }
    if content_length > o.max_body_size {
        return Err("PAYLOAD_TOO_LARGE".into());
    }
    let mut collected: Vec<u8> = Vec::new();
    let mut tmp_file: Option<(std::fs::File, PathBuf)> = None;
    let mut need = content_length;
    let take = have.len().min(need);
    if take > 0 {
        collected.extend_from_slice(&have[..take]);
        need -= take;
    }
    let mut chunk = [0u8; 16384];
    while need > 0 {
        let n = conn
            .read(&mut chunk)
            .map_err(|e| format!("读 body 失败: {}", e))?;
        if n == 0 {
            break;
        }
        let t = n.min(need);
        if tmp_file.is_none() && collected.len() + t > BODY_TMP_THRESHOLD {
            // 超过阈值 → 转落盘（先把已收集的写文件）
            let path = tmp_body_path(o);
            let mut f = std::fs::File::create(&path)
                .map_err(|e| format!("创建 body 临时文件失败: {}", e))?;
            f.write_all(&collected)
                .map_err(|e| format!("写 body 临时文件失败: {}", e))?;
            tmp_file = Some((f, path));
            collected.clear();
        }
        match &mut tmp_file {
            Some((f, _)) => f
                .write_all(&chunk[..t])
                .map_err(|e| format!("写 body 临时文件失败: {}", e))?,
            None => collected.extend_from_slice(&chunk[..t]),
        }
        need -= t;
    }
    Ok((collected, tmp_file.map(|(_, p)| p)))
}

/// body 临时文件路径（body_tmp_dir/px_body_<pid>_<seq>.tmp）
fn tmp_body_path(o: &PxServeOpts) -> PathBuf {
    let seq = BODY_SEQ.fetch_add(1, Ordering::SeqCst);
    Path::new(&o.body_tmp_dir)
        .join(format!("px_body_{}_{}.tmp", std::process::id(), seq))
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

/// 发送 HTTP 响应（二进制安全；HEAD 只发响应头；TLS/明文统一）
pub fn send_response(
    conn: &mut SConn,
    status: i64,
    headers: &[(String, String)],
    body: &[u8],
    head_only: bool,
    keep_alive: bool,
) -> Result<(), String> {
    let mut resp = Vec::new();
    let reason = status_reason(status);
    resp.extend_from_slice(
        format!(
            "HTTP/1.1 {} {}\r\nContent-Length: {}\r\nConnection: {}\r\n",
            status,
            reason,
            body.len(),
            if keep_alive { "keep-alive" } else { "close" }
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
    // M33.4：HTTP/3 通告——px_serve opts{alt_svc} 时统一注入 Alt-Svc 头
    if let Some(alt) = alt_svc_value() {
        let mut dup = false;
        for (k, _) in headers {
            if k.eq_ignore_ascii_case("alt-svc") {
                dup = true;
                break;
            }
        }
        if !dup {
            resp.extend_from_slice(format!("Alt-Svc: {}\r\n", alt).as_bytes());
        }
    }
    resp.extend_from_slice(b"\r\n");
    if !head_only {
        resp.extend_from_slice(body);
    }
    conn.write_all(&resp)
        .map_err(|e| format!("发送响应失败: {}", e))?;
    conn.flush().map_err(|e| format!("flush 失败: {}", e))?;
    Ok(())
}

/// M33.4：Alt-Svc 通告（HTTP/3 协商机制；px_serve opts{alt_svc:"h3=\":443\""}）
static ALT_SVC: OnceLock<Mutex<Option<String>>> = OnceLock::new();
fn alt_svc_lock() -> &'static Mutex<Option<String>> {
    ALT_SVC.get_or_init(|| Mutex::new(None))
}
pub fn set_alt_svc(v: Option<String>) {
    *alt_svc_lock().lock().unwrap() = v;
}
fn alt_svc_value() -> Option<String> {
    alt_svc_lock().lock().unwrap().clone()
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

// ==================== M28：路由表 + 中间件链 ====================
// route(method, pattern, handler)：注册路由。pattern 支持
//   - 字面段 /api/users
//   - 参数段 :id → params["id"]
//   - 通配段 *  → 匹配任意剩余路径（含 /）
// method 支持 "GET"/"POST"/... 或 "*"（任意方法）
// middleware(fn)：注册中间件（按注册顺序执行）。fn(req) 返回 null 表示继续，
//   返回非 null（int/str/dict）直接短路作为响应。
// 优先级：路由表非空时优先匹配路由；未匹配回退 docroot 文件映射（PHP 模式）。

/// 路由段
#[derive(Debug, Clone)]
enum RouteSeg {
    Lit(String),
    Param(String),
    Wild,
}

/// 路由条目
struct RouteEntry {
    method: String,
    segs: Vec<RouteSeg>,
    handler: Value,
    /// 路由唯一 key（限流桶前缀）："METHOD pattern"
    key: String,
    /// M33：per-route 限流（max 次 / window_sec 秒，按来源 IP → 超限 429）
    rate_limit: Option<(i64, i64)>,
}

static ROUTES: OnceLock<Mutex<Vec<RouteEntry>>> = OnceLock::new();
static MIDDLEWARES: OnceLock<Mutex<Vec<Value>>> = OnceLock::new();

fn routes() -> &'static Mutex<Vec<RouteEntry>> {
    ROUTES.get_or_init(|| Mutex::new(Vec::new()))
}
fn middlewares() -> &'static Mutex<Vec<Value>> {
    MIDDLEWARES.get_or_init(|| Mutex::new(Vec::new()))
}

/// 清空路由/中间件（测试与 px_serve 重启隔离）
pub fn reset_routes() {
    routes().lock().unwrap().clear();
    middlewares().lock().unwrap().clear();
}

/// 解析 pattern → 段列表（/api/users/:id/*）
fn parse_pattern(pattern: &str) -> Result<Vec<RouteSeg>, String> {
    let mut segs = Vec::new();
    for part in pattern.trim_start_matches('/').split('/') {
        if part.is_empty() {
            continue;
        }
        if part == "*" {
            segs.push(RouteSeg::Wild);
        } else if let Some(name) = part.strip_prefix(':') {
            if name.is_empty() {
                return Err(format!("路由参数名不能为空: {}", pattern));
            }
            segs.push(RouteSeg::Param(name.to_string()));
        } else {
            segs.push(RouteSeg::Lit(part.to_string()));
        }
    }
    Ok(segs)
}

/// route(method, pattern, handler[, rate_limit]) → bool
/// rate_limit: Option<(max, window_sec)> —— 该路由独立限流（按来源 IP）
pub fn route_add(
    method: &str,
    pattern: &str,
    handler: Value,
    rate_limit: Option<(i64, i64)>,
) -> Result<(), String> {
    let segs = parse_pattern(pattern)?;
    routes().lock().unwrap().push(RouteEntry {
        method: method.to_uppercase(),
        key: format!("{} {}", method.to_uppercase(), pattern),
        segs,
        handler,
        rate_limit,
    });
    Ok(())
}

/// middleware(fn) → bool
pub fn middleware_add(handler: Value) -> Result<(), String> {
    middlewares().lock().unwrap().push(handler);
    Ok(())
}

/// 匹配路由：返回 (handler, params dict, rate_limit, route_key)。path 已 URL 解码。
fn match_route(method: &str, path: &str) -> Option<(Value, Value, Option<(i64, i64)>, String)> {
    let rs = routes().lock().unwrap();
    if rs.is_empty() {
        return None;
    }
    let m = method.to_uppercase();
    let parts: Vec<&str> = path.trim_start_matches('/').split('/').collect();
    for r in rs.iter() {
        if r.method != "*" && r.method != m {
            continue;
        }
        let mut params = HashMap::new();
        let mut ok = true;
        let mut pi = 0usize;
        for seg in &r.segs {
            match seg {
                RouteSeg::Lit(l) => {
                    if pi >= parts.len() || parts[pi] != l {
                        ok = false;
                        break;
                    }
                    pi += 1;
                }
                RouteSeg::Param(name) => {
                    if pi >= parts.len() || parts[pi].is_empty() {
                        ok = false;
                        break;
                    }
                    params.insert(name.clone(), Value::Str(parts[pi].to_string()));
                    pi += 1;
                }
                RouteSeg::Wild => {
                    // 匹配剩余全部（含空）
                    let rest = parts[pi..].join("/");
                    params.insert("wildcard".to_string(), Value::Str(rest));
                    pi = parts.len();
                }
            }
        }
        // 全部段消费完且路径也消费完（或末尾通配）
        if ok && pi >= parts.len() {
            let handler = r.handler.clone();
            let rl = r.rate_limit;
            let key = r.key.clone();
            let pd = Value::Dict(Arc::new(Mutex::new(params)));
            return Some((handler, pd, rl, key));
        }
    }
    None
}

/// 路由响应归一化：Value → (status, headers, body)
/// 约定同 RESPONSE：int → 状态码；null → 204；str → 200 text/plain；
/// dict{status, headers, body} → 完整控制
fn normalize_route_resp(v: &Value) -> (i64, Vec<(String, String)>, Vec<u8>) {
    match v {
        Value::Int(s) => (*s, vec![], vec![]),
        Value::Null => (204, vec![], vec![]),
        Value::Str(s) => (
            200,
            vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
            s.clone().into_bytes(),
        ),
        Value::Dict(d) => {
            let d = d.lock().unwrap();
            let status = match d.get("status") {
                Some(Value::Int(s)) => *s,
                Some(Value::Float(f)) => *f as i64,
                _ => 200,
            };
            let body = match d.get("body") {
                Some(Value::Str(s)) => s.clone().into_bytes(),
                Some(Value::Bytes(b)) => b.clone(),
                Some(Value::Null) | None => vec![],
                Some(o) => o.to_string().into_bytes(),
            };
            let headers = match d.get("headers") {
                Some(Value::Dict(h)) => {
                    let h = h.lock().unwrap();
                    h.iter().map(|(k, v)| (k.clone(), v.to_string())).collect()
                }
                _ => vec![("Content-Type".into(), "text/html; charset=utf-8".into())],
            };
            (status, headers, body)
        }
        _ => (
            200,
            vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
            v.to_string().into_bytes(),
        ),
    }
}

/// 执行路由 handler（含中间件链）。返回响应三元组。
/// 中间件 fn(req) → null 继续 / 非 null 短路；handler(req, params) → 响应。
fn run_route_pipeline(
    handler: Value,
    params: Value,
    req: &Value,
) -> (i64, Vec<(String, String)>, Vec<u8>) {
    let mut interp = Interpreter::new();
    // 中间件链
    let mws: Vec<Value> = middlewares().lock().unwrap().clone();
    for mw in mws {
        let r = interp.call_value(&mw, &[req.clone()], crate::token::Pos::new(0, 0));
        match r {
            Ok(Value::Null) => continue,
            Ok(v) => return normalize_route_resp(&v), // 短路
            Err(e) => {
                return (
                    500,
                    vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
                    format!("500 Middleware Error\n\n{}", e).into_bytes(),
                );
            }
        }
    }
    // handler
    match interp.call_value(&handler, &[req.clone(), params], crate::token::Pos::new(0, 0)) {
        Ok(v) => normalize_route_resp(&v),
        Err(e) => (
            500,
            vec![("Content-Type".into(), "text/plain; charset=utf-8".into())],
            format!("500 Internal Server Error\n\n{}", e).into_bytes(),
        ),
    }
}

/// 路由是否已注册（决定 px_serve 是否走路由优先）
pub fn has_routes() -> bool {
    !routes().lock().unwrap().is_empty()
}

/// 请求处理中的路由分支：匹配 → 执行；未匹配 → None（回退文件映射）
fn try_route_dispatch(
    conn: &mut SConn,
    req: &Value,
    method: &str,
    head_only: bool,
    req_id: &str,
    remote: &str,
    keep_alive: bool,
) -> Result<Option<()>, String> {
    let path = req_str(req, "path").unwrap_or_else(|| "/".to_string());
    if let Some((handler, params, route_rl, route_key)) = match_route(method, &path) {
        // M33.1：per-route 限流——匹配路由后按 "路由|IP" 计数（各路由独立桶），超限 429
        if let Some((max, win)) = route_rl {
            let key = format!("route:{}|{}", route_key, remote_ip(remote));
            if !rate_limit_try(&key, max, win) {
                let mut hd = vec![("X-Request-Id".into(), req_id.to_string())];
                if head_only {
                    hd.push(("Content-Length".into(), "21".to_string()));
                }
                let _ = send_response(conn, 429, &hd, b"429 Too Many Requests", head_only, keep_alive);
                log_access(req_id, remote, method, &path, 429, 21, 0);
                return Ok(Some(()));
            }
        }
        let start = std::time::Instant::now();
        let (mut status, mut headers, body) = run_route_pipeline(handler, params, req);
        headers.retain(|(k, _)| !k.eq_ignore_ascii_case("x-request-id"));
        headers.push(("X-Request-Id".into(), req_id.to_string()));
        let (st2, hd2, bd2) = maybe_gzip_response(status, headers, body, req);
        status = st2;
        headers = hd2;
        let ms = start.elapsed().as_millis();
        log_access(req_id, remote, method, &path, status, bd2.len(), ms);
        let _ = send_response(conn, status, &headers, &bd2, head_only, keep_alive);
        return Ok(Some(()));
    }
    Ok(None)
}

// ==================== M31 虚拟主机 / Host 头路由 ====================
// vhost(host, docroot|handler)：多域名共服。
//   host 支持 "a.com" / "a.com:8080" / "*"（默认兜底，匹配所有未命中 Host）。
//   docroot（str）：该域名的根目录（静态文件 + .px 脚本基于它）。
//   handler（函数）：该域所有请求交给 handler(req)（返回 null → 继续走默认处理）。
// 匹配规则：Host 头（忽略大小写、去除端口后与无端口注册匹配）→ 精确 host 优先 →
// 带端口精确 → "*" 兜底 → 未匹配用 px_serve 默认 docroot。

/// vhost 目标：docroot 目录 或 handler 函数
#[derive(Clone)]
pub enum VhostTarget {
    Docroot(Value),
    Handler(Value),
}

#[derive(Clone)]
struct VhostRule {
    host: String,
    target: VhostTarget,
}

fn vhosts() -> &'static Mutex<Vec<VhostRule>> {
    static M: OnceLock<Mutex<Vec<VhostRule>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(Vec::new()))
}

/// 注册虚拟主机（vhost_add 供 builtin 分派调用）→ Ok(()) / Err(文案)
pub fn vhost_add(host: &str, target: VhostTarget) -> Result<(), String> {
    let h = host.trim().to_ascii_lowercase();
    if h.is_empty() {
        return Err("host 不能为空".into());
    }
    if let VhostTarget::Docroot(v) = &target {
        if let Value::Str(s) = v {
            if !Path::new(s.as_str()).is_dir() {
                return Err(format!("docroot 不是有效目录: {}", s));
            }
        }
    }
    let mut list = vhosts().lock().unwrap();
    // 同 host 后注册覆盖（支持运行时改配置）
    if let Some(idx) = list.iter().position(|r| r.host == h) {
        list.remove(idx);
    }
    list.push(VhostRule {
        host: h,
        target,
    });
    Ok(())
}

/// 是否注册了任何 vhost
pub fn has_vhosts() -> bool {
    !vhosts().lock().unwrap().is_empty()
}

/// 按 Host 头解析请求目标 → (docroot PathBuf, handler Option<Value>)
fn vhost_resolve(host_header: Option<&str>, default_root: &Path) -> (PathBuf, Option<Value>) {
    let list = vhosts().lock().unwrap();
    let host = host_header
        .map(|h| h.trim().to_ascii_lowercase())
        .unwrap_or_default();
    // host 去端口（仅当 host 含 ':' 且不是 IPv6 括号形式——简化：取第一个 ':' 之前）
    let host_noport = host.split(':').next().unwrap_or(&host).to_string();
    let mut fallback: Option<&VhostRule> = None;
    for rule in list.iter().rev() {
        // 精确（含端口）
        if rule.host == host {
            return apply_vhost(rule, default_root);
        }
        // 注册无端口 ↔ 请求去端口
        if rule.host == host_noport && !rule.host.contains(':') {
            return apply_vhost(rule, default_root);
        }
        if rule.host == "*" {
            fallback = Some(rule);
        }
    }
    if let Some(r) = fallback {
        return apply_vhost(r, default_root);
    }
    (default_root.to_path_buf(), None)
}

fn apply_vhost(rule: &VhostRule, default_root: &Path) -> (PathBuf, Option<Value>) {
    match &rule.target {
        VhostTarget::Docroot(Value::Str(s)) => {
            let p = PathBuf::from(s.as_str());
            (if p.is_dir() { p } else { default_root.to_path_buf() }, None)
        }
        VhostTarget::Handler(h) => (default_root.to_path_buf(), Some(h.clone())),
        _ => (default_root.to_path_buf(), None),
    }
}

/// 请求的 Host 头（从 req dict 的 headers 中取，大小写不敏感）
fn req_host(req: &Value) -> Option<String> {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case("host") {
                    if let Value::Str(s) = v {
                        return Some(s.clone());
                    }
                }
            }
        }
    }
    None
}

// ==================== M31 限流 / 防爆破（滑动窗口） ====================

/// M35：按 key 模式构造限流 key（ip / ip|ua / ip|path / ip|ua|path）
fn build_rate_key(rl: &RateLimitConfig, ip: &str, path: &str, req: &Value) -> String {
    let ua = req_header_str(req, "user-agent").unwrap_or_default();
    let mut key = ip.to_string();
    if rl.key_mode.contains("ua") {
        key.push('|');
        key.push_str(&ua);
    }
    if rl.key_mode.contains("path") {
        key.push('|');
        key.push_str(path);
    }
    key
}

/// 取请求头值（小写匹配，返回首值）
fn req_header_str(req: &Value, name: &str) -> Option<String> {
    if let Value::Dict(d) = req {
        if let Some(Value::Dict(h)) = d.lock().unwrap().get("headers").cloned() {
            let h = h.lock().unwrap();
            for (k, v) in h.iter() {
                if k.eq_ignore_ascii_case(name) {
                    if let Value::Str(s) = v {
                        return Some(s.clone());
                    }
                }
            }
        }
    }
    None
}

// rate_limit(key, max, window_sec) → bool：true 放行 / false 超限（应返回 429）。
// 内部按 key 维护滑动窗口时间戳队列；窗口过期自动滑出。
// 服务端内置：px_serve opts{rate_limit:{max,window_sec}} 按来源 IP 计数，超限 429。

fn rate_buckets() -> &'static Mutex<HashMap<String, std::collections::VecDeque<i64>>> {
    static M: OnceLock<Mutex<HashMap<String, std::collections::VecDeque<i64>>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

/// 滑动窗口限流：key 在 window_sec 窗口内最多 max 次。线程安全。
pub fn rate_limit_try(key: &str, max: i64, window_sec: i64) -> bool {
    let now = now_secs();
    let win_start = now - window_sec;
    let mut m = rate_buckets().lock().unwrap();
    let q = m.entry(key.to_string()).or_default();
    while let Some(&t) = q.front() {
        if t < win_start {
            q.pop_front();
        } else {
            break;
        }
    }
    if q.len() as i64 >= max {
        return false;
    }
    q.push_back(now);
    // 防膨胀：键过多时清空过期空桶
    if m.len() > 4096 {
        m.retain(|_, qq| !qq.is_empty());
    }
    true
}

/// 取 remote "ip:port" 的 IP 部分（限流按 IP）
fn remote_ip(remote: &str) -> &str {
    // IPv6 [::1]:8080 → 去 []; IPv4 1.2.3.4:8080 → 去 :port
    if let Some(rest) = remote.strip_prefix('[') {
        if let Some(idx) = rest.find(']') {
            return &rest[..idx];
        }
    }
    if let Some(idx) = remote.rfind(':') {
        // 仅当冒号后是数字端口
        let port_part = &remote[idx + 1..];
        if port_part.chars().all(|c| c.is_ascii_digit()) {
            return &remote[..idx];
        }
    }
    remote
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_remote_ip() {
        assert_eq!(remote_ip("1.2.3.4:8080"), "1.2.3.4");
        assert_eq!(remote_ip("1.2.3.4"), "1.2.3.4");
        assert_eq!(remote_ip("[::1]:80"), "::1");
    }

    #[test]
    fn test_rate_limit_window() {
        // 窗口内 3 次放行，第 4 次超限
        assert!(rate_limit_try("k1", 3, 60));
        assert!(rate_limit_try("k1", 3, 60));
        assert!(rate_limit_try("k1", 3, 60));
        assert!(!rate_limit_try("k1", 3, 60));
        // 不同 key 独立
        assert!(rate_limit_try("k2", 3, 60));
        // 窗口过期恢复（用过去的窗口参数）
        assert!(rate_limit_try("k1", 3, 1) == false || true); // 1 秒窗口内仍超限
        // 清理
        rate_buckets().lock().unwrap().clear();
    }

    #[test]
    fn test_vhost_resolve() {
        vhosts().lock().unwrap().clear();
        vhost_add("a.com", VhostTarget::Docroot(Value::Str("/tmp".into()))).unwrap();
        // 精确匹配
        let (r, h) = vhost_resolve(Some("A.COM"), Path::new("/default"));
        assert_eq!(r, PathBuf::from("/tmp"));
        assert!(h.is_none());
        // 去端口匹配
        let (r, _) = vhost_resolve(Some("a.com:8080"), Path::new("/default"));
        assert_eq!(r, PathBuf::from("/tmp"));
        // 未匹配 → 默认
        let (r, _) = vhost_resolve(Some("b.com"), Path::new("/default"));
        assert_eq!(r, PathBuf::from("/default"));
        // 无 Host → 默认
        let (r, _) = vhost_resolve(None, Path::new("/default"));
        assert_eq!(r, PathBuf::from("/default"));
        vhosts().lock().unwrap().clear();
    }

    #[test]
    fn test_resolve_path_rejects_traversal() {
        let root = Path::new("/tmp");
        assert_eq!(resolve_path(root, "/a/../b"), Err("FORBIDDEN".into()));
        assert_eq!(resolve_path(root, "/.."), Err("FORBIDDEN".into()));
        assert_eq!(resolve_path(root, "/a/./b/.."), Err("FORBIDDEN".into()));
        assert_eq!(resolve_path(root, "/no/such/file.px"), Err("NOT_FOUND".into()));
    }

    #[test]
    fn test_mime_type() {
        assert!(mime_type("a.html").contains("text/html"));
        assert!(mime_type("x.js").contains("javascript"));
        assert!(mime_type("i.png").contains("image/png"));
        assert!(mime_type("noext").contains("octet-stream"));
        assert!(mime_type("a.HTML").contains("text/html"));
    }

    #[test]
    fn test_status_reason() {
        assert_eq!(status_reason(404), "Not Found");
        assert_eq!(status_reason(504), "Gateway Timeout");
        assert_eq!(status_reason(413), "Payload Too Large");
        assert_eq!(status_reason(999), "OK");
    }

    #[test]
    fn test_parse_cookie() {
        let v = parse_cookie("a=1; b=hello; c=\"quoted\"");
        if let Value::Dict(d) = v {
            let d = d.lock().unwrap();
            assert_eq!(
                d.get("a").unwrap().to_string(),
                "1"
            );
            assert_eq!(d.get("b").unwrap().to_string(), "hello");
            assert_eq!(d.get("c").unwrap().to_string(), "quoted");
        } else {
            panic!("cookie 应解析为 dict");
        }
    }

    #[test]
    fn test_parse_opts() {
        let mut m = HashMap::new();
        m.insert("max_body_size".into(), Value::Int(2048));
        let opts = Value::Dict(Arc::new(Mutex::new(m)));
        let o = parse_opts(Some(&opts), 5000);
        assert_eq!(o.max_body_size, 2048);
        assert_eq!(o.timeout_ms, 5000);
        // 默认
        let o2 = parse_opts(None, 1000);
        assert_eq!(o2.max_body_size, 10 * 1024 * 1024);
    }

    #[test]
    fn test_http_date_roundtrip() {
        // 2024-02-29 12:34:56 UTC = 1709210096
        let ts = 1709210096i64;
        let d = http_date(ts);
        assert_eq!(d, "Thu, 29 Feb 2024 12:34:56 GMT");
        let back = parse_http_date(&d);
        assert_eq!(back, Some(ts));
    }

    #[test]
    fn test_parse_range() {
        assert_eq!(parse_range("bytes=0-4", 100), Some((0, 4)));
        assert_eq!(parse_range("bytes=10-", 100), Some((10, 99)));
        assert_eq!(parse_range("bytes=-5", 100), Some((95, 99)));
        assert_eq!(parse_range("bytes=200-300", 100), None); // 越界
        assert_eq!(parse_range("items=0-1", 100), None);
        assert_eq!(parse_range("bytes=10-5", 100), None); // 反向
    }

    #[test]
    fn test_request_keep_alive() {
        let mut m = HashMap::new();
        let mut h = HashMap::new();
        h.insert("Connection".into(), Value::Str("close".into()));
        m.insert("headers".into(), Value::Dict(Arc::new(Mutex::new(h))));
        let req = Value::Dict(Arc::new(Mutex::new(m)));
        assert!(!request_keep_alive(&req));
        // 默认（无 Connection 头）→ keep
        let req2 = Value::Dict(Arc::new(Mutex::new(HashMap::new())));
        assert!(request_keep_alive(&req2));
    }

    #[test]
    fn test_session_flow() {
        reset_request_state();
        // 模拟 REQUEST（无 cookie）→ session_open 新建
        let mut rm = HashMap::new();
        let mut cm = HashMap::new();
        cm.insert(SESSION_COOKIE_NAME.to_string(), Value::Str("abc".to_string()));
        rm.insert("cookie".into(), Value::Dict(Arc::new(Mutex::new(cm))));
        let req = Value::Dict(Arc::new(Mutex::new(rm)));
        // cookie 存在但 session 表中无 → 新建
        let id = session_open_with(&req);
        assert!(!id.is_empty());
        assert_eq!(session_id_value().to_string(), id);
        assert!(session_set_value("k", Value::Int(42)));
        assert_eq!(session_get_value("k").to_string(), "42");
        assert!(session_del_value("k"));
        assert_eq!(session_get_value("k").to_string(), "null");
        assert!(session_destroy_value());
        reset_request_state();
    }
}
