//! 普贤 (PuXian) 静态检查器（px lint）
//!
//! 基于 AST 的轻量静态分析，检查项：
//!   L001 未使用变量（let/var 声明后从未引用）
//!   L002 未定义变量引用
//!   L003 不可达代码（return/break/continue 之后的语句）
//!   L004 空块（if/for/while/def body 为空）
//!   L005 重复顶层定义（def/struct/enum/trait 同名）
//!   L006 命名规范（函数/变量 snake_case，类型 PascalCase）
//!   L007 长行（> 100 字符）
//!   L008 行尾空白
//!
//! 退出码约定：有 Error 级诊断 → 1；仅 Warning → 0（--strict 时 Warning 也 → 1）

use std::collections::HashSet;

use crate::ast::*;
use crate::token::Pos;
use crate::value::Builtin;

#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub enum Level {
    Warning,
    Error,
}

#[derive(Debug, Clone)]
pub struct LintDiag {
    pub pos: Pos,
    pub level: Level,
    pub code: &'static str,
    pub msg: String,
}

impl LintDiag {
    fn warn(code: &'static str, pos: Pos, msg: impl Into<String>) -> Self {
        LintDiag {
            pos,
            level: Level::Warning,
            code,
            msg: msg.into(),
        }
    }
    fn error(code: &'static str, pos: Pos, msg: impl Into<String>) -> Self {
        LintDiag {
            pos,
            level: Level::Error,
            code,
            msg: msg.into(),
        }
    }
}

/// 检查单个文件：源码（用于行级检查）+ 已解析的 AST
pub fn lint(prog: &Program, src: &str) -> Vec<LintDiag> {
    let mut diags = Vec::new();

    // ---- 源码层面检查（L007 长行 / L008 行尾空白）----
    for (i, line) in src.lines().enumerate() {
        let line_no = i + 1;
        if line.chars().count() > 100 {
            diags.push(LintDiag::warn(
                "L007",
                Pos::new(line_no, 101),
                format!("行过长（{} 字符 > 100）", line.chars().count()),
            ));
        }
        if line.ends_with(' ') || line.ends_with('\t') {
            diags.push(LintDiag::warn(
                "L008",
                Pos::new(line_no, line.chars().count().max(1)),
                "行尾存在空白字符",
            ));
        }
    }

    // ---- 顶层定义集合 + 重复定义检查（L005）----
    let mut top_names: HashSet<String> = HashSet::new();
    let mut diag_count = 0usize;
    for stmt in &prog.items {
        let (name, pos) = match stmt {
            Stmt::FuncDef { name, pos, .. } => (name, pos),
            Stmt::StructDef { name, pos, .. } => (name, pos),
            Stmt::EnumDef { name, pos, .. } => (name, pos),
            Stmt::TraitDef { name, pos, .. } => (name, pos),
            _ => continue,
        };
        if !top_names.insert(name.clone()) {
            diags.push(LintDiag::error(
                "L005",
                *pos,
                format!("重复顶层定义: '{}'", name),
            ));
            diag_count += 1;
        }
    }

    // 命名规范（L006）
    for stmt in &prog.items {
        match stmt {
            Stmt::FuncDef { name, pos, .. } => {
                if !is_snake_case(name) {
                    diags.push(LintDiag::warn(
                        "L006",
                        *pos,
                        format!("函数名 '{}' 应为 snake_case", name),
                    ));
                }
            }
            Stmt::StructDef { name, pos, .. } | Stmt::EnumDef { name, pos, .. } | Stmt::TraitDef { name, pos, .. } => {
                if !is_pascal_case(name) {
                    diags.push(LintDiag::warn(
                        "L006",
                        *pos,
                        format!("类型名 '{}' 应为 PascalCase", name),
                    ));
                }
            }
            _ => {}
        }
    }

    // ---- 函数体检查 ----
    let builtin_names = builtin_names();
    let mut known = top_names.clone();
    known.extend(builtin_names.iter().cloned());
    // 合并 std 库函数名：import std.collections 后 unique/sort_by 等应视为已定义
    known.extend(collect_std_names(prog));

    for stmt in &prog.items {
        if let Stmt::FuncDef {
            name,
            params,
            body,
            pos,
            ..
        } = stmt
        {
            let mut fn_ctx = FnCtx::new(&known);
            for p in params {
                fn_ctx.declare(&p.name);
            }
            check_block(&mut fn_ctx, body, &mut diags);
            check_unused_vars(&fn_ctx, *pos, name, &mut diags);
            check_empty_body(body, *pos, name, &mut diags);
        }
        // 顶层 if/for 等也要检查（顶层语句）
        if !matches!(
            stmt,
            Stmt::FuncDef { .. }
                | Stmt::StructDef { .. }
                | Stmt::EnumDef { .. }
                | Stmt::TraitDef { .. }
                | Stmt::ImplDef { .. }
                | Stmt::Import { .. }
        ) {
            let mut fn_ctx = FnCtx::new(&known);
            check_stmt(&mut fn_ctx, stmt, &mut diags);
        }
    }
    let _ = diag_count;
    diags
}

/// 扫描 import std.* 模块，加载 stdlib/<mod>.px 收集顶层函数名
fn collect_std_names(prog: &Program) -> HashSet<String> {
    let mut names = HashSet::new();
    let mut mods: Vec<String> = Vec::new();
    for stmt in &prog.items {
        if let Stmt::Import { module, .. } = stmt {
            if module.first().map(|s| s.as_str()) == Some("std") && module.len() >= 2 {
                let name = module[1].clone();
                if !mods.contains(&name) {
                    mods.push(name);
                }
            }
        }
    }
    let stdlib_dir = std::path::Path::new(env!("CARGO_MANIFEST_DIR"))
        .join("..")
        .join("stdlib");
    for m in &mods {
        let path = stdlib_dir.join(format!("{}.px", m));
        if let Ok(src) = std::fs::read_to_string(&path) {
            if let Ok(tokens) = crate::lexer::Lexer::new(&src).tokenize() {
                let mut p = crate::parser::Parser::new(tokens);
                if let Ok(sp) = p.parse_program() {
                    for stmt in sp.items {
                        if let Stmt::FuncDef { name, .. } = stmt {
                            names.insert(name);
                        }
                    }
                }
            }
        }
    }
    names
}

// ==================== 函数体上下文 ====================

struct FnCtx<'a> {
    /// 全部可见变量（参数 + 局部声明）
    declared: HashSet<String>,
    /// 实际被引用的变量
    used: HashSet<String>,
    /// 只读引用（用于未使用检查：声明名如果只在声明处出现则未使用）
    read_only: HashSet<String>,
    /// 已知名字（顶层定义 + 内置函数 + std 库函数）
    known: &'a HashSet<String>,
}

impl<'a> FnCtx<'a> {
    fn new(known: &'a HashSet<String>) -> Self {
        FnCtx {
            declared: HashSet::new(),
            used: HashSet::new(),
            read_only: HashSet::new(),
            known,
        }
    }
    fn declare(&mut self, name: &str) {
        self.declared.insert(name.to_string());
    }
    fn use_var(&mut self, name: &str) {
        if self.declared.contains(name) {
            self.used.insert(name.to_string());
        }
    }
}

// ==================== 语句遍历 ====================

fn check_block(ctx: &mut FnCtx<'_>, stmts: &[Stmt], diags: &mut Vec<LintDiag>) {
    let mut terminated = false;
    for stmt in stmts {
        if terminated {
            diags.push(LintDiag::warn(
                "L003",
                stmt_pos(stmt),
                "不可达代码：前面的 return/break/continue 已终止本块",
            ));
        }
        if matches!(
            stmt,
            Stmt::Return { .. } | Stmt::Break { .. } | Stmt::Continue { .. }
        ) {
            terminated = true;
        }
        check_stmt(ctx, stmt, diags);
    }
}

fn check_stmt(ctx: &mut FnCtx<'_>, stmt: &Stmt, diags: &mut Vec<LintDiag>) {
    match stmt {
        Stmt::VarDecl { name, value, .. } => {
            // 声明名：不视为"使用"
            ctx.declare(name);
            if let Some(v) = value {
                check_expr(ctx, v, diags);
            }
        }
        Stmt::Assign { target, value, .. } => {
            check_expr(ctx, target, diags);
            check_expr(ctx, value, diags);
        }
        Stmt::ExprStmt { expr, .. } => check_expr(ctx, expr, diags),
        Stmt::If {
            branches,
            else_branch,
            ..
        } => {
            for (cond, body) in branches {
                check_expr(ctx, cond, diags);
                let mut sub = FnCtx {
                    declared: ctx.declared.clone(),
                    used: ctx.used.clone(),
                    read_only: ctx.read_only.clone(),
                    known: ctx.known,
                };
                check_block(&mut sub, body, diags);
                ctx.used.extend(sub.used);
                ctx.declared.extend(sub.declared);
            }
            if let Some(body) = else_branch {
                let mut sub = FnCtx {
                    declared: ctx.declared.clone(),
                    used: ctx.used.clone(),
                    read_only: ctx.read_only.clone(),
                    known: ctx.known,
                };
                check_block(&mut sub, body, diags);
                ctx.used.extend(sub.used);
                ctx.declared.extend(sub.declared);
            }
            check_empty_body_branches(branches, else_branch, diags);
        }
        Stmt::For {
            var,
            iterable,
            body,
            pos,
        } => {
            check_expr(ctx, iterable, diags);
            let mut sub = FnCtx {
                declared: ctx.declared.clone(),
                used: ctx.used.clone(),
                read_only: ctx.read_only.clone(),
                known: ctx.known,
            };
            sub.declare(var);
            check_block(&mut sub, body, diags);
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
            if body.is_empty() {
                diags.push(LintDiag::warn("L004", *pos, "for 循环体为空"));
            }
        }
        Stmt::While { cond, body, pos } => {
            check_expr(ctx, cond, diags);
            let mut sub = FnCtx {
                declared: ctx.declared.clone(),
                used: ctx.used.clone(),
                read_only: ctx.read_only.clone(),
                known: ctx.known,
            };
            check_block(&mut sub, body, diags);
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
            if body.is_empty() {
                diags.push(LintDiag::warn("L004", *pos, "while 循环体为空"));
            }
        }
        Stmt::Return { value, .. } => {
            if let Some(v) = value {
                check_expr(ctx, v, diags);
            }
        }
        Stmt::Break { .. } | Stmt::Continue { .. } | Stmt::Empty { .. } => {}
        Stmt::FuncDef { name, params, body, pos, .. } => {
            // 嵌套函数定义：独立上下文
            let mut sub = FnCtx {
                declared: HashSet::new(),
                used: HashSet::new(),
                read_only: HashSet::new(),
                known: ctx.known,
            };
            for p in params {
                sub.declare(&p.name);
            }
            check_block(&mut sub, body, diags);
            check_unused_vars(&sub, *pos, name, diags);
            check_empty_body(body, *pos, name, diags);
            // 函数名可被外层使用
            ctx.use_var(name);
        }
        Stmt::StructDef { .. } | Stmt::EnumDef { .. } | Stmt::TraitDef { .. } => {}
        Stmt::ImplDef { methods, .. } => {
            for m in methods {
                let mut sub = FnCtx {
                    declared: HashSet::new(),
                    used: HashSet::new(),
                    read_only: HashSet::new(),
                    known: ctx.known,
                };
                sub.declare("self");
                for p in &m.params {
                    sub.declare(&p.name);
                }
                check_block(&mut sub, &m.body, diags);
                check_unused_vars(&sub, m.pos, &m.name, diags);
                check_empty_body(&m.body, m.pos, &m.name, diags);
            }
        }
        Stmt::Import { .. } => {}
        Stmt::Spawn { expr, .. } => check_expr(ctx, expr, diags),
        Stmt::ChanDecl { name, .. } => {
            ctx.declare(name);
        }
        Stmt::Send { chan, value, .. } => {
            check_expr(ctx, chan, diags);
            check_expr(ctx, value, diags);
        }
        Stmt::Recv { chan, .. } => check_expr(ctx, chan, diags),
        Stmt::Select {
            arms,
            else_branch,
            pos,
        } => {
            for (binding, expr, body) in arms {
                check_expr(ctx, expr, diags);
                let mut sub = FnCtx {
                    declared: ctx.declared.clone(),
                    used: ctx.used.clone(),
                    read_only: ctx.read_only.clone(),
                    known: ctx.known,
                };
                if let Some(b) = binding {
                    sub.declare(b);
                }
                check_block(&mut sub, body, diags);
                ctx.used.extend(sub.used);
                ctx.declared.extend(sub.declared);
                if body.is_empty() {
                    diags.push(LintDiag::warn("L004", *pos, "select 分支体为空"));
                }
            }
            if let Some(body) = else_branch {
                let mut sub = FnCtx {
                    declared: ctx.declared.clone(),
                    used: ctx.used.clone(),
                    read_only: ctx.read_only.clone(),
                    known: ctx.known,
                };
                check_block(&mut sub, body, diags);
                ctx.used.extend(sub.used);
                ctx.declared.extend(sub.declared);
            }
        }
    }
}

// ==================== 表达式遍历 ====================

fn check_expr(ctx: &mut FnCtx<'_>, expr: &Expr, diags: &mut Vec<LintDiag>) {
    match expr {
        Expr::Int { .. } | Expr::Float { .. } | Expr::Str { .. } | Expr::Bool { .. } | Expr::Null { .. } => {}
        Expr::List { items, .. } => {
            for i in items {
                check_expr(ctx, i, diags);
            }
        }
        Expr::Tuple { items, .. } => {
            for i in items {
                check_expr(ctx, i, diags);
            }
        }
        Expr::Dict { entries, .. } => {
            for (k, v) in entries {
                check_expr(ctx, k, diags);
                check_expr(ctx, v, diags);
            }
        }
        Expr::Var { name, pos } => {
            ctx.use_var(name);
            let known = ctx.declared.contains(name)
                || ctx.known.contains(name)
                || name == "self";
            if !known {
                diags.push(LintDiag::error(
                    "L002",
                    *pos,
                    format!("未定义变量: '{}'", name),
                ));
            }
        }
        Expr::Field { obj, name, .. } => {
            check_expr(ctx, obj, diags);
            let _ = name;
        }
        Expr::OptionalField { obj, name, .. } => {
            check_expr(ctx, obj, diags);
            let _ = name;
        }
        Expr::Index { obj, index, .. } => {
            check_expr(ctx, obj, diags);
            check_expr(ctx, index, diags);
        }
        Expr::Slice { obj, start, end, step, .. } => {
            check_expr(ctx, obj, diags);
            if let Some(s) = start {
                check_expr(ctx, s, diags);
            }
            if let Some(e) = end {
                check_expr(ctx, e, diags);
            }
            if let Some(k) = step {
                check_expr(ctx, k, diags);
            }
        }
        Expr::Call { callee, args, .. } => {
            // 方法调用 obj.m(...) 的 obj 是 Field/Var
            check_expr(ctx, callee, diags);
            for a in args {
                check_expr(ctx, a, diags);
            }
        }
        Expr::Unary { operand, .. } => check_expr(ctx, operand, diags),
        Expr::Binary { left, right, .. } => {
            check_expr(ctx, left, diags);
            check_expr(ctx, right, diags);
        }
        Expr::Pipe { value, func, .. } => {
            check_expr(ctx, value, diags);
            check_expr(ctx, func, diags);
        }
        Expr::NullCoalesce { left, right, .. } => {
            check_expr(ctx, left, diags);
            check_expr(ctx, right, diags);
        }
        Expr::Try { expr, .. } => check_expr(ctx, expr, diags),
        Expr::ForceUnwrap { expr, .. } => check_expr(ctx, expr, diags),
        Expr::IfExpr { cond, then, else_, .. } => {
            check_expr(ctx, cond, diags);
            check_expr(ctx, then, diags);
            check_expr(ctx, else_, diags);
        }
        Expr::ListComp { expr, clauses, cond, .. } => {
            for cl in clauses {
                check_expr(ctx, &cl.iterable, diags);
            }
            let mut sub = FnCtx {
                declared: ctx.declared.clone(),
                used: ctx.used.clone(),
                read_only: ctx.read_only.clone(),
                known: ctx.known,
            };
            for cl in clauses {
                for v in &cl.vars {
                    sub.declare(v);
                }
            }
            check_expr(&mut sub, expr, diags);
            if let Some(c) = cond {
                check_expr(&mut sub, c, diags);
            }
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
        }
        Expr::DictComp { key, value, clauses, cond, .. } => {
            check_expr(ctx, key, diags);
            check_expr(ctx, value, diags);
            for cl in clauses {
                check_expr(ctx, &cl.iterable, diags);
            }
            let mut sub = FnCtx {
                declared: ctx.declared.clone(),
                used: ctx.used.clone(),
                read_only: ctx.read_only.clone(),
                known: ctx.known,
            };
            for cl in clauses {
                for v in &cl.vars {
                    sub.declare(v);
                }
            }
            if let Some(c) = cond {
                check_expr(&mut sub, c, diags);
            }
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
        }
        Expr::Closure {
            params,
            body,
            captures,
            ..
        } => {
            let mut sub = FnCtx {
                declared: HashSet::new(),
                used: HashSet::new(),
                read_only: HashSet::new(),
                known: ctx.known,
            };
            for p in params {
                sub.declare(&p.name);
            }
            // 闭包捕获的变量视为使用
            for c in captures {
                sub.declare(c);
                sub.use_var(c);
                ctx.use_var(c);
            }
            check_expr(&mut sub, body, diags);
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
        }
        Expr::Block { stmts, .. } => {
            let mut sub = FnCtx {
                declared: ctx.declared.clone(),
                used: ctx.used.clone(),
                read_only: ctx.read_only.clone(),
                known: ctx.known,
            };
            check_block(&mut sub, stmts, diags);
            ctx.used.extend(sub.used);
            ctx.declared.extend(sub.declared);
        }
        Expr::Match { subject, arms, .. } => {
            check_expr(ctx, subject, diags);
            for arm in arms {
                let mut sub = FnCtx {
                    declared: ctx.declared.clone(),
                    used: ctx.used.clone(),
                    read_only: ctx.read_only.clone(),
                    known: ctx.known,
                };
                collect_pattern_bindings(&arm.pattern, &mut sub);
                if let Some(g) = &arm.guard {
                    check_expr(&mut sub, g, diags);
                }
                check_expr(&mut sub, &arm.body, diags);
                ctx.used.extend(sub.used);
                ctx.declared.extend(sub.declared);
            }
        }
        Expr::Constructor { args, .. } => {
            for a in args {
                check_expr(ctx, a, diags);
            }
        }
    }
}

fn collect_pattern_bindings(p: &Pattern, ctx: &mut FnCtx<'_>) {
    match p {
        Pattern::Binding(name) => ctx.declare(name),
        Pattern::Tuple(items) => {
            for i in items {
                collect_pattern_bindings(i, ctx);
            }
        }
        Pattern::Constructor(_, items) => {
            for i in items {
                collect_pattern_bindings(i, ctx);
            }
        }
        _ => {}
    }
}

// ==================== 未使用变量 / 空块 ====================

fn check_unused_vars(
    ctx: &FnCtx<'_>,
    fn_pos: Pos,
    fn_name: &str,
    diags: &mut Vec<LintDiag>,
) {
    let unused: Vec<&String> = ctx
        .declared
        .iter()
        .filter(|n| !ctx.used.contains(*n) && !ctx.read_only.contains(*n))
        .collect();
    if !unused.is_empty() {
        let mut names: Vec<&String> = unused;
        names.sort();
        let joined = names
            .iter()
            .map(|s| s.as_str())
            .collect::<Vec<_>>()
            .join(", ");
        diags.push(LintDiag::warn(
            "L001",
            fn_pos,
            format!("函数 '{}' 中未使用变量: {}", fn_name, joined),
        ));
    }
}

fn check_empty_body(body: &[Stmt], pos: Pos, name: &str, diags: &mut Vec<LintDiag>) {
    if body.is_empty() {
        diags.push(LintDiag::warn(
            "L004",
            pos,
            format!("函数 '{}' 体为空", name),
        ));
    }
}

fn check_empty_body_branches(
    branches: &[(Expr, Vec<Stmt>)],
    else_branch: &Option<Vec<Stmt>>,
    diags: &mut Vec<LintDiag>,
) {
    for (_, body) in branches {
        if body.is_empty() {
            diags.push(LintDiag::warn("L004", Pos::new(0, 0), "if 分支体为空"));
        }
    }
    if let Some(body) = else_branch {
        if body.is_empty() {
            diags.push(LintDiag::warn("L004", Pos::new(0, 0), "else 分支体为空"));
        }
    }
}

fn stmt_pos(stmt: &Stmt) -> Pos {
    match stmt {
        Stmt::VarDecl { pos, .. }
        | Stmt::Assign { pos, .. }
        | Stmt::ExprStmt { pos, .. }
        | Stmt::If { pos, .. }
        | Stmt::For { pos, .. }
        | Stmt::While { pos, .. }
        | Stmt::Return { pos, .. }
        | Stmt::Break { pos }
        | Stmt::Continue { pos }
        | Stmt::FuncDef { pos, .. }
        | Stmt::StructDef { pos, .. }
        | Stmt::EnumDef { pos, .. }
        | Stmt::TraitDef { pos, .. }
        | Stmt::ImplDef { pos, .. }
        | Stmt::Import { pos, .. }
        | Stmt::Spawn { pos, .. }
        | Stmt::ChanDecl { pos, .. }
        | Stmt::Send { pos, .. }
        | Stmt::Recv { pos, .. }
        | Stmt::Select { pos, .. }
        | Stmt::Empty { pos } => *pos,
    }
}

// ==================== 命名 / 内置 ====================

fn is_snake_case(s: &str) -> bool {
    !s.is_empty()
        && s.chars()
            .all(|c| c.is_ascii_lowercase() || c.is_ascii_digit() || c == '_')
        && !s.starts_with('_')
}

fn is_pascal_case(s: &str) -> bool {
    !s.is_empty() && s.chars().next().map(|c| c.is_ascii_uppercase()).unwrap_or(false)
}

/// 内置函数名集合（从 Builtin 枚举提取）
pub fn builtin_names() -> HashSet<String> {
    let all = [
        Builtin::Print,
        Builtin::Len,
        Builtin::Range,
        Builtin::Type,
        Builtin::Str,
        Builtin::Int,
        Builtin::Float,
        Builtin::Bool,
        Builtin::Assert,
        Builtin::Panic,
        Builtin::Input,
        Builtin::Exit,
        Builtin::Sleep,
        Builtin::ToUpper,
        Builtin::ToLower,
        Builtin::Trim,
        Builtin::Split,
        Builtin::Join,
        Builtin::Contains,
        Builtin::Replace,
        Builtin::StartsWith,
        Builtin::EndsWith,
        Builtin::Abs,
        Builtin::Sqrt,
        Builtin::Min,
        Builtin::Max,
        Builtin::Pow,
        Builtin::Sorted,
        Builtin::Reversed,
        Builtin::Sum,
        Builtin::NowMs,
        Builtin::ReadFile,
        Builtin::WriteFile,
        Builtin::AppendFile,
        Builtin::ReadAt,
        Builtin::WriteAt,
        Builtin::FileSize,
        Builtin::FsyncFile,
        Builtin::TruncateFile,
        Builtin::Sha256,
        Builtin::Xxhash,
        Builtin::Exists,
        Builtin::ListDir,
        Builtin::Mkdir,
        Builtin::Remove,
        Builtin::JsonParse,
        Builtin::JsonStringify,
        Builtin::Now,
        Builtin::Env,
        Builtin::Args,
        Builtin::Map,
        Builtin::Filter,
        Builtin::Reduce,
        Builtin::TcpListen,
        Builtin::TcpAccept,
        Builtin::TcpConnect,
        Builtin::TcpSend,
        Builtin::TcpRecv,
        Builtin::TcpClose,
        Builtin::HttpGet,
        Builtin::HttpPost,
        Builtin::HttpServe,
        Builtin::PxExec,
        Builtin::PxServe,
        Builtin::SetTimeout,
        Builtin::SetInterval,
        Builtin::ClearTimer,
        Builtin::Base64Encode,
        Builtin::Base64Decode,
        Builtin::SseServe,
        Builtin::SseSend,
        Builtin::SseClose,
        // 后续里程碑补充的内置函数（M15 正则 / M19 AES/XML/ZIP / M21 SSE 客户端 /
        // M22 位运算/WS / M23 进程/SSE 客户端/bytes/RSA / M24 连接池 / M25 心跳 /
        // M26 ushr 无新增 / M27 TLS/Session / M28 路由/时间/cron/SQLite / M29 JSON 路径 /
        // M30 整数↔bytes / M31 沙箱/虚拟主机/限流）
        Builtin::RegexFind,
        Builtin::RegexMatch,
        Builtin::RegexSearch,
        Builtin::RegexFindAll,
        Builtin::RegexReplace,
        Builtin::RegexSplit,
        Builtin::AesEncrypt,
        Builtin::AesDecrypt,
        Builtin::AesGcmEncrypt,
        Builtin::AesGcmDecrypt,
        Builtin::XmlParse,
        Builtin::XmlEscape,
        Builtin::XmlUnescape,
        Builtin::XmlBuild,
        Builtin::ZipPack,
        Builtin::ZipUnpack,
        Builtin::SseConnect,
        Builtin::SseRead,
        Builtin::IntToHex,
        Builtin::HexToInt,
        Builtin::BytesToHex,
        Builtin::HexToBytes,
        Builtin::BitCount,
        Builtin::BitLength,
        Builtin::WsServe,
        Builtin::WsConnect,
        Builtin::WsSend,
        Builtin::WsRecv,
        Builtin::WsClose,
        Builtin::WsPing,
        Builtin::WsHeartbeat,
        Builtin::OsPid,
        Builtin::OsSpawn,
        Builtin::OsWait,
        Builtin::OsKill,
        Builtin::Signal,
        Builtin::RsaGenKey,
        Builtin::RsaEncrypt,
        Builtin::RsaDecrypt,
        Builtin::RsaSign,
        Builtin::RsaVerify,
        Builtin::Bytes,
        Builtin::BytesLen,
        Builtin::BytesGet,
        Builtin::BytesSet,
        Builtin::BytesSlice,
        Builtin::BytesConcat,
        Builtin::BytesToStr,
        Builtin::BytesBase64,
        Builtin::Base64ToBytes,
        Builtin::BytesFind,
        Builtin::ReadBytes,
        Builtin::WriteBytes,
        Builtin::IntToBytes,
        Builtin::BytesToInt,
        Builtin::HttpRequest,
        Builtin::HttpGetStream,
        Builtin::Gc,
        Builtin::TlsServer,
        Builtin::SessionOpen,
        Builtin::SessionId,
        Builtin::SessionGet,
        Builtin::SessionSet,
        Builtin::SessionDel,
        Builtin::SessionDestroy,
        Builtin::BasicAuth,
        Builtin::Route,
        Builtin::Middleware,
        Builtin::TimeFormat,
        Builtin::TimeParse,
        Builtin::TzOffset,
        Builtin::Cron,
        Builtin::SqliteOpen,
        Builtin::SqliteExec,
        Builtin::SqliteQuery,
        Builtin::SqliteClose,
        Builtin::SqliteEscape,
        Builtin::SqliteLastInsertRowid,
        Builtin::JsonPath,
        Builtin::JsonPathSet,
        Builtin::SandboxEnter,
        Builtin::Vhost,
        Builtin::RateLimit,
    ];
    all.iter().map(|b| b.name().to_string()).collect()
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;
    use crate::parser::Parser;

    fn parse(src: &str) -> Program {
        let tokens = Lexer::new(src).tokenize().unwrap();
        let mut p = Parser::new(tokens);
        p.parse_program().unwrap()
    }

    fn codes(diags: &[LintDiag]) -> Vec<&'static str> {
        diags.iter().map(|d| d.code).collect()
    }

    #[test]
    fn test_lint_unused_var() {
        let src = "def f():\n    let x = 1\n    return 2\n";
        let d = lint(&parse(src), src);
        assert!(codes(&d).contains(&"L001"), "{:?}", codes(&d));
    }

    #[test]
    fn test_lint_undefined_var() {
        let src = "def f():\n    return y\n";
        let d = lint(&parse(src), src);
        assert!(codes(&d).contains(&"L002"), "{:?}", codes(&d));
    }

    #[test]
    fn test_lint_defined_var_ok() {
        let src = "def f():\n    let x = 1\n    return x\n";
        let d = lint(&parse(src), src);
        assert!(!codes(&d).contains(&"L002"));
        assert!(!codes(&d).contains(&"L001"));
    }

    #[test]
    fn test_lint_unreachable() {
        let src = "def f():\n    return 1\n    print(2)\n";
        let d = lint(&parse(src), src);
        assert!(codes(&d).contains(&"L003"), "{:?}", codes(&d));
    }

    #[test]
    fn test_lint_empty_body() {
        // 普贤语法要求块至少一条语句，故合法代码不应报 L004
        let src2 = "def g():\n    return 1\n";
        let d = lint(&parse(src2), src2);
        assert!(!codes(&d).contains(&"L004"), "{:?}", codes(&d));
    }

    #[test]
    fn test_lint_dup_def() {
        let src = "def f():\n    return 1\ndef f():\n    return 2\n";
        let d = lint(&parse(src), src);
        assert!(codes(&d).contains(&"L005"), "{:?}", codes(&d));
    }

    #[test]
    fn test_lint_builtin_ok() {
        let src = "def main():\n    print(len([1, 2, 3]))\n";
        let d = lint(&parse(src), src);
        assert!(!codes(&d).contains(&"L002"), "{:?}", codes(&d));
    }
}
