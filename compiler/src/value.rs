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
        }
    }
}

/// 用户函数
#[derive(Debug)]
pub struct Function {
    pub name: String,
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

// ==================== 显示（print 输出） ====================

fn fmt_value(v: &Value) -> String {
    match v {
        Value::Int(i) => i.to_string(),
        Value::Float(f) => fmt_float(*f),
        Value::Str(s) => s.clone(),
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
