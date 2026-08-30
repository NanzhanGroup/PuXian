//! 普贤 (PuXian) 词法单元定义
//! 对应 spec.md §2 词法

use std::fmt;

/// Token 在源码中的位置
#[derive(Debug, Clone, Copy, PartialEq, Eq)]
pub struct Pos {
    pub line: usize,
    pub col: usize,
}

impl Pos {
    pub fn new(line: usize, col: usize) -> Self {
        Pos { line, col }
    }
}

impl fmt::Display for Pos {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        write!(f, "{}:{}", self.line, self.col)
    }
}

/// Token 种类
#[derive(Debug, Clone, PartialEq)]
pub enum TokenKind {
    // ---- 字面量 ----
    Int(i64),
    Float(f64),
    Str(String),
    Ident(String),

    // ---- 关键字（spec §2.4）----
    Let,
    Var,
    Const,
    Def,
    Fn,
    Struct,
    Enum,
    Trait,
    Impl,
    Match,
    Case,
    If,
    Elif,
    Else,
    For,
    While,
    In,
    Return,
    Break,
    Continue,
    Import,
    From,
    Pub,
    As,
    Spawn,
    Chan,
    Send,
    Recv,
    Select,
    True,
    False,
    Null,
    Self_,
    Type,
    Capture,

    // ---- 运算符（spec §2.6）----
    Plus,        // +
    Minus,       // -
    Star,        // *
    Slash,       // /
    IntDiv,      // //
    Mod,         // %
    Pow,         // **
    Eq,          // ==
    Ne,          // !=
    Lt,          // <
    Le,          // <=
    Gt,          // >
    Ge,          // >=
    And,         // and
    Or,          // or
    Not,         // not
    BitAnd,      // &
    BitOr,       // |
    BitXor,      // ^
    Shl,         // <<
    Shr,         // >>
    BitNot,      // ~
    Assign,      // =
    PlusAssign,  // +=
    MinusAssign, // -=
    StarAssign,  // *=
    SlashAssign, // /=
    IntDivAssign,// //=
    ModAssign,   // %=
    PowAssign,   // **=
    BitAndAssign,// &=
    BitOrAssign, // |=
    BitXorAssign,// ^=
    ShlAssign,   // <<=
    ShrAssign,   // >>=
    Pipe,        // |>
    OptionalChain, // ?.
    NullCoalesce,  // ??
    Question,      // ?（错误传播 / 可空标记）
    Colon,         // :
    Arrow,         // ->
    Dot,           // .
    Bang,          // !（强制解包）

    // ---- 分隔符（spec §2.7）----
    LParen,   // (
    RParen,   // )
    LBracket, // [
    RBracket, // ]
    LBrace,   // {
    RBrace,   // }
    Comma,    // ,

    // ---- 结构 ----
    Newline, // 换行（语句结束）
    Indent,  // 缩进增加
    Dedent,  // 缩进减少
    /// 注释（仅保留注释模式下产生，供 fmt 使用；含原文，块注释可含换行）
    Comment(String),
    Eof,
}

/// 带位置的 Token
#[derive(Debug, Clone, PartialEq)]
pub struct Token {
    pub kind: TokenKind,
    pub pos: Pos,
}

impl Token {
    pub fn new(kind: TokenKind, pos: Pos) -> Self {
        Token { kind, pos }
    }
}

impl fmt::Display for TokenKind {
    fn fmt(&self, f: &mut fmt::Formatter<'_>) -> fmt::Result {
        use TokenKind::*;
        let s = match self {
            Int(v) => return write!(f, "整数 {}", v),
            Float(v) => return write!(f, "浮点 {}", v),
            Str(v) => return write!(f, "字符串 {:?}", v),
            Ident(v) => return write!(f, "标识符 {}", v),
            Let => "let", Var => "var", Const => "const", Def => "def", Fn => "fn",
            Struct => "struct", Enum => "enum", Trait => "trait", Impl => "impl",
            Match => "match", Case => "case", If => "if", Elif => "elif", Else => "else",
            For => "for", While => "while", In => "in", Return => "return",
            Break => "break", Continue => "continue", Import => "import", From => "from",
            Pub => "pub", As => "as", Spawn => "spawn", Chan => "chan", Send => "send",
            Recv => "recv", Select => "select", True => "true", False => "false",
            Null => "null", Self_ => "self", Type => "type", Capture => "capture",
            Plus => "+", Minus => "-", Star => "*", Slash => "/", IntDiv => "//",
            Mod => "%", Pow => "**", Eq => "==", Ne => "!=", Lt => "<", Le => "<=",
            Gt => ">", Ge => ">=", And => "and", Or => "or", Not => "not",
            BitAnd => "&", BitOr => "|", BitXor => "^", Shl => "<<", Shr => ">>",
            BitNot => "~", Assign => "=", PlusAssign => "+=", MinusAssign => "-=",
            StarAssign => "*=", SlashAssign => "/=", IntDivAssign => "//=",
            ModAssign => "%=", PowAssign => "**=", BitAndAssign => "&=",
            BitOrAssign => "|=", BitXorAssign => "^=", ShlAssign => "<<=",
            ShrAssign => ">>=", Pipe => "|>", OptionalChain => "?.", NullCoalesce => "??",
            Question => "?", Colon => ":", Arrow => "->", Dot => ".", Bang => "!",
            LParen => "(", RParen => ")", LBracket => "[", RBracket => "]",
            LBrace => "{", RBrace => "}", Comma => ",",
            Newline => "换行", Indent => "缩进", Dedent => "去缩进",
            Comment(v) => return write!(f, "注释 {}", v),
            Eof => "EOF",
        };
        write!(f, "{}", s)
    }
}
