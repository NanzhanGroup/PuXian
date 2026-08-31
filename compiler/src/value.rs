//! 普贤 (PuXian) 运行时值表示
//! 脚本模式（解释器）使用；编译模式（M4）生成对等 C 结构

use std::sync::{Arc, Condvar, Mutex};
use std::collections::HashMap;
use std::collections::VecDeque;
use std::fmt;

use crate::ast::{Param, Stmt, TypeExpr};
use crate::env::EnvRef;

/// 运行时值
#[derive(Debug, Clone)]
pub enum Value {
    Int(i64),
    Float(f64),
    Str(String),
    /// M23b：二进制安全字节串（带长度，可含任意字节含 NUL；存储/加密基石）
    Bytes(Vec<u8>),
    Bool(bool),
    Null,
    /// 可变列表（Arc<Mutex> 共享可变，方法 append/pop 等就地修改）
    List(Arc<Mutex<Vec<Value>>>),
    /// 不可变元组
    Tuple(Vec<Value>),
    /// 可变字典（键限定为字符串，M2 简化）
    Dict(Arc<Mutex<HashMap<String, Value>>>),
    /// 用户函数（含闭包环境）
    Func(Arc<Function>),
    /// 内置函数
    Builtin(Builtin),
    /// 结构体实例
    StructInstance {
        type_name: String,
        fields: Arc<Mutex<HashMap<String, Value>>>,
    },
    /// 枚举值
    EnumValue {
        type_name: String,
        variant: String,
        payload: Option<Box<Value>>,
    },
    /// 惰性区间（range() 返回，for 迭代）
    Range { start: i64, end: i64, step: i64 },
    /// 通道（M3 协程运行时；M2 先同步阻塞版）
    Chan(ChanRef),
    /// 互斥锁（M13：P1 锁原语，pxdb 并发场景）
    Mutex(MutexRef),
    /// 读写锁（M13：读多写少场景，多读者并行）
    RWLock(RWLockRef),
    /// 类型对象（Color.Red 枚举构造 / Point(1,2) 结构体构造）
    TypeRef(TypeRefKind),
}

#[derive(Debug, Clone)]
pub enum TypeRefKind {
    Struct(String),
    Enum(String),
}

/// 内置函数标识（call_builtin 中分派）
#[derive(Debug, Clone, Copy, PartialEq)]
pub enum Builtin {
    Print,
    Len,
    Range,
    Type,
    Str,
    Int,
    Float,
    Bool,
    Assert,
    Panic,
    Input,
    Exit,
    Sleep,
    // std.string 快捷
    ToUpper,
    ToLower,
    Trim,
    Split,
    Join,
    Contains,
    Replace,
    StartsWith,
    EndsWith,
    // std.math 快捷
    Abs,
    Sqrt,
    Min,
    Max,
    Pow,
    // std.collections 快捷
    Sorted,
    Reversed,
    Sum,
    // std.time 快捷
    NowMs,
    // ==================== M5 标准库 ====================
    // std.io
    ReadFile,
    WriteFile,
    AppendFile,
    // M12 P0：文件随机读写 + fsync（WAL / 增量日志基石）
    ReadAt,
    WriteAt,
    FileSize,
    FsyncFile,
    TruncateFile,
    // M14 P1：crypto 哈希（签名校验 / 缓存 key / 数据指纹）
    Sha256,
    Xxhash,
    // M15 P1：正则表达式（文本解析 / 日志分析 / 参数抽取）
    RegexFind,
    RegexMatch,
    RegexSearch,
    RegexFindAll,
    RegexReplace,
    RegexSplit,
    // std.fs
    Exists,
    ListDir,
    Mkdir,
    Remove,
    // std.json
    JsonParse,
    JsonStringify,
    // std.time
    Now,
    // std.os
    Env,
    Args,
    // std.collections（高阶函数，可调用普贤函数值）
    Map,
    Filter,
    Reduce,
    // std.net（M5.2）：TCP + HTTP 客户端
    TcpListen,
    TcpAccept,
    TcpConnect,
    TcpSend,
    TcpRecv,
    TcpClose,
    HttpGet,
    HttpPost,
    // M23c P1：HTTP 生产化——http_request(url, method, body?, headers?) → dict{status,headers,body}
    //（keep-alive 连接池复用，同 host 连接复用）+ http_get_stream(url, chunk_handler) 流式下载
    HttpRequest,
    HttpGetStream,
    HttpServe,
    // M17 P1：.px 脚本执行机制（PHP/OpenResty 式应用平台）
    // px_exec(path, params?) —— 内嵌解释器执行 .px 脚本，捕获 print 输出返回（语言层嵌入 API）
    // px_serve(port, docroot[, timeout_ms]) —— PHP 式应用服务器：静态文件 + .px 脚本执行
    PxExec,
    PxServe,
    // M18 P1：后台定时任务 / 定时器原语（TTL 过期扫描 / 定期快照落盘 / 会话清理）
    // set_timeout(fn, ms, ...args) → int 一次性定时器；set_interval(fn, ms, ...args) → int 周期定时器
    // clear_timer(id) → bool 取消定时器（已执行/已取消返回 false）
    SetTimeout,
    SetInterval,
    ClearTimer,
    // M19 P1：AES 加密（企微回调加解密 / 数据落盘加密 / Cookie 签名）
    // aes_encrypt(data, key, iv) → hex（AES-CBC-PKCS7，key 16/24/32 字节 → 128/192/256 位）
    // aes_decrypt(hex, key, iv) → str 或 null（padding 非法 / 非 UTF-8 → null）
    // aes_gcm_encrypt(data, key, iv) → hex（密文 + 16 字节 tag）
    // aes_gcm_decrypt(hex, key, iv) → str 或 null（tag 校验失败 → null）
    AesEncrypt,
    AesDecrypt,
    AesGcmEncrypt,
    AesGcmDecrypt,
    // M19 P1：XML 解析（企微回调 Encrypt 报文 / 配置文件 / 文档）
    // xml_parse(xml) → dict{name, attrs, children, text} 或 null；xml_escape / xml_unescape
    // M24：xml_build(node) → str（从 dict 树生成 XML，与 xml_parse 结构对称）
    XmlParse,
    XmlEscape,
    XmlUnescape,
    XmlBuild,
    // M19 P1：zip 打包/解压（docx/xlsx/pptx 本质是 zip+xml，文档工具基石）
    // zip_pack(files, out_path) → bool；zip_unpack(zip_path, out_dir) → int 解压文件数
    ZipPack,
    ZipUnpack,
    // M21 P1：base64 编解码（字节传输 / 数据落盘 / 数据 URI）
    // base64_encode(data) → str（RFC 4648 标准，带 padding）
    // base64_decode(b64) → str 或 null（非法输入返回 null，不抛错）
    Base64Encode,
    Base64Decode,
    // M21 P1：SSE 服务端（LLM 流式推送 / 实时通知 / 服务器推送）
    // sse_serve(port, handler) —— 启动 SSE 服务器（每连接一线程，handler 内可 sse_send）
    // sse_send(conn, data) —— 向指定连接发送 SSE 帧（任意线程可调用）
    // sse_close(conn) —— 结束 SSE 连接
    SseServe,
    SseSend,
    SseClose,
    // M23 P1：SSE 客户端（LLM 流式消费 / 事件订阅，网络层收尾）
    // sse_connect(url) → int conn | null（HTTP GET 握手，校验 text/event-stream）
    // sse_read(conn) → dict{event,data,id,retry} | null（阻塞读一条事件；断开 → null）
    // sse_close(conn) → bool（关闭客户端连接）
    SseConnect,
    SseRead,
    // M22 P1：位运算 / 二进制数据视图（存储引擎序列化基石；运算符 & | ^ ~ << >> 已具备）
    // int_to_hex(n, width) → str（固定宽度小写 hex，负数按补码）
    // hex_to_int(hex) → int 或 null（非法 → null）
    // bytes_to_hex(data) → str（字节 → 小写 hex）
    // hex_to_bytes(hex) → str 或 null（hex → 原始字节，非法 → null）
    // bit_count(n) → int（popcount：二进制中 1 的个数）
    // bit_length(n) → int（二进制位数：floor(log2(n))+1；n<=0 → 0）
    IntToHex,
    HexToInt,
    BytesToHex,
    HexToBytes,
    BitCount,
    BitLength,
    // M22 P1：WebSocket（RFC 6455，微信/QQ/飞书长连接 / LLM 流式 / 实时推送）
    // ws_serve(port, handler) —— 服务端：每连接握手后调 handler(conn)
    // ws_connect(host, port, path) → int conn | null（客户端握手）
    // ws_send(conn, data) → bool（文本帧）；ws_recv(conn) → str|null（阻塞读一条消息）
    // ws_close(conn) → bool（发送 close 帧并关闭）
    WsServe,
    WsConnect,
    WsSend,
    WsRecv,
    WsClose,
    // M23 P1：WebSocket 心跳/超时（长连接保活）
    // ws_ping(conn) → bool（发送 ping 帧；对端应回 pong）
    // ws_recv(conn[, timeout_ms]) → str|null（可选超时：超时/断开 → null）
    // M26：ws_heartbeat(conn, interval_ms, timeout_ms) → bool（内置自动心跳：定时 ping + 死链检测）
    WsPing,
    WsHeartbeat,
    // M23 P1：进程/信号（文殊场景收尾：外部工具编排、守护进程、优雅停机）
    // os_pid() → int 当前进程 PID
    // os_spawn(cmd, args) → int pid | null（fork+exec 启动子进程，不等待）
    // os_wait(pid) → int 退出码（信号终止 → 128+sig；失败 → -1）
    // os_kill(pid, sig) → bool（发送信号）
    // signal(sig, handler) → bool（注册信号处理回调，信号到达时在新线程调 handler(sig)）
    OsPid,
    OsSpawn,
    OsWait,
    OsKill,
    Signal,
    // M23d P1：RSA 非对称加密（PKCS#1 v1.5；密钥/密文/签名均 hex 字符串）
    // rsa_gen_key(bits) → dict{n,e,d,p,q}（hex；bits ≥ 512）
    // rsa_encrypt(data, n, e) → hex 密文 | null（type 2；数据 ≤ 模长-11 字节）
    // rsa_decrypt(ct_hex, n, d) → 明文 | null（type 2 解码）
    // rsa_sign(data, n, d) → hex 签名 | null（type 1；直接签数据，不包 DigestInfo）
    // rsa_verify(data, sig_hex, n, e) → bool
    RsaGenKey,
    RsaEncrypt,
    RsaDecrypt,
    RsaSign,
    RsaVerify,
    // M23b P1：二进制安全字节串（带长度字符串 / bytes；存储与加密基石）
    // bytes(s) → bytes（字符串 UTF-8 字节原样）
    // bytes_len(b) → int / bytes_get(b, i) → int|null / bytes_set(b, i, v) → bool
    // bytes_slice(b, start, end) → bytes（负索引/越界 clamp，同切片语义）
    // bytes_concat(a, b, ...) → bytes / bytes_append(a, b) → bytes
    // bytes_to_str(b) → str（UTF-8 lossy）/ bytes_to_hex(b) → hex
    // bytes_base64(b) → base64 / base64_to_bytes(s) → bytes|null（严格）
    // bytes_find(b, sub) → int|null（子串下标）
    // read_bytes(path) → bytes / write_bytes(path, b) → bool
    Bytes,
    BytesLen,
    BytesGet,
    BytesSet,
    BytesSlice,
    BytesConcat,
    BytesToStr,
    BytesBase64,
    Base64ToBytes,
    BytesFind,
    ReadBytes,
    WriteBytes,
    // M22 P1：解释器循环引用回收（追踪式 GC）
    // gc() → int（强制运行一次垃圾回收；返回 0 = 有并发线程跳过，1 = 已执行）
    Gc,
    // M27 P0：WebServer 生产化四件套
    // ① 服务端 TLS：tls_server(cert_pem, key_pem) → bool（PEM 路径或内容；注册后
    //    px_serve/ws_serve/sse_serve 自动 HTTPS/WSS/SSE-over-TLS）
    // ② 请求体流式+大小可配：px_serve(port, docroot, timeout_ms, opts{max_body_size, body_tmp_dir})
    // ③ Cookie/Session/认证：REQUEST.cookie；session_open()/session_id()/session_get(k)/
    //    session_set(k,v)/session_del(k)/session_destroy()；basic_auth(user, pass) → bool
    // ④ 优雅关闭：SIGINT/SIGTERM → px_serve 停止 accept + 等在途请求
    TlsServer,
    SessionOpen,
    SessionId,
    SessionGet,
    SessionSet,
    SessionDel,
    SessionDestroy,
    BasicAuth,
}

impl Builtin {
    pub fn name(&self) -> &'static str {
        match self {
            Builtin::Print => "print",
            Builtin::Len => "len",
            Builtin::Range => "range",
            Builtin::Type => "type",
            Builtin::Str => "str",
            Builtin::Int => "int",
            Builtin::Float => "float",
            Builtin::Bool => "bool",
            Builtin::Assert => "assert",
            Builtin::Panic => "panic",
            Builtin::Input => "input",
            Builtin::Exit => "exit",
            Builtin::Sleep => "sleep",
            Builtin::ToUpper => "to_upper",
            Builtin::ToLower => "to_lower",
            Builtin::Trim => "trim",
            Builtin::Split => "split",
            Builtin::Join => "join",
            Builtin::Contains => "contains",
            Builtin::Replace => "replace",
            Builtin::StartsWith => "starts_with",
            Builtin::EndsWith => "ends_with",
            Builtin::Abs => "abs",
            Builtin::Sqrt => "sqrt",
            Builtin::Min => "min",
            Builtin::Max => "max",
            Builtin::Pow => "pow",
            Builtin::Sorted => "sorted",
            Builtin::Reversed => "reversed",
            Builtin::Sum => "sum",
            Builtin::NowMs => "now_ms",
            Builtin::ReadFile => "read_file",
            Builtin::WriteFile => "write_file",
            Builtin::AppendFile => "append_file",
            Builtin::ReadAt => "read_at",
            Builtin::WriteAt => "write_at",
            Builtin::FileSize => "file_size",
            Builtin::FsyncFile => "fsync_file",
            Builtin::TruncateFile => "truncate_file",
            Builtin::Sha256 => "sha256",
            Builtin::Xxhash => "xxhash",
            Builtin::RegexFind => "regex_find",
            Builtin::RegexMatch => "regex_match",
            Builtin::RegexSearch => "regex_search",
            Builtin::RegexFindAll => "regex_find_all",
            Builtin::RegexReplace => "regex_replace",
            Builtin::RegexSplit => "regex_split",
            Builtin::Exists => "exists",
            Builtin::ListDir => "list_dir",
            Builtin::Mkdir => "mkdir",
            Builtin::Remove => "remove",
            Builtin::JsonParse => "json_parse",
            Builtin::JsonStringify => "json_stringify",
            Builtin::Now => "now",
            Builtin::Env => "env",
            Builtin::Args => "args",
            Builtin::Map => "map",
            Builtin::Filter => "filter",
            Builtin::Reduce => "reduce",
            Builtin::TcpListen => "tcp_listen",
            Builtin::TcpAccept => "tcp_accept",
            Builtin::TcpConnect => "tcp_connect",
            Builtin::TcpSend => "tcp_send",
            Builtin::TcpRecv => "tcp_recv",
            Builtin::TcpClose => "tcp_close",
            Builtin::HttpGet => "http_get",
            Builtin::HttpPost => "http_post",
            Builtin::HttpRequest => "http_request",
            Builtin::HttpGetStream => "http_get_stream",
            Builtin::HttpServe => "http_serve",
            Builtin::PxExec => "px_exec",
            Builtin::PxServe => "px_serve",
            Builtin::SetTimeout => "set_timeout",
            Builtin::SetInterval => "set_interval",
            Builtin::ClearTimer => "clear_timer",
            Builtin::AesEncrypt => "aes_encrypt",
            Builtin::AesDecrypt => "aes_decrypt",
            Builtin::AesGcmEncrypt => "aes_gcm_encrypt",
            Builtin::AesGcmDecrypt => "aes_gcm_decrypt",
            Builtin::XmlParse => "xml_parse",
            Builtin::XmlEscape => "xml_escape",
            Builtin::XmlUnescape => "xml_unescape",
            Builtin::XmlBuild => "xml_build",
            Builtin::ZipPack => "zip_pack",
            Builtin::ZipUnpack => "zip_unpack",
            Builtin::Base64Encode => "base64_encode",
            Builtin::Base64Decode => "base64_decode",
            Builtin::SseServe => "sse_serve",
            Builtin::SseSend => "sse_send",
            Builtin::SseClose => "sse_close",
            Builtin::SseConnect => "sse_connect",
            Builtin::SseRead => "sse_read",
            Builtin::IntToHex => "int_to_hex",
            Builtin::HexToInt => "hex_to_int",
            Builtin::BytesToHex => "bytes_to_hex",
            Builtin::HexToBytes => "hex_to_bytes",
            Builtin::BitCount => "bit_count",
            Builtin::BitLength => "bit_length",
            Builtin::WsServe => "ws_serve",
            Builtin::WsConnect => "ws_connect",
            Builtin::WsSend => "ws_send",
            Builtin::WsRecv => "ws_recv",
            Builtin::WsClose => "ws_close",
            Builtin::WsPing => "ws_ping",
            Builtin::WsHeartbeat => "ws_heartbeat",
            Builtin::OsPid => "os_pid",
            Builtin::OsSpawn => "os_spawn",
            Builtin::OsWait => "os_wait",
            Builtin::OsKill => "os_kill",
            Builtin::Signal => "signal",
            Builtin::RsaGenKey => "rsa_gen_key",
            Builtin::RsaEncrypt => "rsa_encrypt",
            Builtin::RsaDecrypt => "rsa_decrypt",
            Builtin::RsaSign => "rsa_sign",
            Builtin::RsaVerify => "rsa_verify",
            Builtin::Bytes => "bytes",
            Builtin::BytesLen => "bytes_len",
            Builtin::BytesGet => "bytes_get",
            Builtin::BytesSet => "bytes_set",
            Builtin::BytesSlice => "bytes_slice",
            Builtin::BytesConcat => "bytes_concat",
            Builtin::BytesToStr => "bytes_to_str",
            Builtin::BytesBase64 => "bytes_base64",
            Builtin::Base64ToBytes => "base64_to_bytes",
            Builtin::BytesFind => "bytes_find",
            Builtin::ReadBytes => "read_bytes",
            Builtin::WriteBytes => "write_bytes",
            Builtin::Gc => "gc",
            Builtin::TlsServer => "tls_server",
            Builtin::SessionOpen => "session_open",
            Builtin::SessionId => "session_id",
            Builtin::SessionGet => "session_get",
            Builtin::SessionSet => "session_set",
            Builtin::SessionDel => "session_del",
            Builtin::SessionDestroy => "session_destroy",
            Builtin::BasicAuth => "basic_auth",
        }
    }
}

/// 容器构造辅助（M22 循环引用回收：创建时注册 Weak 到 GC 注册表）
impl Value {
    pub fn new_list(inner: Vec<Value>) -> Value {
        let arc = Arc::new(Mutex::new(inner));
        crate::gc::register_list(&arc);
        Value::List(arc)
    }
    pub fn new_dict(inner: HashMap<String, Value>) -> Value {
        let arc = Arc::new(Mutex::new(inner));
        crate::gc::register_dict(&arc);
        Value::Dict(arc)
    }
    pub fn new_struct_fields(fields: HashMap<String, Value>) -> Arc<Mutex<HashMap<String, Value>>> {
        let arc = Arc::new(Mutex::new(fields));
        crate::gc::register_struct(&arc);
        arc
    }
    pub fn new_chan(chan: ChanRef) -> Value {
        crate::gc::register_chan(&chan);
        Value::Chan(chan)
    }
}

/// 用户函数
#[derive(Debug)]
pub struct Function {    pub name: String,
    pub params: Vec<Param>,
    #[allow(dead_code)]
    pub ret_ty: Option<TypeExpr>,
    pub body: Vec<Stmt>,
    pub closure: EnvRef,
    /// 闭包（fn 表达式）为 true：函数体是块表达式，末尾隐式返回最后表达式值
    pub implicit_return: bool,
}

/// 通道引用（M3：Arc + Mutex + Condvar，支持跨线程与阻塞语义）
pub type ChanRef = Arc<ChanState>;

#[derive(Debug)]
pub struct ChanState {
    pub inner: Mutex<ChanInner>,
    pub cv: Condvar,
}

#[derive(Debug)]
pub struct ChanInner {
    pub buf: VecDeque<Value>,
    pub cap: usize,
    /// 0 = 无缓冲（同步 rendezvous）；>0 = 有缓冲
    pub closed: bool,
    /// 等待接收的协程数（无缓冲通道 send 需等待 recv 就绪）
    pub recv_waiting: usize,
}

// ==================== 锁原语（M13：P1 mutex / rwlock） ====================
// 与 chan 同构：Arc 共享 + Mutex/Condvar 实现真正阻塞（不忙等）
// 编译模式（C 运行时）用 pthread_mutex_t / pthread_rwlock_t 对等实现

/// 互斥锁引用
pub type MutexRef = Arc<MutexState>;

#[derive(Debug)]
pub struct MutexState {
    /// 保护 held 标志的锁
    mu: Mutex<bool>,
    /// 等待者（lock 阻塞时挂起，unlock 唤醒）
    cv: Condvar,
}

impl MutexState {
    pub fn new() -> Self {
        Self {
            mu: Mutex::new(false),
            cv: Condvar::new(),
        }
    }
    /// 阻塞加锁
    pub fn lock(&self) {
        let mut g = self.mu.lock().unwrap();
        while *g {
            g = self.cv.wait(g).unwrap();
        }
        *g = true;
    }
    /// 非阻塞尝试加锁：成功返回 true
    pub fn try_lock(&self) -> bool {
        let mut g = self.mu.lock().unwrap();
        if *g {
            return false;
        }
        *g = true;
        true
    }
    /// 解锁（唤醒一个等待者）
    pub fn unlock(&self) {
        let mut g = self.mu.lock().unwrap();
        *g = false;
        self.cv.notify_one();
    }
}

/// 读写锁引用
pub type RWLockRef = Arc<RWLockState>;

#[derive(Debug)]
pub struct RWLockState {
    mu: Mutex<RWLockInner>,
    cv: Condvar,
}

#[derive(Debug)]
struct RWLockInner {
    readers: u32,
    writer: bool,
    /// 等待中的写者数（写优先：写者等待期间阻塞新读者，防读饿死写）
    writer_waiting: u32,
}

impl RWLockState {
    pub fn new() -> Self {
        Self {
            mu: Mutex::new(RWLockInner {
                readers: 0,
                writer: false,
                writer_waiting: 0,
            }),
            cv: Condvar::new(),
        }
    }
    /// 读锁：多读者并行；有写者持有或等待时阻塞
    pub fn rlock(&self) {
        let mut g = self.mu.lock().unwrap();
        while g.writer || g.writer_waiting > 0 {
            g = self.cv.wait(g).unwrap();
        }
        g.readers += 1;
    }
    pub fn try_rlock(&self) -> bool {
        let mut g = self.mu.lock().unwrap();
        if g.writer || g.writer_waiting > 0 {
            return false;
        }
        g.readers += 1;
        true
    }
    pub fn runlock(&self) {
        let mut g = self.mu.lock().unwrap();
        g.readers -= 1;
        if g.readers == 0 {
            self.cv.notify_all();
        }
    }
    /// 写锁：独占；等待所有读者释放
    pub fn wlock(&self) {
        let mut g = self.mu.lock().unwrap();
        g.writer_waiting += 1;
        while g.writer || g.readers > 0 {
            g = self.cv.wait(g).unwrap();
        }
        g.writer_waiting -= 1;
        g.writer = true;
    }
    pub fn try_wlock(&self) -> bool {
        let mut g = self.mu.lock().unwrap();
        if g.writer || g.readers > 0 {
            return false;
        }
        g.writer = true;
        true
    }
    pub fn wunlock(&self) {
        let mut g = self.mu.lock().unwrap();
        g.writer = false;
        self.cv.notify_all();
    }
}

// ==================== 显示（print 输出） ====================

fn fmt_value(v: &Value) -> String {
    match v {
        Value::Int(i) => i.to_string(),
        Value::Float(f) => fmt_float(*f),
        Value::Str(s) => s.clone(),
        Value::Bytes(b) => format!("<bytes {}>", b.len()),
        Value::Bool(b) => b.to_string(),
        Value::Null => "null".to_string(),
        Value::List(items) => {
            let inner: Vec<String> = items.lock().unwrap().iter().map(fmt_value).collect();
            format!("[{}]", inner.join(", "))
        }
        Value::Tuple(items) => {
            let inner: Vec<String> = items.iter().map(fmt_value).collect();
            format!("({})", inner.join(", "))
        }
        Value::Dict(map) => {
            let mut parts: Vec<String> = map
                .lock().unwrap()
                .iter()
                .map(|(k, v)| format!("{}: {}", k, fmt_value(v)))
                .collect();
            parts.sort();
            format!("{{{}}}", parts.join(", "))
        }
        Value::Func(f) => format!("<fn {}>", f.name),
        Value::Builtin(b) => format!("<builtin {}>", b.name()),
        Value::StructInstance { type_name, .. } => {
            format!("<struct {}>", type_name)
        }
        Value::EnumValue {
            type_name,
            variant,
            payload,
        } => {
            if let Some(p) = payload {
                format!("{}.{}({})", type_name, variant, fmt_value(p))
            } else {
                format!("{}.{}", type_name, variant)
            }
        }
        Value::Range { start, end, step } => format!("range({}, {}, {})", start, end, step),
        Value::Chan(_) => "<chan>".to_string(),
        Value::Mutex(_) => "<mutex>".to_string(),
        Value::RWLock(_) => "<rwlock>".to_string(),
        Value::TypeRef(t) => match t {
            TypeRefKind::Struct(n) | TypeRefKind::Enum(n) => format!("<type {}>", n),
        },
    }
}

fn fmt_float(f: f64) -> String {
    if f == f.trunc() && f.is_finite() && f.abs() < 1e15 {
        format!("{:.1}", f)
    } else {
        format!("{}", f)
    }
}

impl fmt::Display for Value {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}", fmt_value(self))
    }
}

// ==================== 相等比较（== / !=） ====================

impl PartialEq for Value {
    fn eq(&self, other: &Self) -> bool {
        match (self, other) {
            (Value::Int(a), Value::Int(b)) => a == b,
            (Value::Int(a), Value::Float(b)) => (*a as f64) == *b,
            (Value::Float(a), Value::Int(b)) => *a == (*b as f64),
            (Value::Float(a), Value::Float(b)) => a == b,
            (Value::Str(a), Value::Str(b)) => a == b,
            (Value::Bytes(a), Value::Bytes(b)) => a == b,
            (Value::Bool(a), Value::Bool(b)) => a == b,
            (Value::Null, Value::Null) => true,
            (Value::List(a), Value::List(b)) => *a.lock().unwrap() == *b.lock().unwrap(),
            (Value::Tuple(a), Value::Tuple(b)) => a == b,
            (Value::Dict(a), Value::Dict(b)) => *a.lock().unwrap() == *b.lock().unwrap(),
            (
                Value::EnumValue {
                    type_name: ta,
                    variant: va,
                    payload: pa,
                },
                Value::EnumValue {
                    type_name: tb,
                    variant: vb,
                    payload: pb,
                },
            ) => ta == tb && va == vb && pa == pb,
            (Value::Range { .. }, Value::Range { .. }) => false,
            _ => false,
        }
    }
}
