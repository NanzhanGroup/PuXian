//! 普贤 (PuXian) M4 编译模式：AST → C 代码生成
//! 策略：每个顶层 def / impl 方法 → C 函数；顶层语句 → main()
//! 变量用 C 局部变量（函数级符号表），表达式用 C99 复合字面量 + statement expression
//! M4.1 支持核心语言；并发（spawn/chan/select）编译时明确报错（M4.2 实现）
#![allow(dead_code)]

use std::collections::HashMap;
use std::collections::HashSet;

use crate::ast::*;
use crate::token::Pos;

pub struct Codegen {
    out: String,
    // 闭包函数定义（生成过程中追加，最后合并到输出）
    closures: String,
    // 类型表
    structs: HashMap<String, Vec<String>>,     // struct 名 → 字段名
    enums: HashMap<String, Vec<String>>,       // enum 名 → 变体名
    impls: HashMap<String, Vec<FuncDef>>,      // 类型名 → impl 方法
    // 函数内变量表：name → C 变量名
    vars: HashMap<String, String>,
    // 简单类型推断：变量名 → 结构体类型名（用于方法静态分派）
    var_types: HashMap<String, String>,
    // 顶层全局名（M18 修复：函数内赋值到全局变量应生成 px_get/set_global，
    // 而非被 collect_assign_vars 预声明为函数局部变量 → null + 1 运行时错误）
    globals: HashSet<String>,
    uid: usize,
    closure_id: usize,
}

impl Codegen {
    pub fn new() -> Self {
        Codegen {
            out: String::new(),
            closures: String::new(),
            structs: HashMap::new(),
            enums: HashMap::new(),
            impls: HashMap::new(),
            vars: HashMap::new(),
            var_types: HashMap::new(),
            globals: HashSet::new(),
            uid: 0,
            closure_id: 0,
        }
    }

    fn uid(&mut self) -> usize {
        self.uid += 1;
        self.uid
    }

    fn tmp(&mut self) -> String {
        format!("_t{}", self.uid())
    }

    fn new_var(&mut self, name: &str) -> String {
        let v = format!("_v{}", self.uid());
        self.vars.insert(name.to_string(), v.clone());
        v
    }

    fn var_of(&self, name: &str) -> Option<&String> {
        self.vars.get(name)
    }

    // ==================== 程序入口 ====================

    pub fn generate(&mut self, prog: &Program) -> Result<String, String> {
        let mut out = String::new();
        out.push_str("/* 由普贤 (PuXian) 编译器自动生成 — px build */\n");
        out.push_str("#include \"runtime.h\"\n");
        out.push_str("#include <string.h>\n\n");

        // 第一遍：收集类型表
        self.collect_types(prog)?;

        // M18 修复：收集顶层全局名（函数 + 顶层变量赋值），供函数内赋值判定
        for stmt in &prog.items {
            match stmt {
                Stmt::FuncDef { name, .. } => {
                    self.globals.insert(name.clone());
                }
                Stmt::VarDecl { name, .. } => {
                    self.globals.insert(name.clone());
                }
                Stmt::Assign { target, .. } => {
                    if let Expr::Var { name, .. } = target {
                        self.globals.insert(name.clone());
                    }
                }
                _ => {}
            }
        }

        // 第二遍：生成函数定义（顶层 def + impl 方法）
        let mut top_defs: Vec<FuncDef> = Vec::new();
        for stmt in &prog.items {
            if let Stmt::FuncDef { name, params, ret_ty, body, pos } = stmt {
                top_defs.push(FuncDef { name: name.clone(), params: params.clone(), ret_ty: ret_ty.clone(), body: body.clone(), pos: *pos });
            }
        }

        // 生成 impl 方法函数：fn_{Type}_{method}（先于顶层函数，静态分派需要前置定义）
        let mut impl_list: Vec<(String, FuncDef)> = Vec::new();
        for (type_name, methods) in &self.impls {
            for m in methods {
                impl_list.push((type_name.clone(), m.clone()));
            }
        }
        for (type_name, m) in &impl_list {
            let fname = format!("fn_{}_{}", self.func_cname(type_name), self.func_cname(&m.name));
            let fdef = self.gen_func_named(m, &fname)?;
            out.push_str(&fdef);
            out.push('\n');
        }

        // 生成顶层 def 函数
        for f in &top_defs {
            out.push_str(&self.gen_func(f)?);
            out.push('\n');
        }

        // 第三遍：生成 main()
        out.push_str("int main(void) {\n");
        out.push_str("    px_register_builtins();\n");

        // 注册顶层函数
        for f in &top_defs {
            let cname = format!("fn_{}", self.func_cname(&f.name));
            out.push_str(&format!(
                "    px_set_global(\"{}\", px_func(\"{}\", {}, NULL));\n",
                f.name, f.name, cname
            ));
        }

        // 注册 impl 方法为 "Type.method"
        for (type_name, m) in &impl_list {
            let fname = format!("fn_{}_{}", self.func_cname(type_name), self.func_cname(&m.name));
            out.push_str(&format!(
                "    px_set_global(\"{}.{}\", px_func(\"{}.{}\", {}, NULL));\n",
                type_name, m.name, type_name, m.name, fname
            ));
        }

        // 执行顶层语句（除 def/struct/enum/trait/impl/import 外）
        for stmt in &prog.items {
            match stmt {
                Stmt::FuncDef { .. } | Stmt::StructDef { .. } | Stmt::EnumDef { .. }
                | Stmt::TraitDef { .. } | Stmt::ImplDef { .. } | Stmt::Import { .. } => {}
                _ => {
                    out.push_str(&self.gen_stmt(stmt, 1)?);
                }
            }
        }

        // 调用顶层 main()
        if let Some(f) = top_defs.iter().find(|f| f.name == "main") {
            let cname = format!("fn_{}", self.func_cname(&f.name));
            out.push_str(&format!(
                "    {{ LXValue _r = {}(NULL, 0, NULL); (void)_r; }}\n",
                cname
            ));
        }

        out.push_str("    return 0;\n}\n");

        // 合并闭包定义（放在所有函数定义之前，避免使用前未声明）
        let mut final_out = String::new();
        let main_part = out.clone();
        if let Some(pos) = main_part.find("int main(void)") {
            let head = &main_part[..pos];
            let tail = &main_part[pos..];
            // 在 head 中第一个函数定义（static LXValue）之前插入闭包定义
            if let Some(fpos) = head.find("static LXValue") {
                final_out.push_str(&head[..fpos]);
                final_out.push_str(&self.closures);
                final_out.push_str("\n");
                final_out.push_str(&head[fpos..]);
            } else {
                final_out.push_str(head);
                final_out.push_str(&self.closures);
                final_out.push_str("\n");
            }
            final_out.push_str(tail);
        } else {
            final_out.push_str(&main_part);
        }

        Ok(final_out)
    }

    fn collect_types(&mut self, prog: &Program) -> Result<(), String> {
        for stmt in &prog.items {
            match stmt {
                Stmt::StructDef { name, fields, .. } => {
                    let names: Vec<String> = fields.iter().map(|f| f.name.clone()).collect();
                    self.structs.insert(name.clone(), names);
                }
                Stmt::EnumDef { name, variants, .. } => {
                    let names: Vec<String> = variants.iter().map(|v| v.name.clone()).collect();
                    self.enums.insert(name.clone(), names);
                }
                Stmt::ImplDef { type_name, methods, .. } => {
                    self.impls.entry(type_name.clone()).or_default().extend(methods.clone());
                }
                _ => {}
            }
        }
        Ok(())
    }

    fn func_cname(&self, name: &str) -> String {
        name.chars()
            .map(|c| if c.is_ascii_alphanumeric() { c } else { '_' })
            .collect()
    }

    // ==================== 函数生成 ====================

    fn gen_func(&mut self, f: &FuncDef) -> Result<String, String> {
        let cname = format!("fn_{}", self.func_cname(&f.name));
        self.gen_func_named(f, &cname)
    }

    fn gen_func_named(&mut self, f: &FuncDef, cname: &str) -> Result<String, String> {
        let mut s = String::new();
        s.push_str(&format!(
            "static LXValue {}(LXValue* args, int nargs, void* ctx) {{\n",
            cname
        ));
        s.push_str("    (void)ctx;\n");
        // 保存外层变量表（函数内新作用域）
        let saved_vars = self.vars.clone();
        let saved_types = self.var_types.clone();
        self.vars.clear();
        self.var_types.clear();
        // 参数绑定
        for (i, p) in f.params.iter().enumerate() {
            let v = self.new_var(&p.name);
            s.push_str(&format!(
                "    LXValue {} = (nargs > {}) ? args[{}] : px_null();\n",
                v, i, i
            ));
        }
        // 预扫描：函数体内赋值（x = ... / x += ...）的变量视为局部变量（Python 语义）
        // 避免生成全局 px_set_global / px_get_global 导致状态泄漏与复合赋值丢失
        // M18 修复：顶层全局名（self.globals）不声明为局部——函数内修改全局变量
        // 应走 px_get_global/px_set_global（与解释器语义一致）；否则全局变量在函数内
        // 被初始化为局部 null，`n = n + 1` 报"无法相加: null + int"。
        let mut assign_names: Vec<String> = Vec::new();
        Self::collect_assign_vars(&f.body, &mut assign_names);
        for name in &assign_names {
            if self.var_of(name).is_some() {
                continue; // 已是参数/已声明
            }
            if self.globals.contains(name) {
                continue; // 顶层全局变量：函数内赋值更新全局
            }
            let v = self.new_var(name);
            s.push_str(&format!("    LXValue {} = px_null();\n", v));
        }
        // 函数体
        for stmt in &f.body {
            s.push_str(&self.gen_stmt(stmt, 1)?);
        }
        s.push_str("    return px_null();\n");
        s.push_str("}\n");
        // 恢复外层变量表
        self.vars = saved_vars;
        self.var_types = saved_types;
        Ok(s)
    }

    /// 收集语句中所有赋值目标变量名（递归进入 if/for/while/block）
    fn collect_assign_vars(stmts: &[Stmt], out: &mut Vec<String>) {
        for stmt in stmts {
            match stmt {
                Stmt::Assign { target, .. } => {
                    if let Expr::Var { name, .. } = target {
                        if !out.contains(name) {
                            out.push(name.clone());
                        }
                    }
                }
                Stmt::If { branches, else_branch, .. } => {
                    for (_, body) in branches {
                        Self::collect_assign_vars(body, out);
                    }
                    if let Some(eb) = else_branch {
                        Self::collect_assign_vars(eb, out);
                    }
                }
                Stmt::For { body, .. } => Self::collect_assign_vars(body, out),
                Stmt::While { body, .. } => Self::collect_assign_vars(body, out),
                _ => {}
            }
        }
    }

    // ==================== 语句生成 ====================

    fn gen_stmt(&mut self, stmt: &Stmt, indent: usize) -> Result<String, String> {
        let pad = "    ".repeat(indent);
        match stmt {
            Stmt::VarDecl { name, value, .. } => {
                let v = self.new_var(name);
                let rhs = match value {
                    Some(e) => self.gen_expr(e)?,
                    None => "px_null()".to_string(),
                };
                // 类型推断：结构体构造（Expr::Call 形式 Point(1,2) 或 Constructor）
                if let Some(e) = value {
                    let cname = match e {
                        Expr::Constructor { name, .. } => Some(name.clone()),
                        Expr::Call { callee, .. } => {
                            if let Expr::Var { name, .. } = callee.as_ref() {
                                Some(name.clone())
                            } else {
                                None
                            }
                        }
                        _ => None,
                    };
                    if let Some(cname) = cname {
                        if self.structs.contains_key(&cname) {
                            self.var_types.insert(name.clone(), cname);
                        }
                    }
                }
                Ok(format!("{}LXValue {} = {};\n", pad, v, rhs))
            }
            Stmt::Assign { target, op, value, .. } => {
                let rhs = self.gen_expr(value)?;
                match target {
                    Expr::Var { name, .. } => {
                        let v = match self.var_of(name) {
                            Some(v) => v.clone(),
                            None => {
                                // 全局变量（M18 修复：复合赋值须先读全局再运算再写回）
                                if *op == AssignOp::Assign {
                                    return Ok(format!(
                                        "{}px_set_global(\"{}\", {});\n",
                                        pad, name, rhs
                                    ));
                                }
                                let full = match op {
                                    AssignOp::Assign => rhs.clone(),
                                    AssignOp::Plus => format!("px_add(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Minus => format!("px_sub(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Star => format!("px_mul(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Slash => format!("px_div(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::IntDiv => format!("px_idiv(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Mod => format!("px_mod(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Pow => format!("px_pow(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::BitAnd => format!("px_bitand(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::BitOr => format!("px_bitor(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::BitXor => format!("px_bitxor(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Shl => format!("px_shl(px_get_global(\"{}\"), {})", name, rhs),
                                    AssignOp::Shr => format!("px_shr(px_get_global(\"{}\"), {})", name, rhs),
                                };
                                return Ok(format!(
                                    "{}px_set_global(\"{}\", {});\n",
                                    pad, name, full
                                ));
                            }
                        };
                        let full = match op {
                            AssignOp::Assign => rhs,
                            AssignOp::Plus => format!("px_add({}, {})", v, rhs),
                            AssignOp::Minus => format!("px_sub({}, {})", v, rhs),
                            AssignOp::Star => format!("px_mul({}, {})", v, rhs),
                            AssignOp::Slash => format!("px_div({}, {})", v, rhs),
                            AssignOp::IntDiv => format!("px_idiv({}, {})", v, rhs),
                            AssignOp::Mod => format!("px_mod({}, {})", v, rhs),
                            AssignOp::Pow => format!("px_pow({}, {})", v, rhs),
                            AssignOp::BitAnd => format!("px_bitand({}, {})", v, rhs),
                            AssignOp::BitOr => format!("px_bitor({}, {})", v, rhs),
                            AssignOp::BitXor => format!("px_bitxor({}, {})", v, rhs),
                            AssignOp::Shl => format!("px_shl({}, {})", v, rhs),
                            AssignOp::Shr => format!("px_shr({}, {})", v, rhs),
                        };
                        Ok(format!("{} {} = {};\n", pad, v, full))
                    }
                    Expr::Field { obj, name, .. } => {
                        let o = self.gen_expr(obj)?;
                        Ok(format!(
                            "{}px_field_set({}, \"{}\", {});\n",
                            pad, o, name, rhs
                        ))
                    }
                    Expr::Index { obj, index, .. } => {
                        let o = self.gen_expr(obj)?;
                        let i = self.gen_expr(index)?;
                        Ok(format!(
                            "{}px_index_set({}, {}, {});\n",
                            pad, o, i, rhs
                        ))
                    }
                    _ => Err(format!("不支持的赋值目标: {:?}", target)),
                }
            }
            Stmt::ExprStmt { expr, .. } => {
                let e = self.gen_expr(expr)?;
                Ok(format!("{}(void)({});\n", pad, e))
            }
            Stmt::If { branches, else_branch, .. } => {
                let mut s = String::new();
                for (i, (cond, body)) in branches.iter().enumerate() {
                    let c = self.gen_expr(cond)?;
                    let kw = if i == 0 { "if" } else { "else if" };
                    s.push_str(&format!("{}{} (px_is_truthy({})) {{\n", pad, kw, c));
                    for st in body {
                        s.push_str(&self.gen_stmt(st, indent + 1)?);
                    }
                    s.push_str(&format!("{}}}\n", pad));
                }
                if let Some(eb) = else_branch {
                    s.push_str(&format!("{}else {{\n", pad));
                    for st in eb {
                        s.push_str(&self.gen_stmt(st, indent + 1)?);
                    }
                    s.push_str(&format!("{}}}\n", pad));
                }
                Ok(s)
            }
            Stmt::While { cond, body, .. } => {
                let c = self.gen_expr(cond)?;
                let mut s = format!("{}while (px_is_truthy({})) {{\n", pad, c);
                for st in body {
                    s.push_str(&self.gen_stmt(st, indent + 1)?);
                }
                s.push_str(&format!("{}}}\n", pad));
                Ok(s)
            }
            Stmt::For { var, iterable, body, .. } => {
                let it = self.gen_expr(iterable)?;
                let it_var = self.tmp();
                let idx_var = self.tmp();
                let v = self.new_var(var);
                let mut s = String::new();
                s.push_str(&format!("{}LXValue {} = {};\n", pad, it_var, it));
                s.push_str(&format!(
                    "{}for (int {} = 0; {} < {}.as.obj->as.list.len; {}++) {{\n",
                    pad, idx_var, idx_var, it_var, idx_var
                ));
                s.push_str(&format!(
                    "{}    LXValue {} = {}.as.obj->as.list.items[{}];\n",
                    pad, v, it_var, idx_var
                ));
                for st in body {
                    s.push_str(&self.gen_stmt(st, indent + 1)?);
                }
                s.push_str(&format!("{}}}\n", pad));
                Ok(s)
            }
            Stmt::Return { value, .. } => {
                match value {
                    Some(e) => {
                        let e = self.gen_expr(e)?;
                        Ok(format!("{}return {};\n", pad, e))
                    }
                    None => Ok(format!("{}return px_null();\n", pad)),
                }
            }
            Stmt::Break { .. } => Ok(format!("{}break;\n", pad)),
            Stmt::Continue { .. } => Ok(format!("{}continue;\n", pad)),
            Stmt::Empty { .. } => Ok(String::new()),
            Stmt::ChanDecl { name, elem_ty: _, pos: _ } => {
                let v = self.new_var(name);
                Ok(format!("{}LXValue {} = px_chan_create(0);\n", pad, v))
            }
            Stmt::Send { chan, value, pos: _ } => {
                let c = self.gen_expr(chan)?;
                let v = self.gen_expr(value)?;
                Ok(format!("{}px_chan_send({}, {});\n", pad, c, v))
            }
            Stmt::Recv { chan, pos: _ } => {
                let c = self.gen_expr(chan)?;
                Ok(format!("{}px_chan_recv({});\n", pad, c))
            }
            Stmt::Spawn { expr, pos: _ } => {
                match expr.as_ref() {
                    Expr::Call { callee, args, .. } => {
                        if let Expr::Var { name: fname, .. } = callee.as_ref() {
                            let mut parts = Vec::new();
                            for a in args {
                                parts.push(self.gen_expr(a)?);
                            }
                            Ok(format!(
                                "{}px_spawn_name(\"{}\", (LXValue[]){{{}}}, {});\n",
                                pad,
                                fname,
                                parts.join(", "),
                                parts.len()
                            ))
                        } else {
                            Err("编译模式 spawn 仅支持直接函数调用（方法调用请用 `lx run`）".to_string())
                        }
                    }
                    _ => Err("编译模式 spawn 仅支持函数调用表达式".to_string()),
                }
            }
            Stmt::Select { arms, else_branch, pos: _ } => {
                self.gen_select(arms, else_branch, indent)
            }
            Stmt::Import { .. } => Ok(format!("{}/* import 忽略（MVP） */\n", pad)),
            Stmt::StructDef { .. } | Stmt::EnumDef { .. } | Stmt::TraitDef { .. }
            | Stmt::ImplDef { .. } | Stmt::FuncDef { .. } => {
                Ok(String::new()) // 顶层定义在 generate 中处理
            }
        }
    }

    // ==================== select 生成（M4.2） ====================

    fn gen_select(
        &mut self,
        arms: &[(Option<String>, Expr, Vec<Stmt>)],
        else_branch: &Option<Vec<Stmt>>,
        indent: usize,
    ) -> Result<String, String> {
        let pad = "    ".repeat(indent);
        let n = arms.len();
        if n == 0 {
            return Err("select 至少需要一个 case 分支".to_string());
        }
        let uid = self.uid();
        let mut s = String::new();

        // select body 是独立块作用域
        let saved_vars = self.vars.clone();
        let saved_types = self.var_types.clone();

        // 为每个 arm 求值 chan 表达式（循环外只求值一次）
        let mut chan_parts = Vec::new();
        for (_, expr, _) in arms {
            let ch_expr = match expr {
                Expr::Call { callee, .. } => {
                    if let Expr::Field { obj, name, .. } = callee.as_ref() {
                        if name == "recv" {
                            obj.as_ref().clone()
                        } else {
                            return Err(format!("select case 仅支持 ch.recv()（不支持 .{}）", name));
                        }
                    } else {
                        return Err("select case 仅支持 ch.recv()".to_string());
                    }
                }
                _ => return Err("select case 仅支持 ch.recv()".to_string()),
            };
            chan_parts.push(self.gen_expr(&ch_expr)?);
        }
        s.push_str(&format!(
            "{}LXValue _chans{}[{}] = {{{}}};\n",
            pad, uid, n, chan_parts.join(", ")
        ));

        s.push_str(&format!("{}_sel_retry_{}: {{\n", pad, uid));
        let order_init: Vec<String> = (0..n).map(|i| i.to_string()).collect();
        s.push_str(&format!(
            "{}    int _ord{}[{}] = {{{}}};\n",
            pad, uid, n, order_init.join(", ")
        ));
        if n > 1 {
            let mut shuffle = format!(
                "{}    for (int _i{} = {} - 1; _i{} > 0; _i{}--) {{ ",
                pad, uid, n, uid, uid
            );
            shuffle.push_str(&format!(
                "int _j{} = rand() % (_i{} + 1); ",
                uid, uid
            ));
            shuffle.push_str(&format!(
                "int _t{} = _ord{}[_i{}]; _ord{}[_i{}] = _ord{}[_j{}]; _ord{}[_j{}] = _t{}; ",
                uid, uid, uid, uid, uid, uid, uid, uid, uid, uid
            ));
            shuffle.push_str("}\n");
            s.push_str(&shuffle);
        }
        s.push_str(&format!("{}    LXValue _rv{} = px_null();\n", pad, uid));
        s.push_str(&format!("{}    int _picked{} = -1;\n", pad, uid));
        s.push_str(&format!(
            "{}    for (int _k{} = 0; _k{} < {}; _k{}++) {{\n",
            pad, uid, uid, n, uid
        ));
        s.push_str(&format!(
            "{}        int _idx{} = _ord{}[_k{}];\n",
            pad, uid, uid, uid
        ));
        s.push_str(&format!(
            "{}        if (px_chan_try_recv(_chans{}[_idx{}], &_rv{})) {{ _picked{} = _idx{}; break; }}\n",
            pad, uid, uid, uid, uid, uid
        ));
        s.push_str(&format!("{}    }}\n", pad));

        // 命中：执行对应 arm body
        s.push_str(&format!("{}    if (_picked{} >= 0) {{\n", pad, uid));
        for (i, (bind, _, body)) in arms.iter().enumerate() {
            let cond = if i == 0 {
                format!("if (_picked{} == {})", uid, i)
            } else {
                format!("else if (_picked{} == {})", uid, i)
            };
            s.push_str(&format!("{}        {} {{\n", pad, cond));
            if let Some(bname) = bind {
                let v = self.new_var(bname);
                s.push_str(&format!("{}            LXValue {} = _rv{};\n", pad, v, uid));
            }
            for st in body {
                s.push_str(&self.gen_stmt(st, indent + 3)?);
            }
            s.push_str(&format!("{}        }}\n", pad));
        }
        s.push_str(&format!("{}        goto _sel_done_{};\n", pad, uid));
        s.push_str(&format!("{}    }}\n", pad));

        // else 分支
        if let Some(eb) = else_branch {
            s.push_str(&format!("{}    {{\n", pad));
            for st in eb {
                s.push_str(&self.gen_stmt(st, indent + 2)?);
            }
            s.push_str(&format!("{}        goto _sel_done_{};\n", pad, uid));
            s.push_str(&format!("{}    }}\n", pad));
        }

        // 无 else：阻塞等待后重试
        s.push_str(&format!("{}    px_select_wait();\n", pad));
        s.push_str(&format!("{}}}\n", pad));
        s.push_str(&format!("{}goto _sel_retry_{};\n", pad, uid));
        s.push_str(&format!("{}_sel_done_{}: ;\n", pad, uid));

        self.vars = saved_vars;
        self.var_types = saved_types;
        Ok(s)
    }

    // ==================== 表达式生成 ====================

    fn gen_expr(&mut self, expr: &Expr) -> Result<String, String> {
        match expr {
            Expr::Int { value, .. } => Ok(format!("px_int({}LL)", value)),
            Expr::Float { value, .. } => Ok(format!("px_float({})", value)),
            Expr::Str { value, .. } => Ok(format!("px_str(\"{}\")", self.escape_str(value))),
            Expr::Bool { value, .. } => Ok(format!("px_bool({})", if *value { "true" } else { "false" })),
            Expr::Null { .. } => Ok("px_null()".to_string()),
            Expr::List { items, .. } => {
                let mut parts = Vec::new();
                for it in items {
                    parts.push(self.gen_expr(it)?);
                }
                Ok(format!(
                    "px_list_n((LXValue[]){{{}}}, {})",
                    parts.join(", "),
                    parts.len()
                ))
            }
            Expr::Tuple { items, .. } => {
                let mut parts = Vec::new();
                for it in items {
                    parts.push(self.gen_expr(it)?);
                }
                Ok(format!(
                    "px_tuple((LXValue[]){{{}}}, {})",
                    parts.join(", "),
                    parts.len()
                ))
            }
            Expr::Dict { entries, .. } => {
                let mut s = format!("({{ LXValue _d = px_dict(); ");
                for (k, v) in entries {
                    let ke = self.gen_expr(k)?;
                    let ve = self.gen_expr(v)?;
                    // 键转字符串：MVP 支持字符串键
                    s.push_str(&format!(
                        "{{ LXValue _k = {}; if (_k.type == PX_STR) px_dict_set(_d, _k.as.obj->as.str.data, {}); }} ",
                        ke, ve
                    ));
                }
                s.push_str("_d; })");
                Ok(s)
            }
            Expr::Var { name, .. } => {
                if let Some(v) = self.var_of(name) {
                    Ok(v.clone())
                } else {
                    // 内置函数 / 全局
                    Ok(format!("px_get_global(\"{}\")", name))
                }
            }
            Expr::Field { obj, name, .. } => {
                // enum 变体访问：Color.Red
                if let Expr::Var { name: oname, .. } = obj.as_ref() {
                    if self.enums.contains_key(oname) {
                        return Ok(format!("px_enum(\"{}\", \"{}\")", oname, name));
                    }
                }
                let o = self.gen_expr(obj)?;
                Ok(format!("px_field({}, \"{}\")", o, name))
            }
            Expr::OptionalField { obj, name, .. } => {
                let o = self.gen_expr(obj)?;
                let t = self.tmp();
                Ok(format!(
                    "({{ LXValue {} = {}; px_is_null({}) ? px_null() : px_field({}, \"{}\"); }})",
                    t, o, t, o, name
                ))
            }
            Expr::Index { obj, index, .. } => {
                let o = self.gen_expr(obj)?;
                let i = self.gen_expr(index)?;
                Ok(format!("px_index({}, {})", o, i))
            }
            Expr::Slice { obj, start, end, .. } => {
                let o = self.gen_expr(obj)?;
                let s = match start {
                    Some(e) => self.gen_expr(e)?,
                    None => "px_null()".to_string(),
                };
                let e2 = match end {
                    Some(e) => self.gen_expr(e)?,
                    None => "px_null()".to_string(),
                };
                Ok(format!("px_slice({}, {}, {})", o, s, e2))
            }
            Expr::Call { callee, args, .. } => {
                // 通道构造：chan[T](cap) / chan[T]() 解析为 Call(Var("chan"))
                if let Expr::Var { name: cname, .. } = callee.as_ref() {
                    if cname == "chan" {
                        let cap = match args.first() {
                            Some(Expr::Int { value, .. }) => value.to_string(),
                            Some(a) => format!("(int)({}).as.i", self.gen_expr(a)?),
                            None => "0".to_string(),
                        };
                        return Ok(format!("px_chan_create({})", cap));
                    }
                    // M13：锁构造 mutex() / rwlock()
                    if cname == "mutex" {
                        return Ok("px_mutex_create()".to_string());
                    }
                    if cname == "rwlock" {
                        return Ok("px_rwlock_create()".to_string());
                    }
                }
                // 结构体/枚举构造：Point(1,2) 解析为 Call(Var("Point"))
                if let Expr::Var { name: cname, .. } = callee.as_ref() {
                    if let Some(fields) = self.structs.get(cname).cloned() {
                        if fields.len() != args.len() {
                            return Err(format!("结构体 {} 需要 {} 个字段，给出 {}", cname, fields.len(), args.len()));
                        }
                        let mut parts = Vec::new();
                        for a in args {
                            parts.push(self.gen_expr(a)?);
                        }
                        let fnames: Vec<String> = fields.iter().map(|f| format!("\"{}\"", f)).collect();
                        return Ok(format!(
                            "px_struct(\"{}\", (char*[]){{{}}}, (LXValue[]){{{}}}, {})",
                            cname,
                            fnames.join(", "),
                            parts.join(", "),
                            fields.len()
                        ));
                    }
                    if self.enums.contains_key(cname) {
                        if args.len() != 1 {
                            return Err(format!("枚举 {} 构造需要一个变体名", cname));
                        }
                        let v = self.gen_expr(&args[0])?;
                        return Ok(format!("px_enum(\"{}\", ({}).as.obj->as.enum_inst.variant)", cname, v));
                    }
                }
                // 方法调用：obj.method(args)
                if let Expr::Field { obj, name, .. } = callee.as_ref() {
                    // 静态分派：已知类型 + impl 方法
                    let static_type: Option<String> = if let Expr::Var { name: oname, .. } = obj.as_ref() {
                        self.var_types.get(oname).cloned()
                    } else {
                        None
                    };
                    if let Some(t) = static_type {
                        let has_method = self.impls.get(&t).map(|ms| ms.iter().any(|m| m.name == *name)).unwrap_or(false);
                        if has_method {
                            let o = self.gen_expr(obj)?;
                            let mut parts = vec![o];
                            for a in args {
                                parts.push(self.gen_expr(a)?);
                            }
                            let fnname = format!("fn_{}_{}", self.func_cname(&t), self.func_cname(name));
                            return Ok(format!(
                                "{}((LXValue[]){{{}}}, {}, NULL)",
                                fnname,
                                parts.join(", "),
                                parts.len()
                            ));
                        }
                    }
                    // 运行时方法分派（内置方法 + 用户方法注册为 "Type.method"）
                    let o = self.gen_expr(obj)?;
                    let mut parts = Vec::new();
                    for a in args {
                        parts.push(self.gen_expr(a)?);
                    }
                    return Ok(format!(
                        "px_method({}, \"{}\", (LXValue[]){{{}}}, {})",
                        o,
                        name,
                        parts.join(", "),
                        parts.len()
                    ));
                }
                // 普通调用
                let c = self.gen_expr(callee)?;
                let mut parts = Vec::new();
                for a in args {
                    parts.push(self.gen_expr(a)?);
                }
                Ok(format!(
                    "px_call({}, (LXValue[]){{{}}}, {})",
                    c,
                    parts.join(", "),
                    parts.len()
                ))
            }
            Expr::Unary { op, operand, .. } => {
                let o = self.gen_expr(operand)?;
                match op {
                    UnaryOp::Neg => Ok(format!("px_neg({})", o)),
                    UnaryOp::Not => Ok(format!("px_not({})", o)),
                    UnaryOp::BitNot => Ok(format!("px_bitnot({})", o)),
                }
            }
            Expr::Binary { op, left, right, .. } => {
                let l = self.gen_expr(left)?;
                let r = self.gen_expr(right)?;
                // 短路 and/or
                if *op == BinaryOp::And {
                    let t = self.tmp();
                    return Ok(format!(
                        "({{ LXValue {} = {}; px_is_truthy({}) ? {} : {}; }})",
                        t, l, t, r, t
                    ));
                }
                if *op == BinaryOp::Or {
                    let t = self.tmp();
                    return Ok(format!(
                        "({{ LXValue {} = {}; px_is_truthy({}) ? {} : {}; }})",
                        t, l, t, t, r
                    ));
                }
                let f = match op {
                    BinaryOp::Add => "px_add",
                    BinaryOp::Sub => "px_sub",
                    BinaryOp::Mul => "px_mul",
                    BinaryOp::Div => "px_div",
                    BinaryOp::IntDiv => "px_idiv",
                    BinaryOp::Mod => "px_mod",
                    BinaryOp::Pow => "px_pow",
                    BinaryOp::Eq => "px_eq",
                    BinaryOp::Ne => "px_ne",
                    BinaryOp::Lt => "px_lt",
                    BinaryOp::Le => "px_le",
                    BinaryOp::Gt => "px_gt",
                    BinaryOp::Ge => "px_ge",
                    BinaryOp::BitAnd => "px_bitand",
                    BinaryOp::BitOr => "px_bitor",
                    BinaryOp::BitXor => "px_bitxor",
                    BinaryOp::Shl => "px_shl",
                    BinaryOp::Shr => "px_shr",
                    _ => "px_add",
                };
                Ok(format!("{}({}, {})", f, l, r))
            }
            Expr::Pipe { value, func, .. } => {
                // x |> f(args)  →  f(x, args)
                let v = self.gen_expr(value)?;
                if let Expr::Call { callee, args, .. } = func.as_ref() {
                    let c = self.gen_expr(callee)?;
                    let mut parts = vec![v];
                    for a in args {
                        parts.push(self.gen_expr(a)?);
                    }
                    Ok(format!(
                        "px_call({}, (LXValue[]){{{}}}, {})",
                        c,
                        parts.join(", "),
                        parts.len()
                    ))
                } else {
                    let f = self.gen_expr(func)?;
                    Ok(format!("px_call({}, (LXValue[]){{{}}}, 1)", f, v))
                }
            }
            Expr::NullCoalesce { left, right, .. } => {
                let l = self.gen_expr(left)?;
                let r = self.gen_expr(right)?;
                let t = self.tmp();
                Ok(format!(
                    "({{ LXValue {} = {}; px_is_null({}) ? {} : {}; }})",
                    t, l, t, r, t
                ))
            }
            Expr::Try { expr, .. } => {
                let e = self.gen_expr(expr)?;
                // MVP：直接返回（null 视为错误）
                let t = self.tmp();
                Ok(format!(
                    "({{ LXValue {} = {}; if (px_is_null({})) px_error(\"unwrap null value\"); {}; }})",
                    t, e, t, t
                ))
            }
            Expr::ForceUnwrap { expr, .. } => {
                let e = self.gen_expr(expr)?;
                let t = self.tmp();
                Ok(format!(
                    "({{ LXValue {} = {}; if (px_is_null({})) px_error(\"force unwrap null\"); {}; }})",
                    t, e, t, t
                ))
            }
            Expr::IfExpr { cond, then, else_, .. } => {
                let c = self.gen_expr(cond)?;
                let a = self.gen_expr(then)?;
                let b = self.gen_expr(else_)?;
                let t = self.tmp();
                Ok(format!(
                    "({{ LXValue {}; if (px_is_truthy({})) {{ {} = {}; }} else {{ {} = {}; }} {}; }})",
                    t, c, t, a, t, b, t
                ))
            }
            Expr::ListComp { expr, var, iterable, cond, .. } => {
                let it = self.gen_expr(iterable)?;
                let rv = self.tmp();
                let iv = self.tmp();
                let idx = self.tmp();
                // 绑定迭代变量到临时名
                let lcvar = format!("_lc{}", self.uid());
                let saved = self.vars.insert(var.clone(), lcvar.clone());
                let e = self.gen_expr(expr)?;
                let cond_c = match cond {
                    Some(c) => Some(self.gen_expr(c)?),
                    None => None,
                };
                match saved {
                    Some(v) => { self.vars.insert(var.clone(), v); }
                    None => { self.vars.remove(var); }
                }
                let mut s = format!(
                    "({{ LXValue {} = px_list(0); LXValue {} = {}; ",
                    rv, iv, it
                );
                s.push_str(&format!(
                    "for (int {} = 0; {} < {}.as.obj->as.list.len; {}++) {{ ",
                    idx, idx, iv, idx
                ));
                s.push_str(&format!(
                    "LXValue {} = {}.as.obj->as.list.items[{}]; ",
                    lcvar, iv, idx
                ));
                if let Some(c) = cond_c {
                    s.push_str(&format!("if (px_is_truthy({})) {{ ", c));
                    s.push_str(&format!("px_list_push({}, {}); ", rv, e));
                    s.push_str("} ");
                } else {
                    s.push_str(&format!("px_list_push({}, {}); ", rv, e));
                }
                s.push_str("} ");
                s.push_str(&format!("{}; }})", rv));
                Ok(s)
            }
            Expr::Closure { params, body, .. } => {
                // 无捕获闭包 → 独立 C 函数
                self.closure_id += 1;
                let cid = self.closure_id;
                let fnname = format!("fn_closure_{}", cid);
                // 生成闭包函数定义
                let mut fdef = String::new();
                fdef.push_str(&format!(
                    "static LXValue {}(LXValue* args, int nargs, void* ctx) {{\n",
                    fnname
                ));
                fdef.push_str("    (void)ctx;\n");
                // 参数
                let saved_vars = self.vars.clone();
                let saved_types = self.var_types.clone();
                self.vars.clear();
                self.var_types.clear();
                for (i, p) in params.iter().enumerate() {
                    let v = self.new_var(&p.name);
                    fdef.push_str(&format!(
                        "    LXValue {} = (nargs > {}) ? args[{}] : px_null();\n",
                        v, i, i
                    ));
                }
                // 闭包体是表达式（通常 Block）
                let b = self.gen_expr(body)?;
                fdef.push_str(&format!("    return {};\n", b));
                fdef.push_str("}\n");
                self.closures.push_str(&fdef);
                self.vars = saved_vars;
                self.var_types = saved_types;
                Ok(format!("px_func(\"<closure{}>\", {}, NULL)", cid, fnname))
            }
            Expr::Block { stmts, .. } => {
                // 块表达式：执行语句，返回最后一个表达式的值
                let mut s = String::from("({ ");
                s.push_str("LXValue _blk = px_null(); ");
                for st in stmts {
                    if let Stmt::ExprStmt { expr, .. } = st {
                        let e = self.gen_expr(expr)?;
                        s.push_str(&format!("_blk = {}; ", e));
                    } else {
                        s.push_str(&self.gen_stmt(st, 0)?);
                    }
                }
                s.push_str("_blk; })");
                Ok(s)
            }
            Expr::Match { subject, arms, .. } => {
                let sub = self.gen_expr(subject)?;
                let t = self.tmp();
                let mut s = format!("({{ LXValue {} = {}; ", t, sub);
                let mut first = true;
                for arm in arms {
                    let cond_c = self.gen_pattern_cond(arm, t.clone())?;
                    let body_c = self.gen_expr(&arm.body)?;
                    let kw = if first { "if" } else { "else if" };
                    s.push_str(&format!("{} ({}) {{ {} = {}; }} ", kw, cond_c, t, body_c));
                    first = false;
                }
                s.push_str(&format!("{}; }})", t));
                Ok(s)
            }
            Expr::Constructor { name, args, .. } => {
                // 结构体构造 Point(1, 2)
                if let Some(fields) = self.structs.get(name).cloned() {
                    if fields.len() != args.len() {
                        return Err(format!("结构体 {} 需要 {} 个字段，给出 {}", name, fields.len(), args.len()));
                    }
                    let mut parts = Vec::new();
                    for a in args {
                        parts.push(self.gen_expr(a)?);
                    }
                    let fnames: Vec<String> = fields.iter().map(|f| format!("\"{}\"", f)).collect();
                    Ok(format!(
                        "px_struct(\"{}\", (char*[]){{{}}}, (LXValue[]){{{}}}, {})",
                        name,
                        fnames.join(", "),
                        parts.join(", "),
                        fields.len()
                    ))
                } else if self.enums.contains_key(name) {
                    // 枚举构造：无载荷变体
                    if args.len() != 1 {
                        return Err(format!("枚举 {} 构造需要一个变体名", name));
                    }
                    let v = self.gen_expr(&args[0])?;
                    Ok(format!("px_enum(\"{}\", ({}).as.obj->as.enum_inst.variant)", name, v))
                } else {
                    // 普通函数调用（大写开头变量当函数）
                    let mut parts = Vec::new();
                    for a in args {
                        parts.push(self.gen_expr(a)?);
                    }
                    Ok(format!(
                        "px_call(px_get_global(\"{}\"), (LXValue[]){{{}}}, {})",
                        name,
                        parts.join(", "),
                        parts.len()
                    ))
                }
            }
        }
    }

    // 生成 match 模式条件
    fn gen_pattern_cond(&mut self, arm: &MatchArm, subject: String) -> Result<String, String> {
        match &arm.pattern {
            Pattern::Literal(expr) => {
                let e = self.gen_expr(expr)?;
                Ok(format!("px_is_truthy(px_eq({}, {}))", subject, e))
            }
            Pattern::Binding(name) => {
                // 大写开头 = 枚举变体匹配；小写 = 变量绑定（绑定在 body 中用）
                if name.chars().next().map(|c| c.is_uppercase()).unwrap_or(false) {
                    Ok(format!(
                        "({}.type == PX_ENUM && strcmp({}.as.obj->as.enum_inst.variant, \"{}\") == 0)",
                        subject, subject, name
                    ))
                } else {
                    // 变量绑定：绑定到 body 中的变量（通过临时 var 表）
                    // MVP：在 body 表达式里直接引用 subject
                    let _ = name;
                    Ok("true".to_string())
                }
            }
            Pattern::Wildcard => Ok("true".to_string()),
            Pattern::Tuple(items) => {
                // 元组模式 MVP：比较第一个元素
                if let Some(p) = items.first() {
                    let arm2 = MatchArm { pattern: p.clone(), guard: None, body: Expr::Null { pos: Pos::new(0, 0) }, pos: Pos::new(0, 0) };
                    self.gen_pattern_cond(&arm2, subject)
                } else {
                    Ok("true".to_string())
                }
            }
            Pattern::Constructor(name, _) => {
                Ok(format!(
                    "({}.type == PX_ENUM && strcmp({}.as.obj->as.enum_inst.variant, \"{}\") == 0)",
                    subject, subject, name
                ))
            }
        }
    }

    fn escape_str(&self, s: &str) -> String {
        let mut out = String::new();
        for c in s.chars() {
            match c {
                '\\' => out.push_str("\\\\"),
                '"' => out.push_str("\\\""),
                '\n' => out.push_str("\\n"),
                '\r' => out.push_str("\\r"),
                '\t' => out.push_str("\\t"),
                _ => out.push(c),
            }
        }
        out
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;
    use crate::parser::Parser;

    fn gen_code(src: &str) -> String {
        let tokens = Lexer::new(src).tokenize().unwrap();
        let prog = Parser::new(tokens).parse_program().unwrap();
        let mut cg = Codegen::new();
        cg.generate(&prog).unwrap()
    }

    #[test]
    fn test_gen_basic() {
        let c = gen_code("let x = 1 + 2\nprint(x)\n");
        assert!(c.contains("int main(void)"));
        assert!(c.contains("px_add"));
    }

    #[test]
    fn test_gen_chan_create() {
        let c = gen_code("let ch = chan[int](4)\nch.send(1)\nlet v = ch.recv()\n");
        assert!(c.contains("px_chan_create(4)"));
        // send/recv 方法调用 → 运行时 px_method 分派（px_method 内部调用 px_chan_send/recv）
        assert!(c.contains("px_method"));
        assert!(c.contains("\"send\""));
        assert!(c.contains("\"recv\""));
    }

    #[test]
    fn test_gen_spawn() {
        let c = gen_code(
            "def worker(id: int, ch):\n    ch.send(id)\nlet ch = chan[int](1)\nspawn worker(1, ch)\n",
        );
        assert!(c.contains("px_spawn_name"));
        assert!(c.contains("fn_worker"));
    }

    #[test]
    fn test_gen_select() {
        let c = gen_code(
            "select:\n    case x = ch.recv():\n        print(x)\n    case _:\n        print(\"empty\")\n",
        );
        assert!(c.contains("px_chan_try_recv"));
        assert!(c.contains("_sel_retry_"));
        assert!(c.contains("goto _sel_done_"));
    }

    #[test]
    fn test_gen_assign_local() {
        let c = gen_code("var got = 0\ngot = 42\nprint(got)\n");
        // 赋值必须有目标变量（M4 修复的 bug：不能生成裸 " = 42;"）
        assert!(!c.contains("     = px_int(42LL);"));
        assert!(c.contains("_v1 = px_int(42LL);"));
    }
}
