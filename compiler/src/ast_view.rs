//! 普贤 (PuXian) AST 树形查看器（px ast）
//!
//! 比 `px parse` 的 Rust Debug 输出更紧凑易读：递归树形打印，
//! 每个节点一行，子节点缩进，显示关键信息（名字/运算符/字面量）。

use crate::ast::*;

/// 生成 AST 树形文本
pub fn render_ast(prog: &Program) -> String {
    let mut out = String::new();
    out.push_str("Program\n");
    for stmt in &prog.items {
        render_stmt(stmt, 1, &mut out);
    }
    out
}

fn indent(level: usize, out: &mut String) {
    for _ in 0..level {
        out.push_str("  ");
    }
}

fn render_stmt(stmt: &Stmt, level: usize, out: &mut String) {
    indent(level, out);
    match stmt {
        Stmt::VarDecl { kind, name, value, pos, .. } => {
            let k = match kind {
                VarKind::Let => "let",
                VarKind::Var => "var",
                VarKind::Const => "const",
            };
            out.push_str(&format!("{} {} @ {}\n", k, name, pos));
            if let Some(v) = value {
                indent(level + 1, out);
                out.push_str("= \n");
                render_expr(v, level + 2, out);
            }
        }
        Stmt::Assign { target, op, value, pos, .. } => {
            out.push_str(&format!("assign {} @ {}\n", op_name(*op), pos));
            render_expr(target, level + 1, out);
            render_expr(value, level + 1, out);
        }
        Stmt::ExprStmt { expr, pos, .. } => {
            out.push_str(&format!("expr @ {}\n", pos));
            render_expr(expr, level + 1, out);
        }
        Stmt::If { branches, else_branch, pos, .. } => {
            out.push_str(&format!("if @ {}\n", pos));
            for (cond, body) in branches {
                indent(level + 1, out);
                out.push_str("cond:\n");
                render_expr(cond, level + 2, out);
                indent(level + 1, out);
                out.push_str("then:\n");
                for s in body {
                    render_stmt(s, level + 2, out);
                }
            }
            if let Some(body) = else_branch {
                indent(level + 1, out);
                out.push_str("else:\n");
                for s in body {
                    render_stmt(s, level + 2, out);
                }
            }
        }
        Stmt::For { var, iterable, body, pos, .. } => {
            out.push_str(&format!("for {} @ {}\n", var, pos));
            indent(level + 1, out);
            out.push_str("in:\n");
            render_expr(iterable, level + 2, out);
            for s in body {
                render_stmt(s, level + 1, out);
            }
        }
        Stmt::While { cond, body, pos, .. } => {
            out.push_str(&format!("while @ {}\n", pos));
            render_expr(cond, level + 1, out);
            for s in body {
                render_stmt(s, level + 1, out);
            }
        }
        Stmt::Return { value, pos, .. } => {
            out.push_str(&format!("return @ {}\n", pos));
            if let Some(v) = value {
                render_expr(v, level + 1, out);
            }
        }
        Stmt::Break { pos } => out.push_str(&format!("break @ {}\n", pos)),
        Stmt::Continue { pos } => out.push_str(&format!("continue @ {}\n", pos)),
        Stmt::FuncDef { name, params, body, pos, .. } => {
            let ps: Vec<String> = params.iter().map(|p| p.name.clone()).collect();
            out.push_str(&format!("def {}({}) @ {}\n", name, ps.join(", "), pos));
            for s in body {
                render_stmt(s, level + 1, out);
            }
        }
        Stmt::StructDef { name, fields, pos, .. } => {
            let fs: Vec<String> = fields.iter().map(|f| f.name.clone()).collect();
            out.push_str(&format!("struct {} {{ {} }} @ {}\n", name, fs.join(", "), pos));
        }
        Stmt::EnumDef { name, variants, pos, .. } => {
            let vs: Vec<String> = variants.iter().map(|v| v.name.clone()).collect();
            out.push_str(&format!("enum {} {{ {} }} @ {}\n", name, vs.join(", "), pos));
        }
        Stmt::TraitDef { name, methods, pos, .. } => {
            out.push_str(&format!("trait {} @ {}\n", name, pos));
            for m in methods {
                render_stmt(
                    &Stmt::FuncDef {
                        name: m.name.clone(),
                        params: m.params.clone(),
                        ret_ty: m.ret_ty.clone(),
                        body: m.body.clone(),
                        pos: m.pos,
                    },
                    level + 1,
                    out,
                );
            }
        }
        Stmt::ImplDef { type_name, methods, pos, .. } => {
            out.push_str(&format!("impl {} @ {}\n", type_name, pos));
            for m in methods {
                render_stmt(
                    &Stmt::FuncDef {
                        name: m.name.clone(),
                        params: m.params.clone(),
                        ret_ty: m.ret_ty.clone(),
                        body: m.body.clone(),
                        pos: m.pos,
                    },
                    level + 1,
                    out,
                );
            }
        }
        Stmt::Import { module, names, pos, .. } => {
            out.push_str(&format!(
                "import {} ({}) @ {}\n",
                module.join("."),
                names.join(", "),
                pos
            ));
        }
        Stmt::Spawn { expr, pos, .. } => {
            out.push_str(&format!("spawn @ {}\n", pos));
            render_expr(expr, level + 1, out);
        }
        Stmt::ChanDecl { name, pos, .. } => {
            out.push_str(&format!("chan {} @ {}\n", name, pos));
        }
        Stmt::Send { chan, value, pos, .. } => {
            out.push_str(&format!("send @ {}\n", pos));
            render_expr(chan, level + 1, out);
            render_expr(value, level + 1, out);
        }
        Stmt::Recv { chan, pos, .. } => {
            out.push_str(&format!("recv @ {}\n", pos));
            render_expr(chan, level + 1, out);
        }
        Stmt::Select { arms, else_branch, pos, .. } => {
            out.push_str(&format!("select @ {}\n", pos));
            for (binding, expr, body) in arms {
                indent(level + 1, out);
                out.push_str(&format!(
                    "case {}:\n",
                    binding.clone().unwrap_or_else(|| "_".to_string())
                ));
                render_expr(expr, level + 2, out);
                for s in body {
                    render_stmt(s, level + 2, out);
                }
            }
            if let Some(body) = else_branch {
                indent(level + 1, out);
                out.push_str("else:\n");
                for s in body {
                    render_stmt(s, level + 2, out);
                }
            }
        }
        Stmt::Empty { pos } => out.push_str(&format!("empty @ {}\n", pos)),
    }
}

fn render_expr(expr: &Expr, level: usize, out: &mut String) {
    indent(level, out);
    match expr {
        Expr::Int { value, pos } => out.push_str(&format!("int {} @ {}\n", value, pos)),
        Expr::Float { value, pos } => out.push_str(&format!("float {} @ {}\n", value, pos)),
        Expr::Str { value, pos } => out.push_str(&format!("str {:?} @ {}\n", value, pos)),
        Expr::Bool { value, pos } => out.push_str(&format!("bool {} @ {}\n", value, pos)),
        Expr::Null { pos } => out.push_str(&format!("null @ {}\n", pos)),
        Expr::List { items, pos } => {
            out.push_str(&format!("list [{}] @ {}\n", items.len(), pos));
            for i in items {
                render_expr(i, level + 1, out);
            }
        }
        Expr::Tuple { items, pos } => {
            out.push_str(&format!("tuple ({}) @ {}\n", items.len(), pos));
            for i in items {
                render_expr(i, level + 1, out);
            }
        }
        Expr::Dict { entries, pos } => {
            out.push_str(&format!("dict {{ {} }} @ {}\n", entries.len(), pos));
            for (k, v) in entries {
                render_expr(k, level + 1, out);
                render_expr(v, level + 1, out);
            }
        }
        Expr::Var { name, pos } => out.push_str(&format!("var {} @ {}\n", name, pos)),
        Expr::Field { obj, name, pos } => {
            out.push_str(&format!("field .{} @ {}\n", name, pos));
            render_expr(obj, level + 1, out);
        }
        Expr::OptionalField { obj, name, pos } => {
            out.push_str(&format!("optional-field ?.{} @ {}\n", name, pos));
            render_expr(obj, level + 1, out);
        }
        Expr::Index { obj, index, pos } => {
            out.push_str(&format!("index @ {}\n", pos));
            render_expr(obj, level + 1, out);
            render_expr(index, level + 1, out);
        }
        Expr::Slice { obj, start, end, step, pos } => {
            out.push_str(&format!("slice @ {}\n", pos));
            render_expr(obj, level + 1, out);
            if let Some(s) = start {
                render_expr(s, level + 1, out);
            }
            if let Some(e) = end {
                render_expr(e, level + 1, out);
            }
            if let Some(k) = step {
                render_expr(k, level + 1, out);
            }
        }
        Expr::Call { callee, args, pos } => {
            out.push_str(&format!("call ({} args) @ {}\n", args.len(), pos));
            render_expr(callee, level + 1, out);
            for a in args {
                render_expr(a, level + 1, out);
            }
        }
        Expr::Unary { op, operand, pos } => {
            out.push_str(&format!("unary {} @ {}\n", unary_name(*op), pos));
            render_expr(operand, level + 1, out);
        }
        Expr::Binary { op, left, right, pos } => {
            out.push_str(&format!("binary {} @ {}\n", binary_name(*op), pos));
            render_expr(left, level + 1, out);
            render_expr(right, level + 1, out);
        }
        Expr::Pipe { value, func, pos } => {
            out.push_str(&format!("pipe |> @ {}\n", pos));
            render_expr(value, level + 1, out);
            render_expr(func, level + 1, out);
        }
        Expr::NullCoalesce { left, right, pos } => {
            out.push_str(&format!("null-coalesce ?? @ {}\n", pos));
            render_expr(left, level + 1, out);
            render_expr(right, level + 1, out);
        }
        Expr::Try { expr, pos } => {
            out.push_str(&format!("try ? @ {}\n", pos));
            render_expr(expr, level + 1, out);
        }
        Expr::ForceUnwrap { expr, pos } => {
            out.push_str(&format!("force-unwrap ! @ {}\n", pos));
            render_expr(expr, level + 1, out);
        }
        Expr::IfExpr { cond, then, else_, pos } => {
            out.push_str(&format!("if-expr @ {}\n", pos));
            render_expr(cond, level + 1, out);
            render_expr(then, level + 1, out);
            render_expr(else_, level + 1, out);
        }
        Expr::ListComp { expr, var, iterable, cond, pos } => {
            out.push_str(&format!("list-comp for {} @ {}\n", var, pos));
            render_expr(expr, level + 1, out);
            render_expr(iterable, level + 1, out);
            if let Some(c) = cond {
                render_expr(c, level + 1, out);
            }
        }
        Expr::Closure { params, body, captures, pos, .. } => {
            let ps: Vec<String> = params.iter().map(|p| p.name.clone()).collect();
            let cs: Vec<String> = captures.clone();
            out.push_str(&format!(
                "closure ({}) capture[{}] @ {}\n",
                ps.join(", "),
                cs.join(", "),
                pos
            ));
            render_expr(body, level + 1, out);
        }
        Expr::Block { stmts, pos } => {
            out.push_str(&format!("block @ {}\n", pos));
            for s in stmts {
                render_stmt(s, level + 1, out);
            }
        }
        Expr::Match { subject, arms, pos } => {
            out.push_str(&format!("match @ {}\n", pos));
            render_expr(subject, level + 1, out);
            for arm in arms {
                indent(level + 1, out);
                out.push_str("arm:\n");
                render_pattern(&arm.pattern, level + 2, out);
                if let Some(g) = &arm.guard {
                    render_expr(g, level + 2, out);
                }
                render_expr(&arm.body, level + 2, out);
            }
        }
        Expr::Constructor { name, args, pos } => {
            out.push_str(&format!("constructor {} ({} args) @ {}\n", name, args.len(), pos));
            for a in args {
                render_expr(a, level + 1, out);
            }
        }
    }
}

fn render_pattern(p: &Pattern, level: usize, out: &mut String) {
    indent(level, out);
    match p {
        Pattern::Literal(e) => {
            out.push_str("pat-literal: ");
            let mut s = String::new();
            render_expr(e, 0, &mut s);
            out.push_str(&s.trim());
            out.push('\n');
        }
        Pattern::Binding(name) => out.push_str(&format!("pat-bind {}\n", name)),
        Pattern::Wildcard => out.push_str("pat-wildcard _\n"),
        Pattern::Tuple(items) => {
            out.push_str("pat-tuple\n");
            for i in items {
                render_pattern(i, level + 1, out);
            }
        }
        Pattern::Constructor(name, items) => {
            out.push_str(&format!("pat-ctor {}\n", name));
            for i in items {
                render_pattern(i, level + 1, out);
            }
        }
    }
}

fn op_name(op: AssignOp) -> &'static str {
    match op {
        AssignOp::Assign => "=",
        AssignOp::Plus => "+=",
        AssignOp::Minus => "-=",
        AssignOp::Star => "*=",
        AssignOp::Slash => "/=",
        AssignOp::IntDiv => "//=",
        AssignOp::Mod => "%=",
        AssignOp::Pow => "**=",
        AssignOp::BitAnd => "&=",
        AssignOp::BitOr => "|=",
        AssignOp::BitXor => "^=",
        AssignOp::Shl => "<<=",
        AssignOp::Shr => ">>=",
    }
}

fn unary_name(op: UnaryOp) -> &'static str {
    match op {
        UnaryOp::Neg => "-",
        UnaryOp::Not => "not",
        UnaryOp::BitNot => "~",
    }
}

fn binary_name(op: BinaryOp) -> &'static str {
    match op {
        BinaryOp::Add => "+",
        BinaryOp::Sub => "-",
        BinaryOp::Mul => "*",
        BinaryOp::Div => "/",
        BinaryOp::IntDiv => "//",
        BinaryOp::Mod => "%",
        BinaryOp::Pow => "**",
        BinaryOp::Eq => "==",
        BinaryOp::Ne => "!=",
        BinaryOp::Lt => "<",
        BinaryOp::Le => "<=",
        BinaryOp::Gt => ">",
        BinaryOp::Ge => ">=",
        BinaryOp::And => "and",
        BinaryOp::Or => "or",
        BinaryOp::BitAnd => "&",
        BinaryOp::BitOr => "|",
        BinaryOp::BitXor => "^",
        BinaryOp::Shl => "<<",
        BinaryOp::Shr => ">>",
    }
}
