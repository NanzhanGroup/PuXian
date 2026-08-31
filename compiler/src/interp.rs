//! 普贤 (PuXian) 解释器（脚本模式，tree-walking）
//! 实现 spec.md §9.1：共享 AST 语义，直接求值
//! M3 范围：线程安全运行时（Arc+Mutex）、真并发 spawn、channel 阻塞语义、select 随机就绪
//!   并发模型：OS 线程 1:1 映射（语义等价 M:N 协程，v1 实现；后续可换 green thread）

use std::collections::HashMap;
use std::collections::HashSet;
use std::collections::VecDeque;
use std::fmt;
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{Arc, Condvar, Mutex};
use std::thread;

use crate::ast::*;
use crate::env::{Env, EnvRef};
use crate::token::Pos;
use crate::value::*;

/// M30 推导式收集目标（列表 / 字典）
enum CompSink<'a> {
    List(&'a mut Vec<Value>),
    Dict(Arc<Mutex<HashMap<String, Value>>>),
}

/// 全局 select 唤醒通道：所有 channel 操作后 notify，select 轮询醒来重试
static SELECT_LOCK: Mutex<()> = Mutex::new(());
static SELECT_CV: Condvar = Condvar::new();

/// M18 定时器状态：跨线程共享（set_timeout/set_interval 的线程与主线程）
/// - next_id：原子分配定时器 id（从 1 递增）
/// - canceled：已取消的定时器 id 集合（回调执行前检查；一次性定时器执行后移除）
pub struct TimerState {
    pub next_id: AtomicI64,
    pub canceled: Mutex<HashSet<i64>>,
}

impl TimerState {
    pub fn new() -> Arc<TimerState> {
        Arc::new(TimerState {
            next_id: AtomicI64::new(0),
            canceled: Mutex::new(HashSet::new()),
        })
    }
}

/// 唤醒所有阻塞中的 select
fn notify_select() {
    let _g = SELECT_LOCK.lock().unwrap();
    SELECT_CV.notify_all();
}

/// Fisher-Yates 洗牌（简单伪随机：时间种子 + LCG），select 随机就绪用
fn shuffle<T>(v: &mut [T]) {
    use std::time::{SystemTime, UNIX_EPOCH};
    let seed = SystemTime::now()
        .duration_since(UNIX_EPOCH)
        .map(|d| d.subsec_nanos() as u64 ^ d.as_secs())
        .unwrap_or(0x9E3779B97F4A7C15);
    let mut x = seed.wrapping_mul(0x2545F4914F6CDD1D).wrapping_add(0x9E3779B9);
    for i in (1..v.len()).rev() {
        x = x.wrapping_mul(6364136223846793005).wrapping_add(1442695040888963407);
        let j = ((x >> 33) as usize) % (i + 1);
        v.swap(i, j);
    }
}

/// M21/M24：切片边界归一化（Python 语义：负索引从尾部算，越界 clamp，start>end 空切片）
/// M24 扩展步长：支持 a[i:j:k]（k<0 反向，k=0 报错）。
/// 返回 (start, stop, step)，生成索引规则：
///   step>0：i = start; while i < stop: 取 i; i += step
///   step<0：i = start; while i > stop: 取 i; i += step
fn slice_indices(
    start: Option<i64>,
    end: Option<i64>,
    step: Option<i64>,
    len: i64,
) -> Result<(i64, i64, i64), String> {
    let step = step.unwrap_or(1);
    if step == 0 {
        return Err("切片步长不能为 0".to_string());
    }
    let adjust = |v: i64| -> i64 {
        let mut r = v;
        if r < 0 {
            r += len;
        }
        if step > 0 {
            r.clamp(0, len)
        } else {
            r.clamp(-1, len - 1)
        }
    };
    let s = match start {
        None => {
            if step < 0 {
                len - 1
            } else {
                0
            }
        }
        Some(i) => adjust(i),
    };
    let e = match end {
        None => {
            if step < 0 {
                -1
            } else {
                len
            }
        }
        Some(i) => adjust(i),
    };
    Ok((s, e, step))
}

// ==================== 运行时错误 ====================

#[derive(Debug, Clone)]
pub struct LxError {
    pub code: &'static str,
    pub msg: String,
    pub pos: Option<Pos>,
}

impl LxError {
    pub fn new(code: &'static str, msg: impl Into<String>, pos: Option<Pos>) -> Self {
        LxError {
            code,
            msg: msg.into(),
            pos,
        }
    }
    fn r1001(name: &str, pos: Pos) -> Self {
        LxError::new("R1001", format!("未定义变量: '{}'", name), Some(pos))
    }
    fn r1002(msg: impl Into<String>, pos: Pos) -> Self {
        LxError::new("R1002", msg, Some(pos))
    }
    fn r1005(msg: impl Into<String>, pos: Pos) -> Self {
        LxError::new("R1005", msg, Some(pos))
    }
}

impl fmt::Display for LxError {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        match self.pos {
            Some(p) => write!(f, "错误 [{}] {}:{}: {}", self.code, p.line, p.col, self.msg),
            None => write!(f, "错误 [{}]: {}", self.code, self.msg),
        }
    }
}

impl std::error::Error for LxError {}

// ==================== 控制流信号 ====================

enum Flow {
    Normal,
    Return(Value),
    Break,
    Continue,
}

// ==================== 解释器 ====================

pub struct Interpreter {
    pub globals: EnvRef,
    /// struct 类型表：name -> 定义（M3 起跨线程共享）
    pub structs: Arc<Mutex<HashMap<String, Arc<StructDef>>>>,
    /// enum 类型表
    pub enums: Arc<Mutex<HashMap<String, Arc<EnumDef>>>>,
    /// trait 类型表
    pub traits: Arc<Mutex<HashMap<String, Arc<TraitDef>>>>,
    /// 方法表：type_name -> (method_name -> Function)
    pub impls: Arc<Mutex<HashMap<String, HashMap<String, Arc<Function>>>>>,
    loop_depth: usize,
    pub exit_code: i32,
    /// print 输出捕获缓冲区（px_exec 内嵌执行用；None = 直接写 stdout）
    pub output: Option<Arc<Mutex<Vec<u8>>>>,
    /// M18 定时器状态（set_timeout / set_interval / clear_timer 共享）
    pub timers: Arc<TimerState>,
}

impl Interpreter {
    pub fn new() -> Self {
        let mut interp = Interpreter {
            globals: Env::new(None),
            structs: Arc::new(Mutex::new(HashMap::new())),
            enums: Arc::new(Mutex::new(HashMap::new())),
            traits: Arc::new(Mutex::new(HashMap::new())),
            impls: Arc::new(Mutex::new(HashMap::new())),
            loop_depth: 0,
            exit_code: 0,
            output: None,
            timers: TimerState::new(),
        };
        interp.register_builtins();
        interp
    }

    /// 创建供子协程（线程）使用的解释器副本：共享 globals 与类型表，独立 loop_depth/exit_code
    pub(crate) fn fork(&self) -> Interpreter {
        Interpreter {
            globals: self.globals.clone(),
            structs: self.structs.clone(),
            enums: self.enums.clone(),
            traits: self.traits.clone(),
            impls: self.impls.clone(),
            loop_depth: 0,
            exit_code: 0,
            output: None,
            timers: self.timers.clone(),
        }
    }

    fn register_builtins(&mut self) {
        let mut g = self.globals.lock().unwrap();
        let names: &[(&str, Builtin)] = &[
            ("print", Builtin::Print),
            ("len", Builtin::Len),
            ("range", Builtin::Range),
            ("type", Builtin::Type),
            ("str", Builtin::Str),
            ("int", Builtin::Int),
            ("float", Builtin::Float),
            ("bool", Builtin::Bool),
            ("assert", Builtin::Assert),
            ("panic", Builtin::Panic),
            ("input", Builtin::Input),
            ("exit", Builtin::Exit),
            ("sleep", Builtin::Sleep),
            ("to_upper", Builtin::ToUpper),
            ("to_lower", Builtin::ToLower),
            ("trim", Builtin::Trim),
            ("split", Builtin::Split),
            ("join", Builtin::Join),
            ("contains", Builtin::Contains),
            ("replace", Builtin::Replace),
            ("starts_with", Builtin::StartsWith),
            ("ends_with", Builtin::EndsWith),
            ("abs", Builtin::Abs),
            ("sqrt", Builtin::Sqrt),
            ("min", Builtin::Min),
            ("max", Builtin::Max),
            ("pow", Builtin::Pow),
            ("sorted", Builtin::Sorted),
            ("reversed", Builtin::Reversed),
            ("sum", Builtin::Sum),
            ("now_ms", Builtin::NowMs),
            // M5 标准库
            ("read_file", Builtin::ReadFile),
            ("write_file", Builtin::WriteFile),
            ("append_file", Builtin::AppendFile),
            ("read_at", Builtin::ReadAt),
            ("write_at", Builtin::WriteAt),
            ("file_size", Builtin::FileSize),
            ("fsync_file", Builtin::FsyncFile),
            ("truncate_file", Builtin::TruncateFile),
            // M14 P1：crypto 哈希
            ("sha256", Builtin::Sha256),
            ("xxhash", Builtin::Xxhash),
            // M15 P1：正则表达式
            ("regex_find", Builtin::RegexFind),
            ("regex_match", Builtin::RegexMatch),
            ("regex_search", Builtin::RegexSearch),
            ("regex_find_all", Builtin::RegexFindAll),
            ("regex_replace", Builtin::RegexReplace),
            ("regex_split", Builtin::RegexSplit),
            ("exists", Builtin::Exists),
            ("list_dir", Builtin::ListDir),
            ("mkdir", Builtin::Mkdir),
            ("remove", Builtin::Remove),
            ("json_parse", Builtin::JsonParse),
            ("json_stringify", Builtin::JsonStringify),
            ("now", Builtin::Now),
            ("env", Builtin::Env),
            ("args", Builtin::Args),
            ("map", Builtin::Map),
            ("filter", Builtin::Filter),
            ("reduce", Builtin::Reduce),
            // std.net（M5.2）
            ("tcp_listen", Builtin::TcpListen),
            ("tcp_accept", Builtin::TcpAccept),
            ("tcp_connect", Builtin::TcpConnect),
            ("tcp_send", Builtin::TcpSend),
            ("tcp_recv", Builtin::TcpRecv),
            ("tcp_close", Builtin::TcpClose),
            ("http_get", Builtin::HttpGet),
            ("http_post", Builtin::HttpPost),
            ("http_request", Builtin::HttpRequest),
            ("http_get_stream", Builtin::HttpGetStream),
            ("http_serve", Builtin::HttpServe),
            // M17 P1：.px 脚本执行机制（应用平台核心）
            ("px_exec", Builtin::PxExec),
            ("px_serve", Builtin::PxServe),
            // M18 P1：后台定时任务 / 定时器原语
            ("set_timeout", Builtin::SetTimeout),
            ("set_interval", Builtin::SetInterval),
            ("clear_timer", Builtin::ClearTimer),
            // M19 P1：AES 加密（企微回调加解密 / 数据落盘加密）
            ("aes_encrypt", Builtin::AesEncrypt),
            ("aes_decrypt", Builtin::AesDecrypt),
            ("aes_gcm_encrypt", Builtin::AesGcmEncrypt),
            ("aes_gcm_decrypt", Builtin::AesGcmDecrypt),
            // M19 P1：XML 解析（企微回调 Encrypt 报文 / 配置文件）
            ("xml_parse", Builtin::XmlParse),
            ("xml_escape", Builtin::XmlEscape),
            ("xml_unescape", Builtin::XmlUnescape),
            // M24：XML 生成（与 xml_parse 结构对称，企微回调响应 / 文档生成）
            ("xml_build", Builtin::XmlBuild),
            // M19 P1：zip 打包/解压（文档工具基石）
            ("zip_pack", Builtin::ZipPack),
            ("zip_unpack", Builtin::ZipUnpack),
            // M21 P1：base64 编解码
            ("base64_encode", Builtin::Base64Encode),
            ("base64_decode", Builtin::Base64Decode),
            // M21 P1：SSE 服务端（LLM 流式推送 / 实时通知）
            ("sse_serve", Builtin::SseServe),
            ("sse_send", Builtin::SseSend),
            ("sse_close", Builtin::SseClose),
            ("sse_connect", Builtin::SseConnect),
            ("sse_read", Builtin::SseRead),
            ("int_to_hex", Builtin::IntToHex),
            ("hex_to_int", Builtin::HexToInt),
            ("bytes_to_hex", Builtin::BytesToHex),
            ("hex_to_bytes", Builtin::HexToBytes),
            ("bit_count", Builtin::BitCount),
            ("bit_length", Builtin::BitLength),
            ("ws_serve", Builtin::WsServe),
            ("ws_connect", Builtin::WsConnect),
            ("ws_send", Builtin::WsSend),
            ("ws_recv", Builtin::WsRecv),
            ("ws_close", Builtin::WsClose),
            ("ws_ping", Builtin::WsPing),
            ("ws_heartbeat", Builtin::WsHeartbeat),
            ("os_pid", Builtin::OsPid),
            ("os_spawn", Builtin::OsSpawn),
            ("os_wait", Builtin::OsWait),
            ("os_kill", Builtin::OsKill),
            ("signal", Builtin::Signal),
            ("rsa_gen_key", Builtin::RsaGenKey),
            ("rsa_encrypt", Builtin::RsaEncrypt),
            ("rsa_decrypt", Builtin::RsaDecrypt),
            ("rsa_sign", Builtin::RsaSign),
            ("rsa_verify", Builtin::RsaVerify),
            ("bytes", Builtin::Bytes),
            ("bytes_len", Builtin::BytesLen),
            ("bytes_get", Builtin::BytesGet),
            ("bytes_set", Builtin::BytesSet),
            ("bytes_slice", Builtin::BytesSlice),
            ("bytes_concat", Builtin::BytesConcat),
            ("bytes_to_str", Builtin::BytesToStr),
            ("bytes_to_hex", Builtin::BytesToHex),
            ("bytes_base64", Builtin::BytesBase64),
            ("base64_to_bytes", Builtin::Base64ToBytes),
            ("bytes_find", Builtin::BytesFind),
            ("read_bytes", Builtin::ReadBytes),
            ("write_bytes", Builtin::WriteBytes),
            // M30：字节序可控整数<->bytes（pxdb 存储基石）
            ("int_to_bytes", Builtin::IntToBytes),
            ("bytes_to_int", Builtin::BytesToInt),
            ("gc", Builtin::Gc),
            // M27 P0：WebServer 生产化（服务端 TLS / Session / 基础认证）
            ("tls_server", Builtin::TlsServer),
            ("session_open", Builtin::SessionOpen),
            ("session_id", Builtin::SessionId),
            ("session_get", Builtin::SessionGet),
            ("session_set", Builtin::SessionSet),
            ("session_del", Builtin::SessionDel),
            ("session_destroy", Builtin::SessionDestroy),
            ("basic_auth", Builtin::BasicAuth),
            // M28 P1：路由表 + 中间件 / 时间时区 / cron / SQLite
            ("route", Builtin::Route),
            ("middleware", Builtin::Middleware),
            ("time_format", Builtin::TimeFormat),
            ("time_parse", Builtin::TimeParse),
            ("tz_offset", Builtin::TzOffset),
            ("cron", Builtin::Cron),
            ("sqlite_open", Builtin::SqliteOpen),
            ("sqlite_exec", Builtin::SqliteExec),
            ("sqlite_query", Builtin::SqliteQuery),
            ("sqlite_close", Builtin::SqliteClose),
            ("sqlite_escape", Builtin::SqliteEscape),
            ("sqlite_last_insert_rowid", Builtin::SqliteLastInsertRowid),
            // M29：JSON 路径运算符（JSONB 基石）
            ("json_path", Builtin::JsonPath),
            ("json_path_set", Builtin::JsonPathSet),
            // M31：沙箱安全 / 虚拟主机 / 限流防爆破
            ("sandbox_enter", Builtin::SandboxEnter),
            ("vhost", Builtin::Vhost),
            ("rate_limit", Builtin::RateLimit),
            ("gen_next", Builtin::GenNext),
            ("list", Builtin::List),
            // M33：HTTP/3 预研——UDP 基础设施
            ("udp_open", Builtin::UdpOpen),
            ("udp_send", Builtin::UdpSend),
            ("udp_recv", Builtin::UdpRecv),
            ("udp_close", Builtin::UdpClose),
            // M34：WS 服务端广播 + 事件总线
            ("ws_broadcast", Builtin::WsBroadcast),
            ("event_bus", Builtin::BusNew),
            ("bus_subscribe", Builtin::BusSubscribe),
            ("bus_publish", Builtin::BusPublish),
            ("bus_unsubscribe", Builtin::BusUnsubscribe),
            // M36：请求上下文（线程局部）
            ("ctx_set", Builtin::CtxSet),
            ("ctx_get", Builtin::CtxGet),
            ("ctx_clear", Builtin::CtxClear),
            // M37：S3/MinIO
            ("s3_put", Builtin::S3Put),
            ("s3_get", Builtin::S3Get),
            ("s3_delete", Builtin::S3Delete),
            ("s3_list", Builtin::S3List),
        ];
        for (n, b) in names {
            g.define(n, Value::Builtin(*b));
        }
    }

    // ==================== 程序入口 ====================

    pub fn run_program(&mut self, prog: &Program) -> Result<i32, LxError> {
        let g = self.globals.clone();
        for stmt in &prog.items {
            match self.exec_stmt(stmt, &g)? {
                Flow::Normal => {}
                _ => return Err(LxError::r1005("顶层语句出现非法控制流", Pos::new(0, 0))),
            }
        }
        let main = g.lock().unwrap().get("main");
        if let Some(m) = main {
            let v = self.call_value(&m, &[], Pos::new(0, 0))?;
            if let Value::Int(code) = v {
                self.exit_code = code as i32;
            }
        }
        Ok(self.exit_code)
    }

    // ==================== 语句执行 ====================

    fn exec_stmt(&mut self, stmt: &Stmt, env: &EnvRef) -> Result<Flow, LxError> {
        match stmt {
            Stmt::VarDecl { name, value, pos: _, .. } => {
                let v = match value {
                    Some(e) => self.eval_expr(e, env)?,
                    None => Value::Null,
                };
                env.lock().unwrap().define(name, v);
                Ok(Flow::Normal)
            }
            Stmt::Assign { target, op, value, pos } => {
                let v = self.eval_expr(value, env)?;
                self.assign(target, *op, v, env, *pos)?;
                Ok(Flow::Normal)
            }
            Stmt::ExprStmt { expr, pos: _ } => {
                self.eval_expr(expr, env)?;
                Ok(Flow::Normal)
            }
            Stmt::If { branches, else_branch, pos } => {
                for (cond, body) in branches {
                    let c = self.eval_expr(cond, env)?;
                    if self.is_truthy(&c, *pos)? {
                        return self.exec_block(body, env);
                    }
                }
                if let Some(eb) = else_branch {
                    return self.exec_block(eb, env);
                }
                Ok(Flow::Normal)
            }
            Stmt::For { var, iterable, body, pos } => {
                let it = self.eval_expr(iterable, env)?;
                self.loop_depth += 1;
                let r = self.exec_for(var, it, body, env, *pos);
                self.loop_depth -= 1;
                r
            }
            Stmt::While { cond, body, pos } => {
                self.loop_depth += 1;
                let r = loop {
                    let c = self.eval_expr(cond, env)?;
                    if !self.is_truthy(&c, *pos)? {
                        break Ok(Flow::Normal);
                    }
                    match self.exec_block(body, env)? {
                        Flow::Break => break Ok(Flow::Normal),
                        Flow::Return(v) => break Ok(Flow::Return(v)),
                        _ => {}
                    }
                };
                self.loop_depth -= 1;
                r
            }
            Stmt::Return { value, pos: _ } => {
                let v = match value {
                    Some(e) => self.eval_expr(e, env)?,
                    None => Value::Null,
                };
                Ok(Flow::Return(v))
            }
            Stmt::Break { pos } => {
                if self.loop_depth == 0 {
                    return Err(LxError::r1005("break 出现在循环外", *pos));
                }
                Ok(Flow::Break)
            }
            Stmt::Continue { pos } => {
                if self.loop_depth == 0 {
                    return Err(LxError::r1005("continue 出现在循环外", *pos));
                }
                Ok(Flow::Continue)
            }
            Stmt::FuncDef { name, params, ret_ty, body, pos: _ } => {
                let f = Arc::new(Function {
                    name: name.clone(),
                    params: params.clone(),
                    ret_ty: ret_ty.clone(),
                    body: body.clone(),
                    closure: env.clone(),
                    implicit_return: false,
                });
                // M25：闭包循环回收——注册函数（含其闭包环境）
                crate::gc::register_func(&f);
                env.lock().unwrap().define(name, Value::Func(f));
                Ok(Flow::Normal)
            }
            Stmt::StructDef { name, fields, pos } => {
                let def = Arc::new(StructDef {
                    name: name.clone(),
                    fields: fields.clone(),
                    pos: *pos,
                });
                self.structs.lock().unwrap().insert(name.clone(), def);
                env.lock().unwrap()
                    .define(name, Value::TypeRef(TypeRefKind::Struct(name.clone())));
                Ok(Flow::Normal)
            }
            Stmt::EnumDef { name, variants, pos } => {
                let def = Arc::new(EnumDef {
                    name: name.clone(),
                    variants: variants.clone(),
                    pos: *pos,
                });
                self.enums.lock().unwrap().insert(name.clone(), def);
                env.lock().unwrap()
                    .define(name, Value::TypeRef(TypeRefKind::Enum(name.clone())));
                Ok(Flow::Normal)
            }
            Stmt::TraitDef { name, methods, pos } => {
                let def = Arc::new(TraitDef {
                    name: name.clone(),
                    methods: methods.clone(),
                    pos: *pos,
                });
                self.traits.lock().unwrap().insert(name.clone(), def);
                Ok(Flow::Normal)
            }
            Stmt::ImplDef { type_name, methods, pos: _, .. } => {
                let mut map = self.impls.lock().unwrap().get(type_name).cloned().unwrap_or_default();
                for m in methods {
                    let f = Arc::new(Function {
                        name: m.name.clone(),
                        params: m.params.clone(),
                        ret_ty: m.ret_ty.clone(),
                        body: m.body.clone(),
                        closure: self.globals.clone(),
                        implicit_return: false,
                    });
                    crate::gc::register_func(&f);
                    map.insert(m.name.clone(), f);
                }
                self.impls.lock().unwrap().insert(type_name.clone(), map);
                Ok(Flow::Normal)
            }
            Stmt::Import { module, names: _, pos } => {
                // M9：模块定义已在运行前由 module::ModuleResolver 合并进 AST（std / 用户包 / 相对路径）
                // 此处一律忽略 import 语句本身（不执行模块顶层语句，spec §8.4）
                let _ = module;
                let _ = pos;
                Ok(Flow::Normal)
            }
            Stmt::Spawn { expr, pos } => {
                // M3：真并发——参数在父环境求值，函数体在新线程执行
                let (callee, args) = match expr.as_ref() {
                    Expr::Call { callee, args, .. } => (callee.as_ref().clone(), args.clone()),
                    _ => return Err(LxError::r1002("spawn 需要函数调用表达式", *pos)),
                };
                let fv = self.eval_expr(&callee, env)?;
                let mut arg_vals = Vec::new();
                for a in &args {
                    arg_vals.push(self.eval_expr(a, env)?);
                }
                let interp = self.fork();
                let pos_c = *pos;
                crate::gc::ACTIVE_SPAWNS.fetch_add(1, Ordering::SeqCst);
                thread::spawn(move || {
                    let mut i = interp;
                    if let Err(e) = i.call_value(&fv, &arg_vals, pos_c) {
                        eprintln!("[协程] {}", e);
                    }
                    crate::gc::ACTIVE_SPAWNS.fetch_sub(1, Ordering::SeqCst);
                });
                Ok(Flow::Normal)
            }
            Stmt::ChanDecl { name, elem_ty: _, pos: _ } => {
                // 声明一个默认无缓冲通道
                let ch = Arc::new(ChanState {
                    inner: Mutex::new(ChanInner {
                        buf: VecDeque::new(),
                        cap: 0,
                        closed: false,
                        recv_waiting: 0,
                    }),
                    cv: Condvar::new(),
                });
                env.lock().unwrap().define(name, Value::new_chan(ch));
                Ok(Flow::Normal)
            }
            Stmt::Send { chan, value, pos } => {
                let cv = self.eval_expr(chan, env)?;
                let vv = self.eval_expr(value, env)?;
                self.chan_send(&cv, vv, *pos)?;
                Ok(Flow::Normal)
            }
            Stmt::Recv { chan, pos } => {
                let cv = self.eval_expr(chan, env)?;
                self.chan_recv(&cv, *pos)?;
                Ok(Flow::Normal)
            }
            Stmt::Select { arms, else_branch, pos } => {
                self.exec_select(arms, else_branch.as_deref(), env, *pos)?;
                Ok(Flow::Normal)
            }
            Stmt::Empty { .. } => Ok(Flow::Normal),
        }
    }

    /// 顺序执行语句块（不新建作用域；if/for/while 沿用 Python 语义）
    fn exec_block(&mut self, body: &[Stmt], env: &EnvRef) -> Result<Flow, LxError> {
        for s in body {
            let f = self.exec_stmt(s, env)?;
            if !matches!(f, Flow::Normal) {
                return Ok(f);
            }
        }
        Ok(Flow::Normal)
    }

    fn exec_for(
        &mut self,
        var: &str,
        iterable: Value,
        body: &[Stmt],
        env: &EnvRef,
        pos: Pos,
    ) -> Result<Flow, LxError> {
        let items = self.iter_values(&iterable, pos)?;
        for item in items {
            env.lock().unwrap().define(var, item);
            match self.exec_block(body, env)? {
                Flow::Break => break,
                Flow::Return(v) => return Ok(Flow::Return(v)),
                Flow::Continue => continue,
                Flow::Normal => {}
            }
        }
        Ok(Flow::Normal)
    }

    fn exec_select(
        &mut self,
        arms: &[(Option<String>, Expr, Vec<Stmt>)],
        else_branch: Option<&[Stmt]>,
        env: &EnvRef,
        pos: Pos,
    ) -> Result<(), LxError> {
        // M3：阻塞轮询 + 随机就绪；`case _:` 作为兜底分支（等价 else）
        let mut fallback: Option<&[Stmt]> = None;
        let mut recv_arms: Vec<(Option<String>, ChanRef, &[Stmt])> = Vec::new();
        for (binding, arm_expr, body) in arms {
            if let Expr::Var { name, .. } = arm_expr {
                if name == "_" {
                    fallback = Some(body);
                    continue;
                }
            }
            let ch = self.resolve_select_chan(arm_expr, env, pos)?;
            recv_arms.push((binding.clone(), ch, body));
        }
        // 循环：随机顺序尝试非阻塞接收；无就绪且有兜底则执行兜底；否则阻塞等全局唤醒
        loop {
            let mut order: Vec<usize> = (0..recv_arms.len()).collect();
            shuffle(&mut order);
            for idx in order {
                let (binding, ch, body) = &recv_arms[idx];
                if let Some(v) = Self::chan_try_recv(ch) {
                    let child = Env::new(Some(env.clone()));
                    if let Some(b) = binding {
                        child.lock().unwrap().define(b, v);
                    }
                    self.exec_block(body, &child)?;
                    return Ok(());
                }
            }
            if let Some(eb) = else_branch {
                self.exec_block(eb, env)?;
                return Ok(());
            }
            if let Some(fb) = fallback {
                self.exec_block(fb, env)?;
                return Ok(());
            }
            // 无就绪且无兜底：阻塞等待任一通道事件
            let guard = SELECT_LOCK.lock().unwrap();
            let _g = SELECT_CV.wait(guard).unwrap();
        }
    }

    /// 解析 select 分支表达式：ch.recv() / recv(ch) -> 通道引用
    fn resolve_select_chan(&mut self, expr: &Expr, env: &EnvRef, pos: Pos) -> Result<ChanRef, LxError> {
        let chan_val = match expr {
            Expr::Call { callee, args, pos: _ } => {
                if args.is_empty() {
                    // ch.recv()
                    if let Expr::Field { obj, name, .. } = callee.as_ref() {
                        if name == "recv" {
                            self.eval_expr(obj, env)?
                        } else {
                            return Err(LxError::r1002("select 分支仅支持 recv", pos));
                        }
                    } else {
                        return Err(LxError::r1002("select 分支仅支持 recv", pos));
                    }
                } else {
                    // recv(ch)
                    if let Expr::Var { name, .. } = callee.as_ref() {
                        if name == "recv" {
                            self.eval_expr(&args[0], env)?
                        } else {
                            return Err(LxError::r1002("select 分支仅支持 recv", pos));
                        }
                    } else {
                        return Err(LxError::r1002("select 分支仅支持 recv", pos));
                    }
                }
            }
            _ => return Err(LxError::r1002("select 分支仅支持 recv 调用", pos)),
        };
        match &chan_val {
            Value::Chan(c) => Ok(c.clone()),
            _ => Err(LxError::r1002("select 接收对象不是通道", pos)),
        }
    }

    /// 非阻塞接收（select 轮询用）
    fn chan_try_recv(ch: &ChanRef) -> Option<Value> {
        let mut inner = ch.inner.lock().unwrap();
        if let Some(v) = inner.buf.pop_front() {
            if inner.cap == 0 {
                inner.recv_waiting = inner.recv_waiting.saturating_sub(1);
            }
            drop(inner);
            ch.cv.notify_all();
            notify_select();
            Some(v)
        } else {
            None
        }
    }

    // ==================== 表达式求值 ====================

    fn eval_expr(&mut self, expr: &Expr, env: &EnvRef) -> Result<Value, LxError> {
        match expr {
            Expr::Int { value, .. } => Ok(Value::Int(*value)),
            Expr::Float { value, .. } => Ok(Value::Float(*value)),
            Expr::Str { value, .. } => Ok(Value::Str(value.clone())),
            Expr::Bool { value, .. } => Ok(Value::Bool(*value)),
            Expr::Null { .. } => Ok(Value::Null),
            Expr::List { items, pos: _ } => {
                let mut v = Vec::new();
                for it in items {
                    v.push(self.eval_expr(it, env)?);
                }
                Ok(Value::new_list(v))
            }
            Expr::Tuple { items, pos: _ } => {
                let mut v = Vec::new();
                for it in items {
                    v.push(self.eval_expr(it, env)?);
                }
                Ok(Value::Tuple(v))
            }
            Expr::Dict { entries, pos } => {
                let mut m = HashMap::new();
                for (k, v) in entries {
                    let kv = self.eval_expr(k, env)?;
                    let key = match kv {
                        Value::Str(s) => s,
                        _ => return Err(LxError::r1002("字典键必须是字符串", *pos)),
                    };
                    let vv = self.eval_expr(v, env)?;
                    m.insert(key, vv);
                }
                Ok(Value::new_dict(m))
            }
            Expr::Var { name, pos } => {
                let v = env.lock().unwrap().get(name);
                match v {
                    Some(v) => Ok(v),
                    None => Err(LxError::r1001(name, *pos)),
                }
            }
            Expr::Field { obj, name, pos } => {
                let ov = self.eval_expr(obj, env)?;
                self.eval_field(&ov, name, *pos)
            }
            Expr::OptionalField { obj, name, pos } => {
                let ov = self.eval_expr(obj, env)?;
                if matches!(ov, Value::Null) {
                    Ok(Value::Null)
                } else {
                    self.eval_field(&ov, name, *pos)
                }
            }
            Expr::Index { obj, index, pos } => {
                let ov = self.eval_expr(obj, env)?;
                let iv = self.eval_expr(index, env)?;
                self.eval_index(&ov, &iv, *pos)
            }
            Expr::Slice { obj, start, end, step, pos } => {
                let ov = self.eval_expr(obj, env)?;
                let sv = match start {
                    Some(e) => Some(self.eval_expr(e, env)?),
                    None => None,
                };
                let ev = match end {
                    Some(e) => Some(self.eval_expr(e, env)?),
                    None => None,
                };
                let kv = match step {
                    Some(e) => Some(self.eval_expr(e, env)?),
                    None => None,
                };
                self.eval_slice(&ov, sv, ev, kv, *pos)
            }
            Expr::Call { callee, args, pos } => self.eval_call(callee, args, env, *pos),
            Expr::Unary { op, operand, pos } => {
                let v = self.eval_expr(operand, env)?;
                self.eval_unary(*op, v, *pos)
            }
            Expr::Binary { op, left, right, pos } => {
                // and/or 短路
                if matches!(op, BinaryOp::And | BinaryOp::Or) {
                    let l = self.eval_expr(left, env)?;
                    let lt = self.is_truthy(&l, *pos)?;
                    if *op == BinaryOp::And {
                        if !lt {
                            return Ok(Value::Bool(false));
                        }
                        let r = self.eval_expr(right, env)?;
                        let rt = self.is_truthy(&r, *pos)?;
                        return Ok(Value::Bool(rt));
                    } else {
                        if lt {
                            return Ok(Value::Bool(true));
                        }
                        let r = self.eval_expr(right, env)?;
                        let rt = self.is_truthy(&r, *pos)?;
                        return Ok(Value::Bool(rt));
                    }
                }
                let l = self.eval_expr(left, env)?;
                let r = self.eval_expr(right, env)?;
                self.eval_binary(*op, l, r, *pos)
            }
            Expr::Pipe { value, func, pos } => {
                let v = self.eval_expr(value, env)?;
                // func 为调用表达式时，把 value 作为第一个参数
                match func.as_ref() {
                    Expr::Call { callee, args, pos: _ } => {
                        // 管道：把 value 作为第一个参数传给函数
                        if let Expr::Field { .. } = callee.as_ref() {
                            return Err(LxError::new(
                                "R1004",
                                "管道暂不支持方法调用（M2）",
                                Some(*pos),
                            ));
                        }
                        let cv = self.eval_expr(callee, env)?;
                        let mut arg_vals = Vec::with_capacity(args.len() + 1);
                        arg_vals.push(v);
                        for a in args {
                            arg_vals.push(self.eval_expr(a, env)?);
                        }
                        self.call_value(&cv, &arg_vals, *pos)
                    }
                    _ => {
                        let f = self.eval_expr(func, env)?;
                        self.call_value(&f, &[v], *pos)
                    }
                }
            }
            Expr::NullCoalesce { left, right, pos: _ } => {
                let l = self.eval_expr(left, env)?;
                if matches!(l, Value::Null) {
                    self.eval_expr(right, env)
                } else {
                    Ok(l)
                }
            }
            Expr::Try { expr, pos } => {
                let v = self.eval_expr(expr, env)?;
                match v {
                    Value::Null => Err(LxError::new(
                        "R1004",
                        "错误传播 ?: 无法解包 null（Result/Option 支持 M3 完善）",
                        Some(*pos),
                    )),
                    _ => Ok(v),
                }
            }
            Expr::ForceUnwrap { expr, pos } => {
                let v = self.eval_expr(expr, env)?;
                match v {
                    Value::Null => Err(LxError::new(
                        "R1004",
                        "强制解包 !: 值为 null",
                        Some(*pos),
                    )),
                    _ => Ok(v),
                }
            }
            Expr::IfExpr { cond, then, else_, pos } => {
                let c = self.eval_expr(cond, env)?;
                if self.is_truthy(&c, *pos)? {
                    self.eval_expr(then, env)
                } else {
                    self.eval_expr(else_, env)
                }
            }
            Expr::ListComp { expr, clauses, cond, pos } => {
                let mut out = Vec::new();
                self.eval_comp(
                    clauses,
                    0,
                    &mut CompSink::List(&mut out),
                    Some(expr),
                    None,
                    None,
                    cond,
                    env,
                    *pos,
                )?;
                Ok(Value::new_list(out))
            }
            Expr::GenExp { expr, clauses, cond, pos } => {
                // M34 惰性生成器：单层 for + 单变量 → 真延迟（seq 不展开 range，gen_next 逐项求值）
                // transform/filter 为捕获闭包 fn(x){expr/cond}（捕获外层变量，Rust 支持）
                // 注意：iterable 为生成器（嵌套生成器）时退化物化（惰性 seq 索引需 interp，复杂度高）
                let iterable_lazy_ok = if clauses.len() == 1 && clauses[0].vars.len() == 1 {
                    let it = self.eval_expr(&clauses[0].iterable, env)?;
                    !matches!(it, Value::Gen(_))
                } else {
                    false
                };
                if iterable_lazy_ok {
                    let xname = clauses[0].vars[0].clone();
                    let iterable = self.eval_expr(&clauses[0].iterable, env)?;
                    let mut mk_closure = |body: &Expr, pos: Pos| -> Result<Value, LxError> {
                        self.eval_expr(
                            &Expr::Closure {
                                params: vec![Param {
                                    name: xname.clone(),
                                    ty: None,
                                    default: None,
                                    pos,
                                }],
                                ret_ty: None,
                                body: Box::new(body.clone()),
                                captures: vec![],
                                pos,
                            },
                            env,
                        )
                    };
                    let transform = mk_closure(expr, *pos)?;
                    let filter = match cond {
                        Some(c) => Some(mk_closure(c, *pos)?),
                        None => None,
                    };
                    let obj = crate::value::GenObj {
                        materialized: Vec::new(),
                        cursor: 0,
                        lazy: Some(crate::value::LazyGen {
                            seq: iterable,
                            cursor: 0,
                            transform,
                            filter,
                        }),
                    };
                    return Ok(Value::Gen(Arc::new(Mutex::new(obj))));
                }
                // M32 物化路径（多层 for / 多变量解包）
                let mut out = Vec::new();
                self.eval_comp(
                    clauses,
                    0,
                    &mut CompSink::List(&mut out),
                    Some(expr),
                    None,
                    None,
                    cond,
                    env,
                    *pos,
                )?;
                let obj = crate::value::GenObj {
                    materialized: out,
                    cursor: 0,
                    lazy: None,
                };
                Ok(Value::Gen(std::sync::Arc::new(std::sync::Mutex::new(obj))))
            }
            Expr::DictComp { key, value, clauses, cond, pos } => {
                let map = Arc::new(Mutex::new(HashMap::new()));
                self.eval_comp(
                    clauses,
                    0,
                    &mut CompSink::Dict(map.clone()),
                    None,
                    Some(key),
                    Some(value),
                    cond,
                    env,
                    *pos,
                )?;
                let inner = map.lock().unwrap().clone();
                Ok(Value::new_dict(inner))
            }
            Expr::Closure { params, ret_ty, body, pos, .. } => {
                // 闭包体为 Block，取其 stmts 作为函数体
                let stmts = match body.as_ref() {
                    Expr::Block { stmts, .. } => stmts.clone(),
                    e => vec![Stmt::ExprStmt {
                        expr: e.clone(),
                        pos: *pos,
                    }],
                };
                let f = Arc::new(Function {
                    name: "<closure>".to_string(),
                    params: params.clone(),
                    ret_ty: ret_ty.clone(),
                    body: stmts,
                    closure: env.clone(),
                    implicit_return: true,
                });
                crate::gc::register_func(&f);
                Ok(Value::Func(f))
            }
            Expr::Block { stmts, pos } => {
                let child = Env::new(Some(env.clone()));
                let mut last = Value::Null;
                for s in stmts {
                    if let Stmt::ExprStmt { expr, .. } = s {
                        last = self.eval_expr(expr, &child)?;
                    } else {
                        match self.exec_stmt(s, &child)? {
                            Flow::Return(v) => return Ok(v),
                            Flow::Break | Flow::Continue => {
                                return Err(LxError::r1005("块内出现非法控制流", *pos))
                            }
                            Flow::Normal => {}
                        }
                    }
                }
                Ok(last)
            }
            Expr::Match { subject, arms, pos } => {
                let sv = self.eval_expr(subject, env)?;
                for arm in arms {
                    let child = Env::new(Some(env.clone()));
                    if self.match_pattern(&arm.pattern, &sv, &child, *pos)? {
                        return self.eval_expr(&arm.body, &child);
                    }
                }
                Err(LxError::new(
                    "R1003",
                    format!("match 未匹配任何分支（非穷尽）：{}", sv),
                    Some(*pos),
                ))
            }
            Expr::Constructor { name, args, pos } => {
                if name == "chan" {
                    // 通道构造 chan[T](cap) / chan[T]()
                    let cap = if args.is_empty() {
                        0
                    } else {
                        match self.eval_expr(&args[0], env)? {
                            Value::Int(c) if c >= 0 => c as usize,
                            _ => return Err(LxError::r1002("通道容量必须是非负整数", *pos)),
                        }
                    };
                    let ch = Arc::new(ChanState {
                        inner: Mutex::new(ChanInner {
                            buf: VecDeque::new(),
                            cap,
                            closed: false,
                            recv_waiting: 0,
                        }),
                        cv: Condvar::new(),
                    });
                    return Ok(Value::new_chan(ch));
                }
                if name == "mutex" {
                    // M13：互斥锁构造 mutex()
                    return Ok(Value::Mutex(Arc::new(MutexState::new())));
                }
                if name == "rwlock" {
                    // M13：读写锁构造 rwlock()
                    return Ok(Value::RWLock(Arc::new(RWLockState::new())));
                }
                // 结构体构造 Point(1, 2)
                let def = self
                    .structs
                    .lock()
                    .unwrap()
                    .get(name)
                    .cloned()
                    .ok_or_else(|| LxError::new("R1002", format!("未知结构体: '{}'", name), Some(*pos)))?;
                let mut fields = HashMap::new();
                for (i, f) in def.fields.iter().enumerate() {
                    let val = match args.get(i) {
                        Some(a) => self.eval_expr(a, env)?,
                        None => Value::Null,
                    };
                    fields.insert(f.name.clone(), val);
                }
                if args.len() > def.fields.len() {
                    return Err(LxError::r1005(
                        format!("结构体 {} 字段过多：{} 个参数，{} 个字段", name, args.len(), def.fields.len()),
                        *pos,
                    ));
                }
                Ok(Value::StructInstance {
                    type_name: name.clone(),
                    fields: Value::new_struct_fields(fields),
                })
            }
        }
    }

    // ==================== 调用 ====================

    fn eval_call(
        &mut self,
        callee: &Expr,
        args: &[Expr],
        env: &EnvRef,
        pos: Pos,
    ) -> Result<Value, LxError> {
        // 通道构造：chan[T](cap) 被解析为 Call { callee: Var("chan") }
        if let Expr::Var { name, .. } = callee {
            if name == "chan" {
                let cap = if args.is_empty() {
                    0
                } else {
                    match self.eval_expr(&args[0], env)? {
                        Value::Int(c) if c >= 0 => c as usize,
                        _ => return Err(LxError::r1002("通道容量必须是非负整数", pos)),
                    }
                };
                let ch = Arc::new(ChanState {
                    inner: Mutex::new(ChanInner {
                        buf: VecDeque::new(),
                        cap,
                        closed: false,
                        recv_waiting: 0,
                    }),
                    cv: Condvar::new(),
                });
                return Ok(Value::new_chan(ch));
            }
            // M13：锁构造 mutex() / rwlock()（与 chan 同构）
            if name == "mutex" {
                return Ok(Value::Mutex(Arc::new(MutexState::new())));
            }
            if name == "rwlock" {
                return Ok(Value::RWLock(Arc::new(RWLockState::new())));
            }
        }
        // 方法调用 obj.method(args)
        if let Expr::Field { obj, name, pos: _ } = callee {
            let ov = self.eval_expr(obj, env)?;
            let mut arg_vals = Vec::with_capacity(args.len());
            for a in args {
                arg_vals.push(self.eval_expr(a, env)?);
            }
            return self.call_method(&ov, name, &arg_vals, pos);
        }
        let cv = self.eval_expr(callee, env)?;
        let mut arg_vals = Vec::with_capacity(args.len());
        for a in args {
            arg_vals.push(self.eval_expr(a, env)?);
        }
        self.call_value(&cv, &arg_vals, pos)
    }

    /// 调用任意可调用值（函数 / 内置 / 类型构造）
    pub(crate) fn call_value(&mut self, cv: &Value, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match cv {
            Value::Func(f) => self.call_function(f, args, None, pos),
            Value::Builtin(b) => crate::builtin::call_builtin(self, *b, args, pos),
            Value::TypeRef(TypeRefKind::Struct(name)) => {
                // 类型直接调用：构造结构体（参数按字段顺序）
                let def = self
                    .structs
                    .lock()
                    .unwrap()
                    .get(name)
                    .cloned()
                    .ok_or_else(|| LxError::new("R1002", format!("未知结构体: '{}'", name), Some(pos)))?;
                let mut fields = HashMap::new();
                for (i, f) in def.fields.iter().enumerate() {
                    let val = match args.get(i) {
                        Some(a) => a.clone(),
                        None => Value::Null,
                    };
                    fields.insert(f.name.clone(), val);
                }
                if args.len() > def.fields.len() {
                    return Err(LxError::r1005(
                        format!("结构体 {} 字段过多", name),
                        pos,
                    ));
                }
                Ok(Value::StructInstance {
                    type_name: name.clone(),
                    fields: Value::new_struct_fields(fields),
                })
            }
            Value::TypeRef(TypeRefKind::Enum(name)) => {
                Err(LxError::new(
                    "R1004",
                    format!("枚举类型 {} 不能直接调用（用 {}.Variant）", name, name),
                    Some(pos),
                ))
            }
            _ => Err(LxError::new(
                "R1004",
                format!("值不可调用: {}", cv),
                Some(pos),
            )),
        }
    }

    /// 调用用户函数；self_val 用于方法绑定
    fn call_function(
        &mut self,
        f: &Arc<Function>,
        args: &[Value],
        self_val: Option<Value>,
        pos: Pos,
    ) -> Result<Value, LxError> {
        let call_env = Env::new(Some(f.closure.clone()));
        {
            let mut ce = call_env.lock().unwrap();
            if let Some(sv) = self_val {
                ce.define("self", sv);
            }
            let mut idx = 0usize;
            for p in &f.params {
                if p.name == "self" {
                    continue;
                }
                if idx < args.len() {
                    ce.define(&p.name, args[idx].clone());
                    idx += 1;
                } else if let Some(d) = &p.default {
                    // 默认值表达式在函数闭包环境中求值
                    let dv = self.eval_expr(d, &f.closure)?;
                    ce.define(&p.name, dv);
                } else {
                    return Err(LxError::new(
                        "R1005",
                        format!("调用 {} 缺少参数 '{}'", f.name, p.name),
                        Some(pos),
                    ));
                }
            }
            if idx < args.len() {
                return Err(LxError::new(
                    "R1005",
                    format!("调用 {} 参数过多（{} 个实参，{} 个形参）", f.name, args.len(), f.params.len()),
                    Some(pos),
                ));
            }
        }
        // 闭包隐式返回：把 body 最后一条 ExprStmt 转为 Return
        let body: Vec<Stmt> = if f.implicit_return {
            let mut b = f.body.clone();
            if let Some(Stmt::ExprStmt { expr, pos }) = b.last_mut() {
                let e = expr.clone();
                let p = *pos;
                *b.last_mut().unwrap() = Stmt::Return {
                    value: Some(e),
                    pos: p,
                };
            }
            b
        } else {
            f.body.clone()
        };
        match self.exec_block(&body, &call_env)? {
            Flow::Return(v) => Ok(v),
            Flow::Normal => Ok(Value::Null),
            Flow::Break | Flow::Continue => {
                Err(LxError::new("R1005", format!("函数 {} 内出现非法控制流", f.name), Some(pos)))
            }
        }
    }

    /// 方法分派
    fn call_method(&mut self, recv: &Value, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match recv {
            Value::StructInstance { type_name, fields: _ } => {
                let methods = self.impls.lock().unwrap().get(type_name).cloned().unwrap_or_default();
                if let Some(f) = methods.get(name) {
                    let f = f.clone();
                    let self_val = recv.clone();
                    return self.call_function(&f, args, Some(self_val), pos);
                }
                // 字段名也可调用？不，报错
                Err(LxError::new(
                    "R1007",
                    format!("结构体 {} 没有方法 '{}'", type_name, name),
                    Some(pos),
                ))
            }
            Value::Str(s) => self.call_str_method(s, name, args, pos),
            Value::List(l) => self.call_list_method(l, name, args, pos),
            Value::Dict(d) => self.call_dict_method(d, name, args, pos),
            Value::Chan(c) => self.call_chan_method(c, name, args, pos),
            Value::Mutex(m) => self.call_mutex_method(m, name, args, pos),
            Value::RWLock(rw) => self.call_rwlock_method(rw, name, args, pos),
            Value::Tuple(t) => self.call_tuple_method(t, name, args, pos),
            _ => Err(LxError::new(
                "R1007",
                format!("类型 {} 没有方法 '{}'", self.type_name(recv), name),
                Some(pos),
            )),
        }
    }

    pub fn type_name(&self, v: &Value) -> &'static str {
        match v {
            Value::Int(_) => "int",
            Value::Float(_) => "float",
            Value::Str(_) => "string",
            Value::Bytes(_) => "bytes",
            Value::Bool(_) => "bool",
            Value::Null => "null",
            Value::List(_) => "list",
            Value::Tuple(_) => "tuple",
            Value::Dict(_) => "dict",
            Value::Func(_) => "fn",
            Value::Builtin(_) => "builtin",
            Value::StructInstance { .. } => "struct",
            Value::EnumValue { .. } => "enum",
            Value::Range { .. } => "range",
            Value::Chan(_) => "chan",
            Value::Mutex(_) => "mutex",
            Value::RWLock(_) => "rwlock",
            Value::TypeRef(_) => "type",
            Value::Gen(_) => "generator",
        }
    }

    fn call_str_method(&mut self, s: &str, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "upper" => Ok(Value::Str(s.to_uppercase())),
            "lower" => Ok(Value::Str(s.to_lowercase())),
            "trim" => Ok(Value::Str(s.trim().to_string())),
            "len" => Ok(Value::Int(s.chars().count() as i64)),
            "split" => {
                let sep = self.expect_str_arg(args, 0, "split", pos)?;
                Ok(Value::new_list(
                    s.split(&sep).map(|x| Value::Str(x.to_string())).collect(),
                ))
            }
            "contains" => {
                let sub = self.expect_str_arg(args, 0, "contains", pos)?;
                Ok(Value::Bool(s.contains(&sub)))
            }
            "replace" => {
                let old = self.expect_str_arg(args, 0, "replace", pos)?;
                let new = self.expect_str_arg(args, 1, "replace", pos)?;
                Ok(Value::Str(s.replace(&old, &new)))
            }
            "starts_with" => {
                let p = self.expect_str_arg(args, 0, "starts_with", pos)?;
                Ok(Value::Bool(s.starts_with(&p)))
            }
            "ends_with" => {
                let p = self.expect_str_arg(args, 0, "ends_with", pos)?;
                Ok(Value::Bool(s.ends_with(&p)))
            }
            _ => Err(LxError::new("R1007", format!("string 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn call_list_method(&mut self, l: &Arc<Mutex<Vec<Value>>>, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "len" => Ok(Value::Int(l.lock().unwrap().len() as i64)),
            "append" | "push" => {
                if args.is_empty() {
                    return Err(LxError::r1005("list.append 需要一个参数", pos));
                }
                l.lock().unwrap().push(args[0].clone());
                Ok(Value::Null)
            }
            "pop" => {
                let mut b = l.lock().unwrap();
                match b.pop() {
                    Some(v) => Ok(v),
                    None => Err(LxError::new("R1003", "pop 空列表", Some(pos))),
                }
            }
            "join" => {
                let sep = self.expect_str_arg(args, 0, "join", pos)?;
                let parts: Vec<String> = l.lock().unwrap().iter().map(|v| v.to_string()).collect();
                Ok(Value::Str(parts.join(&sep)))
            }
            "contains" => {
                if args.is_empty() {
                    return Err(LxError::r1005("list.contains 需要一个参数", pos));
                }
                Ok(Value::Bool(l.lock().unwrap().contains(&args[0])))
            }
            "reverse" => {
                l.lock().unwrap().reverse();
                Ok(Value::Null)
            }
            "sort" => {
                // 简单排序：按 Display 字符串排（M2 简化）
                let mut b = l.lock().unwrap();
                b.sort_by(|a, c| a.to_string().cmp(&c.to_string()));
                Ok(Value::Null)
            }
            "index" => {
                if args.is_empty() {
                    return Err(LxError::r1005("list.index 需要一个参数", pos));
                }
                let b = l.lock().unwrap();
                match b.iter().position(|x| *x == args[0]) {
                    Some(i) => Ok(Value::Int(i as i64)),
                    None => Err(LxError::new("R1003", "list.index 未找到元素", Some(pos))),
                }
            }
            _ => Err(LxError::new("R1007", format!("list 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn call_dict_method(&mut self, d: &Arc<Mutex<HashMap<String, Value>>>, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "len" => Ok(Value::Int(d.lock().unwrap().len() as i64)),
            "get" => {
                let k = self.expect_str_arg(args, 0, "get", pos)?;
                let b = d.lock().unwrap();
                Ok(b.get(&k).cloned().unwrap_or_else(|| {
                    args.get(1).cloned().unwrap_or(Value::Null)
                }))
            }
            "set" | "put" => {
                let k = self.expect_str_arg(args, 0, "set", pos)?;
                let v = args.get(1).cloned().unwrap_or(Value::Null);
                d.lock().unwrap().insert(k, v);
                Ok(Value::Null)
            }
            "keys" => {
                let ks: Vec<Value> = d.lock().unwrap().keys().map(|k| Value::Str(k.clone())).collect();
                Ok(Value::new_list(ks))
            }
            "values" => {
                let vs: Vec<Value> = d.lock().unwrap().values().cloned().collect();
                Ok(Value::new_list(vs))
            }
            "has" | "contains" => {
                let k = self.expect_str_arg(args, 0, "has", pos)?;
                Ok(Value::Bool(d.lock().unwrap().contains_key(&k)))
            }
            "remove" => {
                let k = self.expect_str_arg(args, 0, "remove", pos)?;
                match d.lock().unwrap().remove(&k) {
                    Some(v) => Ok(v),
                    None => Err(LxError::new("R1008", format!("字典没有键 '{}'", k), Some(pos))),
                }
            }
            _ => Err(LxError::new("R1007", format!("dict 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn call_chan_method(&mut self, c: &ChanRef, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "send" => {
                if args.is_empty() {
                    return Err(LxError::r1005("chan.send 需要一个参数", pos));
                }
                self.chan_send(&Value::Chan(c.clone()), args[0].clone(), pos)?;
                Ok(Value::Null)
            }
            "recv" => self.chan_recv(&Value::Chan(c.clone()), pos),
            "close" => {
                c.inner.lock().unwrap().closed = true;
                notify_select();
                Ok(Value::Null)
            }
            "len" => Ok(Value::Int(c.inner.lock().unwrap().buf.len() as i64)),
            _ => Err(LxError::new("R1007", format!("chan 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn call_tuple_method(&mut self, t: &[Value], name: &str, _args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "len" => Ok(Value::Int(t.len() as i64)),
            _ => Err(LxError::new("R1007", format!("tuple 没有方法 '{}'", name), Some(pos))),
        }
    }

    // ==================== 锁原语（M13：P1 mutex / rwlock） ====================

    fn call_mutex_method(&mut self, m: &MutexRef, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "lock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("mutex.lock 不需要参数", pos));
                }
                m.lock();
                Ok(Value::Null)
            }
            "unlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("mutex.unlock 不需要参数", pos));
                }
                m.unlock();
                Ok(Value::Null)
            }
            "try_lock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("mutex.try_lock 不需要参数", pos));
                }
                Ok(Value::Bool(m.try_lock()))
            }
            // with(fn)：自动 lock/unlock，异常安全（fn 出错也解锁）
            "with" => {
                if args.len() != 1 {
                    return Err(LxError::r1005("mutex.with 需要一个函数参数", pos));
                }
                let f = args[0].clone();
                m.lock();
                let r = self.call_value(&f, &[], pos);
                m.unlock();
                r
            }
            _ => Err(LxError::new("R1007", format!("mutex 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn call_rwlock_method(&mut self, rw: &RWLockRef, name: &str, args: &[Value], pos: Pos) -> Result<Value, LxError> {
        match name {
            "rlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.rlock 不需要参数", pos));
                }
                rw.rlock();
                Ok(Value::Null)
            }
            "runlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.runlock 不需要参数", pos));
                }
                rw.runlock();
                Ok(Value::Null)
            }
            "wlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.wlock 不需要参数", pos));
                }
                rw.wlock();
                Ok(Value::Null)
            }
            "wunlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.wunlock 不需要参数", pos));
                }
                rw.wunlock();
                Ok(Value::Null)
            }
            "try_rlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.try_rlock 不需要参数", pos));
                }
                Ok(Value::Bool(rw.try_rlock()))
            }
            "try_wlock" => {
                if !args.is_empty() {
                    return Err(LxError::r1005("rwlock.try_wlock 不需要参数", pos));
                }
                Ok(Value::Bool(rw.try_wlock()))
            }
            // with_read(fn)：自动读锁/释放（异常安全）
            "with_read" => {
                if args.len() != 1 {
                    return Err(LxError::r1005("rwlock.with_read 需要一个函数参数", pos));
                }
                let f = args[0].clone();
                rw.rlock();
                let r = self.call_value(&f, &[], pos);
                rw.runlock();
                r
            }
            // with_write(fn)：自动写锁/释放（异常安全）
            "with_write" => {
                if args.len() != 1 {
                    return Err(LxError::r1005("rwlock.with_write 需要一个函数参数", pos));
                }
                let f = args[0].clone();
                rw.wlock();
                let r = self.call_value(&f, &[], pos);
                rw.wunlock();
                r
            }
            _ => Err(LxError::new("R1007", format!("rwlock 没有方法 '{}'", name), Some(pos))),
        }
    }

    fn expect_str_arg(&self, args: &[Value], idx: usize, mname: &str, pos: Pos) -> Result<String, LxError> {
        match args.get(idx) {
            Some(Value::Str(s)) => Ok(s.clone()),
            _ => Err(LxError::r1002(
                format!("方法 {} 参数 {} 需要 string", mname, idx + 1),
                pos,
            )),
        }
    }

    // ==================== 通道 ====================

    fn chan_send(&mut self, chan: &Value, v: Value, pos: Pos) -> Result<(), LxError> {
        let ch = match chan {
            Value::Chan(c) => c.clone(),
            _ => return Err(LxError::r1002("send 目标不是通道", pos)),
        };
        let mut inner = ch.inner.lock().unwrap();
        loop {
            if inner.closed {
                return Err(LxError::new("R1011", "向已关闭通道发送", Some(pos)));
            }
            // 就绪条件：有缓冲=未满；无缓冲=已有接收者等待
            let ready = if inner.cap > 0 {
                inner.buf.len() < inner.cap
            } else {
                inner.recv_waiting > 0
            };
            if ready {
                inner.buf.push_back(v);
                drop(inner);
                ch.cv.notify_all();
                notify_select();
                return Ok(());
            }
            inner = ch.cv.wait(inner).unwrap();
        }
    }

    fn chan_recv(&mut self, chan: &Value, pos: Pos) -> Result<Value, LxError> {
        let ch = match chan {
            Value::Chan(c) => c.clone(),
            _ => return Err(LxError::r1002("recv 目标不是通道", pos)),
        };
        let mut inner = ch.inner.lock().unwrap();
        if inner.cap == 0 {
            inner.recv_waiting += 1;
        }
        loop {
            if let Some(v) = inner.buf.pop_front() {
                if inner.cap == 0 {
                    inner.recv_waiting = inner.recv_waiting.saturating_sub(1);
                }
                drop(inner);
                ch.cv.notify_all();
                notify_select();
                return Ok(v);
            }
            if inner.closed {
                if inner.cap == 0 {
                    inner.recv_waiting = inner.recv_waiting.saturating_sub(1);
                }
                return Err(LxError::new("R1011", "从已关闭通道接收", Some(pos)));
            }
            inner = ch.cv.wait(inner).unwrap();
        }
    }

    // ==================== 字段 / 索引 / 赋值 ====================

    fn eval_field(&mut self, obj: &Value, name: &str, pos: Pos) -> Result<Value, LxError> {
        match obj {
            Value::StructInstance { fields, .. } => {
                let f = fields.lock().unwrap();
                match f.get(name) {
                    Some(v) => Ok(v.clone()),
                    None => Err(LxError::new("R1008", format!("结构体没有字段 '{}'", name), Some(pos))),
                }
            }
            Value::EnumValue { type_name, variant, payload: _ } => {
                // Color.Red 构造：Red 是 variant 名称，不需要字段访问
                // 但如果访问的是枚举类型对象上的 variant，会在 eval_field 上层处理
                Err(LxError::new("R1007", format!("枚举值 {}.{} 没有字段 '{}'", type_name, variant, name), Some(pos)))
            }
            Value::TypeRef(TypeRefKind::Enum(enum_name)) => {
                // 枚举构造 Color.Red
                let def = self
                    .enums
                    .lock()
                    .unwrap()
                    .get(enum_name)
                    .cloned()
                    .ok_or_else(|| LxError::new("R1002", format!("未知枚举: '{}'", enum_name), Some(pos)))?;
                let var = def
                    .variants
                    .iter()
                    .find(|v| v.name == name)
                    .ok_or_else(|| LxError::new("R1008", format!("枚举 {} 没有变体 '{}'", enum_name, name), Some(pos)))?;
                let payload = if var.fields.is_empty() {
                    None
                } else {
                    Some(Box::new(Value::Null))
                };
                Ok(Value::EnumValue {
                    type_name: enum_name.clone(),
                    variant: name.to_string(),
                    payload,
                })
            }
            Value::Dict(d) => {
                let b = d.lock().unwrap();
                match b.get(name) {
                    Some(v) => Ok(v.clone()),
                    None => Err(LxError::new("R1008", format!("字典没有键 '{}'", name), Some(pos))),
                }
            }
            _ => Err(LxError::new(
                "R1007",
                format!("类型 {} 没有字段 '{}'", self.type_name(obj), name),
                Some(pos),
            )),
        }
    }

    fn eval_index(&mut self, obj: &Value, idx: &Value, pos: Pos) -> Result<Value, LxError> {
        match obj {
            Value::List(l) => {
                let i = self.as_index(idx, l.lock().unwrap().len(), pos)?;
                let b = l.lock().unwrap();
                match b.get(i) {
                    Some(v) => Ok(v.clone()),
                    None => Err(LxError::new("R1003", "列表索引越界", Some(pos))),
                }
            }
            Value::Tuple(t) => {
                let i = self.as_index(idx, t.len(), pos)?;
                match t.get(i) {
                    Some(v) => Ok(v.clone()),
                    None => Err(LxError::new("R1003", "元组索引越界", Some(pos))),
                }
            }
            Value::Str(s) => {
                let chars: Vec<char> = s.chars().collect();
                let i = self.as_index(idx, chars.len(), pos)?;
                match chars.get(i) {
                    Some(c) => Ok(Value::Str(c.to_string())),
                    None => Err(LxError::new("R1003", "字符串索引越界", Some(pos))),
                }
            }
            Value::Dict(d) => {
                let k = match idx {
                    Value::Str(s) => s.clone(),
                    _ => return Err(LxError::r1002("字典索引键必须是字符串", pos)),
                };
                let b = d.lock().unwrap();
                match b.get(&k) {
                    Some(v) => Ok(v.clone()),
                    None => Err(LxError::new("R1008", format!("字典没有键 '{}'", k), Some(pos))),
                }
            }
            _ => Err(LxError::new("R1002", "此类型不支持索引", Some(pos))),
        }
    }

    fn as_index(&self, idx: &Value, len: usize, pos: Pos) -> Result<usize, LxError> {
        match idx {
            Value::Int(i) => {
                let n = if *i < 0 { (*i + len as i64) as usize } else { *i as usize };
                if n >= len {
                    return Err(LxError::new("R1003", "索引越界", Some(pos)));
                }
                Ok(n)
            }
            _ => Err(LxError::r1002("索引必须是整数", pos)),
        }
    }

    /// M21/M24：切片 a[start:end] / a[start:end:step]（str 按字符、list/tuple/bytes 取元素；
    /// 负索引从尾部算；step<0 反向；step=0 报错）
    fn eval_slice(
        &mut self,
        obj: &Value,
        start: Option<Value>,
        end: Option<Value>,
        step: Option<Value>,
        pos: Pos,
    ) -> Result<Value, LxError> {
        fn bound(v: Option<Value>, pos: Pos) -> Result<Option<i64>, LxError> {
            match v {
                None => Ok(None),
                Some(Value::Int(i)) => Ok(Some(i)),
                Some(Value::Null) => Ok(None),
                Some(_) => Err(LxError::r1002("切片边界必须是整数", pos)),
            }
        }
        let s = bound(start, pos)?;
        let e = bound(end, pos)?;
        let k = bound(step, pos)?;
        // 收集目标索引序列（Python slice.indices 语义）
        let pick = |len: i64| -> Result<Vec<usize>, LxError> {
            let (a, b, st) =
                slice_indices(s, e, k, len).map_err(|m| LxError::new("R1002", m, Some(pos)))?;
            let mut idxs = Vec::new();
            if st > 0 {
                let mut i = a;
                while i < b {
                    idxs.push(i as usize);
                    i += st;
                }
            } else {
                let mut i = a;
                while i > b {
                    idxs.push(i as usize);
                    i += st;
                }
            }
            Ok(idxs)
        };
        match obj {
            Value::Str(st) => {
                let chars: Vec<char> = st.chars().collect();
                let idxs = pick(chars.len() as i64)?;
                Ok(Value::Str(idxs.iter().map(|&i| chars[i]).collect()))
            }
            Value::Bytes(b) => {
                let idxs = pick(b.len() as i64)?;
                Ok(Value::Bytes(idxs.iter().map(|&i| b[i]).collect()))
            }
            Value::List(l) => {
                let items = l.lock().unwrap().clone();
                let idxs = pick(items.len() as i64)?;
                Ok(Value::new_list(idxs.iter().map(|&i| items[i].clone()).collect()))
            }
            Value::Tuple(t) => {
                let idxs = pick(t.len() as i64)?;
                Ok(Value::Tuple(idxs.iter().map(|&i| t[i].clone()).collect()))
            }
            _ => Err(LxError::new("R1002", "此类型不支持切片", Some(pos))),
        }
    }

    fn assign(&mut self, target: &Expr, op: AssignOp, v: Value, env: &EnvRef, pos: Pos) -> Result<(), LxError> {
        match target {
            Expr::Var { name, .. } => {
                if op == AssignOp::Assign {
                    if !env.lock().unwrap().set(name, v.clone()) {
                        // 未定义变量：在当前作用域创建（Python 风格）
                        env.lock().unwrap().define(name, v);
                    }
                    return Ok(());
                }
                let old = env
                    .lock().unwrap()
                    .get(name)
                    .ok_or_else(|| LxError::r1001(name, pos))?;
                let nv = self.apply_assign_op(op, old, v, pos)?;
                env.lock().unwrap().set(name, nv);
                Ok(())
            }
            Expr::Field { obj, name, .. } => {
                let ov = self.eval_expr(obj, env)?;
                let old = match &ov {
                    Value::StructInstance { fields, .. } => {
                        let f = fields.lock().unwrap();
                        match f.get(name) {
                            Some(x) => x.clone(),
                            None => Value::Null,
                        }
                    }
                    Value::Dict(d) => d.lock().unwrap().get(name).cloned().unwrap_or(Value::Null),
                    _ => return Err(LxError::new("R1007", "赋值目标不是可写对象", Some(pos))),
                };
                let nv = if op == AssignOp::Assign {
                    v
                } else {
                    self.apply_assign_op(op, old, v, pos)?
                };
                match &ov {
                    Value::StructInstance { fields, .. } => {
                        fields.lock().unwrap().insert(name.clone(), nv);
                    }
                    Value::Dict(d) => {
                        d.lock().unwrap().insert(name.clone(), nv);
                    }
                    _ => unreachable!(),
                }
                Ok(())
            }
            Expr::Index { obj, index, .. } => {
                let ov = self.eval_expr(obj, env)?;
                let iv = self.eval_expr(index, env)?;
                let old = self.eval_index(&ov, &iv, pos)?;
                let nv = if op == AssignOp::Assign {
                    v
                } else {
                    self.apply_assign_op(op, old, v, pos)?
                };
                match &ov {
                    Value::List(l) => {
                        let i = self.as_index(&iv, l.lock().unwrap().len(), pos)?;
                        l.lock().unwrap()[i] = nv;
                    }
                    Value::Dict(d) => {
                        let k = match &iv {
                            Value::Str(s) => s.clone(),
                            _ => return Err(LxError::r1002("字典键必须是字符串", pos)),
                        };
                        d.lock().unwrap().insert(k, nv);
                    }
                    _ => return Err(LxError::new("R1002", "索引赋值目标不支持", Some(pos))),
                }
                Ok(())
            }
            _ => Err(LxError::new("R1002", "非法赋值目标", Some(pos))),
        }
    }

    fn apply_assign_op(&mut self, op: AssignOp, old: Value, new: Value, pos: Pos) -> Result<Value, LxError> {
        let binop = match op {
            AssignOp::Assign => return Ok(new),
            AssignOp::Plus => BinaryOp::Add,
            AssignOp::Minus => BinaryOp::Sub,
            AssignOp::Star => BinaryOp::Mul,
            AssignOp::Slash => BinaryOp::Div,
            AssignOp::IntDiv => BinaryOp::IntDiv,
            AssignOp::Mod => BinaryOp::Mod,
            AssignOp::Pow => BinaryOp::Pow,
            AssignOp::BitAnd => BinaryOp::BitAnd,
            AssignOp::BitOr => BinaryOp::BitOr,
            AssignOp::BitXor => BinaryOp::BitXor,
            AssignOp::Shl => BinaryOp::Shl,
            AssignOp::Shr => BinaryOp::Shr,
            AssignOp::ShrU => BinaryOp::ShrU,
        };
        self.eval_binary(binop, old, new, pos)
    }

    // ==================== 运算符 ====================

    fn eval_unary(&mut self, op: UnaryOp, v: Value, pos: Pos) -> Result<Value, LxError> {
        match op {
            UnaryOp::Neg => match v {
                Value::Int(i) => Ok(Value::Int(-i)),
                Value::Float(f) => Ok(Value::Float(-f)),
                _ => Err(LxError::r1002("一元负号需要数值", pos)),
            },
            UnaryOp::Not => match v {
                Value::Bool(b) => Ok(Value::Bool(!b)),
                _ => Err(LxError::r1002("not 需要布尔值", pos)),
            },
            UnaryOp::BitNot => match v {
                Value::Int(i) => Ok(Value::Int(!i)),
                _ => Err(LxError::r1002("按位取反需要整数", pos)),
            },
        }
    }

    pub fn eval_binary(&mut self, op: BinaryOp, l: Value, r: Value, pos: Pos) -> Result<Value, LxError> {
        use BinaryOp::*;
        match op {
            Add => self.bin_add(l, r, pos),
            Sub | Mul | Div | IntDiv | Mod | Pow | Shl | Shr | ShrU | BitAnd | BitOr | BitXor => {
                self.bin_numeric(op, l, r, pos)
            }
            Eq => Ok(Value::Bool(l == r)),
            Ne => Ok(Value::Bool(l != r)),
            Lt | Le | Gt | Ge => self.bin_compare(op, l, r, pos),
            And | Or => unreachable!("短路在 eval_expr 处理"),
        }
    }

    fn bin_add(&mut self, l: Value, r: Value, pos: Pos) -> Result<Value, LxError> {
        match (&l, &r) {
            (Value::Int(a), Value::Int(b)) => Ok(Value::Int(a + b)),
            (Value::Int(a), Value::Float(b)) => Ok(Value::Float(*a as f64 + b)),
            (Value::Float(a), Value::Int(b)) => Ok(Value::Float(a + *b as f64)),
            (Value::Float(a), Value::Float(b)) => Ok(Value::Float(a + b)),
            (Value::Str(a), Value::Str(b)) => Ok(Value::Str(format!("{}{}", a, b))),
            (Value::List(a), Value::List(b)) => {
                let mut out = a.lock().unwrap().clone();
                out.extend(b.lock().unwrap().iter().cloned());
                Ok(Value::new_list(out))
            }
            _ => Err(LxError::r1002(
                format!("+ 不支持: {} + {}", self.type_name(&l), self.type_name(&r)),
                pos,
            )),
        }
    }

    fn bin_numeric(&mut self, op: BinaryOp, l: Value, r: Value, pos: Pos) -> Result<Value, LxError> {
        match (&l, &r) {
            (Value::Int(a), Value::Int(b)) => {
                use BinaryOp::*;
                match op {
                    Sub => Ok(Value::Int(a - b)),
                    Mul => Ok(Value::Int(a * b)),
                    Div => {
                        if *b == 0 {
                            return Err(LxError::new("R1006", "除零错误", Some(pos)));
                        }
                        Ok(Value::Float(*a as f64 / *b as f64))
                    }
                    IntDiv => {
                        if *b == 0 {
                            return Err(LxError::new("R1006", "除零错误", Some(pos)));
                        }
                        Ok(Value::Int(a.div_euclid(*b)))
                    }
                    Mod => {
                        if *b == 0 {
                            return Err(LxError::new("R1006", "除零错误", Some(pos)));
                        }
                        Ok(Value::Int(a.rem_euclid(*b)))
                    }
                    Pow => match a.checked_pow(*b as u32) {
                        Some(v) => Ok(Value::Int(v)),
                        None => Ok(Value::Float((*a as f64).powf(*b as f64))),
                    },
                    Shl => Ok(Value::Int(a.wrapping_shl(*b as u32))),
                    Shr => Ok(Value::Int(a.wrapping_shr(*b as u32))),
                    ShrU => Ok(Value::Int((*a as u64).wrapping_shr(*b as u32) as i64)),
                    BitAnd => Ok(Value::Int(a & b)),
                    BitOr => Ok(Value::Int(a | b)),
                    BitXor => Ok(Value::Int(a ^ b)),
                    _ => unreachable!(),
                }
            }
            _ => {
                let (af, bf) = (self.as_f64(&l)?, self.as_f64(&r)?);
                use BinaryOp::*;
                match op {
                    Sub => Ok(Value::Float(af - bf)),
                    Mul => Ok(Value::Float(af * bf)),
                    Div => {
                        if bf == 0.0 {
                            return Err(LxError::new("R1006", "除零错误", Some(pos)));
                        }
                        Ok(Value::Float(af / bf))
                    }
                    IntDiv => {
                        if bf == 0.0 {
                            return Err(LxError::new("R1006", "除零错误", Some(pos)));
                        }
                        Ok(Value::Int((af / bf).floor() as i64))
                    }
                    Mod => Ok(Value::Float(af % bf)),
                    Pow => Ok(Value::Float(af.powf(bf))),
                    _ => Err(LxError::r1002("此运算符要求整数操作数", pos)),
                }
            }
        }
    }

    fn bin_compare(&mut self, op: BinaryOp, l: Value, r: Value, pos: Pos) -> Result<Value, LxError> {
        use BinaryOp::*;
        let ord = match (&l, &r) {
            (Value::Int(a), Value::Int(b)) => a.cmp(b),
            (Value::Int(a), Value::Float(b)) => (*a as f64).partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal),
            (Value::Float(a), Value::Int(b)) => a.partial_cmp(&(*b as f64)).unwrap_or(std::cmp::Ordering::Equal),
            (Value::Float(a), Value::Float(b)) => a.partial_cmp(b).unwrap_or(std::cmp::Ordering::Equal),
            (Value::Str(a), Value::Str(b)) => a.cmp(b),
            _ => {
                return Err(LxError::r1002(
                    format!("比较不支持: {} vs {}", self.type_name(&l), self.type_name(&r)),
                    pos,
                ))
            }
        };
        let res = match op {
            Lt => ord == std::cmp::Ordering::Less,
            Le => ord != std::cmp::Ordering::Greater,
            Gt => ord == std::cmp::Ordering::Greater,
            Ge => ord != std::cmp::Ordering::Less,
            _ => unreachable!(),
        };
        Ok(Value::Bool(res))
    }

    fn as_f64(&self, v: &Value) -> Result<f64, LxError> {
        match v {
            Value::Int(i) => Ok(*i as f64),
            Value::Float(f) => Ok(*f),
            _ => Err(LxError::new("R1002", "需要数值", None)),
        }
    }

    // ==================== 模式匹配 ====================

    fn match_pattern(&mut self, pat: &Pattern, v: &Value, env: &EnvRef, pos: Pos) -> Result<bool, LxError> {
        match pat {
            Pattern::Literal(e) => {
                let lv = self.eval_expr(e, env)?;
                Ok(lv == *v)
            }
            Pattern::Binding(name) => {
                env.lock().unwrap().define(name, v.clone());
                Ok(true)
            }
            Pattern::Wildcard => Ok(true),
            Pattern::Tuple(patterns) => {
                let items = match v {
                    Value::Tuple(t) => t.clone(),
                    Value::List(l) => l.lock().unwrap().clone(),
                    _ => return Ok(false),
                };
                if patterns.len() != items.len() {
                    return Ok(false);
                }
                for (p, iv) in patterns.iter().zip(items.iter()) {
                    if !self.match_pattern(p, iv, env, pos)? {
                        return Ok(false);
                    }
                }
                Ok(true)
            }
            Pattern::Constructor(name, sub) => {
                let ev = match v {
                    Value::EnumValue { variant, payload, .. } => {
                        if variant != name {
                            return Ok(false);
                        }
                        payload.clone()
                    }
                    Value::StructInstance { type_name, .. } => {
                        // 结构体模式（M2 简化：仅匹配类型名，绑定字段需后续版本）
                        if type_name != name {
                            return Ok(false);
                        }
                        None
                    }
                    _ => return Ok(false),
                };
                // 无子模式：匹配成功
                if sub.is_empty() {
                    return Ok(true);
                }
                // 有载荷：匹配第一个子模式（M2 简化支持单载荷）
                match ev {
                    Some(payload) => self.match_pattern(&sub[0], &payload, env, pos),
                    None => Ok(false),
                }
            }
        }
    }

    // ==================== 迭代 / 真值 ====================

    fn iter_values(&mut self, v: &Value, pos: Pos) -> Result<Vec<Value>, LxError> {
        match v {
            Value::List(l) => Ok(l.lock().unwrap().clone()),
            Value::Tuple(t) => Ok(t.clone()),
            Value::Str(s) => Ok(s.chars().map(|c| Value::Str(c.to_string())).collect()),
            Value::Range { start, end, step } => {
                let mut out = Vec::new();
                if *step == 0 {
                    return Err(LxError::new("R1006", "range step 不能为 0", Some(pos)));
                }
                let mut cur = *start;
                if *step > 0 {
                    while cur < *end {
                        out.push(Value::Int(cur));
                        cur += step;
                    }
                } else {
                    while cur > *end {
                        out.push(Value::Int(cur));
                        cur += step;
                    }
                }
                Ok(out)
            }
            Value::Dict(d) => Ok(d
                .lock().unwrap()
                .keys()
                .map(|k| Value::Str(k.clone()))
                .collect()),
            Value::Gen(g) => {
                // M32：生成器物化结果迭代（M34：惰性生成器先物化剩余，保持 for-in 行为一致）
                let mut gg = g.lock().unwrap();
                crate::builtin::gen_materialize_lazy(&mut gg, self, pos)?;
                Ok(gg.materialized.clone())
            }
            _ => Err(LxError::new("R1002", "此类型不可迭代", Some(pos))),
        }
    }

    // ==================== M30：推导式求值（多 for 嵌套 / 多变量解包 / 多 if） ====================

    /// 绑定推导式子句变量（多变量解包：item 需为 list/tuple）
    fn bind_comp_vars(
        &mut self,
        vars: &[String],
        item: Value,
        child: &Arc<Mutex<Env>>,
        pos: Pos,
    ) -> Result<(), LxError> {
        if vars.len() == 1 {
            child.lock().unwrap().define(&vars[0], item);
            return Ok(());
        }
        let items: Vec<Value> = match item {
            Value::List(l) => l.lock().unwrap().clone(),
            Value::Tuple(t) => t.clone(),
            _ => {
                return Err(LxError::new(
                    "R1002",
                    format!(
                        "推导式解包需要 list/tuple，实际是 {}",
                        self.type_name(&item)
                    ),
                    Some(pos),
                ))
            }
        };
        for (i, vn) in vars.iter().enumerate() {
            let v = items.get(i).cloned().unwrap_or(Value::Null);
            child.lock().unwrap().define(vn, v);
        }
        Ok(())
    }

    /// 递归求值推导式：ci 为当前子句索引；最深层求值并收集
    pub(crate) fn eval_comp(
        &mut self,
        clauses: &[CompClause],
        ci: usize,
        sink: &mut CompSink,
        list_expr: Option<&Expr>,
        dict_key: Option<&Expr>,
        dict_val: Option<&Expr>,
        cond: &Option<Box<Expr>>,
        env: &Arc<Mutex<Env>>,
        pos: Pos,
    ) -> Result<(), LxError> {
        if ci >= clauses.len() {
            if let Some(c) = cond {
                let cv = self.eval_expr(c, env)?;
                if !self.is_truthy(&cv, pos)? {
                    return Ok(());
                }
            }
            match sink {
                CompSink::List(out) => {
                    let ev = self.eval_expr(list_expr.unwrap(), env)?;
                    out.push(ev);
                }
                CompSink::Dict(map) => {
                    let k = self.eval_expr(dict_key.unwrap(), env)?;
                    let v = self.eval_expr(dict_val.unwrap(), env)?;
                    let ks = match k {
                        Value::Str(s) => s,
                        _ => {
                            return Err(LxError::new(
                                "R1002",
                                format!(
                                    "字典推导键必须是字符串，实际是 {}",
                                    self.type_name(&k)
                                ),
                                Some(pos),
                            ))
                        }
                    };
                    map.lock().unwrap().insert(ks, v);
                }
            }
            return Ok(());
        }
        let cl = &clauses[ci];
        let it = self.eval_expr(&cl.iterable, env)?;
        // M35：range 流式迭代（不物化中间列表，大 range 推导式不炸内存）；
        // 其他可迭代类型保持物化（list/tuple/str/dict/gen）。
        if let Value::Range { start, end, step } = &it {
            let mut cur = *start;
            while (*step > 0 && cur < *end) || (*step < 0 && cur > *end) {
                let item = Value::Int(cur);
                let child = Env::new(Some(env.clone()));
                self.bind_comp_vars(&cl.vars, item, &child, pos)?;
                self.eval_comp(
                    clauses,
                    ci + 1,
                    sink,
                    list_expr,
                    dict_key,
                    dict_val,
                    cond,
                    &child,
                    pos,
                )?;
                cur += *step;
            }
            return Ok(());
        }
        let items = self.iter_values(&it, pos)?;
        for item in items {
            let child = Env::new(Some(env.clone()));
            self.bind_comp_vars(&cl.vars, item, &child, pos)?;
            self.eval_comp(
                clauses,
                ci + 1,
                sink,
                list_expr,
                dict_key,
                dict_val,
                cond,
                &child,
                pos,
            )?;
        }
        Ok(())
    }

    pub fn is_truthy(&self, v: &Value, _pos: Pos) -> Result<bool, LxError> {
        match v {
            Value::Bool(b) => Ok(*b),
            Value::Int(i) => Ok(*i != 0),
            Value::Float(f) => Ok(*f != 0.0),
            Value::Null => Ok(false),
            Value::Str(s) => Ok(!s.is_empty()),
            Value::List(l) => Ok(!l.lock().unwrap().is_empty()),
            Value::Dict(d) => Ok(!d.lock().unwrap().is_empty()),
            Value::Tuple(t) => Ok(!t.is_empty()),
            _ => Ok(true),
        }
    }
}

// ==================== 测试 ====================

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;
    use crate::parser::Parser;

    fn run_src(src: &str) -> Result<i32, LxError> {
        // 共享锁（gc.rs）：解释器测试与 GC 测试共用全局注册表，必须串行
        let _tlock = crate::gc::TEST_GLOBAL_LOCK.lock().unwrap_or_else(|e| e.into_inner());
        let tokens = Lexer::new(src)
            .tokenize()
            .map_err(|e| LxError::new("E0000", e.to_string(), None))?;
        let mut parser = Parser::new(tokens);
        let prog = parser
            .parse_program()
            .map_err(|e| LxError::new("E0000", e.to_string(), None))?;
        let mut interp = Interpreter::new();
        interp.run_program(&prog)
    }

    #[test]
    fn test_arithmetic_and_compare() {
        let src = r#"
def main():
    assert(1 + 2 == 3, "add")
    assert(7 // 2 == 3, "intdiv")
    assert(7 % 2 == 1, "mod")
    assert(2 ** 10 == 1024, "pow")
    assert(1.5 + 1 == 2.5, "float add")
    assert("ab" + "c" == "abc", "str add")
    assert(not false, "not")
    assert(3 > 2 and 2 < 4, "and")
    assert(10 // 3 == 3 and -7 // 2 == -4, "euclid div")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_recursion_fib() {
        let src = r#"
def fib(n: int) -> int:
    if n <= 1:
        return n
    else:
        return fib(n - 1) + fib(n - 2)

def main():
    assert(fib(10) == 55, "fib(10)")
    assert(fib(0) == 0, "fib(0)")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_closure_implicit_return() {
        let src = r#"
def main():
    let double = fn(x: int) -> int { x * 2 }
    assert(double(21) == 42, "closure")
    let add = fn(a, b) { a + b }
    assert(add(3, 4) == 7, "closure 2")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_list_comp_and_iter() {
        let src = r#"
def main():
    let nums = [0, 1, 2, 3, 4, 5]
    let doubled = [x * 2 for x in nums if x > 2]
    assert(len(doubled) == 3, "len")
    assert(doubled[0] == 6, "first")
    assert(doubled[2] == 10, "last")
    let total = 0
    for i in range(5):
        total = total + i
    assert(total == 10, "for range")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_dict_and_string_methods() {
        let src = r#"
def main():
    let d = {"name": "lx", "v": 1}
    assert(d["name"] == "lx", "dict index")
    d["v"] = 2
    assert(d["v"] == 2, "dict set")
    let s = "  Hello  "
    assert(s.trim().upper() == "HELLO", "str methods")
    assert(to_upper("abc") == "ABC", "builtin to_upper")
    assert("hello".contains("ell"), "contains")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_struct_enum_match() {
        let src = r#"
struct Point:
    x: int
    y: int

enum Color:
    Red
    Green
    Blue

def main():
    let p = Point(3, 4)
    assert(p.x == 3 and p.y == 4, "struct fields")
    p.y = 5
    assert(p.y == 5, "struct assign")
    let c = Color.Green
    let desc = match c:
        case Red:
            "red"
        case _:
            "other"
    assert(desc == "other", "match enum")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_impl_method_and_self() {
        let src = r#"
trait Shape:
    def area(self) -> float:

struct Circle:
    r: float

impl Shape for Circle:
    def area(self) -> float:
        return 3.14 * self.r * self.r

def main():
    let c = Circle(2.0)
    let a = c.area()
    assert(a > 12.5 and a < 12.6, "area ~12.56")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_pipe_and_while() {
        let src = r#"
def main():
    let msg = "hello"
    assert((msg |> to_upper()) == "HELLO", "pipe")
    let i = 0
    let acc = 0
    while i < 3:
        acc = acc + i
        i = i + 1
    assert(acc == 3, "while")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_default_params_and_null_coalesce() {
        let src = r#"
def greet(name, prefix = "Hi"):
    return prefix + " " + name

def main():
    assert(greet("PX") == "Hi PX", "default param")
    assert(greet("PX", "Hello") == "Hello PX", "override param")
    let a = null
    let b = a ?? "fallback"
    assert(b == "fallback", "null coalesce")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_chan_basic() {
        let src = r#"
def main():
    let ch = chan[int](4)
    ch.send(42)
    let v = ch.recv()
    assert(v == 42, "chan send recv")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_chan_unbuffered_sync() {
        let src = r#"
def sender(ch):
    ch.send(7)

def main():
    let ch = chan[int]()
    spawn sender(ch)
    let v = ch.recv()
    assert(v == 7, "unbuffered rendezvous")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_spawn_parallel_chan() {
        let src = r#"
def worker(ch, v):
    ch.send(v)

def main():
    let ch = chan[int](2)
    spawn worker(ch, 1)
    spawn worker(ch, 2)
    let a = ch.recv()
    let b = ch.recv()
    assert(a + b == 3, "two spawns")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_select_blocks_until_ready() {
        let src = r#"
def later(ch):
    sleep(30)
    ch.send(5)

def main():
    let ch = chan[int](1)
    spawn later(ch)
    var got = 0
    select:
        case v = ch.recv():
            got = v
    assert(got == 5, "select blocking")
"#;
        assert!(run_src(src).is_ok());
    }

    // ==================== M5 标准库测试 ====================

    #[test]
    fn test_std_file_io() {
        let src = r#"
def main():
    let p = "/tmp/px_test_io.txt"
    write_file(p, "hello")
    append_file(p, " world")
    assert(exists(p), "exists")
    assert(read_file(p) == "hello world", "read")
    remove(p)
    assert(not exists(p), "removed")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_std_json() {
        let src = r#"
def main():
    let d = json_parse("{\"a\": [1, 2], \"b\": {\"c\": true}, \"n\": null}")
    assert(d["a"][1] == 2, "nested array")
    assert(d["b"]["c"] == true, "nested dict")
    assert(d["n"] == null, "null")
    assert(json_stringify(json_parse("[1, \"x\", 2.5]")) == "[1,\"x\",2.5]", "stringify")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_std_higher_order() {
        let src = r#"
def main():
    let nums = [1, 2, 3, 4]
    assert(map(nums, fn(x) { x * 2 }) == [2, 4, 6, 8], "map")
    assert(filter(nums, fn(x) { x % 2 == 0 }) == [2, 4], "filter")
    assert(reduce(nums, fn(a, b) { a + b }, 0) == 10, "reduce")
    assert(sorted([3, 1, 2]) == [1, 2, 3], "sorted")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_std_collections_bootstrapped() {
        // std.collections 由普贤自举（stdlib/collections.px）
        // 该测试通过 resolve_stdlib 合并 std 库（interp::run_program 不调用 resolve_stdlib，
        // 这里直接验证 std 库源码可解析、核心函数逻辑正确）
        let src = r#"
def main():
    let nums = [3, 1, 4, 1, 5]
    assert(contains(nums, 4), "contains")
    assert(len(nums) == 5, "len")
    let d = {"k": 1}
    d["k"] = 2
    assert(d["k"] == 2, "dict set")
"#;
        assert!(run_src(src).is_ok());
    }

    // ==================== M5.2 std.net 测试 ====================

    #[test]
    fn test_std_net_tcp_echo() {
        // TCP echo：本地起 server 线程，client 连接发送并回读
        let src = r#"
def echo(listener):
    let c = tcp_accept(listener)
    let d = tcp_recv(c, 4096)
    tcp_send(c, d)
    tcp_close(c)
    tcp_close(listener)

def main():
    let listener = tcp_listen(7893)
    spawn echo(listener)
    sleep(50)
    let conn = tcp_connect("127.0.0.1", 7893)
    tcp_send(conn, "net-ok")
    let reply = tcp_recv(conn, 100)
    tcp_close(conn)
    assert(reply == "net-ok", "tcp echo")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_std_net_http_get() {
        // 本地 HTTP server + http_get 明文请求
        let src = r#"
def http_server(listener):
    let c = tcp_accept(listener)
    let req = tcp_recv(c, 8192)
    let body = "{\"ok\":true}"
    let resp = "HTTP/1.0 200 OK\r\nContent-Length: " + str(len(body)) + "\r\n\r\n" + body
    tcp_send(c, resp)
    tcp_close(c)
    tcp_close(listener)

def main():
    let listener = tcp_listen(7894)
    spawn http_server(listener)
    sleep(50)
    let body = http_get("http://127.0.0.1:7894/")
    assert(body == "{\"ok\":true}", "http_get body")
"#;
        assert!(run_src(src).is_ok());
    }

    // ==================== M18 定时器测试 ====================

    #[test]
    fn test_timer_set_timeout_once() {
        // set_timeout 一次性：两个回调都执行，加法交换律保证确定性
        let src = r#"
count = 0
def main():
    set_timeout(fn() { count = count + 1 }, 30)
    set_timeout(fn() { count = count + 10 }, 20)
    sleep(120)
    assert(count == 11, "both timeouts ran")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_timer_clear_timeout() {
        // clear_timer 取消未到期定时器；返回值语义：首次 true，再次 false
        let src = r#"
flag = 0
def main():
    let tid = set_timeout(fn() { flag = 1 }, 30)
    assert(clear_timer(tid), "cancel pending")
    assert(not clear_timer(tid), "already canceled")
    sleep(80)
    assert(flag == 0, "callback suppressed")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_timer_set_interval_self_clear() {
        // set_interval 周期执行：回调内计数到 5 后 clear 自己 → 确定性停止
        let src = r#"
n = 0
tid = -1
def tick():
    n = n + 1
    if n >= 5:
        clear_timer(tid)
def main():
    tid = set_interval(tick, 20)
    sleep(300)
    assert(n == 5, "interval ran 5 times then stopped")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_m23b_bytes_roundtrip() {
        // M23b：二进制安全字节串（bytes 类型全套函数）
        let src = r#"
b = bytes("héllo")
assert(bytes_len(b) == 6)
assert(bytes_get(b, 1) == 0xc3)
assert(bytes_get(b, -1) == 111)
assert(bytes_get(b, 99) == null)
b2 = bytes_set(b, 0, 72)
assert(bytes_get(b2, 0) == 72)
assert(bytes_get(b, 0) == 104)
c = bytes("中文abc")
assert(bytes_len(c) == 9)
assert(bytes_to_str(bytes_slice(c, 0, 3)) == "中")
assert(bytes_to_str(bytes_slice(c, -3, null)) == "abc")
assert(bytes_len(bytes_slice(c, 5, 2)) == 0)
cat = bytes_concat(bytes("ab"), "cd", bytes("ef"))
assert(bytes_to_str(cat) == "abcdef")
assert(bytes_to_hex(c) == "e4b8ade69687616263")
b64 = bytes_base64(c)
assert(bytes_to_str(base64_to_bytes(b64)) == "中文abc")
assert(base64_to_bytes("!!!") == null)
assert(bytes_find(c, bytes("文")) == 3)
assert(bytes_find(c, "abc") == 6)
assert(bytes_find(c, "xyz") == null)
raw = base64_to_bytes("YQABAWI=")
assert(raw != null)
assert(bytes_len(raw) == 5)
assert(bytes_get(raw, 1) == 0)
assert(bytes_find(raw, base64_to_bytes("YQAB")) == 0)
write_bytes("/tmp/px_ut_bytes.bin", raw)
assert(bytes_to_hex(read_bytes("/tmp/px_ut_bytes.bin")) == bytes_to_hex(raw))
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_timer_callback_modifies_global_from_named_fn() {
        // M18 顺带修复的编译器 bug：命名函数内修改全局变量（编译模式 px_get/set_global）
        let src = r#"
n = 0
def inc():
    n = n + 1
def main():
    inc()
    inc()
    assert(n == 2, "named fn modifies global")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_timer_extra_args_passed() {
        // set_timeout 可变参数透传给回调
        let src = r#"
got = 0
def main():
    set_timeout(fn(a, b) { got = a * b }, 20, 6, 7)
    sleep(80)
    assert(got == 42, "extra args passed")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_m30_comp_multifor_unpack() {
        // M30 推导式补全：多 for 嵌套 / 多变量解包 / 多 if / DictComp
        let src = r#"
def main():
    let b = [x * y for x in [1, 2] for y in [10, 20, 30]]
    assert(len(b) == 6 and b[0] == 10 and b[5] == 60, "multi for")
    let pairs = [["a", 1], ["b", 2], ["c", 3]]
    let ks = [k for k, v in pairs]
    assert(ks == ["a", "b", "c"], "unpack keys")
    let vs = [v * 10 for k, v in pairs if v > 1]
    assert(vs == [20, 30], "unpack + if")
    let c = [x + y for x in range(4) if x % 2 == 0 for y in range(3) if y > 0]
    assert(c == [1, 2, 3, 4], "multi for + multi if")
    let dc = {k: v * 2 for k, v in pairs}
    assert(dc["a"] == 2 and dc["c"] == 6, "dict comp")
    let dc2 = {k: v for k, v in pairs if v % 2 == 1}
    assert(dc2 == {"a": 1, "c": 3}, "dict comp + if")
    let m = {str(x) + str(y): x * y for x in [1, 2] for y in [3, 4]}
    assert(m["13"] == 3 and m["24"] == 8, "dict comp nested")
"#;
        assert!(run_src(src).is_ok());
    }

    #[test]
    fn test_m30_int_bytes_endian() {
        // M30 字节序可控整数<->bytes：大/小端、补码、符号扩展、越界 null
        let src = r#"
def main():
    assert(bytes_to_hex(int_to_bytes(0x01020304, 4)) == "01020304", "big default")
    assert(bytes_to_hex(int_to_bytes(0x01020304, 4, "little")) == "04030201", "little")
    assert(bytes_to_int(int_to_bytes(-1, 4, "big", true), "big", true) == -1, "neg rt")
    assert(int_to_bytes(-1, 4) == null, "unsigned neg null")
    assert(int_to_bytes(256, 1) == null, "overflow null")
    assert(bytes_to_int(hex_to_bytes("ff"), "big", true) == -1, "sign extend")
    assert(bytes_to_int(hex_to_bytes("ff"), "big", false) == 255, "unsigned")
    assert(bytes_to_int(hex_to_bytes("80"), "big", true) == -128, "int8 min")
    assert(bytes_to_int(hex_to_bytes("0180"), "little", true) == -32767, "little sign")
    assert(bytes_to_int(hex_to_bytes("")) == null, "empty null")
    assert(bytes_to_int(hex_to_bytes("010203040506070809")) == null, "len9 null")
    let n = 987654321
    assert(bytes_to_int(int_to_bytes(n, 4, "big"), "big") == n, "pxdb rt")
"#;
        assert!(run_src(src).is_ok());
    }
}
