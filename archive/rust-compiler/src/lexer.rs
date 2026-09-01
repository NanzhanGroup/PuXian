//! 普贤 (PuXian) 词法分析器
//! 实现 spec.md §2 词法规则：
//!   - 缩进块（禁止 tab，E1003）
//!   - 注释：单行 #、块注释 #| |#（可嵌套）、文档注释 ##（lexer 阶段跳过）
//!   - 字面量：整数（0x/0b/0o/_）、浮点（含指数）、字符串（' " 多行 """ 原始 r"）
//!   - 运算符最长匹配（含 ?. ?? |> //= **= <<= >>= 等）

use crate::token::{Pos, Token, TokenKind};

/// 词法错误（带 spec 错误码）
#[derive(Debug, Clone)]
pub struct LexError {
    pub code: &'static str,
    pub msg: String,
    pub pos: Pos,
}

impl std::fmt::Display for LexError {
    fn fmt(&self, f: &mut std::fmt::Formatter<'_>) -> std::fmt::Result {
        write!(f, "{}: 词法错误 {}: {}", self.pos, self.code, self.msg)
    }
}

pub type LexResult<T> = Result<T, LexError>;

/// 词法分析器
pub struct Lexer {
    src: Vec<char>,
    pos: usize,
    line: usize,
    col: usize,
    indent_stack: Vec<usize>,
    at_line_start: bool,
    tokens: Vec<Token>,
    /// 保留注释模式（px fmt 使用）：注释以 TokenKind::Comment 输出，不跳过
    pub preserve_comments: bool,
    /// 字符串插值展开产生的中间 token（Plus / str( / 表达式 / ) / Plus）
    /// 由 scan_string / scan_multiline_string 压入，next_token 依次取出
    pending: Vec<Token>,
}

/// 关键字表（spec §2.4）
fn keyword(kw: &str) -> Option<TokenKind> {
    use TokenKind::*;
    Some(match kw {
        "let" => Let,
        "var" => Var,
        "const" => Const,
        "def" => Def,
        "fn" => Fn,
        "struct" => Struct,
        "enum" => Enum,
        "trait" => Trait,
        "impl" => Impl,
        "match" => Match,
        "case" => Case,
        "if" => If,
        "elif" => Elif,
        "else" => Else,
        "for" => For,
        "while" => While,
        "in" => In,
        "and" => And,
        "or" => Or,
        "not" => Not,
        "return" => Return,
        "break" => Break,
        "continue" => Continue,
        "import" => Import,
        "from" => From,
        "pub" => Pub,
        "as" => As,
        "spawn" => Spawn,
        "chan" => Chan,
        "send" => Send,
        "recv" => Recv,
        "select" => Select,
        "true" => True,
        "false" => False,
        "null" => Null,
        "None" => Null, // 别名（spec §2.5）
        "self" => Self_,
        // "type" 不保留：type() 是内置函数（M5），作普通标识符
        "capture" => Capture,
        _ => return None,
    })
}

impl Lexer {
    pub fn new(src: &str) -> Self {
        Lexer {
            src: src.chars().collect(),
            pos: 0,
            line: 1,
            col: 1,
            indent_stack: vec![0],
            at_line_start: true,
            tokens: Vec::new(),
            preserve_comments: false,
            pending: Vec::new(),
        }
    }

    /// 保留注释模式（px fmt 用）：注释以 Comment token 输出
    pub fn new_with_comments(src: &str) -> Self {
        let mut lx = Self::new(src);
        lx.preserve_comments = true;
        lx
    }

    /// 将整个源码词法化为 Token 列表
    pub fn tokenize(mut self) -> LexResult<Vec<Token>> {
        loop {
            let tok = self.next_token()?;
            let done = matches!(tok.kind, TokenKind::Eof);
            self.tokens.push(tok);
            if done {
                break;
            }
        }
        Ok(self.tokens)
    }

    // ---- 基础字符操作 ----

    fn peek(&self) -> Option<char> {
        self.src.get(self.pos).copied()
    }

    fn peek2(&self) -> Option<char> {
        self.src.get(self.pos + 1).copied()
    }

    fn peek3(&self) -> Option<char> {
        self.src.get(self.pos + 2).copied()
    }

    fn advance(&mut self) -> Option<char> {
        let c = self.peek()?;
        self.pos += 1;
        if c == '\n' {
            self.line += 1;
            self.col = 1;
        } else {
            self.col += 1;
        }
        Some(c)
    }

    fn here(&self) -> Pos {
        Pos::new(self.line, self.col)
    }

    fn err(&self, code: &'static str, msg: impl Into<String>) -> LexError {
        LexError {
            code,
            msg: msg.into(),
            pos: self.here(),
        }
    }

    fn emit(&mut self, kind: TokenKind) -> Token {
        let tok = Token::new(kind, self.here());
        tok
    }

    // ---- 行首缩进处理 ----

    /// 处理一行的开头：统计缩进，产生 INDENT/DEDENT。
    /// 空行与纯注释行不产生缩进变化，循环跳过直到遇到真正的代码行。
    fn handle_line_start(&mut self) -> LexResult<()> {
        debug_assert!(self.at_line_start);
        loop {
            let mut indent = 0usize;
            // 统计行首空格
            while let Some(' ') = self.peek() {
                self.advance();
                indent += 1;
            }
            // tab 缩进 → E1003
            if let Some('\t') = self.peek() {
                return Err(self.err("E1003", "缩进禁止使用 tab，请使用空格"));
            }
            match self.peek() {
                None => return Ok(()), // EOF：交给主循环处理
                Some('\n') => {
                    // 空行：跳过
                    self.advance();
                    continue;
                }
                Some('#') => {
                    // 注释行：整行跳过（含块注释跨行）
                    let cpos = self.here();
                    let text = self.skip_comment()?;
                    if self.preserve_comments {
                        self.tokens.push(Token::new(TokenKind::Comment(text), cpos));
                        // 注释行后的换行（若存在）补一个 Newline，供 fmt 换行
                        if self.peek() == Some('\n') {
                            let p = self.here();
                            self.advance();
                            self.tokens.push(Token::new(TokenKind::Newline, p));
                        }
                    }
                    continue;
                }
                _ => {}
            }
            // 产生缩进变化
            let cur = *self.indent_stack.last().unwrap();
            if indent > cur {
                self.indent_stack.push(indent);
                let pos = self.here();
                self.tokens.push(Token::new(TokenKind::Indent, pos));
            } else if indent < cur {
                while *self.indent_stack.last().unwrap() > indent {
                    self.indent_stack.pop();
                    let pos = self.here();
                    self.tokens.push(Token::new(TokenKind::Dedent, pos));
                }
                if *self.indent_stack.last().unwrap() != indent {
                    return Err(self.err("E2002", format!("缩进不一致：当前缩进 {} 与上层缩进不匹配", indent)));
                }
            }
            self.at_line_start = false;
            return Ok(());
        }
    }

    /// 跳过注释并返回注释原文（含 `#`）。`#` 单行到行尾；`#| ... |#` 块注释可嵌套（可跨行）。
    fn skip_comment(&mut self) -> LexResult<String> {
        // 当前 peek 一定是 '#'
        let mut text = String::new();
        if self.peek2() == Some('|') {
            // 块注释，可嵌套
            text.push(self.advance().unwrap()); // #
            text.push(self.advance().unwrap()); // |
            let mut depth = 1usize;
            while depth > 0 {
                match self.peek() {
                    None => {
                        return Err(self.err("E1002", "块注释未闭合（缺少 |#）"));
                    }
                    Some('#') if self.peek2() == Some('|') => {
                        text.push(self.advance().unwrap());
                        text.push(self.advance().unwrap());
                        depth += 1;
                    }
                    Some('|') if self.peek2() == Some('#') => {
                        text.push(self.advance().unwrap());
                        text.push(self.advance().unwrap());
                        depth -= 1;
                    }
                    _ => {
                        text.push(self.advance().unwrap());
                    }
                }
            }
        } else {
            // 单行注释（含 ## 文档注释，lexer 阶段跳过，doc 收集在后续版本）
            while let Some(c) = self.peek() {
                if c == '\n' {
                    break;
                }
                text.push(self.advance().unwrap());
            }
        }
        Ok(text)
    }

    // ---- 主循环 ----

    fn next_token(&mut self) -> LexResult<Token> {
        // 字符串插值展开产生的中间 token 优先取出（不重新扫描源码）
        if !self.pending.is_empty() {
            return Ok(self.pending.remove(0));
        }
        if self.at_line_start {
            self.handle_line_start()?;
        }
        let pos = self.here();
        let c = match self.peek() {
            None => {
                // 文件结束：弹出所有缩进
                while self.indent_stack.len() > 1 {
                    self.indent_stack.pop();
                    let pos = self.here();
                    self.tokens.push(Token::new(TokenKind::Dedent, pos));
                }
                return Ok(self.emit(TokenKind::Eof));
            }
            Some(c) => c,
        };

        // 换行 → 语句结束
        if c == '\n' {
            self.advance();
            self.at_line_start = true;
            return Ok(Token::new(TokenKind::Newline, pos));
        }

        // 空白（非行首）
        if c == ' ' || c == '\t' {
            self.advance();
            return self.next_token();
        }

        // 注释（行中）
        if c == '#' {
            let text = self.skip_comment()?;
            if self.preserve_comments {
                return Ok(Token::new(TokenKind::Comment(text), pos));
            }
            return self.next_token();
        }

        // 字符串
        if c == '"' || c == '\'' {
            return self.scan_string(true);
        }

        // 数字
        if c.is_ascii_digit() {
            return self.scan_number();
        }

        // 标识符 / 关键字（支持 Unicode）
        if is_ident_start(c) {
            return self.scan_ident();
        }

        // 运算符与分隔符
        self.scan_operator()
    }

    // ---- 标识符 ----

    fn scan_ident(&mut self) -> LexResult<Token> {
        let pos = self.here();
        let mut s = String::new();
        while let Some(c) = self.peek() {
            if is_ident_continue(c) {
                s.push(c);
                self.advance();
            } else {
                break;
            }
        }
        let kind = keyword(&s).unwrap_or(TokenKind::Ident(s));
        Ok(Token::new(kind, pos))
    }

    // ---- 数字 ----

    fn scan_number(&mut self) -> LexResult<Token> {
        let pos = self.here();
        // 前缀进制
        if self.peek() == Some('0') {
            match self.peek2() {
                Some('x') | Some('X') => return self.scan_radix(16, pos),
                Some('b') | Some('B') => return self.scan_radix(2, pos),
                Some('o') | Some('O') => return self.scan_radix(8, pos),
                _ => {}
            }
        }
        let mut digits = String::new();
        while let Some(c) = self.peek() {
            if c.is_ascii_digit() || c == '_' {
                digits.push(c);
                self.advance();
            } else {
                break;
            }
        }
        // 浮点：小数点后跟数字，或指数
        let mut is_float = false;
        if self.peek() == Some('.') && self.peek2().map_or(false, |c| c.is_ascii_digit()) {
            is_float = true;
            digits.push('.');
            self.advance();
            while let Some(c) = self.peek() {
                if c.is_ascii_digit() || c == '_' {
                    digits.push(c);
                    self.advance();
                } else {
                    break;
                }
            }
        }
        if let Some('e') | Some('E') = self.peek() {
            // 指数（后面须有数字或 +/- 数字）
            let save = (self.pos, self.line, self.col);
            let mut exp = String::new();
            exp.push(self.advance().unwrap());
            if let Some('+') | Some('-') = self.peek() {
                exp.push(self.advance().unwrap());
            }
            if self.peek().map_or(false, |c| c.is_ascii_digit()) {
                while let Some(c) = self.peek() {
                    if c.is_ascii_digit() || c == '_' {
                        exp.push(c);
                        self.advance();
                    } else {
                        break;
                    }
                }
                digits.push_str(&exp);
                is_float = true;
            } else {
                // 不是指数（如 1e 后面没有数字），回退
                self.pos = save.0;
                self.line = save.1;
                self.col = save.2;
            }
        }
        let clean: String = digits.chars().filter(|c| *c != '_').collect();
        if is_float {
            match clean.parse::<f64>() {
                Ok(v) => Ok(Token::new(TokenKind::Float(v), pos)),
                Err(_) => Err(self.err("E1004", format!("无效浮点数: {}", digits))),
            }
        } else {
            match clean.parse::<i64>() {
                Ok(v) => Ok(Token::new(TokenKind::Int(v), pos)),
                Err(_) => Err(self.err("E1004", format!("无效整数: {}", digits))),
            }
        }
    }

    fn scan_radix(&mut self, radix: u32, pos: Pos) -> LexResult<Token> {
        self.advance(); // 0
        self.advance(); // x/b/o
        let mut digits = String::new();
        while let Some(c) = self.peek() {
            if c.is_ascii_alphanumeric() || c == '_' {
                digits.push(c);
                self.advance();
            } else {
                break;
            }
        }
        if digits.is_empty() {
            return Err(self.err("E1004", format!("进制字面量缺少数字（基数 {}）", radix)));
        }
        let clean: String = digits.chars().filter(|c| *c != '_').collect();
        match i64::from_str_radix(&clean, radix) {
            Ok(v) => Ok(Token::new(TokenKind::Int(v), pos)),
            Err(_) => Err(self.err("E1004", format!("无效 {}-进制字面量: {}", radix, clean))),
        }
    }

    // ---- 字符串 ----

    /// 扫描普通字符串（interp=true 时支持 ${expr} 插值，展开为 str(expr) 拼接）
    /// 返回本字符串产生的第一个 token；其余中间 token 压入 self.pending 队列
    /// 扫描普通字符串（interp=true 时支持 ${expr} 插值，展开为 str(expr) 拼接）
    /// 返回本字符串产生的第一个 token；其余中间 token 压入 self.pending 队列
    fn scan_string(&mut self, interp: bool) -> LexResult<Token> {
        let toks = self.scan_string_tokens(interp)?;
        let first = toks[0].clone();
        self.pending.extend(toks.into_iter().skip(1));
        Ok(first)
    }

    /// 扫描普通字符串，返回完整 token 列表（含插值展开；供嵌套场景直接收集）
    fn scan_string_tokens(&mut self, interp: bool) -> LexResult<Vec<Token>> {
        let pos = self.here();
        let quote = self.advance().unwrap();
        // 多行字符串 """ 或 '''
        if quote == '"' && self.peek() == Some('"') && self.peek2() == Some('"') {
            self.advance();
            self.advance();
            return self.scan_multiline_string_tokens('"', pos, interp);
        }
        if quote == '\'' && self.peek() == Some('\'') && self.peek2() == Some('\'') {
            self.advance();
            self.advance();
            return self.scan_multiline_string_tokens('\'', pos, interp);
        }
        // 普通字符串（单行）
        let mut out: Vec<Token> = Vec::new();
        let mut s = String::new();
        loop {
            match self.peek() {
                None => {
                    return Err(LexError {
                        code: "E1002",
                        msg: format!("字符串未闭合（缺少 {}）", quote),
                        pos,
                    })
                }
                Some(c) if c == quote => {
                    self.advance();
                    break;
                }
                Some('\\') => {
                    let esc = self.scan_escape(pos)?;
                    s.push_str(&esc);
                }
                Some('\n') => {
                    return Err(LexError {
                        code: "E1002",
                        msg: "单行字符串不能跨行，请使用 \"\"\" 多行字符串".to_string(),
                        pos,
                    })
                }
                Some('$') if interp && self.peek2() == Some('{') => {
                    // 插值开始：flush 文本段 → + str( → 表达式 → ) → +
                    self.advance(); // $
                    self.advance(); // {
                    out.push(Token::new(TokenKind::Str(std::mem::take(&mut s)), pos));
                    out.push(Token::new(TokenKind::Plus, pos));
                    out.push(Token::new(TokenKind::Ident("str".to_string()), pos));
                    out.push(Token::new(TokenKind::LParen, pos));
                    let expr = self.scan_interp_expr(pos)?;
                    out.extend(expr);
                    out.push(Token::new(TokenKind::RParen, pos));
                    out.push(Token::new(TokenKind::Plus, pos));
                }
                Some(c) => {
                    s.push(c);
                    self.advance();
                }
            }
        }
        out.push(Token::new(TokenKind::Str(s), pos));
        Ok(out)
    }

    /// 扫描多行字符串（interp=true 时支持 ${expr} 插值），返回完整 token 列表
    fn scan_multiline_string_tokens(&mut self, quote: char, pos: Pos, interp: bool) -> LexResult<Vec<Token>> {
        let mut out: Vec<Token> = Vec::new();
        let mut s = String::new();
        loop {
            match self.peek() {
                None => {
                    return Err(LexError {
                        code: "E1002",
                        msg: "多行字符串未闭合".to_string(),
                        pos,
                    })
                }
                Some(c) if c == quote && self.peek2() == Some(quote) && self.peek3() == Some(quote) => {
                    self.advance();
                    self.advance();
                    self.advance();
                    break;
                }
                Some('\\') => {
                    let esc = self.scan_escape(pos)?;
                    s.push_str(&esc);
                }
                Some('$') if interp && self.peek2() == Some('{') => {
                    self.advance(); // $
                    self.advance(); // {
                    out.push(Token::new(TokenKind::Str(std::mem::take(&mut s)), pos));
                    out.push(Token::new(TokenKind::Plus, pos));
                    out.push(Token::new(TokenKind::Ident("str".to_string()), pos));
                    out.push(Token::new(TokenKind::LParen, pos));
                    let expr = self.scan_interp_expr(pos)?;
                    out.extend(expr);
                    out.push(Token::new(TokenKind::RParen, pos));
                    out.push(Token::new(TokenKind::Plus, pos));
                }
                Some(c) => {
                    s.push(c);
                    self.advance();
                }
            }
        }
        out.push(Token::new(TokenKind::Str(s), pos));
        Ok(out)
    }

    /// 扫描字符串插值 ${...} 内的表达式 token 序列（消费到匹配的 }）
    /// 限定：不支持 {} 字面量（dict/set）、注释、跨行；表达式内字符串字面量支持嵌套插值
    fn scan_interp_expr(&mut self, str_pos: Pos) -> LexResult<Vec<Token>> {
        let mut toks: Vec<Token> = Vec::new();
        loop {
            match self.peek() {
                None => {
                    return Err(LexError {
                        code: "E1002",
                        msg: "字符串插值 ${ 未闭合（缺少 }）".to_string(),
                        pos: str_pos,
                    })
                }
                Some('}') => {
                    self.advance();
                    break;
                }
                Some('{') => {
                    return Err(self.err(
                        "E1006",
                        "插值表达式不支持 {} 字面量（dict/set），请先用变量保存",
                    ))
                }
                Some('\n') => {
                    return Err(LexError {
                        code: "E1002",
                        msg: "字符串插值表达式不能跨行".to_string(),
                        pos: str_pos,
                    })
                }
                Some('#') => {
                    return Err(self.err("E1006", "插值表达式不支持注释"))
                }
                Some(c) if c == ' ' || c == '\t' => {
                    self.advance();
                }
                Some(c) if c == '"' || c == '\'' => {
                    // 表达式内的字符串字面量：支持嵌套插值（递归展开）
                    let t = self.scan_string_tokens(true)?;
                    toks.extend(t);
                }
                Some(c) if c.is_ascii_digit() => {
                    let t = self.scan_number()?;
                    toks.push(t);
                }
                Some(c) if is_ident_start(c) => {
                    let t = self.scan_ident()?;
                    toks.push(t);
                }
                Some(_) => {
                    let t = self.scan_operator()?;
                    toks.push(t);
                }
            }
        }
        Ok(toks)
    }

    /// 解析转义序列，返回解码后的字符串
    fn scan_escape(&mut self, str_pos: Pos) -> LexResult<String> {
        self.advance(); // 反斜杠
        let c = match self.peek() {
            None => {
                return Err(LexError {
                    code: "E1002",
                    msg: "字符串在转义序列处意外结束".to_string(),
                    pos: str_pos,
                })
            }
            Some(c) => c,
        };
        let out = match c {
            'n' => "\n",
            't' => "\t",
            'r' => "\r",
            '\\' => "\\",
            '"' => "\"",
            '\'' => "'",
            '0' => "\0",
            '$' => "$", // \${ 转义为字面 ${（字符串插值用）
            'u' => {
                // \u{XXXX}
                self.advance(); // u
                if self.peek() != Some('{') {
                    return Err(self.err("E1005", "Unicode 转义须为 \\u{XXXX} 形式"));
                }
                self.advance(); // {
                let mut hex = String::new();
                while let Some(h) = self.peek() {
                    if h == '}' {
                        break;
                    }
                    if h.is_ascii_hexdigit() {
                        hex.push(h);
                        self.advance();
                    } else {
                        return Err(self.err("E1005", "Unicode 转义含非法字符"));
                    }
                }
                if self.peek() != Some('}') {
                    return Err(self.err("E1005", "Unicode 转义缺少 }"));
                }
                self.advance(); // }
                let cp = u32::from_str_radix(&hex, 16).map_err(|_| self.err("E1005", "Unicode 转义无效"))?;
                return Ok(char::from_u32(cp)
                    .map(|ch| ch.to_string())
                    .unwrap_or_else(|| "\u{FFFD}".to_string()));
            }
            _ => {
                return Err(self.err("E1005", format!("非法转义序列 \\{}", c)));
            }
        };
        self.advance();
        Ok(out.to_string())
    }

    // ---- 运算符与分隔符 ----

    fn scan_operator(&mut self) -> LexResult<Token> {
        use TokenKind::*;
        let pos = self.here();
        let c = self.advance().unwrap();
        let two = |l: &mut Lexer, second: char, yes: TokenKind, no: TokenKind| -> Token {
            if l.peek() == Some(second) {
                l.advance();
                Token::new(yes, pos)
            } else {
                Token::new(no, pos)
            }
        };
        let tok = match c {
            '(' => Token::new(LParen, pos),
            ')' => Token::new(RParen, pos),
            '[' => Token::new(LBracket, pos),
            ']' => Token::new(RBracket, pos),
            '{' => Token::new(LBrace, pos),
            '}' => Token::new(RBrace, pos),
            ',' => Token::new(Comma, pos),
            ':' => Token::new(Colon, pos),
            '.' => {
                if self.peek() == Some('.') {
                    // .. 暂未定义（未来 range），先报错
                    return Err(self.err("E1001", "运算符 '..' 未定义（range 语法尚未支持）"));
                }
                Token::new(Dot, pos)
            }
            '+' => two(self, '=', PlusAssign, Plus),
            '-' => {
                if self.peek() == Some('>') {
                    self.advance();
                    Token::new(Arrow, pos)
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(MinusAssign, pos)
                } else {
                    Token::new(Minus, pos)
                }
            }
            '*' => {
                if self.peek() == Some('*') {
                    self.advance();
                    if self.peek() == Some('=') {
                        self.advance();
                        Token::new(PowAssign, pos)
                    } else {
                        Token::new(Pow, pos)
                    }
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(StarAssign, pos)
                } else {
                    Token::new(Star, pos)
                }
            }
            '/' => {
                if self.peek() == Some('/') {
                    self.advance();
                    if self.peek() == Some('=') {
                        self.advance();
                        Token::new(IntDivAssign, pos)
                    } else {
                        Token::new(IntDiv, pos)
                    }
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(SlashAssign, pos)
                } else {
                    Token::new(Slash, pos)
                }
            }
            '%' => two(self, '=', ModAssign, Mod),
            '^' => two(self, '=', BitXorAssign, BitXor),
            '~' => Token::new(BitNot, pos),
            '&' => two(self, '=', BitAndAssign, BitAnd),
            '|' => {
                if self.peek() == Some('>') {
                    self.advance();
                    Token::new(Pipe, pos)
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(BitOrAssign, pos)
                } else {
                    Token::new(BitOr, pos)
                }
            }
            '=' => {
                if self.peek() == Some('=') {
                    self.advance();
                    Token::new(Eq, pos)
                } else {
                    Token::new(Assign, pos)
                }
            }
            '!' => {
                if self.peek() == Some('=') {
                    self.advance();
                    Token::new(Ne, pos)
                } else {
                    Token::new(Bang, pos)
                }
            }
            '<' => {
                if self.peek() == Some('<') {
                    self.advance();
                    if self.peek() == Some('=') {
                        self.advance();
                        Token::new(ShlAssign, pos)
                    } else {
                        Token::new(Shl, pos)
                    }
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(Le, pos)
                } else {
                    Token::new(Lt, pos)
                }
            }
            '>' => {
                if self.peek() == Some('>') {
                    self.advance();
                    if self.peek() == Some('>') {
                        // >>>（无符号右移）或 >>>=
                        self.advance();
                        if self.peek() == Some('=') {
                            self.advance();
                            Token::new(ShrUAssign, pos)
                        } else {
                            Token::new(ShrU, pos)
                        }
                    } else if self.peek() == Some('=') {
                        self.advance();
                        Token::new(ShrAssign, pos)
                    } else {
                        Token::new(Shr, pos)
                    }
                } else if self.peek() == Some('=') {
                    self.advance();
                    Token::new(Ge, pos)
                } else {
                    Token::new(Gt, pos)
                }
            }
            '?' => {
                if self.peek() == Some('.') {
                    self.advance();
                    Token::new(OptionalChain, pos)
                } else if self.peek() == Some('?') {
                    self.advance();
                    Token::new(NullCoalesce, pos)
                } else {
                    Token::new(Question, pos)
                }
            }
            other => {
                return Err(self.err("E1001", format!("非法字符: {:?}", other)));
            }
        };
        Ok(tok)
    }
}

// ---- 字符分类 ----

fn is_ident_start(c: char) -> bool {
    c.is_alphabetic() || c == '_'
}

fn is_ident_continue(c: char) -> bool {
    c.is_alphanumeric() || c == '_'
}

#[cfg(test)]
mod tests {
    use super::*;

    fn toks(src: &str) -> Vec<TokenKind> {
        Lexer::new(src).tokenize().unwrap().into_iter().map(|t| t.kind).collect()
    }

    #[test]
    fn test_basic() {
        let t = toks("let x = 42\n");
        assert!(t.contains(&TokenKind::Let));
        assert!(t.contains(&TokenKind::Ident("x".into())));
        assert!(t.contains(&TokenKind::Int(42)));
    }

    #[test]
    fn test_indent() {
        let t = toks("if true\n    let a = 1\nlet b = 2\n");
        // if true NEWLINE INDENT let ... NEWLINE DEDENT let ...
        let mut idx = 0;
        assert_eq!(t[idx], TokenKind::If); idx += 1;
        assert_eq!(t[idx], TokenKind::True); idx += 1;
        assert_eq!(t[idx], TokenKind::Newline); idx += 1;
        assert_eq!(t[idx], TokenKind::Indent); idx += 1;
        assert_eq!(t[idx], TokenKind::Let); idx += 1;
        assert_eq!(t[idx], TokenKind::Ident("a".into())); idx += 1;
        assert_eq!(t[idx], TokenKind::Assign); idx += 1;
        assert_eq!(t[idx], TokenKind::Int(1)); idx += 1;
        assert_eq!(t[idx], TokenKind::Newline); idx += 1;
        assert_eq!(t[idx], TokenKind::Dedent); idx += 1;
        assert_eq!(t[idx], TokenKind::Let); idx += 1;
        let _ = idx;
    }

    #[test]
    fn test_number() {
        let t = toks("0xFF 0b1010 0o17 1_000_000 3.14 1e10");
        assert!(t.contains(&TokenKind::Int(255)));
        assert!(t.contains(&TokenKind::Int(10)));
        assert!(t.contains(&TokenKind::Int(15)));
        assert!(t.contains(&TokenKind::Int(1_000_000)));
        assert!(t.contains(&TokenKind::Float(3.14)));
        assert!(t.contains(&TokenKind::Float(1e10)));
    }

    #[test]
    fn test_string() {
        let t = toks("\"hi\\n\" 'a' \"\"\"multi\nline\"\"\"");
        assert!(t.contains(&TokenKind::Str("hi\n".into())));
        assert!(t.contains(&TokenKind::Str("a".into())));
        assert!(t.contains(&TokenKind::Str("multi\nline".into())));
    }

    #[test]
    fn test_string_interp() {
        // "a${x}b" → Str("a") + str( x ) + Str("b")
        let t = toks("\"a${x}b\"");
        assert_eq!(t[0], TokenKind::Str("a".into()));
        assert_eq!(t[1], TokenKind::Plus);
        assert_eq!(t[2], TokenKind::Ident("str".into()));
        assert_eq!(t[3], TokenKind::LParen);
        assert_eq!(t[4], TokenKind::Ident("x".into()));
        assert_eq!(t[5], TokenKind::RParen);
        assert_eq!(t[6], TokenKind::Plus);
        assert_eq!(t[7], TokenKind::Str("b".into()));
    }

    #[test]
    fn test_string_interp_expr() {
        // "${1 + 2}" → Str("") + str( 1 + 2 ) + Str("")（末尾含 Eof）
        let t = toks("\"${1 + 2}\"");
        assert_eq!(t[0], TokenKind::Str("".into()));
        assert_eq!(t[2], TokenKind::Ident("str".into()));
        assert_eq!(t[4], TokenKind::Int(1));
        assert_eq!(t[5], TokenKind::Plus);
        assert_eq!(t[6], TokenKind::Int(2));
        assert_eq!(t[8], TokenKind::Plus);
        assert_eq!(t[9], TokenKind::Str("".into()));
    }

    #[test]
    fn test_string_interp_nested() {
        // "${"a${i}b"}" → 表达式内字符串字面量递归展开为 "a" + str(i) + "b"
        let t = toks("\"${\"a${i}b\"}\"");
        assert_eq!(t[0], TokenKind::Str("".into()));   // 外层空前缀
        assert_eq!(t[2], TokenKind::Ident("str".into()));
        assert_eq!(t[4], TokenKind::Str("a".into()));  // 内层第一文本段
        assert_eq!(t[8], TokenKind::Ident("i".into()));
        assert_eq!(t[11], TokenKind::Str("b".into()));
        assert_eq!(t[14], TokenKind::Str("".into()));  // 外层空后缀
    }

    #[test]
    fn test_string_interp_escape() {
        // "\${x}" → 字面 ${x}（\$ 转义），无插值展开（t[0]=Str，t[1]=Eof）
        let t = toks("\"\\${x}\"");
        assert_eq!(t[0], TokenKind::Str("${x}".into()));
        assert_eq!(t.len(), 2);
        // 裸 $（非 { 前缀）原样
        let t2 = toks("\"price $5\"");
        assert_eq!(t2[0], TokenKind::Str("price $5".into()));
    }

    #[test]
    fn test_string_interp_multiline() {
        let t = toks("\"\"\"a${x}b\"\"\"");
        assert_eq!(t[0], TokenKind::Str("a".into()));
        assert_eq!(t[1], TokenKind::Plus);
        assert_eq!(t[4], TokenKind::Ident("x".into()));
        assert_eq!(t[7], TokenKind::Str("b".into()));
    }

    #[test]
    fn test_string_interp_errors() {
        // 未闭合
        assert!(Lexer::new("print(\"${x\")").tokenize().is_err());
        // 跨行
        assert!(Lexer::new("print(\"${x\ny}\")").tokenize().is_err());
        // {} 字面量
        let r = Lexer::new("print(\"${ {1:2} }\")").tokenize();
        assert!(r.is_err());
        assert_eq!(r.unwrap_err().code, "E1006");
        // 注释
        let r = Lexer::new("print(\"${x #c}\")").tokenize();
        assert!(r.is_err());
        assert_eq!(r.unwrap_err().code, "E1006");
    }

    #[test]
    fn test_ops() {
        let t = toks("a |> b ?. c ?? d a // b a ** b <<= >>=");
        assert!(t.contains(&TokenKind::Pipe));
        assert!(t.contains(&TokenKind::OptionalChain));
        assert!(t.contains(&TokenKind::NullCoalesce));
        assert!(t.contains(&TokenKind::IntDiv));
        assert!(t.contains(&TokenKind::Pow));
        assert!(t.contains(&TokenKind::ShlAssign));
        assert!(t.contains(&TokenKind::ShrAssign));
    }

    #[test]
    fn test_comment() {
        let t = toks("# comment\nlet a = 1 # inline\n#| block\nnested #| inner |#\nend |#\nlet b = 2\n");
        assert_eq!(t.iter().filter(|k| **k == TokenKind::Let).count(), 2);
        // 注释不应产生多余 Newline / Indent
        assert_eq!(t.iter().filter(|k| **k == TokenKind::Indent).count(), 0);
    }

    #[test]
    fn test_tab_error() {
        let r = Lexer::new("\tlet x = 1\n").tokenize();
        assert!(r.is_err());
        assert_eq!(r.unwrap_err().code, "E1003");
    }

    #[test]
    fn test_bad_indent() {
        let r = Lexer::new("if true\n    let a = 1\n  let b = 2\n").tokenize();
        assert!(r.is_err());
        assert_eq!(r.unwrap_err().code, "E2002");
    }
}
