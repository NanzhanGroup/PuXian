//! 普贤 (PuXian) 语言服务器（px lsp）
//!
//! LSP（Language Server Protocol）实现，JSON-RPC 2.0 over stdio：
//!   - 生命周期：initialize / initialized / shutdown / exit
//!   - 文档同步：didOpen / didChange / didSave / didClose
//!   - 诊断：textDocument/publishDiagnostics（词法 + 语法 + lint）
//!   - 补全：textDocument/completion（关键字 + 内置函数 + std 函数 + 用户符号 + 局部变量）
//!   - 跳转：textDocument/definition（def/struct/enum/trait/impl 定义定位）
//!   - Hover：textDocument/hover（函数签名 / 类型定义 / 文档注释）
//!
//! 零依赖：帧解析与 JSON 序列化均用自研 json.rs。

use std::collections::HashMap;
use std::io::{self, BufRead, Write};

use crate::ast::{self, Program, Stmt};
use crate::json::Json;
use crate::lexer::Lexer;
use crate::lint;
use crate::parser::Parser;
use crate::token::Pos;

/// LSP 协议版本
#[allow(dead_code)]
const LSP_VERSION: &str = "3.17";

// ==================== JSON-RPC 帧 ====================

/// 从 stdio 读取一条 JSON-RPC 消息（Content-Length 帧）。EOF 返回 Ok(None)。
pub fn read_message<R: BufRead>(r: &mut R) -> io::Result<Option<Json>> {
    let mut content_length: Option<usize> = None;
    loop {
        let mut line = String::new();
        let n = r.read_line(&mut line)?;
        if n == 0 {
            return Ok(None);
        }
        let trimmed = line.trim_end();
        if trimmed.is_empty() {
            break;
        }
        if let Some(rest) = trimmed.strip_prefix("Content-Length:") {
            content_length = rest.trim().parse::<usize>().ok();
        }
        // 其他头（Content-Type 等）忽略
    }
    let len = content_length.unwrap_or(0);
    let mut buf = vec![0u8; len];
    r.read_exact(&mut buf)?;
    let s = String::from_utf8_lossy(&buf);
    match crate::json::parse(&s) {
        Ok(v) => Ok(Some(v)),
        Err(e) => Err(io::Error::new(io::ErrorKind::InvalidData, format!("JSON 解析失败: {}", e))),
    }
}

/// 向 stdio 写一条 JSON-RPC 消息（Content-Length 帧）。
pub fn write_message<W: Write>(w: &mut W, msg: &Json) -> io::Result<()> {
    let body = crate::json::stringify(msg);
    write!(w, "Content-Length: {}\r\n\r\n{}", body.len(), body)?;
    w.flush()
}

// ==================== JSON 辅助 ====================

fn obj() -> Json {
    Json::obj()
}

/// LSP 请求的消息体字段都在 `params` 下（JSON-RPC 包裹），此处取 params 内的指定字段
fn param<'a>(msg: &'a Json, key: &str) -> Option<&'a Json> {
    msg.get("params").and_then(|p| p.get(key))
}

fn pos_json(pos: Pos) -> Json {
    // LSP Position 是 0-based；普贤 Pos 是 1-based line/col
    let mut p = obj();
    p.set("line", Json::Int(pos.line as i64 - 1));
    p.set("character", Json::Int(pos.col as i64 - 1));
    p
}

fn range_json(start: Pos, end: Pos) -> Json {
    let mut r = obj();
    r.set("start", pos_json(start));
    r.set("end", pos_json(end));
    r
}

// ==================== 诊断 ====================

/// 将词法/语法错误转为 LSP Diagnostic
fn error_diag(pos: Pos, msg: &str) -> Json {
    let mut d = obj();
    d.set("range", range_json(pos, pos));
    d.set("severity", Json::Int(1)); // Error
    d.set("source", Json::Str("px".into()));
    d.set("message", Json::Str(msg.into()));
    d
}

/// 将 lint 诊断转为 LSP Diagnostic
fn lint_diag(d: &lint::LintDiag) -> Json {
    let mut j = obj();
    j.set("range", range_json(d.pos, d.pos));
    j.set("severity", Json::Int(match d.level {
        lint::Level::Error => 1,
        lint::Level::Warning => 2,
    }));
    j.set("code", Json::Str(d.code.into()));
    j.set("source", Json::Str("px".into()));
    j.set("message", Json::Str(d.msg.clone()));
    j
}

/// 分析源码：返回 (解析成功的 Program, 诊断列表)
fn analyze(src: &str) -> (Option<Program>, Vec<Json>) {
    let mut diags = Vec::new();
    let tokens = match Lexer::new(src).tokenize() {
        Ok(t) => t,
        Err(e) => {
            diags.push(error_diag(e.pos, &e.to_string()));
            return (None, diags);
        }
    };
    let mut parser = Parser::new(tokens);
    let prog = match parser.parse_program() {
        Ok(p) => p,
        Err(e) => {
            diags.push(error_diag(e.pos, &e.to_string()));
            return (None, diags);
        }
    };
    // 注意：lint 需要完整源码与 AST（含 std 合并? 此处直接 lint，std 函数名由 lint::builtin_names 处理）
    for d in lint::lint(&prog, src) {
        diags.push(lint_diag(&d));
    }
    (Some(prog), diags)
}

// ==================== 符号表 ====================

#[derive(Clone)]
struct Symbol {
    name: String,
    kind: &'static str, // function / struct / enum / trait / impl / var / param
    pos: Pos,
    detail: String, // 签名/类型描述
}

/// 收集源码中定义的符号（顶层定义 + 文档内变量声明）
fn collect_symbols(prog: &Program) -> Vec<Symbol> {
    let mut syms: Vec<Symbol> = Vec::new();
    for stmt in &prog.items {
        match stmt {
            Stmt::FuncDef { name, params, ret_ty, pos, .. } => {
                let mut detail = format!("def {}({})", name, params.iter().map(|p| p.name.clone()).collect::<Vec<_>>().join(", "));
                if let Some(rt) = ret_ty {
                    detail.push_str(&format!(" -> {:?}", rt));
                }
                syms.push(Symbol { name: name.clone(), kind: "function", pos: *pos, detail });
            }
            Stmt::StructDef { name, pos, .. } => {
                syms.push(Symbol { name: name.clone(), kind: "struct", pos: *pos, detail: format!("struct {}", name) });
            }
            Stmt::EnumDef { name, pos, .. } => {
                syms.push(Symbol { name: name.clone(), kind: "enum", pos: *pos, detail: format!("enum {}", name) });
            }
            Stmt::TraitDef { name, pos, .. } => {
                syms.push(Symbol { name: name.clone(), kind: "trait", pos: *pos, detail: format!("trait {}", name) });
            }
            Stmt::ImplDef { type_name, pos, .. } => {
                syms.push(Symbol { name: type_name.clone(), kind: "impl", pos: *pos, detail: format!("impl {}", type_name) });
            }
            Stmt::VarDecl { name, pos, .. } => {
                syms.push(Symbol { name: name.clone(), kind: "var", pos: *pos, detail: format!("let {}", name) });
            }
            _ => {}
        }
    }
    syms
}

/// 收集文档内出现的所有标识符（用于补全候选：局部变量/参数等）
fn collect_local_names(prog: &Program) -> Vec<String> {
    let mut names: Vec<String> = Vec::new();
    fn walk_stmt(s: &Stmt, out: &mut Vec<String>) {
        match s {
            Stmt::VarDecl { name, value, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                if let Some(v) = value {
                    walk_expr(v, out);
                }
            }
            Stmt::Assign { target, value, .. } => {
                walk_expr(target, out);
                walk_expr(value, out);
            }
            Stmt::ExprStmt { expr, .. } => walk_expr(expr, out),
            Stmt::If { branches, else_branch, .. } => {
                for (c, body) in branches {
                    walk_expr(c, out);
                    for b in body {
                        walk_stmt(b, out);
                    }
                }
                if let Some(eb) = else_branch {
                    for b in eb {
                        walk_stmt(b, out);
                    }
                }
            }
            Stmt::For { var, iterable, body, .. } => {
                if !out.contains(var) {
                    out.push(var.clone());
                }
                walk_expr(iterable, out);
                for b in body {
                    walk_stmt(b, out);
                }
            }
            Stmt::While { cond, body, .. } => {
                walk_expr(cond, out);
                for b in body {
                    walk_stmt(b, out);
                }
            }
            Stmt::Return { value, .. } => {
                if let Some(v) = value {
                    walk_expr(v, out);
                }
            }
            Stmt::FuncDef { name, params, body, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                for p in params {
                    if !out.contains(&p.name) {
                        out.push(p.name.clone());
                    }
                }
                for b in body {
                    walk_stmt(b, out);
                }
            }
            Stmt::Spawn { expr, .. } => walk_expr(expr, out),
            Stmt::ChanDecl { name, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
            }
            Stmt::Send { chan, value, .. } => {
                walk_expr(chan, out);
                walk_expr(value, out);
            }
            Stmt::Recv { chan, .. } => walk_expr(chan, out),
            Stmt::Select { arms, else_branch, .. } => {
                for (bind, recv_expr, body) in arms {
                    if let Some(b) = bind {
                        if !out.contains(b) {
                            out.push(b.clone());
                        }
                    }
                    walk_expr(recv_expr, out);
                    for b in body {
                        walk_stmt(b, out);
                    }
                }
                if let Some(eb) = else_branch {
                    for b in eb {
                        walk_stmt(b, out);
                    }
                }
            }
            Stmt::StructDef { name, fields, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                for f in fields {
                    if !out.contains(&f.name) {
                        out.push(f.name.clone());
                    }
                }
            }
            Stmt::EnumDef { name, variants, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                for v in variants {
                    if !out.contains(&v.name) {
                        out.push(v.name.clone());
                    }
                }
            }
            Stmt::TraitDef { name, methods, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                for m in methods {
                    walk_stmt(&Stmt::FuncDef { name: m.name.clone(), params: m.params.clone(), ret_ty: m.ret_ty.clone(), body: m.body.clone(), pos: m.pos }, out);
                }
            }
            Stmt::ImplDef { type_name, trait_name, methods, .. } => {
                if !out.contains(type_name) {
                    out.push(type_name.clone());
                }
                if let Some(t) = trait_name {
                    if !out.contains(t) {
                        out.push(t.clone());
                    }
                }
                for m in methods {
                    walk_stmt(&Stmt::FuncDef { name: m.name.clone(), params: m.params.clone(), ret_ty: m.ret_ty.clone(), body: m.body.clone(), pos: m.pos }, out);
                }
            }
            Stmt::Import { module, names, .. } => {
                for m in module {
                    if !out.contains(m) {
                        out.push(m.clone());
                    }
                }
                for n in names {
                    if !out.contains(n) {
                        out.push(n.clone());
                    }
                }
            }
            _ => {}
        }
    }
    fn walk_expr(e: &ast::Expr, out: &mut Vec<String>) {
        match e {
            ast::Expr::Var { name, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
            }
            ast::Expr::Field { obj, name, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                walk_expr(obj, out);
            }
            ast::Expr::OptionalField { obj, name, .. } => {
                if !out.contains(name) {
                    out.push(name.clone());
                }
                walk_expr(obj, out);
            }
            ast::Expr::Index { obj, index, .. } => {
                walk_expr(obj, out);
                walk_expr(index, out);
            }
            ast::Expr::Slice { obj, start, end, .. } => {
                walk_expr(obj, out);
                if let Some(s) = start {
                    walk_expr(s, out);
                }
                if let Some(e) = end {
                    walk_expr(e, out);
                }
            }
            ast::Expr::Call { callee, args, .. } => {
                walk_expr(callee, out);
                for a in args {
                    walk_expr(a, out);
                }
            }
            ast::Expr::Unary { operand, .. } => walk_expr(operand, out),
            ast::Expr::Binary { left, right, .. } => {
                walk_expr(left, out);
                walk_expr(right, out);
            }
            ast::Expr::Pipe { value, func, .. } => {
                walk_expr(value, out);
                walk_expr(func, out);
            }
            ast::Expr::NullCoalesce { left, right, .. } => {
                walk_expr(left, out);
                walk_expr(right, out);
            }
            ast::Expr::Try { expr, .. } => walk_expr(expr, out),
            ast::Expr::ForceUnwrap { expr, .. } => walk_expr(expr, out),
            ast::Expr::IfExpr { cond, then, else_, .. } => {
                walk_expr(cond, out);
                walk_expr(then, out);
                walk_expr(else_, out);
            }
            ast::Expr::ListComp { expr, var, iterable, cond, .. } => {
                if !out.contains(var) {
                    out.push(var.clone());
                }
                walk_expr(expr, out);
                walk_expr(iterable, out);
                if let Some(c) = cond {
                    walk_expr(c, out);
                }
            }
            ast::Expr::Closure { params, body, .. } => {
                for p in params {
                    if !out.contains(&p.name) {
                        out.push(p.name.clone());
                    }
                }
                walk_expr(body, out);
            }
            ast::Expr::Block { stmts, .. } => {
                for s in stmts {
                    walk_stmt(s, out);
                }
            }
            ast::Expr::Match { subject, arms, .. } => {
                walk_expr(subject, out);
                for arm in arms {
                    walk_expr(&arm.body, out);
                }
            }
            ast::Expr::Constructor { args, .. } => {
                for a in args {
                    walk_expr(a, out);
                }
            }
            ast::Expr::List { items, .. } => {
                for i in items {
                    walk_expr(i, out);
                }
            }
            ast::Expr::Tuple { items, .. } => {
                for i in items {
                    walk_expr(i, out);
                }
            }
            ast::Expr::Dict { entries, .. } => {
                for (k, v) in entries {
                    walk_expr(k, out);
                    walk_expr(v, out);
                }
            }
            _ => {}
        }
    }
    for s in &prog.items {
        walk_stmt(s, &mut names);
    }
    names
}

// ==================== 关键字/内置函数/标准库 ====================

const KEYWORDS: &[&str] = &[
    "let", "var", "const", "def", "fn", "struct", "enum", "trait", "impl",
    "match", "case", "if", "elif", "else", "for", "while", "in", "return",
    "break", "continue", "import", "from", "pub", "as", "spawn", "chan",
    "send", "recv", "select", "true", "false", "null", "self", "type", "capture",
];

/// std.collections 自举库函数（M5 标准库）
const STD_FUNCS: &[&str] = &[
    "each", "unique", "flatten", "zip_lists", "chunk", "group_by", "sort_by",
];

/// 补全候选：关键字 + 内置函数 + std 函数 + 用户符号 + 局部变量
fn completion_items(prog: Option<&Program>) -> Vec<Json> {
    let mut items: Vec<Json> = Vec::new();
    // 关键字
    for kw in KEYWORDS {
        let mut it = obj();
        it.set("label", Json::Str((*kw).into()));
        it.set("kind", Json::Int(14)); // Keyword
        it.set("detail", Json::Str("关键字".into()));
        it.set("insertText", Json::Str((*kw).into()));
        items.push(it);
    }
    // 内置函数
    for name in lint::builtin_names() {
        let mut it = obj();
        it.set("label", Json::Str(name.clone()));
        it.set("kind", Json::Int(3)); // Function
        it.set("detail", Json::Str("内置函数".into()));
        it.set("insertText", Json::Str(name));
        items.push(it);
    }
    // std 库函数
    for name in STD_FUNCS {
        let mut it = obj();
        it.set("label", Json::Str((*name).into()));
        it.set("kind", Json::Int(3));
        it.set("detail", Json::Str("std.collections 函数".into()));
        it.set("insertText", Json::Str((*name).into()));
        items.push(it);
    }
    // 用户符号
    if let Some(p) = prog {
        for s in collect_symbols(p) {
            let mut it = obj();
            it.set("label", Json::Str(s.name.clone()));
            it.set("kind", Json::Int(match s.kind {
                "function" => 3,
                "struct" | "enum" | "trait" | "impl" => 7, // Class/Interface
                _ => 6, // Variable
            }));
            it.set("detail", Json::Str(s.detail));
            it.set("insertText", Json::Str(s.name));
            items.push(it);
        }
        // 局部变量/参数
        for name in collect_local_names(p) {
            let mut it = obj();
            it.set("label", Json::Str(name.clone()));
            it.set("kind", Json::Int(6)); // Variable
            it.set("detail", Json::Str("变量".into()));
            it.set("insertText", Json::Str(name));
            items.push(it);
        }
    }
    items
}

/// 光标处单词（当前行前缀）
fn word_at(src: &str, line: usize, character: usize) -> String {
    // line/character 是 0-based
    let line_str = match src.lines().nth(line) {
        Some(l) => l,
        None => return String::new(),
    };
    let chars: Vec<char> = line_str.chars().collect();
    // 行首没有正在输入的单词
    if character == 0 {
        return String::new();
    }
    let mut col = character.min(chars.len());
    // 向左扫描标识符
    while col > 0 {
        let c = chars[col - 1];
        if c.is_alphanumeric() || c == '_' {
            col -= 1;
        } else {
            break;
        }
    }
    let mut word = String::new();
    for c in &chars[col..] {
        if c.is_alphanumeric() || *c == '_' {
            word.push(*c);
        } else {
            break;
        }
    }
    word
}

// ==================== 定义跳转 / hover ====================

/// 查找符号定义位置（精确名匹配）
fn find_symbol(prog: &Program, name: &str) -> Option<Symbol> {
    collect_symbols(prog).into_iter().find(|s| s.name == name)
}

/// hover 内容：显示符号签名/类型；内置函数显示签名
fn hover_text(prog: Option<&Program>, word: &str) -> Option<String> {
    if let Some(p) = prog {
        if let Some(s) = find_symbol(p, word) {
            return Some(format!("**{}**\n\n{}", s.name, s.detail));
        }
    }
    // 内置函数
    if lint::builtin_names().contains(word) {
        return Some(format!("**{}**\n\n内置函数", word));
    }
    // 关键字
    if KEYWORDS.contains(&word) {
        return Some(format!("**{}**\n\n关键字", word));
    }
    None
}

// ==================== 主循环 ====================

/// LSP 服务器入口（阻塞，直到 exit / EOF）
pub fn run_lsp() -> i32 {
    let stdin = io::stdin();
    let mut reader = stdin.lock();
    let stdout = io::stdout();
    let mut writer = stdout.lock();

    // 打开的文档 uri -> 源码
    let mut docs: HashMap<String, String> = HashMap::new();
    // 分析缓存 uri -> (Program, 诊断)（didChange 后失效）
    let mut shutting_down = false;

    loop {
        let msg = match read_message(&mut reader) {
            Ok(Some(m)) => m,
            Ok(None) => break, // EOF
            Err(e) => {
                eprintln!("[px-lsp] 读取消息失败: {}", e);
                break;
            }
        };

        let method = msg.get_str("method").unwrap_or("").to_string();
        let id = msg.get("id").cloned();
        let is_notification = id.is_none();

        let mut response: Option<Json> = None;
        let mut notify: Option<Json> = None; // 服务器主动通知（诊断）

        match method.as_str() {
            "initialize" => {
                let mut capabilities = obj();
                let mut tsync = obj();
                tsync.set("openClose", Json::Bool(true));
                tsync.set("change", Json::Int(1)); // Full 同步
                capabilities.set("textDocumentSync", tsync);
                capabilities.set("completionProvider", Json::Bool(true));
                capabilities.set("definitionProvider", Json::Bool(true));
                capabilities.set("hoverProvider", Json::Bool(true));
                let mut result = obj();
                result.set("capabilities", capabilities);
                let mut server_info = obj();
                server_info.set("name", Json::Str("px-lsp".into()));
                server_info.set("version", Json::Str(env!("CARGO_PKG_VERSION").into()));
                result.set("serverInfo", server_info);
                result.set("jsonrpc", Json::Str("2.0".into()));
                response = Some(result);
            }
            "initialized" => { /* 忽略 */ }
            "shutdown" => {
                shutting_down = true;
                response = Some(obj());
            }
            "exit" => break,
            "textDocument/didOpen" => {
                if let Some(td) = param(&msg, "textDocument") {
                    if let (Some(uri), Some(text)) = (td.get_str("uri"), td.get_str("text")) {
                        docs.insert(uri.to_string(), text.to_string());
                        // 推送诊断
                        let (prog, diags) = analyze(text);
                        let _ = prog;
                        notify = Some(diagnostics_notification(uri, &diags));
                    }
                }
            }
            "textDocument/didChange" => {
                if let Some(td) = param(&msg, "textDocument") {
                    if let Some(uri) = td.get_str("uri") {
                        // Full 同步：取最后一个 contentChanges[].text
                        if let Some(changes) = param(&msg, "contentChanges") {
                            if let Json::Arr(arr) = changes {
                                if let Some(last) = arr.last() {
                                    if let Some(text) = last.get_str("text") {
                                        docs.insert(uri.to_string(), text.to_string());
                                        let (_, diags) = analyze(text);
                                        notify = Some(diagnostics_notification(uri, &diags));
                                    }
                                }
                            }
                        }
                    }
                }
            }
            "textDocument/didSave" => {
                if let Some(td) = param(&msg, "textDocument") {
                    if let Some(uri) = td.get_str("uri") {
                        if let Some(text) = docs.get(uri) {
                            let (_, diags) = analyze(text);
                            notify = Some(diagnostics_notification(uri, &diags));
                        }
                    }
                }
            }
            "textDocument/didClose" => {
                if let Some(td) = param(&msg, "textDocument") {
                    if let Some(uri) = td.get_str("uri") {
                        docs.remove(uri);
                        // 关闭后清空诊断
                        notify = Some(diagnostics_notification(uri, &Vec::new()));
                    }
                }
            }
            "textDocument/completion" => {
                let uri = param(&msg, "textDocument").and_then(|td| td.get_str("uri")).unwrap_or("").to_string();
                let line = param(&msg, "position").and_then(|p| p.get("line")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let character = param(&msg, "position").and_then(|p| p.get("character")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let text = docs.get(&uri).cloned().unwrap_or_default();
                let prog = analyze(&text).0;
                let prefix = word_at(&text, line, character);
                let all_items = completion_items(prog.as_ref());
                let filtered: Vec<Json> = if prefix.is_empty() {
                    all_items
                } else {
                    all_items.into_iter().filter(|it| {
                        it.get_str("label").map(|l| l.starts_with(&prefix)).unwrap_or(false)
                    }).collect()
                };
                let mut result = obj();
                result.set("isIncomplete", Json::Bool(false));
                result.set("items", Json::Arr(filtered));
                response = Some(result);
            }
            "textDocument/definition" => {
                let uri = param(&msg, "textDocument").and_then(|td| td.get_str("uri")).unwrap_or("").to_string();
                let line = param(&msg, "position").and_then(|p| p.get("line")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let character = param(&msg, "position").and_then(|p| p.get("character")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let text = docs.get(&uri).cloned().unwrap_or_default();
                let word = word_at(&text, line, character);
                let prog = analyze(&text).0;
                let location = prog.as_ref().and_then(|p| find_symbol(p, &word)).map(|s| {
                    let mut l = obj();
                    l.set("uri", Json::Str(uri.clone()));
                    l.set("range", range_json(s.pos, s.pos));
                    l
                });
                response = Some(location.unwrap_or(Json::Null));
            }
            "textDocument/hover" => {
                let uri = param(&msg, "textDocument").and_then(|td| td.get_str("uri")).unwrap_or("").to_string();
                let line = param(&msg, "position").and_then(|p| p.get("line")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let character = param(&msg, "position").and_then(|p| p.get("character")).and_then(|v| match v { Json::Int(i) => Some(*i), _ => None }).unwrap_or(0) as usize;
                let text = docs.get(&uri).cloned().unwrap_or_default();
                let word = word_at(&text, line, character);
                let prog = analyze(&text).0;
                let hover = hover_text(prog.as_ref(), &word);
                let result = hover.map(|contents| {
                    let mut h = obj();
                    let mut c = obj();
                    c.set("kind", Json::Str("markdown".into()));
                    c.set("value", Json::Str(contents));
                    h.set("contents", c);
                    h
                });
                response = Some(result.unwrap_or(Json::Null));
            }
            _ => {
                if !is_notification {
                    let mut err = obj();
                    err.set("code", Json::Int(-32601));
                    err.set("message", Json::Str(format!("方法未找到: {}", method)));
                    let mut resp = obj();
                    if let Some(idv) = id {
                        resp.set("id", idv);
                    }
                    resp.set("jsonrpc", Json::Str("2.0".into()));
                    resp.set("error", err);
                    let _ = write_message(&mut writer, &resp);
                    continue;
                }
            }
        }

        // 发送响应（请求才响应；通知不响应）
        if let Some(r) = response {
            let mut resp = obj();
            if let Some(idv) = id {
                resp.set("id", idv);
            } else {
                resp.set("id", Json::Null);
            }
            resp.set("jsonrpc", Json::Str("2.0".into()));
            resp.set("result", r);
            if let Err(e) = write_message(&mut writer, &resp) {
                eprintln!("[px-lsp] 写响应失败: {}", e);
                break;
            }
        }

        // 发送主动通知（诊断）
        if let Some(n) = notify {
            if let Err(e) = write_message(&mut writer, &n) {
                eprintln!("[px-lsp] 写通知失败: {}", e);
                break;
            }
        }
    }
    if shutting_down { 0 } else { 0 }
}

/// 构建 publishDiagnostics 通知
fn diagnostics_notification(uri: &str, diags: &[Json]) -> Json {
    let mut n = obj();
    n.set("jsonrpc", Json::Str("2.0".into()));
    n.set("method", Json::Str("textDocument/publishDiagnostics".into()));
    let mut params = obj();
    params.set("uri", Json::Str(uri.into()));
    params.set("diagnostics", Json::Arr(diags.to_vec()));
    n.set("params", params);
    n
}

// ==================== 测试 ====================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_word_at() {
        let src = "def fib(n):\n    if n <= 1:\n        return n\n    return fib(n - 1) + fib(n - 2)\n";
        // 光标在单词上 → 取完整单词
        assert_eq!(word_at(src, 0, 4), "fib");
        assert_eq!(word_at(src, 0, 5), "fib");
        assert_eq!(word_at(src, 3, 12), "fib");
        assert_eq!(word_at(src, 3, 13), "fib");
        // 行首 → 空
        assert_eq!(word_at(src, 0, 0), "");
        assert_eq!(word_at(src, 3, 0), "");
        // 光标在单字符变量上
        assert_eq!(word_at(src, 1, 7), "n");
        assert_eq!(word_at(src, 3, 15), "n");
        // 越界行/列 → 空
        assert_eq!(word_at(src, 99, 0), "");
    }

    #[test]
    fn test_analyze_ok() {
        let src = "def add(a, b):\n    return a + b\nprint(add(1, 2))\n";
        let (prog, diags) = analyze(src);
        assert!(prog.is_some());
        assert!(diags.is_empty(), "diags: {:?}", diags);
    }

    #[test]
    fn test_analyze_syntax_error() {
        let src = "def add(a, b:\n    return a + b\n";
        let (prog, diags) = analyze(src);
        assert!(prog.is_none());
        assert!(!diags.is_empty());
    }

    #[test]
    fn test_collect_symbols() {
        let src = "def add(a, b):\n    return a + b\nstruct Point:\n    x: int\n    y: int\nenum Color:\n    Red\n    Green\n";
        let (prog, _) = analyze(src);
        let syms = collect_symbols(prog.as_ref().unwrap());
        let names: Vec<&str> = syms.iter().map(|s| s.name.as_str()).collect();
        assert!(names.contains(&"add"));
        assert!(names.contains(&"Point"));
        assert!(names.contains(&"Color"));
        let add = syms.iter().find(|s| s.name == "add").unwrap();
        assert!(add.detail.contains("def add(a, b)"));
    }

    #[test]
    fn test_completion_contains_keywords_and_builtins() {
        let src = "def add(a, b):\n    return a + b\n";
        let (prog, _) = analyze(src);
        let items = completion_items(prog.as_ref());
        let labels: Vec<String> = items.iter().filter_map(|i| i.get_str("label").map(|s| s.to_string())).collect();
        assert!(labels.contains(&"def".to_string()));
        assert!(labels.contains(&"print".to_string()));
        assert!(labels.contains(&"add".to_string()));
        assert!(labels.contains(&"a".to_string()));
    }

    #[test]
    fn test_find_symbol() {
        let src = "def hello():\n    print(\"hi\")\n";
        let (prog, _) = analyze(src);
        let p = prog.as_ref().unwrap();
        assert!(find_symbol(p, "hello").is_some());
        assert!(find_symbol(p, "nope").is_none());
    }

    #[test]
    fn test_hover_text() {
        let src = "def foo(x):\n    return x * 2\n";
        let (prog, _) = analyze(src);
        let ht = hover_text(prog.as_ref(), "foo");
        assert!(ht.is_some());
        assert!(ht.unwrap().contains("def foo(x)"));
        assert!(hover_text(prog.as_ref(), "print").is_some());
        assert!(hover_text(prog.as_ref(), "zzz").is_none());
    }

    #[test]
    fn test_write_read_roundtrip() {
        // 验证帧读写对称
        let mut msg = obj();
        msg.set("jsonrpc", Json::Str("2.0".into()));
        msg.set("method", Json::Str("test".into()));
        let body = crate::json::stringify(&msg);
        let frame = format!("Content-Length: {}\r\n\r\n{}", body.len(), body);
        let mut cursor = io::Cursor::new(frame.into_bytes());
        let parsed = read_message(&mut cursor).unwrap().unwrap();
        assert_eq!(parsed.get_str("method"), Some("test"));
    }

    #[test]
    fn test_param_reads_under_params() {
        // LSP 请求字段在 JSON-RPC 的 params 下（回归：didOpen/completion 曾直接取顶层导致取不到）
        let raw = r#"{"jsonrpc":"2.0","id":2,"method":"textDocument/completion","params":{"textDocument":{"uri":"file:///a.px"},"position":{"line":1,"character":4}}}"#;
        let msg = crate::json::parse(raw).unwrap();
        let td = param(&msg, "textDocument").unwrap();
        assert_eq!(td.get_str("uri"), Some("file:///a.px"));
        let pos = param(&msg, "position").unwrap();
        assert_eq!(pos.get("line"), Some(&Json::Int(1)));
        assert_eq!(pos.get("character"), Some(&Json::Int(4)));
        assert!(param(&msg, "nonexistent").is_none());
    }

    #[test]
    fn test_completion_prefix_filter() {
        // completion 按 word_at 前缀过滤：输入 "ad" 只返回以 ad 开头的候选
        let src = "def add(a, b):\n    return a + b\nprint(ad)\n";
        let (prog, _) = analyze(src);
        let items = completion_items(prog.as_ref());
        let filtered: Vec<&Json> = items.iter().filter(|it| {
            it.get_str("label").map(|l| l.starts_with("ad")).unwrap_or(false)
        }).collect();
        let labels: Vec<String> = filtered.iter().filter_map(|i| i.get_str("label").map(|s| s.to_string())).collect();
        assert!(labels.contains(&"add".to_string()), "labels: {:?}", labels);
    }
}
