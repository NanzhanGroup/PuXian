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
    /// 生成器对象（M32：生成器表达式 (x for x in xs)；M34：惰性——单层 for 延迟求值）
    Gen(Arc<Mutex<GenObj>>),
    /// Result 值（M39：Ok(T) | Err(E)，spec §3.5 错误处理唯一通道；
    /// Option 无运行时包装：None 即 null，Some(x) 即 x 本身）
    Result { ok: bool, value: Box<Value> },
}

/// 生成器对象：物化结果 + 游标 +（M34 惰性）序列 + 变换/过滤闭包
/// - M32：创建时立即物化（多层/解包场景），cursor 记录 gen_next 位置（Python 语义：耗尽后 null）
/// - M34：单层 for 惰性——保存 seq（不展开 range）+ transform/filter 普贤闭包，
///   gen_next 逐项求值（真延迟）；for-in / list() / len 时先物化剩余保持行为一致
pub struct GenObj {
    pub materialized: Vec<Value>,
    pub cursor: usize,
    pub lazy: Option<LazyGen>,
}

/// M34 惰性生成器状态：从 seq 逐项取 → filter 检查 → transform 求值
#[derive(Clone)]
pub struct LazyGen {
    /// 迭代源：list / range（不展开）
    pub seq: Value,
    /// 已消费位置
    pub cursor: usize,
    /// 变换闭包 fn(x){expr}
    pub transform: Value,
    /// 过滤闭包 fn(x){cond}（None = 全通过）
    pub filter: Option<Value>,
}

impl std::fmt::Debug for GenObj {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(
            f,
            "Gen(items={}, cursor={}, lazy={})",
            self.materialized.len(),
            self.cursor,
            self.lazy.is_some()
        )
    }
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
    // M30 P1：字节序可控整数↔bytes（pxdb 存储基石）
    // int_to_bytes(n, size[, endian[, signed]]) → bytes|null（大/小端、补码/无符号）
    // bytes_to_int(b[, endian[, signed]]) → int|null（长度 1..8，越界 null）
    IntToBytes,
    BytesToInt,
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
    // M28 P1：路由表 + 中间件（WebServer 生产化）
    // route(method, pattern, handler) → bool：注册路由（:id 参数 / * 通配）
    // middleware(fn) → bool：注册中间件（fn(req) → null 继续 / 非 null 短路）
    Route,
    Middleware,
    // M28 P1：时间 / 时区（日志时间戳 / 到期计算 / TZ 转换）
    // time_format(ts, fmt[, tz]) → str（%Y-%m-%d %H:%M:%S %z 等；tz 默认 UTC，支持 +08:00）
    // time_parse(str, fmt[, tz]) → int|null（解析回 epoch 秒，失败 null）
    // tz_offset(tz) → int（时区偏移秒）
    TimeFormat,
    TimeParse,
    TzOffset,
    // M28 P1：cron 定时调度（6 字段：秒 分 时 日 月 周）
    // cron(expr, fn, ...args) → int（后台每秒 tick 匹配触发；clear_timer(id) 取消）
    Cron,
    // M28 P1：SQLite 绑定（SQL 执行 + 参数绑定 + 结果集）
    // sqlite_open(path) → int|null / sqlite_exec(db, sql) → int
    // sqlite_query(db, sql[, params]) → list[dict]|null（params: list→? / dict→:name）
    // sqlite_close(db) → bool / sqlite_escape(s) → str
    SqliteOpen,
    SqliteExec,
    SqliteQuery,
    SqliteClose,
    SqliteEscape,
    SqliteLastInsertRowid,
    // M29：JSON 路径运算符（JSONB 基石，清歌场景 A pxdb）
    // json_path(json_or_str, "$.a[0].b") → Value|null（按路径取；支持 .key ["key"] [n] 负索引）
    // json_path_set(json_or_str, path, value) → Value（返回更新后的新值；路径不存在自动创建）
    JsonPath,
    JsonPathSet,
    // M31 P2：安全/多租户/防爆破三件套
    // sandbox_enter(opts{memory_mb, deny, drop_priv}) → bool：进程级沙箱
    //   memory_mb：内存上限（setrlimit RLIMIT_AS = 当前 VSS + memory_mb）
    //   deny：禁用的内置函数名列表（后续调用该函数报错"沙箱：函数 X 已被禁用"）
    //   drop_priv：root 降权到 nobody(uid/gid 65534)
    // vhost(host, docroot|handler) → bool：虚拟主机（Host 头路由，多域名共服）
    //   host 支持 "a.com" / "a.com:8080" / "*"（默认）；docroot 为根目录（str）
    //   或 handler 为函数（该域所有请求交给 handler(req)）
    // rate_limit(key, max, window_sec) → bool：滑动窗口限流（true 放行 / false 超限）
    //   服务端内置：px_serve opts{rate_limit:{max,window_sec}} 按 IP 限流 → 429
    SandboxEnter,
    Vhost,
    RateLimit,
    // M32：生成器表达式（延迟物化）
    // gen_next(g) → Value|null（逐项取值；耗尽后 null）
    // list(x) → list（list/range/gen/tuple → list；str → chars；dict → keys）
    GenNext,
    List,
    // M33：HTTP/3 / QUIC 预研——UDP 基础设施（QUIC 底层是 UDP socket）
    // udp_open(port) → int（bind 0.0.0.0:port；返回 socket id）
    // udp_send(sock, ip, port, data) → int（发送字节数）
    // udp_recv(sock, maxlen) → dict{data, ip, port} | null（阻塞接收）
    // udp_close(sock) → bool
    UdpOpen,
    UdpSend,
    UdpRecv,
    UdpClose,
    // M34：WebSocket 服务端广播 + 事件总线（pub/sub）
    // ws_broadcast(data) → int：向 ws_serve 全部活跃连接群发，返回成功数
    // event_bus() → int（创建事件总线）；bus_subscribe(bus, topic, fn) → bool
    // bus_publish(bus, topic, data) → int（同步调用所有订阅者 fn(topic, data)）
    // bus_unsubscribe(bus, topic, fn) → bool
    WsBroadcast,
    BusNew,
    BusSubscribe,
    BusPublish,
    BusUnsubscribe,
    // M36：请求上下文（线程局部：中间件 ctx_set 传值，handler ctx_get 读取；每请求自动清除）
    // ctx_set(key, value) / ctx_get(key) / ctx_clear()
    CtxSet,
    CtxGet,
    CtxClear,
    // M37：S3/MinIO 对象存储客户端（AWS SigV4）
    // s3_put(endpoint, bucket, key, data, ak, sk) → bool
    // s3_get(endpoint, bucket, key, ak, sk) → str|null
    // s3_delete(endpoint, bucket, key, ak, sk) → bool
    // s3_list(endpoint, bucket, prefix, ak, sk) → list
    S3Put,
    S3Get,
    S3Delete,
    S3List,
    // M38：UDP echo 服务端（udp_serve(port, handler)：handler(ip, port, data) → 响应发送回对端）
    UdpServe,
    // M38：WebSocket 客户端自动重连（ws_connect_auto(url, reconnect_ms)：断线自动重连）
    WsConnectAuto,
    // M39：Result/Option 构造函数（spec §3.5 错误处理唯一通道）
    // Ok(x) → Result{ok:true, x}；Err(e) → Result{ok:false, e}；Some(x) → x（None 即 null）
    Ok_,
    Err_,
    Some_,
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
            Builtin::IntToBytes => "int_to_bytes",
            Builtin::BytesToInt => "bytes_to_int",
            Builtin::Gc => "gc",
            Builtin::TlsServer => "tls_server",
            Builtin::SessionOpen => "session_open",
            Builtin::SessionId => "session_id",
            Builtin::SessionGet => "session_get",
            Builtin::SessionSet => "session_set",
            Builtin::SessionDel => "session_del",
            Builtin::SessionDestroy => "session_destroy",
            Builtin::BasicAuth => "basic_auth",
            Builtin::Route => "route",
            Builtin::Middleware => "middleware",
            Builtin::TimeFormat => "time_format",
            Builtin::TimeParse => "time_parse",
            Builtin::TzOffset => "tz_offset",
            Builtin::Cron => "cron",
            Builtin::SqliteOpen => "sqlite_open",
            Builtin::SqliteExec => "sqlite_exec",
            Builtin::SqliteQuery => "sqlite_query",
            Builtin::SqliteClose => "sqlite_close",
            Builtin::SqliteEscape => "sqlite_escape",
            Builtin::SqliteLastInsertRowid => "sqlite_last_insert_rowid",
            Builtin::JsonPath => "json_path",
            Builtin::JsonPathSet => "json_path_set",
            Builtin::SandboxEnter => "sandbox_enter",
            Builtin::Vhost => "vhost",
            Builtin::RateLimit => "rate_limit",
            Builtin::GenNext => "gen_next",
            Builtin::List => "list",
            Builtin::UdpOpen => "udp_open",
            Builtin::UdpSend => "udp_send",
            Builtin::UdpRecv => "udp_recv",
            Builtin::UdpClose => "udp_close",
            Builtin::WsBroadcast => "ws_broadcast",
            Builtin::BusNew => "event_bus",
            Builtin::BusSubscribe => "bus_subscribe",
            Builtin::BusPublish => "bus_publish",
            Builtin::BusUnsubscribe => "bus_unsubscribe",
            Builtin::CtxSet => "ctx_set",
            Builtin::CtxGet => "ctx_get",
            Builtin::CtxClear => "ctx_clear",
            Builtin::S3Put => "s3_put",
            Builtin::S3Get => "s3_get",
            Builtin::S3Delete => "s3_delete",
            Builtin::S3List => "s3_list",
            Builtin::UdpServe => "udp_serve",
            Builtin::WsConnectAuto => "ws_connect_auto",
            Builtin::Ok_ => "Ok",
            Builtin::Err_ => "Err",
            Builtin::Some_ => "Some",
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
        Value::Gen(g) => {
            let g = g.lock().unwrap();
            format!(
                "<gen {} items, cursor={}>",
                g.materialized.len(),
                g.cursor
            )
        }
        Value::Result { ok, value } => {
            if *ok {
                format!("Ok({})", fmt_value(value))
            } else {
                format!("Err({})", fmt_value(value))
            }
        }
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
            (Value::Gen(a), Value::Gen(b)) => {
                let (ga, gb) = (a.lock().unwrap(), b.lock().unwrap());
                ga.materialized == gb.materialized
            }
            (
                Value::Result { ok: a_ok, value: a_v },
                Value::Result { ok: b_ok, value: b_v },
            ) => a_ok == b_ok && a_v == b_v,
            _ => false,
        }
    }
}
