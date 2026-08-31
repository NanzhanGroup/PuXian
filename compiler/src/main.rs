//! 普贤 (PuXian) 编译器 CLI 入口
//! 当前版本：M9 包管理器 / 模块化
//!   支持子命令：px lex <file.px>    （词法分析调试）
//!               px parse <file.px>   （语法分析调试，打印 AST）
//!               px ast <file.px>     （树形查看 AST）
//!               px run <file.px>     （脚本模式执行）
//!               px build <file.px>   （编译为静态二进制）
//!               px fmt <file.px>     （代码格式化，-w 写回 / --check 检查）
//!               px lint <file.px>    （静态检查，--strict 时 Warning 也失败）
//!               px test <file.px>    （测试运行器，def test_*）
//!               px bench <file.px> <func> （基准测试，--count/--repeat）
//!               px doc <file.px>     （文档生成，--output out.md）
//!               px lsp               （语言服务器，LSP 协议）
//!               px mcp               （MCP 服务器，AI 工具调用）
//!               px pkg               （包管理器：init/add/install/list/remove）

mod ast;
mod aes;
mod ast_view;
mod bench;
mod builtin;
mod codegen;
mod crypto;
mod doc;
mod env;
mod fmt;
mod gc;
mod interp;
mod json;
mod lexer;
mod lint;
mod lsp;
mod mcp;
mod module;
mod parser;
mod pkg;
mod regex;
mod rsa;
mod test;
mod tls;
mod token;
mod value;

mod web;
mod ws;
mod xml;
mod zip;

use std::sync::{Arc, Mutex};

use std::env as os_env;
use std::io::Write;
use std::process::ExitCode;

const VERSION: &str = env!("CARGO_PKG_VERSION");
const NAME: &str = "px";
const CN_NAME: &str = "普贤 PuXian";

fn main() -> ExitCode {
    let args: Vec<String> = os_env::args().collect();

    match args.get(1).map(|s| s.as_str()) {
        Some("--version") | Some("-V") => {
            println!("{} {} ({})", NAME, VERSION, CN_NAME);
            ExitCode::SUCCESS
        }
        Some("--help") | Some("-h") | None => {
            print_usage();
            ExitCode::SUCCESS
        }
        Some("lex") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} lex <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_lex(file)
        }
        Some("parse") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} parse <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_parse(file)
        }
        Some("run") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} run <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_script(file)
        }
        Some("build") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} build <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_build(file)
        }
        Some("fmt") => run_fmt(&args),
        Some("lint") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} lint <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_lint(file)
        }
        Some("test") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} test <file.px> [--filter 关键字]", NAME);
                    return ExitCode::from(2);
                }
            };
            let filter = args
                .iter()
                .position(|a| a == "--filter")
                .and_then(|i| args.get(i + 1))
                .map(|s| s.clone());
            ExitCode::from(test::run_tests(file, filter.as_deref()).clamp(0, 255) as u8)
        }
        Some("bench") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} bench <file.px> <func> [--count N] [--repeat R]", NAME);
                    return ExitCode::from(2);
                }
            };
            let func = match args.get(3) {
                Some(f) => f.clone(),
                None => {
                    eprintln!("用法: {} bench <file.px> <func> [--count N] [--repeat R]", NAME);
                    return ExitCode::from(2);
                }
            };
            let mut opts = bench::BenchOptions::default();
            if let Some(i) = args.iter().position(|a| a == "--count") {
                if let Some(v) = args.get(i + 1).and_then(|s| s.parse::<usize>().ok()) {
                    opts.count = v;
                }
            }
            if let Some(i) = args.iter().position(|a| a == "--repeat") {
                if let Some(v) = args.get(i + 1).and_then(|s| s.parse::<usize>().ok()) {
                    opts.repeat = v;
                }
            }
            ExitCode::from(bench::run_bench(&file, &func, &opts).clamp(0, 255) as u8)
        }
        Some("doc") => run_doc(&args),
        Some("ast") => {
            let file = match args.get(2) {
                Some(f) => f,
                None => {
                    eprintln!("用法: {} ast <file.px>", NAME);
                    return ExitCode::from(2);
                }
            };
            run_ast(file)
        }
        Some("lsp") => ExitCode::from(lsp::run_lsp().clamp(0, 255) as u8),
        Some("pkg") => run_pkg(&args),
        Some("mcp") => ExitCode::from(mcp::run_mcp().clamp(0, 255) as u8),
        Some("--worker") => run_worker(),
        Some(other) => {
            eprintln!("未知子命令: {}", other);
            print_usage();
            ExitCode::from(2)
        }
    }
}


/// px pkg 子命令入口（M9 包管理器）
fn run_pkg(args: &[String]) -> ExitCode {
    let sub = args.get(2).map(|s| s.as_str()).unwrap_or("help");
    let dir = std::env::current_dir().unwrap_or_else(|_| std::path::PathBuf::from("."));
    let r = match sub {
        "init" => {
            let name = args
                .iter()
                .position(|a| a == "--name")
                .and_then(|i| args.get(i + 1))
                .map(|s| s.clone());
            pkg::pkg_init(&dir, name.as_deref())
        }
        "add" => {
            let path = match args.get(3) {
                Some(p) => p,
                None => {
                    eprintln!("用法: px pkg add <path> [--name 别名]");
                    return ExitCode::from(2);
                }
            };
            let alias = args
                .iter()
                .position(|a| a == "--name")
                .and_then(|i| args.get(i + 1))
                .map(|s| s.clone());
            pkg::pkg_add(&dir, path, alias.as_deref())
        }
        "install" => pkg::pkg_install(&dir),
        "list" => pkg::pkg_list(&dir),
        "remove" => {
            let name = match args.get(3) {
                Some(n) => n,
                None => {
                    eprintln!("用法: px pkg remove <name>");
                    return ExitCode::from(2);
                }
            };
            pkg::pkg_remove(&dir, name)
        }
        "help" | "-h" | "--help" => {
            print_pkg_usage();
            return ExitCode::SUCCESS;
        }
        other => {
            eprintln!("未知子命令: {}", other);
            print_pkg_usage();
            return ExitCode::from(2);
        }
    };
    match r {
        Ok(msg) => {
            println!("{}", msg);
            ExitCode::SUCCESS
        }
        Err(e) => {
            eprintln!("错误: {}", e);
            ExitCode::from(1)
        }
    }
}

/// px pkg 帮助
fn print_pkg_usage() {
    println!("普贤包管理器 (px pkg)");
    println!("  px pkg init [--name NAME]       初始化项目（生成 px.toml）");
    println!("  px pkg add <path|url> [--name 别名]  添加依赖：本地路径复制；URL（http/https）远程下载（zip 解压 / .px 单文件）");
    println!("  px pkg install                  按 px.toml 安装依赖（URL 依赖带缓存与可选 #sha256 校验）");
    println!("  px pkg list                     列出依赖");
    println!("  px pkg remove <name>            移除依赖");
}

fn print_usage() {
    println!(
        "{} {} ({})\n\
         普贤 (PuXian) —— 面向 AI 的原生语言（Linux）\n\
         \n\
         用法:\n\
         \x20 px <子命令> [参数]\n\
         \n\
         子命令:\n\
         \x20 lex <file.px>    词法分析并打印 Token 流（调试）\n\
         \x20 parse <file.px>  语法分析并打印 AST（调试）\n\
         \x20 ast <file.px>    以树形结构查看 AST\n\
         \x20 run <file.px>    脚本模式执行（解释器）\n\
         \x20 build <file.px>  编译为静态二进制\n\
         \x20 fmt <file.px>    格式化代码（-w 写回文件；--check 仅检查）\n\
         \x20 lint <file.px>   静态检查（--strict 时 Warning 也失败）\n\
         \x20 test <file.px>   运行测试（def test_*；--filter 关键字）\n\
         \x20 bench <file.px> <func>  基准测试（--count N --repeat R）\n\
         \x20 doc <file.px>    生成 API 文档（--output out.md）\n\
         \x20 lsp             语言服务器（LSP，JSON-RPC over stdio）\n\
         \x20 mcp             MCP 服务器（AI 工具调用，JSON-RPC over stdio）\n\
         \x20 pkg            包管理器（init/add/install/list/remove）\n\
         \x20 --version        输出版本号\n\
         \x20 --help           显示帮助",
        NAME, VERSION, CN_NAME
    );
}

/// 读取文件并词法化
fn tokenize_file(file: &str) -> Result<Vec<token::Token>, String> {
    let src = std::fs::read_to_string(file).map_err(|e| format!("无法读取文件 {}: {}", file, e))?;
    lexer::Lexer::new(&src)
        .tokenize()
        .map_err(|e| e.to_string())
}

/// 解析 import 并合并模块定义（M9 模块化）
/// - 支持 std.*（标准库 .px 自举库）/ 用户点分包 / 相对路径 import "..."
/// - 仅合并定义类语句（def/struct/enum/trait/impl/const），不执行模块顶层语句（spec §8.4）
/// - 同名定义去重，用户定义优先（插在后面执行时覆盖）
/// - 兼容入口：base_dir 取 /tmp（模块解析器会回退到 stdlib 目录）
fn resolve_stdlib(mut prog: ast::Program) -> ast::Program {
    resolve_modules(prog, "/tmp")
}

/// M9 通用模块解析入口：base_dir = 主文件所在目录（用户模块相对于此解析）
fn resolve_modules(prog: ast::Program, base_dir: &str) -> ast::Program {
    let mut r = module::ModuleResolver::new(std::path::Path::new(base_dir));
    r.resolve(prog)
}

/// M17：Json → Value（PX_INIT_GLOBALS 环境变量注入全局变量用）
fn json_to_value(j: &json::Json) -> crate::value::Value {
    use crate::value::Value;
    match j {
        json::Json::Null => Value::Null,
        json::Json::Bool(b) => Value::Bool(*b),
        json::Json::Int(i) => Value::Int(*i),
        json::Json::Float(f) => Value::Float(*f),
        json::Json::Str(s) => Value::Str(s.clone()),
        json::Json::Arr(items) => Value::List(std::sync::Arc::new(std::sync::Mutex::new(
            items.iter().map(json_to_value).collect(),
        ))),
        json::Json::Obj(entries) => {
            let mut m = std::collections::HashMap::new();
            for (k, v) in entries {
                m.insert(k.clone(), json_to_value(v));
            }
            Value::Dict(std::sync::Arc::new(std::sync::Mutex::new(m)))
        }
    }
}

// ==================== M25 .px 进程池 worker（px --worker） ====================
// 编译模式 px_serve 的 .px 脚本执行从"每请求 fork+exec `px run`"升级为
// "预派生 N 个 `px --worker` 解释器进程复用"（PHP-FPM 风格进程池）：
//   - 每个 worker 常驻，从 stdin 读"长度前缀任务帧"：path\0env_json\0dump\0timeout_ms
//   - 执行目标脚本（捕获 print 输出、注入全局变量、可选 RESPONSE dump）
//   - 向 stdout 写"长度前缀结果帧"：exit_code\0output
// 帧长度 4 字节大端；父进程负责超时 kill / 崩溃补位（见 runtime.c px_pool_*）。

/// 读一个长度前缀帧；EOF 返回 None
fn read_frame(r: &mut impl std::io::Read) -> std::io::Result<Option<Vec<u8>>> {
    let mut lenb = [0u8; 4];
    match r.read_exact(&mut lenb) {
        Ok(_) => {}
        Err(e) if e.kind() == std::io::ErrorKind::UnexpectedEof => return Ok(None),
        Err(e) => return Err(e),
    }
    let len = u32::from_be_bytes(lenb) as usize;
    if len == 0 || len > 128 * 1024 * 1024 {
        return Ok(None);
    }
    let mut buf = vec![0u8; len];
    r.read_exact(&mut buf)?;
    Ok(Some(buf))
}

/// 执行单个 worker 任务：返回 (退出码, 捕获的输出字节)
fn run_script_worker_task(path: &str, env_json: &str, dump_response: bool) -> (i32, Vec<u8>) {
    let src = match std::fs::read_to_string(path) {
        Ok(s) => s,
        Err(e) => return (1, format!("无法读取文件 {}: {}\n", path, e).into_bytes()),
    };
    let tokens = match lexer::Lexer::new(&src).tokenize() {
        Ok(t) => t,
        Err(e) => return (1, format!("{}\n", e).into_bytes()),
    };
    let mut parser = parser::Parser::new(tokens);
    let prog = match parser.parse_program() {
        Ok(p) => p,
        Err(e) => return (1, format!("{}\n", e).into_bytes()),
    };
    let base_dir = std::path::Path::new(path)
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = resolve_modules(prog, &base_dir);
    let mut interp = interp::Interpreter::new();
    interp.output = Some(std::sync::Arc::new(std::sync::Mutex::new(Vec::new())));
    if !env_json.is_empty() {
        if let Ok(j) = json::parse(env_json) {
            let v = json_to_value(&j);
            if let crate::value::Value::Dict(d) = v {
                let mut g = interp.globals.lock().unwrap();
                for (k, v) in d.lock().unwrap().iter() {
                    g.define(k, v.clone());
                }
            }
        }
    }
    let result = interp.run_program(&prog);
    let mut out = interp
        .output
        .take()
        .map(|o| o.lock().unwrap().clone())
        .unwrap_or_default();
    let code = match result {
        Ok(c) => c,
        Err(e) => {
            out.extend_from_slice(format!("运行时错误: {}\n", e).as_bytes());
            1
        }
    };
    if dump_response {
        // M27：合并 session Set-Cookie / basic_auth 401 注入（与 web.rs exec_script 一致）
        let (cookies, auth) = crate::web::take_response_injections();
        let mut resp = interp.globals.lock().unwrap().get("RESPONSE");
        let has_inject = !cookies.is_empty() || auth.is_some();
        if resp.is_some() || has_inject {
            if resp.is_none() {
                let mut m = std::collections::HashMap::new();
                m.insert("status".to_string(), crate::value::Value::Int(200));
                m.insert(
                    "headers".to_string(),
                    crate::value::Value::Dict(Arc::new(Mutex::new(
                        std::collections::HashMap::new(),
                    ))),
                );
                resp = Some(crate::value::Value::Dict(Arc::new(Mutex::new(m))));
            }
            if let Some(crate::value::Value::Dict(d)) = &resp {
                let mut d = d.lock().unwrap();
                if let Some(realm) = &auth {
                    let is200 = d
                        .get("status")
                        .map(|s| matches!(s, crate::value::Value::Int(200)))
                        .unwrap_or(true);
                    if is200 {
                        d.insert("status".to_string(), crate::value::Value::Int(401));
                    }
                    let mut h = d.get("headers").cloned().unwrap_or_else(|| {
                        crate::value::Value::Dict(Arc::new(Mutex::new(
                            std::collections::HashMap::new(),
                        )))
                    });
                    if let crate::value::Value::Dict(hd) = &h {
                        hd.lock().unwrap().insert(
                            "WWW-Authenticate".to_string(),
                            crate::value::Value::Str(format!("Basic realm=\"{}\"", realm)),
                        );
                    }
                    d.insert("headers".to_string(), h);
                }
                if !cookies.is_empty() {
                    let mut h = d.get("headers").cloned().unwrap_or_else(|| {
                        crate::value::Value::Dict(Arc::new(Mutex::new(
                            std::collections::HashMap::new(),
                        )))
                    });
                    if let crate::value::Value::Dict(hd) = &h {
                        for c in &cookies {
                            hd.lock().unwrap().insert(
                                "Set-Cookie".to_string(),
                                crate::value::Value::Str(c.clone()),
                            );
                        }
                    }
                    d.insert("headers".to_string(), h);
                }
            }
            if let Ok(s) = crate::builtin::json_stringify(&resp.unwrap()) {
                out.extend_from_slice(format!("__PX_RESPONSE__:{}\n", s).as_bytes());
            }
        }
    }
    (code, out)
}

/// `px --worker`：进程池 worker 主循环（由编译模式 px_serve 的池管理进程派生）
fn run_worker() -> ExitCode {
    let stdin = std::io::stdin();
    let mut rin = stdin.lock();
    let stdout = std::io::stdout();
    let mut wout = stdout.lock();
    loop {
        let frame = match read_frame(&mut rin) {
            Ok(Some(f)) => f,
            _ => break,
        };
        let parts: Vec<&[u8]> = frame.split(|&b| b == 0).collect();
        if parts.len() < 4 {
            continue;
        }
        let path = String::from_utf8_lossy(parts[0]).to_string();
        let env_json = String::from_utf8_lossy(parts[1]).to_string();
        let dump = parts[2] == b"1";
        // parts[3] = timeout_ms（信息性；父进程负责超时 kill）
        let (code, out) = run_script_worker_task(&path, &env_json, dump);
        let mut payload = Vec::new();
        payload.extend_from_slice(code.to_string().as_bytes());
        payload.push(0);
        payload.extend_from_slice(&out);
        let len = payload.len() as u32;
        if wout.write_all(&len.to_be_bytes()).is_err() {
            break;
        }
        if wout.write_all(&payload).is_err() {
            break;
        }
        if wout.flush().is_err() {
            break;
        }
    }
    ExitCode::SUCCESS
}

/// 词法分析调试入口
fn run_lex(file: &str) -> ExitCode {
    let tokens = match tokenize_file(file) {
        Ok(t) => t,
        Err(e) => {
            eprintln!("错误: {}", e);
            return ExitCode::from(1);
        }
    };
    for t in &tokens {
        println!("{:>4}:{:<4} {}", t.pos.line, t.pos.col, t.kind);
    }
    println!("共 {} 个 Token", tokens.len());
    ExitCode::SUCCESS
}

/// 语法分析调试入口：打印 AST
fn run_parse(file: &str) -> ExitCode {
    let tokens = match tokenize_file(file) {
        Ok(t) => t,
        Err(e) => {
            eprintln!("错误: {}", e);
            return ExitCode::from(1);
        }
    };
    let mut parser = parser::Parser::new(tokens);
    match parser.parse_program() {
        Ok(prog) => {
            println!("{:#?}", prog);
            println!("语法分析成功：{} 条顶层语句", prog.items.len());
            ExitCode::SUCCESS
        }
        Err(e) => {
            eprintln!("{}", e);
            ExitCode::from(1)
        }
    }
}

/// AST 树形查看入口（M6）
fn run_ast(file: &str) -> ExitCode {
    let src = match std::fs::read_to_string(file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let tokens = match lexer::Lexer::new(&src).tokenize() {
        Ok(t) => t,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let mut parser = parser::Parser::new(tokens);
    match parser.parse_program() {
        Ok(prog) => {
            print!("{}", ast_view::render_ast(&prog));
            ExitCode::SUCCESS
        }
        Err(e) => {
            eprintln!("{}", e);
            ExitCode::from(1)
        }
    }
}

/// 代码格式化入口（M6）：默认输出到 stdout；-w 写回文件；--check 仅检查
fn run_fmt(args: &[String]) -> ExitCode {
    let write_back = args.iter().any(|a| a == "-w" || a == "--write");
    let check_only = args.iter().any(|a| a == "--check");
    let diff_only = args.iter().any(|a| a == "--diff");
    let file = match args.iter().skip(2).find(|a| !a.starts_with('-')) {
        Some(f) => f.clone(),
        None => {
            eprintln!("用法: {} fmt <file.px> [-w] [--check] [--diff]", NAME);
            return ExitCode::from(2);
        }
    };

    let src = match std::fs::read_to_string(&file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let formatted = match fmt::format(&src) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("格式化失败: {}", e);
            return ExitCode::from(1);
        }
    };

    if diff_only {
        // M25：--diff 不写回，打印 unified diff（无差异输出为空，退出 0）
        if formatted != src {
            print!("{}", fmt::unified_diff(&src, &formatted));
        }
        ExitCode::SUCCESS
    } else if check_only {
        if formatted == src {
            println!("{}: 格式正确", file);
            ExitCode::SUCCESS
        } else {
            eprintln!("{}: 格式需要调整（运行 `{} fmt -w {}` 修复）", file, NAME, file);
            ExitCode::from(1)
        }
    } else if write_back {
        match std::fs::write(&file, &formatted) {
            Ok(_) => {
                println!("已格式化: {}", file);
                ExitCode::SUCCESS
            }
            Err(e) => {
                eprintln!("写入失败: {}", e);
                ExitCode::from(1)
            }
        }
    } else {
        print!("{}", formatted);
        ExitCode::SUCCESS
    }
}

/// 静态检查入口（M6）
fn run_lint(file: &str) -> ExitCode {
    let strict = std::env::args().any(|a| a == "--strict");
    let src = match std::fs::read_to_string(file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let tokens = match lexer::Lexer::new(&src).tokenize() {
        Ok(t) => t,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let mut parser = parser::Parser::new(tokens);
    let prog = match parser.parse_program() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let diags = lint::lint(&prog, &src);
    if diags.is_empty() {
        println!("{}: 无 lint 问题", file);
        return ExitCode::SUCCESS;
    }
    let mut errors = 0usize;
    let mut warnings = 0usize;
    for d in &diags {
        let level = match d.level {
            lint::Level::Error => {
                errors += 1;
                "error"
            }
            lint::Level::Warning => {
                warnings += 1;
                "warning"
            }
        };
        println!("{}:{}:{}: {} [{}] {}", file, d.pos.line, d.pos.col, level, d.code, d.msg);
    }
    println!("\n{} 个 error, {} 个 warning", errors, warnings);
    if errors > 0 || (strict && warnings > 0) {
        ExitCode::from(1)
    } else {
        ExitCode::SUCCESS
    }
}

/// 文档生成入口（M6）
fn run_doc(args: &[String]) -> ExitCode {
    let file = match args.iter().skip(2).find(|a| !a.starts_with('-')) {
        Some(f) => f.clone(),
        None => {
            eprintln!("用法: {} doc <file.px> [--output out.md]", NAME);
            return ExitCode::from(2);
        }
    };
    let output = args
        .iter()
        .position(|a| a == "--output" || a == "-o")
        .and_then(|i| args.get(i + 1))
        .map(|s| s.clone());

    let src = match std::fs::read_to_string(&file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let md = match doc::generate_doc(&src, &file) {
        Ok(m) => m,
        Err(e) => {
            eprintln!("文档生成失败: {}", e);
            return ExitCode::from(1);
        }
    };
    match output {
        Some(path) => match std::fs::write(&path, &md) {
            Ok(_) => {
                println!("已生成文档: {}", path);
                ExitCode::SUCCESS
            }
            Err(e) => {
                eprintln!("写入失败: {}", e);
                ExitCode::from(1)
            }
        },
        None => {
            print!("{}", md);
            ExitCode::SUCCESS
        }
    }
}

/// 脚本模式执行入口
fn run_script(file: &str) -> ExitCode {
    let src = match std::fs::read_to_string(file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let tokens = match lexer::Lexer::new(&src).tokenize() {
        Ok(t) => t,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let mut parser = parser::Parser::new(tokens);
    let prog = match parser.parse_program() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let base_dir = std::path::Path::new(file)
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = resolve_modules(prog, &base_dir);
    let mut interp = interp::Interpreter::new();
    // M17：PX_INIT_GLOBALS 环境变量 → 注入全局变量（JSON dict）。
    // 编译模式 px_serve 用子进程 `px run` 执行 .px 脚本时，通过它传递 REQUEST/GET/POST/SERVER，
    // 使双模式脚本行为一致。
    if let Ok(init) = os_env::var("PX_INIT_GLOBALS") {
        if !init.is_empty() {
            if let Ok(j) = json::parse(&init) {
                let v = json_to_value(&j);
                if let crate::value::Value::Dict(d) = v {
                    let mut g = interp.globals.lock().unwrap();
                    for (k, v) in d.lock().unwrap().iter() {
                        g.define(k, v.clone());
                    }
                }
            }
        }
    }
    match interp.run_program(&prog) {
        Ok(code) => {
            // M17：PX_DUMP_RESPONSE=1（编译模式 px_serve 的子进程）→ 把脚本设置的
            // RESPONSE 全局变量序列化到 stdout 尾部（服务器解析为精确响应控制）
            // M27：合并 session Set-Cookie / basic_auth 401 注入（与 web.rs exec_script 一致）
            if os_env::var("PX_DUMP_RESPONSE").map(|v| v == "1").unwrap_or(false) {
                let (cookies, auth) = crate::web::take_response_injections();
                let mut resp = interp.globals.lock().unwrap().get("RESPONSE");
                let has_inject = !cookies.is_empty() || auth.is_some();
                if resp.is_some() || has_inject {
                    if resp.is_none() {
                        let mut m = std::collections::HashMap::new();
                        m.insert("status".to_string(), crate::value::Value::Int(200));
                        m.insert(
                            "headers".to_string(),
                            crate::value::Value::Dict(Arc::new(Mutex::new(
                                std::collections::HashMap::new(),
                            ))),
                        );
                        resp = Some(crate::value::Value::Dict(Arc::new(Mutex::new(m))));
                    }
                    if let Some(crate::value::Value::Dict(d)) = &resp {
                        let mut d = d.lock().unwrap();
                        if let Some(realm) = &auth {
                            let is200 = d
                                .get("status")
                                .map(|s| matches!(s, crate::value::Value::Int(200)))
                                .unwrap_or(true);
                            if is200 {
                                d.insert("status".to_string(), crate::value::Value::Int(401));
                            }
                            let mut h = d.get("headers").cloned().unwrap_or_else(|| {
                                crate::value::Value::Dict(Arc::new(Mutex::new(
                                    std::collections::HashMap::new(),
                                )))
                            });
                            if let crate::value::Value::Dict(hd) = &h {
                                hd.lock().unwrap().insert(
                                    "WWW-Authenticate".to_string(),
                                    crate::value::Value::Str(format!("Basic realm=\"{}\"", realm)),
                                );
                            }
                            d.insert("headers".to_string(), h);
                        }
                        if !cookies.is_empty() {
                            let mut h = d.get("headers").cloned().unwrap_or_else(|| {
                                crate::value::Value::Dict(Arc::new(Mutex::new(
                                    std::collections::HashMap::new(),
                                )))
                            });
                            if let crate::value::Value::Dict(hd) = &h {
                                for c in &cookies {
                                    hd.lock().unwrap().insert(
                                        "Set-Cookie".to_string(),
                                        crate::value::Value::Str(c.clone()),
                                    );
                                }
                            }
                            d.insert("headers".to_string(), h);
                        }
                    }
                    if let Ok(s) = crate::builtin::json_stringify(&resp.unwrap()) {
                        println!("__PX_RESPONSE__:{}", s);
                    }
                }
            }
            if code != 0 {
                eprintln!("进程退出码: {}", code);
            }
            ExitCode::from(code.clamp(0, 255) as u8)
        }
        Err(e) => {
            eprintln!("运行时错误: {}", e);
            ExitCode::from(1)
        }
    }
}

/// 编译模式入口：AST → C → gcc 静态二进制
fn run_build(file: &str) -> ExitCode {
    let src = match std::fs::read_to_string(file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return ExitCode::from(1);
        }
    };
    let tokens = match lexer::Lexer::new(&src).tokenize() {
        Ok(t) => t,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let mut parser = parser::Parser::new(tokens);
    let prog = match parser.parse_program() {
        Ok(p) => p,
        Err(e) => {
            eprintln!("{}", e);
            return ExitCode::from(1);
        }
    };
    let base_dir = std::path::Path::new(file)
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = resolve_modules(prog, &base_dir);
    let mut cg = codegen::Codegen::new();
    let c_code = match cg.generate(&prog) {
        Ok(c) => c,
        Err(e) => {
            eprintln!("代码生成错误: {}", e);
            return ExitCode::from(1);
        }
    };

    // 输出目录：<file 目录>/build
    let file_path = std::path::Path::new(file);
    let base = file_path
        .file_stem()
        .map(|s| s.to_string_lossy().to_string())
        .unwrap_or_else(|| "a".to_string());
    let parent = file_path
        .parent()
        .map(|p| p.to_path_buf())
        .unwrap_or_else(|| std::path::PathBuf::from("."));
    let build_dir = parent.join("build");
    let _ = std::fs::create_dir_all(&build_dir);

    let c_path = build_dir.join(format!("{}.c", base));
    let out_path = build_dir.join(&base);

    if let Err(e) = std::fs::write(&c_path, &c_code) {
        eprintln!("错误: 写入 {} 失败: {}", c_path.display(), e);
        return ExitCode::from(1);
    }

    // 复制 runtime.h / runtime.c / M19 模块（AES/XML/zip）/ miniz 到构建目录
    let runtime_dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR")).join("runtime");
    let rt_files = [
        "runtime.h",
        "runtime.c",
        "runtime_aes.c",
        "runtime_xml.c",
        "runtime_zip.c",
        "runtime_ws.c",
        "runtime_rsa.c",
    ];
    let mut ok_copy = true;
    for f in rt_files {
        let src = runtime_dir.join(f);
        let dst = build_dir.join(f);
        match std::fs::read(&src) {
            Ok(c) => {
                if std::fs::write(&dst, c).is_err() {
                    ok_copy = false;
                }
            }
            Err(_) => ok_copy = false,
        }
    }
    if !ok_copy {
        eprintln!("错误: 找不到 runtime 文件（{}）", runtime_dir.display());
        return ExitCode::from(1);
    }
    // miniz 第三方库（M19 zip：raw deflate）
    let miniz_dir = runtime_dir.join("third_party").join("miniz");
    let miniz_srcs = ["miniz.c", "miniz_tinfl.c", "miniz_tdef.c"];
    for f in miniz_srcs {
        let src = miniz_dir.join(f);
        let dst = build_dir.join(f);
        if let Ok(c) = std::fs::read(&src) {
            let _ = std::fs::write(&dst, c);
        }
    }

    // gcc 编译（-static 静态链接；-pthread 支持并发原语；-lmbedtls 支持 HTTPS/AES）
    // mbedtls 静态库位于 compiler/runtime/mbedtls/（M10 HTTPS / M19 AES）
    let mbedtls_dir = runtime_dir.join("mbedtls");
    let mbedtls_lib = mbedtls_dir.join("lib");
    let mut gcc_cmd = std::process::Command::new("gcc");
    gcc_cmd
        .args(["-static", "-O2", "-pthread", "-o"])
        .arg(&out_path)
        .arg(&c_path)
        .arg(build_dir.join("runtime.c"))
        .arg(build_dir.join("runtime_aes.c"))
        .arg(build_dir.join("runtime_xml.c"))
        .arg(build_dir.join("runtime_zip.c"))
        .arg(build_dir.join("runtime_ws.c"))
        .arg(build_dir.join("runtime_rsa.c"))
        .arg(build_dir.join("miniz.c"))
        .arg(build_dir.join("miniz_tinfl.c"))
        .arg(build_dir.join("miniz_tdef.c"))
        .arg("-I")
        .arg(miniz_dir.clone())
        .arg("-I")
        .arg(mbedtls_dir.join("include"))
        .arg(mbedtls_lib.join("libmbedtls.a"))
        .arg(mbedtls_lib.join("libmbedx509.a"))
        .arg(mbedtls_lib.join("libmbedcrypto.a"))
        .arg("-lm");
    let gcc_out = gcc_cmd.output();

    match gcc_out {
        Ok(o) if o.status.success() => {
            println!(
                "编译成功: {} （{} 字节）",
                out_path.display(),
                std::fs::metadata(&out_path).map(|m| m.len()).unwrap_or(0)
            );
            ExitCode::SUCCESS
        }
        Ok(o) => {
            eprintln!("gcc 编译失败:");
            eprintln!("{}", String::from_utf8_lossy(&o.stderr));
            ExitCode::from(1)
        }
        Err(e) => {
            eprintln!("无法执行 gcc: {}", e);
            ExitCode::from(1)
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    fn parse_src(src: &str) -> ast::Program {
        let tokens = lexer::Lexer::new(src).tokenize().expect("lex");
        let mut p = parser::Parser::new(tokens);
        p.parse_program().expect("parse")
    }

    #[test]
    fn test_resolve_stdlib_merges_defs() {
        // import std.collections 应把 stdlib/collections.px 的 def 合并进主程序（最前）
        let src = "import std.collections\nprint(unique([1, 2, 2]))\n";
        let prog = parse_src(src);
        assert_eq!(prog.items.len(), 2);
        let merged = resolve_stdlib(prog);
        // std 库 def（each/unique/flatten/zip_lists/chunk/group_by/sort_by）+ 原 2 条
        assert!(merged.items.len() >= 9, "std defs merged, got {}", merged.items.len());
        // 第一条是 std 库 def（unique 或 each），保证顶层语句执行前已定义
        assert!(matches!(merged.items[0], ast::Stmt::FuncDef { .. }));
    }

    #[test]
    fn test_resolve_stdlib_user_priority() {
        // 用户自定义同名函数优先，std 库 def 不覆盖
        let src = "import std.collections\ndef unique(items):\n    return \"user\"\n";
        let prog = parse_src(src);
        let merged = resolve_stdlib(prog);
        let mut total_unique = 0;
        for stmt in &merged.items {
            if let ast::Stmt::FuncDef { name, .. } = stmt {
                if name == "unique" {
                    total_unique += 1;
                }
            }
        }
        assert_eq!(total_unique, 2, "std unique + user unique 都存在");
        // 最后一个 def 应是用户的 unique（用户 def 在 std def 之后，运行时覆盖 std）
        let last_def = merged.items.iter().rev().find_map(|s| {
            if let ast::Stmt::FuncDef { name, .. } = s {
                Some(name.clone())
            } else {
                None
            }
        });
        assert_eq!(last_def.as_deref(), Some("unique"), "用户 unique 在最后（覆盖 std）");
    }
}
