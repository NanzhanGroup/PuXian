//! 普贤 (PuXian) 内置函数实现（spec.md §10.2）
//! 自由函数（非方法）分派

use std::sync::{Arc, Mutex};

use crate::interp::{Interpreter, LxError};
use crate::token::Pos;
use crate::value::{Builtin, Value};

fn err(msg: impl Into<String>, pos: Pos) -> LxError {
    LxError::new("R1002", msg, Some(pos))
}

/// 调用内置函数
pub fn call_builtin(interp: &mut Interpreter, b: Builtin, args: &[Value], pos: Pos) -> Result<Value, LxError> {
    match b {
        Builtin::Print => {
            let parts: Vec<String> = args.iter().map(|v| v.to_string()).collect();
            println!("{}", parts.join(" "));
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
            Ok(Value::List(Arc::new(Mutex::new(parts))))
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
            Ok(Value::List(Arc::new(Mutex::new(sorted))))
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
            Ok(Value::List(Arc::new(Mutex::new(items))))
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
                    Ok(Value::List(Arc::new(Mutex::new(names))))
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
            Ok(Value::List(Arc::new(Mutex::new(a))))
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
            Ok(Value::List(Arc::new(Mutex::new(out))))
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
            Ok(Value::List(Arc::new(Mutex::new(out))))
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
            match http_get_impl(&url) {
                Ok(body) => Ok(Value::Str(body)),
                Err(e) => Err(LxError::new("R3009", format!("net: http_get 失败: {}", e), Some(pos))),
            }
        }
    }
}

// ==================== std.net 辅助（Rust 实现） ====================

use std::collections::HashMap;
use std::net::{TcpListener, TcpStream};
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::OnceLock;

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

/// 极简 HTTP GET（仅明文 http://，HTTP/1.0 请求，不处理重定向/chunked）
fn http_get_impl(url: &str) -> Result<String, String> {
    let rest = url
        .strip_prefix("http://")
        .ok_or_else(|| format!("仅支持 http:// 明文协议: {}", url))?;
    let (hostport, path) = match rest.find('/') {
        Some(i) => (&rest[..i], &rest[i..]),
        None => (rest, "/"),
    };
    let (host, port) = match hostport.find(':') {
        Some(i) => (
            &hostport[..i],
            hostport[i + 1..].parse::<u16>().map_err(|_| format!("端口非法: {}", hostport))?,
        ),
        None => (hostport, 80u16),
    };
    if host.is_empty() {
        return Err("主机名为空".to_string());
    }
    use std::io::{Read, Write};
    let addr = format!("{}:{}", host, port);
    let mut stream = std::net::TcpStream::connect(&addr)
        .map_err(|e| format!("连接 {} 失败: {}", addr, e))?;
    let req = format!(
        "GET {} HTTP/1.0\r\nHost: {}\r\nUser-Agent: PuXian/0.1\r\nConnection: close\r\n\r\n",
        path, hostport
    );
    stream
        .write_all(req.as_bytes())
        .map_err(|e| format!("发送请求失败: {}", e))?;
    let mut resp = Vec::new();
    stream
        .read_to_end(&mut resp)
        .map_err(|e| format!("读取响应失败: {}", e))?;
    let resp = String::from_utf8_lossy(&resp).to_string();
    // 分离响应头与响应体
    let body = match resp.find("\r\n\r\n") {
        Some(i) => resp[i + 4..].to_string(),
        None => resp,
    };
    Ok(body)
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
                    return Ok(Value::Dict(Arc::new(Mutex::new(map))));
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
                Ok(Value::Dict(Arc::new(Mutex::new(map))))
            }
            Some(b'[') => {
                self.idx += 1;
                let mut arr = Vec::new();
                self.skip_ws();
                if self.peek() == Some(b']') {
                    self.idx += 1;
                    return Ok(Value::List(Arc::new(Mutex::new(arr))));
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
                Ok(Value::List(Arc::new(Mutex::new(arr))))
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
        let mut s = String::new();
        while let Some(c) = self.peek() {
            self.idx += 1;
            match c {
                b'"' => return Ok(s),
                b'\\' => {
                    let esc = self
                        .peek()
                        .ok_or_else(|| "字符串转义不完整".to_string())?;
                    self.idx += 1;
                    match esc {
                        b'"' => s.push('"'),
                        b'\\' => s.push('\\'),
                        b'/' => s.push('/'),
                        b'n' => s.push('\n'),
                        b't' => s.push('\t'),
                        b'r' => s.push('\r'),
                        b'b' => s.push('\u{8}'),
                        b'f' => s.push('\u{c}'),
                        b'u' => {
                            if self.idx + 4 > self.bytes.len() {
                                return Err("\\u 转义不完整".to_string());
                            }
                            let hex = std::str::from_utf8(&self.bytes[self.idx..self.idx + 4])
                                .map_err(|_| "\\u 非法".to_string())?;
                            let code = u32::from_str_radix(hex, 16).map_err(|_| "\\u 非法".to_string())?;
                            self.idx += 4;
                            s.push(char::from_u32(code).unwrap_or('\u{fffd}'));
                        }
                        _ => return Err(format!("非法转义 \\{}", esc as char)),
                    }
                }
                _ => {
                    // 逐字节收集（JSON 字符串应为 ASCII/UTF-8 字节）
                    s.push(c as char);
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
fn json_stringify(v: &Value) -> Result<String, String> {
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
