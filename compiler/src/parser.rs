//! 普贤 (PuXian) 递归下降解析器
//! 实现 spec.md §3-§8、附录 B 运算符优先级

use crate::ast::*;
use crate::token::{Pos, Token, TokenKind};

/// 语法错误
#[derive(Debug, Clone)]
pub struct ParseError {
    pub code: &'static str,
    pub msg: String,
    pub pos: Pos,
}

impl std::fmt::Display for ParseError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}: 语法错误 {}: {}", self.pos, self.code, self.msg)
    }
}

impl From<crate::lexer::LexError> for ParseError {
    fn from(e: crate::lexer::LexError) -> Self {
        ParseError {
            code: e.code,
            msg: e.msg,
            pos: e.pos,
        }
    }
}

pub type ParseResult<T> = Result<T, ParseError>;

pub struct Parser {
    tokens: Vec<Token>,
    pos: usize,
}

impl Parser {
    pub fn new(tokens: Vec<Token>) -> Self {
        Parser { tokens, pos: 0 }
    }

    // ==================== 工具 ====================

    fn peek(&self) -> &Token {
        &self.tokens[self.pos]
    }

    fn peek_kind(&self) -> &TokenKind {
        &self.tokens[self.pos].kind
    }

    fn peek2_kind(&self) -> &TokenKind {
        self.tokens
            .get(self.pos + 1)
            .map(|t| &t.kind)
            .unwrap_or(&TokenKind::Eof)
    }

    fn advance(&mut self) -> Token {
        let t = self.tokens[self.pos].clone();
        if self.pos + 1 < self.tokens.len() {
            self.pos += 1;
        }
        t
    }

    fn check(&self, kind: &TokenKind) -> bool {
        self.peek_kind() == kind
    }

    fn err(&self, code: &'static str, msg: impl Into<String>) -> ParseError {
        ParseError {
            code,
            msg: msg.into(),
            pos: self.peek().pos,
        }
    }

    #[allow(dead_code)]
    fn err_at(&self, pos: Pos, code: &'static str, msg: impl Into<String>) -> ParseError {
        ParseError {
            code,
            msg: msg.into(),
            pos,
        }
    }

    fn expect(&mut self, kind: TokenKind, what: &str) -> ParseResult<Token> {
        if self.peek_kind() == &kind {
            Ok(self.advance())
        } else {
            Err(self.err(
                "E2001",
                format!("期望 {}，实际得到 {}", what, self.peek_kind()),
            ))
        }
    }

    fn expect_ident(&mut self, what: &str) -> ParseResult<String> {
        match self.peek_kind() {
            TokenKind::Ident(name) => {
                let name = name.clone();
                self.advance();
                Ok(name)
            }
            _ => Err(self.err("E2001", format!("期望{}，实际得到 {}", what, self.peek_kind()))),
        }
    }

    /// 接受标识符或关键字作为名字（成员名/参数名，如 .send、.recv、self）
    fn expect_name(&mut self, what: &str) -> ParseResult<String> {
        match self.peek_kind() {
            TokenKind::Ident(name) => {
                let name = name.clone();
                self.advance();
                Ok(name)
            }
            k if is_name_token(k) => {
                let s = k.to_string();
                self.advance();
                Ok(s)
            }
            _ => Err(self.err("E2001", format!("期望{}，实际得到 {}", what, self.peek_kind()))),
        }
    }

    fn skip_newlines(&mut self) {
        while self.check(&TokenKind::Newline) {
            self.advance();
        }
    }

    /// 大括号块体内缩进无关紧要（{} 已界定范围）：跳过行首 Indent/Dedent token
    fn skip_brace_indents(&mut self) {
        while self.check(&TokenKind::Indent) || self.check(&TokenKind::Dedent) {
            self.advance();
        }
    }

    /// 跳过 Newline；若遇到 Dedent 则停止（块结束标记）
    fn skip_newlines_in_block(&mut self) {
        while self.check(&TokenKind::Newline) {
            self.advance();
        }
    }

    // ==================== 程序 ====================

    pub fn parse_program(&mut self) -> ParseResult<Program> {
        let mut items = Vec::new();
        self.skip_newlines();
        while !self.check(&TokenKind::Eof) {
            items.push(self.parse_stmt()?);
            self.skip_newlines();
        }
        Ok(Program { items })
    }

    // ==================== 语句 ====================

    fn parse_stmt(&mut self) -> ParseResult<Stmt> {
        let kind = self.peek_kind().clone();
        match kind {
            TokenKind::Let => self.parse_var_decl(VarKind::Let),
            TokenKind::Var => self.parse_var_decl(VarKind::Var),
            TokenKind::Const => self.parse_var_decl(VarKind::Const),
            TokenKind::If => self.parse_if(),
            TokenKind::For => self.parse_for(),
            TokenKind::While => self.parse_while(),
            TokenKind::Def => self.parse_func_def(),
            TokenKind::Struct => self.parse_struct_def(),
            TokenKind::Enum => self.parse_enum_def(),
            TokenKind::Trait => self.parse_trait_def(),
            TokenKind::Impl => self.parse_impl_def(),
            TokenKind::Import => self.parse_import(),
            TokenKind::From => self.parse_import_from(),
            TokenKind::Return => {
                let pos = self.advance().pos;
                if self.check(&TokenKind::Newline) || self.check(&TokenKind::Dedent)
                    || self.check(&TokenKind::Eof)
                {
                    Ok(Stmt::Return { value: None, pos })
                } else {
                    let value = self.parse_expr()?;
                    Ok(Stmt::Return {
                        value: Some(value),
                        pos,
                    })
                }
            }
            TokenKind::Break => {
                let pos = self.advance().pos;
                Ok(Stmt::Break { pos })
            }
            TokenKind::Continue => {
                let pos = self.advance().pos;
                Ok(Stmt::Continue { pos })
            }
            TokenKind::Spawn => {
                let pos = self.advance().pos;
                let expr = self.parse_expr()?;
                Ok(Stmt::Spawn {
                    expr: Box::new(expr),
                    pos,
                })
            }
            TokenKind::Select => self.parse_select(),
            TokenKind::Fn => {
                // 匿名函数作为表达式语句
                let expr = self.parse_expr()?;
                let pos = expr_pos(&expr);
                Ok(Stmt::ExprStmt { expr, pos })
            }
            _ => self.parse_assign_or_expr(),
        }
    }

    /// 解析变量声明（let/var/const）
    fn parse_var_decl(&mut self, kind: VarKind) -> ParseResult<Stmt> {
        let kw = self.advance();
        let pos = kw.pos;
        // 解构 let (a, b) = pair
        if self.check(&TokenKind::LParen) {
            // 暂不支持解构（MVP），报友好错误
            return Err(self.err("E2001", "解构声明 let (a, b) = ... 尚未支持（v0.1 后续版本）"));
        }
        let name = self.expect_ident("变量名")?;
        // 类型注解
        let ty = if self.check(&TokenKind::Colon) {
            self.advance();
            Some(self.parse_type()?)
        } else {
            None
        };
        let value = if self.check(&TokenKind::Assign) {
            self.advance();
            Some(self.parse_expr()?)
        } else {
            None
        };
        Ok(Stmt::VarDecl {
            kind,
            name,
            ty,
            value,
            pos,
        })
    }

    /// 赋值语句或表达式语句
    fn parse_assign_or_expr(&mut self) -> ParseResult<Stmt> {
        let expr = self.parse_expr()?;
        let pos = expr_pos(&expr);
        // 赋值运算符
        let op = match self.peek_kind() {
            TokenKind::Assign => Some(AssignOp::Assign),
            TokenKind::PlusAssign => Some(AssignOp::Plus),
            TokenKind::MinusAssign => Some(AssignOp::Minus),
            TokenKind::StarAssign => Some(AssignOp::Star),
            TokenKind::SlashAssign => Some(AssignOp::Slash),
            TokenKind::IntDivAssign => Some(AssignOp::IntDiv),
            TokenKind::ModAssign => Some(AssignOp::Mod),
            TokenKind::PowAssign => Some(AssignOp::Pow),
            TokenKind::BitAndAssign => Some(AssignOp::BitAnd),
            TokenKind::BitOrAssign => Some(AssignOp::BitOr),
            TokenKind::BitXorAssign => Some(AssignOp::BitXor),
            TokenKind::ShlAssign => Some(AssignOp::Shl),
            TokenKind::ShrAssign => Some(AssignOp::Shr),
            _ => None,
        };
        if let Some(op) = op {
            self.advance();
            let value = self.parse_expr()?;
            Ok(Stmt::Assign {
                target: expr,
                op,
                value,
                pos,
            })
        } else {
            Ok(Stmt::ExprStmt { expr, pos })
        }
    }

    /// if / elif / else 语句
    fn parse_if(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // if
        let cond = self.parse_expr()?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        let body = self.parse_block()?;
        let mut branches = vec![(cond, body)];
        let mut else_branch = None;
        loop {
            if self.check(&TokenKind::Elif) {
                self.advance();
                let c = self.parse_expr()?;
                self.expect(TokenKind::Colon, "':'")?;
                self.expect(TokenKind::Newline, "换行")?;
                let b = self.parse_block()?;
                branches.push((c, b));
            } else if self.check(&TokenKind::Else) {
                self.advance();
                self.expect(TokenKind::Colon, "':'")?;
                self.expect(TokenKind::Newline, "换行")?;
                else_branch = Some(self.parse_block()?);
                break;
            } else {
                break;
            }
        }
        Ok(Stmt::If {
            branches,
            else_branch,
            pos,
        })
    }

    /// for 循环
    fn parse_for(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // for
        let var = self.expect_ident("循环变量")?;
        self.expect(TokenKind::In, "'in'")?;
        let iterable = self.parse_expr()?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        let body = self.parse_block()?;
        Ok(Stmt::For {
            var,
            iterable,
            body,
            pos,
        })
    }

    /// while 循环
    fn parse_while(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // while
        let cond = self.parse_expr()?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        let body = self.parse_block()?;
        Ok(Stmt::While { cond, body, pos })
    }

    /// 解析缩进块（调用前当前 token 为 Indent）
    fn parse_block(&mut self) -> ParseResult<Vec<Stmt>> {
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut stmts = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            let stmt = self.parse_stmt()?;
            stmts.push(stmt);
            // 不强制换行检查：块表达式（match/闭包）可能已消费 Dedent，
            // 之后直接是下一行语句。真正的语法错误留给语义检查阶段。
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "代码块未正确结束（缺少去缩进）"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(stmts)
    }

    /// def 函数定义
    fn parse_func_def(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // def
        let name = self.expect_ident("函数名")?;
        let params = self.parse_params()?;
        let ret_ty = if self.check(&TokenKind::Arrow) {
            self.advance();
            Some(self.parse_type()?)
        } else {
            None
        };
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        let body = self.parse_block()?;
        Ok(Stmt::FuncDef {
            name,
            params,
            ret_ty,
            body,
            pos,
        })
    }

    /// struct 定义
    fn parse_struct_def(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // struct
        let name = self.expect_ident("结构体名")?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut fields = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            let fpos = self.peek().pos;
            let fname = self.expect_ident("字段名")?;
            self.expect(TokenKind::Colon, "':'")?;
            let fty = self.parse_type()?;
            fields.push(StructField {
                name: fname,
                ty: fty,
                pos: fpos,
            });
            if !self.check(&TokenKind::Newline) && !self.check(&TokenKind::Dedent) {
                return Err(self.err("E2001", "结构体字段后期望换行"));
            }
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "结构体定义未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(Stmt::StructDef { name, fields, pos })
    }

    /// enum 定义
    fn parse_enum_def(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // enum
        let name = self.expect_ident("枚举名")?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut variants = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            let vpos = self.peek().pos;
            let vname = self.expect_ident("变体名")?;
            let mut fields = Vec::new();
            if self.check(&TokenKind::LParen) {
                self.advance();
                if !self.check(&TokenKind::RParen) {
                    loop {
                        fields.push(self.parse_type()?);
                        if self.check(&TokenKind::Comma) {
                            self.advance();
                            continue;
                        }
                        break;
                    }
                }
                self.expect(TokenKind::RParen, "')'")?;
            }
            variants.push(EnumVariant {
                name: vname,
                fields,
                pos: vpos,
            });
            if !self.check(&TokenKind::Newline) && !self.check(&TokenKind::Dedent) {
                return Err(self.err("E2001", "枚举变体后期望换行"));
            }
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "枚举定义未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(Stmt::EnumDef { name, variants, pos })
    }

    /// trait 定义
    fn parse_trait_def(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // trait
        let name = self.expect_ident("trait 名")?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut methods = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            let mpos = self.peek().pos;
            if !self.check(&TokenKind::Def) {
                return Err(self.err("E2001", "trait 内只允许 def 方法"));
            }
            self.advance(); // def
            let mname = self.expect_ident("方法名")?;
            let mparams = self.parse_params()?;
            let mret = if self.check(&TokenKind::Arrow) {
                self.advance();
                Some(self.parse_type()?)
            } else {
                None
            };
            self.expect(TokenKind::Colon, "':'")?;
            // 方法体可省略（trait 声明）或提供默认实现
            let body = if self.check(&TokenKind::Newline) {
                self.advance();
                if self.check(&TokenKind::Indent) {
                    self.parse_block()?
                } else {
                    Vec::new()
                }
            } else {
                Vec::new()
            };
            methods.push(Stmt::FuncDef {
                name: mname,
                params: mparams,
                ret_ty: mret,
                body,
                pos: mpos,
            });
            if !self.check(&TokenKind::Newline) && !self.check(&TokenKind::Dedent) {
                return Err(self.err("E2001", "trait 方法后期望换行"));
            }
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "trait 定义未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        // 把 FuncDef 语句转成 trait 方法签名（AST 复用 FuncDef）
        let methods = methods
            .into_iter()
            .filter_map(|m| match m {
                Stmt::FuncDef {
                    name,
                    params,
                    ret_ty,
                    body,
                    pos,
                } => Some(FuncDef {
                    name,
                    params,
                    ret_ty,
                    body,
                    pos,
                }),
                _ => None,
            })
            .collect();
        Ok(Stmt::TraitDef { name, methods, pos })
    }

    /// impl 定义（impl TypeName: 或 impl TraitName for TypeName:）
    fn parse_impl_def(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // impl
        let first = self.expect_ident("类型名或 trait 名")?;
        let (trait_name, type_name) = if self.check(&TokenKind::For) {
            self.advance();
            let ty = self.expect_ident("类型名")?;
            (Some(first), ty)
        } else {
            (None, first)
        };
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut methods = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            let mpos = self.peek().pos;
            if !self.check(&TokenKind::Def) {
                return Err(self.err("E2001", "impl 内只允许 def 方法"));
            }
            self.advance();
            let mname = self.expect_ident("方法名")?;
            let mparams = self.parse_params()?;
            let mret = if self.check(&TokenKind::Arrow) {
                self.advance();
                Some(self.parse_type()?)
            } else {
                None
            };
            self.expect(TokenKind::Colon, "':'")?;
            self.expect(TokenKind::Newline, "换行")?;
            let mbody = self.parse_block()?;
            methods.push(FuncDef {
                name: mname,
                params: mparams,
                ret_ty: mret,
                body: mbody,
                pos: mpos,
            });
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "impl 定义未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(Stmt::ImplDef {
            type_name,
            trait_name,
            methods,
            pos,
        })
    }

    /// import a.b.c
    /// 或 import "path/to/file.px"（相对路径导入，module 为单元素含 '/' 或 '.px' 后缀）
    fn parse_import(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // import
        // 字符串路径导入（M9）
        if let TokenKind::Str(path) = self.peek_kind() {
            let path = path.clone();
            self.advance();
            return Ok(Stmt::Import {
                module: vec![path],
                names: Vec::new(),
                pos,
            });
        }
        let mut module = Vec::new();
        loop {
            module.push(self.expect_ident("模块名")?);
            if self.check(&TokenKind::Dot) {
                self.advance();
                continue;
            }
            break;
        }
        Ok(Stmt::Import {
            module,
            names: Vec::new(),
            pos,
        })
    }

    /// from a.b import x, y
    fn parse_import_from(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // from
        let mut module = Vec::new();
        loop {
            module.push(self.expect_ident("模块名")?);
            if self.check(&TokenKind::Dot) {
                self.advance();
                continue;
            }
            break;
        }
        self.expect(TokenKind::Import, "'import'")?;
        let mut names = Vec::new();
        loop {
            names.push(self.expect_ident("导入名")?);
            if self.check(&TokenKind::Comma) {
                self.advance();
                continue;
            }
            break;
        }
        Ok(Stmt::Import { module, names, pos })
    }

    /// select 多路等待
    fn parse_select(&mut self) -> ParseResult<Stmt> {
        let pos = self.advance().pos; // select
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut arms = Vec::new();
        let mut else_branch = None;
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            self.expect(TokenKind::Case, "'case'")?;
            // 默认分支 case _:
            if matches!(self.peek_kind(), TokenKind::Ident(n) if n == "_") {
                // 仅当是裸 _ 且后面是冒号时视为默认分支
                let save = self.pos;
                self.advance();
                if self.check(&TokenKind::Colon) {
                    self.advance();
                    self.expect(TokenKind::Newline, "换行")?;
                    else_branch = Some(self.parse_case_body()?);
                    continue;
                } else {
                    self.pos = save;
                }
            }
            // case v = ch.recv(): 或 case expr:
            // 先看是否为 "标识符 = 表达式" 绑定形式
            let binding = if matches!(self.peek_kind(), TokenKind::Ident(_))
                && matches!(self.peek2_kind(), TokenKind::Assign)
            {
                let name = self.expect_ident("绑定变量")?;
                self.expect(TokenKind::Assign, "'='")?;
                Some(name)
            } else {
                None
            };
            let arm_expr = self.parse_expr()?;
            // select 分支暂不支持 guard（与 match 不同）
            let _guard: Option<Expr> = None;
            self.expect(TokenKind::Colon, "':'")?;
            self.expect(TokenKind::Newline, "换行")?;
            let body = self.parse_case_body()?;
            arms.push((binding, arm_expr, body));
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "select 定义未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(Stmt::Select {
            arms,
            else_branch,
            pos,
        })
    }

    /// select / match 分支体：单行表达式 或 缩进块
    fn parse_case_body(&mut self) -> ParseResult<Vec<Stmt>> {
        if self.check(&TokenKind::Indent) {
            self.parse_block()
        } else {
            // 单行表达式（到换行）
            let expr = self.parse_expr()?;
            let pos = expr_pos(&expr);
            Ok(vec![Stmt::ExprStmt { expr, pos }])
        }
    }

    // ==================== 参数 ====================

    fn parse_params(&mut self) -> ParseResult<Vec<Param>> {
        self.expect(TokenKind::LParen, "'('")?;
        let mut params = Vec::new();
        if !self.check(&TokenKind::RParen) {
            loop {
                let pos = self.peek().pos;
                let name = self.expect_name("参数名")?;
                let ty = if self.check(&TokenKind::Colon) {
                    self.advance();
                    Some(self.parse_type()?)
                } else {
                    None
                };
                let default = if self.check(&TokenKind::Assign) {
                    self.advance();
                    Some(self.parse_expr()?)
                } else {
                    None
                };
                params.push(Param {
                    name,
                    ty,
                    default,
                    pos,
                });
                if self.check(&TokenKind::Comma) {
                    self.advance();
                    continue;
                }
                break;
            }
        }
        self.expect(TokenKind::RParen, "')'")?;
        Ok(params)
    }

    // ==================== 表达式（Pratt 解析） ====================

    pub fn parse_expr(&mut self) -> ParseResult<Expr> {
        self.parse_pipe()
    }

    // |>（最低优先级）
    fn parse_pipe(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_null_coalesce()?;
        while self.check(&TokenKind::Pipe) {
            let pos = self.advance().pos;
            let right = self.parse_null_coalesce()?;
            left = Expr::Pipe {
                value: Box::new(left),
                func: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // ??
    fn parse_null_coalesce(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_or()?;
        while self.check(&TokenKind::NullCoalesce) {
            let pos = self.advance().pos;
            let right = self.parse_or()?;
            left = Expr::NullCoalesce {
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // or
    fn parse_or(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_and()?;
        while self.check(&TokenKind::Or) {
            let pos = self.advance().pos;
            let right = self.parse_and()?;
            left = Expr::Binary {
                op: BinaryOp::Or,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // and
    fn parse_and(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_comparison()?;
        while self.check(&TokenKind::And) {
            let pos = self.advance().pos;
            let right = self.parse_comparison()?;
            left = Expr::Binary {
                op: BinaryOp::And,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // 比较
    fn parse_comparison(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_bitor()?;
        loop {
            let op = match self.peek_kind() {
                TokenKind::Eq => BinaryOp::Eq,
                TokenKind::Ne => BinaryOp::Ne,
                TokenKind::Lt => BinaryOp::Lt,
                TokenKind::Le => BinaryOp::Le,
                TokenKind::Gt => BinaryOp::Gt,
                TokenKind::Ge => BinaryOp::Ge,
                _ => break,
            };
            let pos = self.advance().pos;
            let right = self.parse_bitor()?;
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // |
    fn parse_bitor(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_bitxor()?;
        while self.check(&TokenKind::BitOr) {
            let pos = self.advance().pos;
            let right = self.parse_bitxor()?;
            left = Expr::Binary {
                op: BinaryOp::BitOr,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // ^
    fn parse_bitxor(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_bitand()?;
        while self.check(&TokenKind::BitXor) {
            let pos = self.advance().pos;
            let right = self.parse_bitand()?;
            left = Expr::Binary {
                op: BinaryOp::BitXor,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // &
    fn parse_bitand(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_shift()?;
        while self.check(&TokenKind::BitAnd) {
            let pos = self.advance().pos;
            let right = self.parse_shift()?;
            left = Expr::Binary {
                op: BinaryOp::BitAnd,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // << >>
    fn parse_shift(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_add()?;
        loop {
            let op = match self.peek_kind() {
                TokenKind::Shl => BinaryOp::Shl,
                TokenKind::Shr => BinaryOp::Shr,
                _ => break,
            };
            let pos = self.advance().pos;
            let right = self.parse_add()?;
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // + -
    fn parse_add(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_mul()?;
        loop {
            let op = match self.peek_kind() {
                TokenKind::Plus => BinaryOp::Add,
                TokenKind::Minus => BinaryOp::Sub,
                _ => break,
            };
            let pos = self.advance().pos;
            let right = self.parse_mul()?;
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // * / // %
    fn parse_mul(&mut self) -> ParseResult<Expr> {
        let mut left = self.parse_pow()?;
        loop {
            let op = match self.peek_kind() {
                TokenKind::Star => BinaryOp::Mul,
                TokenKind::Slash => BinaryOp::Div,
                TokenKind::IntDiv => BinaryOp::IntDiv,
                TokenKind::Mod => BinaryOp::Mod,
                _ => break,
            };
            let pos = self.advance().pos;
            let right = self.parse_pow()?;
            left = Expr::Binary {
                op,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            };
        }
        Ok(left)
    }

    // **（右结合）
    fn parse_pow(&mut self) -> ParseResult<Expr> {
        let left = self.parse_unary()?;
        if self.check(&TokenKind::Pow) {
            let pos = self.advance().pos;
            let right = self.parse_pow()?;
            Ok(Expr::Binary {
                op: BinaryOp::Pow,
                left: Box::new(left),
                right: Box::new(right),
                pos,
            })
        } else {
            Ok(left)
        }
    }

    // 一元 - not ~
    fn parse_unary(&mut self) -> ParseResult<Expr> {
        let kind = self.peek_kind().clone();
        match kind {
            TokenKind::Minus => {
                let pos = self.advance().pos;
                let operand = self.parse_unary()?;
                Ok(Expr::Unary {
                    op: UnaryOp::Neg,
                    operand: Box::new(operand),
                    pos,
                })
            }
            TokenKind::Not => {
                let pos = self.advance().pos;
                let operand = self.parse_unary()?;
                Ok(Expr::Unary {
                    op: UnaryOp::Not,
                    operand: Box::new(operand),
                    pos,
                })
            }
            TokenKind::BitNot => {
                let pos = self.advance().pos;
                let operand = self.parse_unary()?;
                Ok(Expr::Unary {
                    op: UnaryOp::BitNot,
                    operand: Box::new(operand),
                    pos,
                })
            }
            _ => self.parse_postfix(),
        }
    }

    // 后缀：调用 下标 成员 可选链 强制解包 错误传播
    fn parse_postfix(&mut self) -> ParseResult<Expr> {
        let mut expr = self.parse_primary()?;
        loop {
            match self.peek_kind() {
                TokenKind::LParen => {
                    let pos = self.peek().pos;
                    let args = self.parse_call_args()?;
                    expr = Expr::Call {
                        callee: Box::new(expr),
                        args,
                        pos,
                    };
                }
                TokenKind::LBracket => {
                    let pos = self.advance().pos;
                    let index = self.parse_expr()?;
                    self.expect(TokenKind::RBracket, "']'")?;
                    expr = Expr::Index {
                        obj: Box::new(expr),
                        index: Box::new(index),
                        pos,
                    };
                }
                TokenKind::Dot => {
                    let pos = self.advance().pos;
                    let name = self.expect_name("成员名")?;
                    expr = Expr::Field {
                        obj: Box::new(expr),
                        name,
                        pos,
                    };
                }
                TokenKind::OptionalChain => {
                    let pos = self.advance().pos;
                    let name = self.expect_name("成员名")?;
                    expr = Expr::OptionalField {
                        obj: Box::new(expr),
                        name,
                        pos,
                    };
                }
                TokenKind::Bang => {
                    let pos = self.advance().pos;
                    expr = Expr::ForceUnwrap {
                        expr: Box::new(expr),
                        pos,
                    };
                }
                TokenKind::Question => {
                    let pos = self.advance().pos;
                    expr = Expr::Try {
                        expr: Box::new(expr),
                        pos,
                    };
                }
                _ => break,
            }
        }
        Ok(expr)
    }

    fn parse_call_args(&mut self) -> ParseResult<Vec<Expr>> {
        self.expect(TokenKind::LParen, "'('")?;
        let mut args = Vec::new();
        if !self.check(&TokenKind::RParen) {
            loop {
                args.push(self.parse_expr()?);
                if self.check(&TokenKind::Comma) {
                    self.advance();
                    continue;
                }
                break;
            }
        }
        self.expect(TokenKind::RParen, "')'")?;
        Ok(args)
    }

    // ==================== 原子表达式 ====================

    fn parse_primary(&mut self) -> ParseResult<Expr> {
        let kind = self.peek_kind().clone();
        match kind {
            TokenKind::Int(v) => {
                let pos = self.advance().pos;
                Ok(Expr::Int { value: v, pos })
            }
            TokenKind::Float(v) => {
                let pos = self.advance().pos;
                Ok(Expr::Float { value: v, pos })
            }
            TokenKind::Str(s) => {
                let pos = self.advance().pos;
                Ok(Expr::Str { value: s, pos })
            }
            TokenKind::True => {
                let pos = self.advance().pos;
                Ok(Expr::Bool { value: true, pos })
            }
            TokenKind::False => {
                let pos = self.advance().pos;
                Ok(Expr::Bool { value: false, pos })
            }
            TokenKind::Null => {
                let pos = self.advance().pos;
                Ok(Expr::Null { pos })
            }
            TokenKind::Self_ => {
                let pos = self.advance().pos;
                Ok(Expr::Var {
                    name: "self".into(),
                    pos,
                })
            }
            TokenKind::Ident(name) => {
                let pos = self.advance().pos;
                // 构造器/枚举变体调用：Ident(args) 已由 postfix 的 Call 处理
                Ok(Expr::Var { name, pos })
            }
            TokenKind::LBracket => self.parse_list_or_comp(),
            TokenKind::LParen => self.parse_paren_or_tuple(),
            TokenKind::LBrace => self.parse_brace(),
            TokenKind::Fn => self.parse_closure(),
            TokenKind::Match => self.parse_match_expr(),
            TokenKind::If => self.parse_if_expr(),
            TokenKind::Chan => {
                // channel 构造：chan[int](4) / chan[int]() / chan
                let pos = self.advance().pos;
                let mut callee = Expr::Var {
                    name: "chan".into(),
                    pos,
                };
                if self.check(&TokenKind::LBracket) {
                    self.advance();
                    let _ty = self.parse_type()?;
                    self.expect(TokenKind::RBracket, "']'")?;
                }
                if self.check(&TokenKind::LParen) {
                    let args = self.parse_call_args()?;
                    callee = Expr::Call {
                        callee: Box::new(callee),
                        args,
                        pos,
                    };
                }
                Ok(callee)
            }
            other => Err(self.err("E2001", format!("意外的 token: {}", other))),
        }
    }

    /// 列表字面量 或 列表推导
    fn parse_list_or_comp(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // [
        if self.check(&TokenKind::RBracket) {
            self.advance();
            return Ok(Expr::List { items: Vec::new(), pos });
        }
        let first = self.parse_expr()?;
        // 列表推导 [expr for x in xs if cond]
        if self.check(&TokenKind::For) {
            self.advance();
            let var = self.expect_ident("推导变量")?;
            self.expect(TokenKind::In, "'in'")?;
            let iterable = self.parse_expr()?;
            let cond = if self.check(&TokenKind::If) {
                self.advance();
                Some(Box::new(self.parse_expr()?))
            } else {
                None
            };
            self.expect(TokenKind::RBracket, "']'")?;
            return Ok(Expr::ListComp {
                expr: Box::new(first),
                var,
                iterable: Box::new(iterable),
                cond,
                pos,
            });
        }
        // 普通列表
        let mut items = vec![first];
        while self.check(&TokenKind::Comma) {
            self.advance();
            if self.check(&TokenKind::RBracket) {
                break;
            }
            items.push(self.parse_expr()?);
        }
        self.expect(TokenKind::RBracket, "']'")?;
        Ok(Expr::List { items, pos })
    }

    /// 括号表达式 或 元组
    fn parse_paren_or_tuple(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // (
        if self.check(&TokenKind::RParen) {
            // 空元组 ()
            self.advance();
            return Ok(Expr::Tuple { items: Vec::new(), pos });
        }
        let first = self.parse_expr()?;
        if self.check(&TokenKind::Comma) {
            // 元组
            let mut items = vec![first];
            while self.check(&TokenKind::Comma) {
                self.advance();
                if self.check(&TokenKind::RParen) {
                    break;
                }
                items.push(self.parse_expr()?);
            }
            self.expect(TokenKind::RParen, "')'")?;
            Ok(Expr::Tuple { items, pos })
        } else {
            self.expect(TokenKind::RParen, "')'")?;
            Ok(first)
        }
    }

    /// 花括号：字典字面量 或 块表达式
    fn parse_brace(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // {
        // 启发式判断字典：{ "key": ... 或 { ident: ...
        let is_dict = match self.peek_kind() {
            TokenKind::Str(_) | TokenKind::Ident(_) => {
                matches!(self.peek2_kind(), TokenKind::Colon)
            }
            _ => false,
        };
        if is_dict {
            let mut entries = Vec::new();
            if !self.check(&TokenKind::RBrace) {
                loop {
                    let k = self.parse_expr()?;
                    self.expect(TokenKind::Colon, "':'")?;
                    let v = self.parse_expr()?;
                    entries.push((k, v));
                    if self.check(&TokenKind::Comma) {
                        self.advance();
                        continue;
                    }
                    break;
                }
            }
            self.expect(TokenKind::RBrace, "'}'")?;
            Ok(Expr::Dict { entries, pos })
        } else {
            // 块表达式
            let mut stmts = Vec::new();
            self.skip_newlines();
            self.skip_brace_indents();
            while !self.check(&TokenKind::RBrace) && !self.check(&TokenKind::Eof) {
                stmts.push(self.parse_stmt()?);
                self.skip_newlines();
                self.skip_brace_indents();
            }
            self.expect(TokenKind::RBrace, "'}'")?;
            Ok(Expr::Block { stmts, pos })
        }
    }

    /// 匿名函数 fn(params) -> T { body } 或 fn(params): expr
    fn parse_closure(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // fn
        let params = self.parse_params()?;
        let ret_ty = if self.check(&TokenKind::Arrow) {
            self.advance();
            Some(self.parse_type()?)
        } else {
            None
        };
        // captures（显式捕获列表）
        let mut captures = Vec::new();
        if self.check(&TokenKind::Capture) {
            self.advance();
            loop {
                captures.push(self.expect_ident("捕获变量")?);
                if self.check(&TokenKind::Comma) {
                    self.advance();
                    continue;
                }
                break;
            }
        }
        let body = if self.check(&TokenKind::LBrace) {
            // 块体
            let bpos = self.advance().pos;
            let mut stmts = Vec::new();
            self.skip_newlines();
            self.skip_brace_indents();
            while !self.check(&TokenKind::RBrace) && !self.check(&TokenKind::Eof) {
                stmts.push(self.parse_stmt()?);
                self.skip_newlines();
                self.skip_brace_indents();
            }
            self.expect(TokenKind::RBrace, "'}'")?;
            Expr::Block { stmts, pos: bpos }
        } else if self.check(&TokenKind::Colon) {
            // 单行体 fn(x): x * 2
            self.advance();
            self.parse_expr()?
        } else {
            return Err(self.err("E2001", "匿名函数体期望 '{' 或 ':'"));
        };
        Ok(Expr::Closure {
            params,
            ret_ty,
            body: Box::new(body),
            captures,
            pos,
        })
    }

    /// match 表达式
    fn parse_match_expr(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // match
        let subject = self.parse_expr()?;
        self.expect(TokenKind::Colon, "':'")?;
        self.expect(TokenKind::Newline, "换行")?;
        self.expect(TokenKind::Indent, "缩进块")?;
        let mut arms = Vec::new();
        loop {
            self.skip_newlines_in_block();
            if self.check(&TokenKind::Dedent) || self.check(&TokenKind::Eof) {
                break;
            }
            self.expect(TokenKind::Case, "'case'")?;
            let pattern = self.parse_pattern()?;
            let guard = if self.check(&TokenKind::If) {
                self.advance();
                Some(self.parse_expr()?)
            } else {
                None
            };
            self.expect(TokenKind::Colon, "':'")?;
            self.expect(TokenKind::Newline, "换行")?;
            let body = if self.check(&TokenKind::Indent) {
                // 块体 → 包成 Block 表达式
                let stmts = self.parse_block()?;
                let bpos = stmts
                    .first()
                    .map(|s| stmt_pos(s))
                    .unwrap_or(pos);
                Expr::Block { stmts, pos: bpos }
            } else {
                self.parse_expr()?
            };
            arms.push(MatchArm {
                pattern,
                guard,
                body,
                pos,
            });
        }
        if self.check(&TokenKind::Eof) {
            return Err(self.err("E2001", "match 表达式未正确结束"));
        }
        self.expect(TokenKind::Dedent, "去缩进")?;
        Ok(Expr::Match {
            subject: Box::new(subject),
            arms,
            pos,
        })
    }

    /// if 表达式（if cond: expr else: expr）
    fn parse_if_expr(&mut self) -> ParseResult<Expr> {
        let pos = self.advance().pos; // if
        let cond = self.parse_expr()?;
        self.expect(TokenKind::Colon, "':'")?;
        let then = self.parse_expr()?;
        self.expect(TokenKind::Else, "'else'")?;
        self.expect(TokenKind::Colon, "':'")?;
        let else_ = self.parse_expr()?;
        Ok(Expr::IfExpr {
            cond: Box::new(cond),
            then: Box::new(then),
            else_: Box::new(else_),
            pos,
        })
    }

    /// 模式解析
    fn parse_pattern(&mut self) -> ParseResult<Pattern> {
        let kind = self.peek_kind().clone();
        match kind {
            TokenKind::Int(_) | TokenKind::Float(_) | TokenKind::Str(_)
            | TokenKind::True | TokenKind::False | TokenKind::Null => {
                let expr = self.parse_primary()?;
                Ok(Pattern::Literal(expr))
            }
            TokenKind::Ident(name) => {
                let _pos = self.peek().pos;
                self.advance();
                // 枚举/构造器模式 Some(x) / Point(a, b)
                if self.check(&TokenKind::LParen) {
                    self.advance();
                    let mut fields = Vec::new();
                    if !self.check(&TokenKind::RParen) {
                        loop {
                            fields.push(self.parse_pattern()?);
                            if self.check(&TokenKind::Comma) {
                                self.advance();
                                continue;
                            }
                            break;
                        }
                    }
                    self.expect(TokenKind::RParen, "')'")?;
                    Ok(Pattern::Constructor(name, fields))
                } else if name == "_" {
                    Ok(Pattern::Wildcard)
                } else if name
                    .chars()
                    .next()
                    .map(|c| c.is_uppercase())
                    .unwrap_or(false)
                {
                    // 大写开头标识符视为枚举变体/构造器（无载荷模式）
                    Ok(Pattern::Constructor(name, vec![]))
                } else {
                    Ok(Pattern::Binding(name))
                }
            }
            TokenKind::LParen => {
                // 元组模式
                let _ = self.advance();
                let mut items = Vec::new();
                if !self.check(&TokenKind::RParen) {
                    loop {
                        items.push(self.parse_pattern()?);
                        if self.check(&TokenKind::Comma) {
                            self.advance();
                            continue;
                        }
                        break;
                    }
                }
                self.expect(TokenKind::RParen, "')'")?;
                Ok(Pattern::Tuple(items))
            }
            other => Err(self.err("E2001", format!("无效的模式: {}", other))),
        }
    }

    // ==================== 类型表达式 ====================

    pub fn parse_type(&mut self) -> ParseResult<TypeExpr> {
        let ty = self.parse_type_base()?;
        // 可空类型 T?
        if self.check(&TokenKind::Question) {
            let pos = self.advance().pos;
            Ok(TypeExpr::Optional(Box::new(ty), pos))
        } else {
            Ok(ty)
        }
    }

    fn parse_type_base(&mut self) -> ParseResult<TypeExpr> {
        let kind = self.peek_kind().clone();
        match kind {
            TokenKind::Ident(name) => {
                let pos = self.advance().pos;
                // 泛型 Foo[T] 或 函数类型？
                if self.check(&TokenKind::LBracket) {
                    // 列表 [T] 或 泛型参数
                    self.advance();
                    let inner = self.parse_type()?;
                    self.expect(TokenKind::RBracket, "']'")?;
                    match name.as_str() {
                        "list" => Ok(TypeExpr::List(Box::new(inner), pos)),
                        _ => Ok(TypeExpr::Generic(name, vec![inner], pos)),
                    }
                } else {
                    Ok(TypeExpr::Named(name, pos))
                }
            }
            TokenKind::LBracket => {
                // [T] 列表类型
                let pos = self.advance().pos;
                let inner = self.parse_type()?;
                self.expect(TokenKind::RBracket, "']'")?;
                Ok(TypeExpr::List(Box::new(inner), pos))
            }
            TokenKind::LBrace => {
                // {K: V} 字典类型
                let pos = self.advance().pos;
                let k = self.parse_type()?;
                self.expect(TokenKind::Colon, "':'")?;
                let v = self.parse_type()?;
                self.expect(TokenKind::RBrace, "'}'")?;
                Ok(TypeExpr::Dict(Box::new(k), Box::new(v), pos))
            }
            TokenKind::LParen => {
                // 元组类型 (T1, T2) 或 函数类型 (T1, T2) -> R
                let pos = self.advance().pos;
                let mut items = Vec::new();
                if !self.check(&TokenKind::RParen) {
                    loop {
                        items.push(self.parse_type()?);
                        if self.check(&TokenKind::Comma) {
                            self.advance();
                            continue;
                        }
                        break;
                    }
                }
                self.expect(TokenKind::RParen, "')'")?;
                if self.check(&TokenKind::Arrow) {
                    self.advance();
                    let ret = self.parse_type()?;
                    Ok(TypeExpr::Func(items, Box::new(ret), pos))
                } else {
                    Ok(TypeExpr::Tuple(items, pos))
                }
            }
            other => Err(self.err("E2001", format!("无效的类型: {}", other))),
        }
    }
}

// ==================== 辅助函数 ====================

/// 是否为可作名字（成员名/参数名）的 token：标识符或关键字
fn is_name_token(kind: &TokenKind) -> bool {
    matches!(
        kind,
        TokenKind::Ident(_)
            | TokenKind::Let
            | TokenKind::Var
            | TokenKind::Const
            | TokenKind::Def
            | TokenKind::Fn
            | TokenKind::Struct
            | TokenKind::Enum
            | TokenKind::Trait
            | TokenKind::Impl
            | TokenKind::Match
            | TokenKind::Case
            | TokenKind::If
            | TokenKind::Elif
            | TokenKind::Else
            | TokenKind::For
            | TokenKind::While
            | TokenKind::In
            | TokenKind::Return
            | TokenKind::Break
            | TokenKind::Continue
            | TokenKind::Import
            | TokenKind::From
            | TokenKind::Pub
            | TokenKind::As
            | TokenKind::Spawn
            | TokenKind::Chan
            | TokenKind::Send
            | TokenKind::Recv
            | TokenKind::Select
            | TokenKind::True
            | TokenKind::False
            | TokenKind::Null
            | TokenKind::Self_
            | TokenKind::Type
            | TokenKind::Capture
    )
}


/// 取表达式位置
pub fn expr_pos(e: &Expr) -> Pos {
    match e {
        Expr::Int { pos, .. }
        | Expr::Float { pos, .. }
        | Expr::Str { pos, .. }
        | Expr::Bool { pos, .. }
        | Expr::Null { pos }
        | Expr::List { pos, .. }
        | Expr::Tuple { pos, .. }
        | Expr::Dict { pos, .. }
        | Expr::Var { pos, .. }
        | Expr::Field { pos, .. }
        | Expr::OptionalField { pos, .. }
        | Expr::Index { pos, .. }
        | Expr::Call { pos, .. }
        | Expr::Unary { pos, .. }
        | Expr::Binary { pos, .. }
        | Expr::Pipe { pos, .. }
        | Expr::NullCoalesce { pos, .. }
        | Expr::Try { pos, .. }
        | Expr::ForceUnwrap { pos, .. }
        | Expr::IfExpr { pos, .. }
        | Expr::ListComp { pos, .. }
        | Expr::Closure { pos, .. }
        | Expr::Block { pos, .. }
        | Expr::Match { pos, .. }
        | Expr::Constructor { pos, .. } => *pos,
    }
}

/// 取语句位置
pub fn stmt_pos(s: &Stmt) -> Pos {
    match s {
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

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;

    fn parse(src: &str) -> ParseResult<Program> {
        let tokens = Lexer::new(src).tokenize()?;
        Parser::new(tokens).parse_program()
    }

    fn parse_ok(src: &str) -> Program {
        parse(src).expect("应解析成功")
    }

    #[test]
    fn test_var_decl() {
        let p = parse_ok("let x = 42\nvar y: int = 1\nconst MAX = 100\n");
        assert_eq!(p.items.len(), 3);
        match &p.items[0] {
            Stmt::VarDecl { kind, name, value, .. } => {
                assert_eq!(kind, &VarKind::Let);
                assert_eq!(name, "x");
                assert!(value.is_some());
            }
            _ => panic!("期望 VarDecl"),
        }
    }

    #[test]
    fn test_if_elif_else() {
        let p = parse_ok("if a:\n    let x = 1\nelif b:\n    let y = 2\nelse:\n    let z = 3\n");
        match &p.items[0] {
            Stmt::If { branches, else_branch, .. } => {
                assert_eq!(branches.len(), 2);
                assert!(else_branch.is_some());
            }
            _ => panic!("期望 If"),
        }
    }

    #[test]
    fn test_for_while() {
        let p = parse_ok("for i in items:\n    print(i)\nwhile cond:\n    break\n");
        assert!(matches!(p.items[0], Stmt::For { .. }));
        assert!(matches!(p.items[1], Stmt::While { .. }));
    }

    #[test]
    fn test_func_def() {
        let p = parse_ok("def add(a: int, b: int = 1) -> int:\n    return a + b\n");
        match &p.items[0] {
            Stmt::FuncDef { name, params, ret_ty, body, .. } => {
                assert_eq!(name, "add");
                assert_eq!(params.len(), 2);
                assert!(ret_ty.is_some());
                assert_eq!(body.len(), 1);
            }
            _ => panic!("期望 FuncDef"),
        }
    }

    #[test]
    fn test_struct_enum() {
        let p = parse_ok(
            "struct Point:\n    x: int\n    y: int\n\nenum Color:\n    Red\n    Green\n",
        );
        assert!(matches!(p.items[0], Stmt::StructDef { .. }));
        assert!(matches!(p.items[1], Stmt::EnumDef { .. }));
    }

    #[test]
    fn test_match_expr() {
        let p = parse_ok(
            "let d = match c:\n    case Red:\n        \"warm\"\n    case _:\n        \"other\"\n",
        );
        match &p.items[0] {
            Stmt::VarDecl { value: Some(Expr::Match { arms, .. }), .. } => {
                assert_eq!(arms.len(), 2);
            }
            _ => panic!("期望 Match 表达式"),
        }
    }

    #[test]
    fn test_closure() {
        let p = parse_ok("let f = fn(x: int) -> int { x * 2 }\n");
        match &p.items[0] {
            Stmt::VarDecl { value: Some(Expr::Closure { params, .. }), .. } => {
                assert_eq!(params.len(), 1);
            }
            _ => panic!("期望 Closure"),
        }
    }

    #[test]
    fn test_concurrent() {
        let p = parse_ok(
            "ch = chan[int](4)\nspawn worker(1, ch)\nselect:\n    case v = ch.recv():\n        print(v)\n    case _:\n        print(\"empty\")\n",
        );
        assert!(matches!(p.items[0], Stmt::Assign { .. }));
        assert!(matches!(p.items[1], Stmt::Spawn { .. }));
        match &p.items[2] {
            Stmt::Select { arms, else_branch, .. } => {
                assert_eq!(arms.len(), 1);
                assert!(else_branch.is_some());
                let (binding, _, _) = &arms[0];
                assert_eq!(binding.as_deref(), Some("v"));
            }
            _ => panic!("期望 Select"),
        }
    }

    #[test]
    fn test_list_comp() {
        let p = parse_ok("let xs = [x * 2 for x in items if x > 1]\n");
        match &p.items[0] {
            Stmt::VarDecl { value: Some(Expr::ListComp { var, cond, .. }), .. } => {
                assert_eq!(var, "x");
                assert!(cond.is_some());
            }
            _ => panic!("期望 ListComp"),
        }
    }

    #[test]
    fn test_precedence() {
        // 2 + 3 * 4 应解析为 2 + (3*4)
        let p = parse_ok("let r = 2 + 3 * 4\n");
        match &p.items[0] {
            Stmt::VarDecl { value: Some(Expr::Binary { op: BinaryOp::Add, right, .. }), .. } => {
                assert!(matches!(**right, Expr::Binary { op: BinaryOp::Mul, .. }));
            }
            _ => panic!("期望 Add 顶层"),
        }
    }

    #[test]
    fn test_error_unclosed() {
        let r = parse("let x = (1\n");
        assert!(r.is_err());
    }

    #[test]
    fn test_error_bad_token() {
        let r = parse("let x = 1 @ 2\n");
        // @ 是非法字符，lexer 报错
        assert!(r.is_err());
    }
}
