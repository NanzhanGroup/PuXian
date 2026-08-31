//! 普贤 (PuXian) 抽象语法树（AST）
//! 对应 spec.md §3-§8
//! 注意：部分字段为 M4 编译模式 / 类型检查预留，当前解释器未全部读取属正常
#![allow(dead_code)]

use crate::token::Pos;

// ==================== 程序 ====================

#[derive(Debug, Clone)]
pub struct Program {
    pub items: Vec<Stmt>,
}

// ==================== 语句 ====================

#[derive(Debug, Clone)]
pub enum Stmt {
    /// let / var / const 声明
    VarDecl {
        kind: VarKind,
        name: String,
        ty: Option<TypeExpr>,
        value: Option<Expr>,
        pos: Pos,
    },
    /// 赋值：x = e / x += e / x.y = e ...
    Assign {
        target: Expr,
        op: AssignOp,
        value: Expr,
        pos: Pos,
    },
    /// 表达式语句（调用、方法调用等）
    ExprStmt {
        expr: Expr,
        pos: Pos,
    },
    /// if / elif / else
    If {
        branches: Vec<(Expr, Vec<Stmt>)>,
        else_branch: Option<Vec<Stmt>>,
        pos: Pos,
    },
    /// for ... in ...
    For {
        var: String,
        iterable: Expr,
        body: Vec<Stmt>,
        pos: Pos,
    },
    While {
        cond: Expr,
        body: Vec<Stmt>,
        pos: Pos,
    },
    Return {
        value: Option<Expr>,
        pos: Pos,
    },
    Break {
        pos: Pos,
    },
    Continue {
        pos: Pos,
    },
    /// def / fn 函数定义
    FuncDef {
        name: String,
        params: Vec<Param>,
        ret_ty: Option<TypeExpr>,
        body: Vec<Stmt>,
        pos: Pos,
    },
    /// struct 定义
    StructDef {
        name: String,
        fields: Vec<StructField>,
        pos: Pos,
    },
    /// enum 定义
    EnumDef {
        name: String,
        variants: Vec<EnumVariant>,
        pos: Pos,
    },
    /// trait 定义
    TraitDef {
        name: String,
        methods: Vec<FuncDef>,
        pos: Pos,
    },
    /// impl 实现
    ImplDef {
        type_name: String,
        trait_name: Option<String>,
        methods: Vec<FuncDef>,
        pos: Pos,
    },
    /// import / from ... import ...
    Import {
        module: Vec<String>,
        names: Vec<String>,
        pos: Pos,
    },
    /// spawn 并发块
    Spawn {
        expr: Box<Expr>,
        pos: Pos,
    },
    /// chan 声明
    ChanDecl {
        name: String,
        elem_ty: Option<TypeExpr>,
        pos: Pos,
    },
    Send {
        chan: Expr,
        value: Expr,
        pos: Pos,
    },
    Recv {
        chan: Expr,
        pos: Pos,
    },
    Select {
        /// (绑定变量, 接收表达式, body)；绑定变量为 None 表示不绑定
        arms: Vec<(Option<String>, Expr, Vec<Stmt>)>,
        else_branch: Option<Vec<Stmt>>,
        pos: Pos,
    },
    /// 空语句（如显式 ;）
    Empty {
        pos: Pos,
    },
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum VarKind {
    Let,
    Var,
    Const,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum AssignOp {
    Assign,
    Plus,
    Minus,
    Star,
    Slash,
    IntDiv,
    Mod,
    Pow,
    BitAnd,
    BitOr,
    BitXor,
    Shl,
    Shr,
}

// ==================== 表达式 ====================

#[derive(Debug, Clone)]
pub enum Expr {
    Int {
        value: i64,
        pos: Pos,
    },
    Float {
        value: f64,
        pos: Pos,
    },
    Str {
        value: String,
        pos: Pos,
    },
    Bool {
        value: bool,
        pos: Pos,
    },
    Null {
        pos: Pos,
    },
    /// 列表字面量 [1, 2, 3]
    List {
        items: Vec<Expr>,
        pos: Pos,
    },
    /// 元组 (1, "a", true)
    Tuple {
        items: Vec<Expr>,
        pos: Pos,
    },
    /// 字典 {"k": v, ...}
    Dict {
        entries: Vec<(Expr, Expr)>,
        pos: Pos,
    },
    /// 变量引用
    Var {
        name: String,
        pos: Pos,
    },
    /// 属性访问 a.b
    Field {
        obj: Box<Expr>,
        name: String,
        pos: Pos,
    },
    /// 可选链 a?.b
    OptionalField {
        obj: Box<Expr>,
        name: String,
        pos: Pos,
    },
    /// 索引 a[i]
    Index {
        obj: Box<Expr>,
        index: Box<Expr>,
        pos: Pos,
    },
    /// 切片 a[start:end]（start/end 可省略；M21）
    Slice {
        obj: Box<Expr>,
        start: Option<Box<Expr>>,
        end: Option<Box<Expr>>,
        pos: Pos,
    },
    /// 调用 f(args) / obj.method(args)
    Call {
        callee: Box<Expr>,
        args: Vec<Expr>,
        pos: Pos,
    },
    /// 一元运算 -x !x not x ~x
    Unary {
        op: UnaryOp,
        operand: Box<Expr>,
        pos: Pos,
    },
    /// 二元运算
    Binary {
        op: BinaryOp,
        left: Box<Expr>,
        right: Box<Expr>,
        pos: Pos,
    },
    /// 管道 x |> f（等价 f(x)）
    Pipe {
        value: Box<Expr>,
        func: Box<Expr>,
        pos: Pos,
    },
    /// 空合并 a ?? b
    NullCoalesce {
        left: Box<Expr>,
        right: Box<Expr>,
        pos: Pos,
    },
    /// 错误传播 x?（Result/Option 解包）
    Try {
        expr: Box<Expr>,
        pos: Pos,
    },
    /// 强制解包 x!
    ForceUnwrap {
        expr: Box<Expr>,
        pos: Pos,
    },
    /// 三元 if 表达式（若支持；spec 建议避免嵌套）
    IfExpr {
        cond: Box<Expr>,
        then: Box<Expr>,
        else_: Box<Expr>,
        pos: Pos,
    },
    /// 列表推导 [f(x) for x in xs if cond]
    ListComp {
        expr: Box<Expr>,
        var: String,
        iterable: Box<Expr>,
        cond: Option<Box<Expr>>,
        pos: Pos,
    },
    /// 闭包/匿名函数
    Closure {
        params: Vec<Param>,
        ret_ty: Option<TypeExpr>,
        body: Box<Expr>, // 单表达式体；块用 BlockExpr
        captures: Vec<String>,
        pos: Pos,
    },
    /// 块表达式 { ... }（返回最后值）
    Block {
        stmts: Vec<Stmt>,
        pos: Pos,
    },
    /// match 表达式
    Match {
        subject: Box<Expr>,
        arms: Vec<MatchArm>,
        pos: Pos,
    },
    /// 类型构造/枚举变体：Point(1, 2) / Result.Ok(x)
    Constructor {
        name: String,
        args: Vec<Expr>,
        pos: Pos,
    },
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum UnaryOp {
    Neg,
    Not,
    BitNot,
}

#[derive(Debug, Clone, Copy, PartialEq)]
pub enum BinaryOp {
    Add,
    Sub,
    Mul,
    Div,
    IntDiv,
    Mod,
    Pow,
    Eq,
    Ne,
    Lt,
    Le,
    Gt,
    Ge,
    And,
    Or,
    BitAnd,
    BitOr,
    BitXor,
    Shl,
    Shr,
}

#[derive(Debug, Clone)]
pub struct MatchArm {
    /// 模式：目前支持字面量 / 标识符（绑定）/ _ 通配
    pub pattern: Pattern,
    pub guard: Option<Expr>,
    pub body: Expr,
    pub pos: Pos,
}

#[derive(Debug, Clone)]
pub enum Pattern {
    /// 字面量模式 1 / "a" / true / null
    Literal(Expr),
    /// 标识符模式（绑定变量）
    Binding(String),
    /// 通配 _
    Wildcard,
    /// 元组模式 (a, b)
    Tuple(Vec<Pattern>),
    /// 枚举/构造器模式 Some(x)
    Constructor(String, Vec<Pattern>),
}

// ==================== 类型表达式 ====================

#[derive(Debug, Clone)]
pub enum TypeExpr {
    /// 具名类型 int / string / bool / float / 自定义类型
    Named(String, Pos),
    /// 可空类型 T?
    Optional(Box<TypeExpr>, Pos),
    /// 列表类型 [T] 或 list[T]
    List(Box<TypeExpr>, Pos),
    /// 字典类型 {K: V}
    Dict(Box<TypeExpr>, Box<TypeExpr>, Pos),
    /// 元组类型 (T1, T2)
    Tuple(Vec<TypeExpr>, Pos),
    /// 函数类型 (T1, T2) -> R
    Func(Vec<TypeExpr>, Box<TypeExpr>, Pos),
    /// 泛型实例类型 Foo[T]
    Generic(String, Vec<TypeExpr>, Pos),
}

// ==================== 函数参数 ====================

#[derive(Debug, Clone)]
pub struct Param {
    pub name: String,
    pub ty: Option<TypeExpr>,
    pub default: Option<Expr>,
    pub pos: Pos,
}

/// 函数定义（trait 方法 / impl 方法复用）
#[derive(Debug, Clone)]
pub struct FuncDef {
    pub name: String,
    pub params: Vec<Param>,
    pub ret_ty: Option<TypeExpr>,
    pub body: Vec<Stmt>,
    pub pos: Pos,
}

/// struct 定义（类型表 / 编译模式复用）
#[derive(Debug, Clone)]
pub struct StructDef {
    pub name: String,
    pub fields: Vec<StructField>,
    pub pos: Pos,
}

/// enum 定义（类型表 / 编译模式复用）
#[derive(Debug, Clone)]
pub struct EnumDef {
    pub name: String,
    pub variants: Vec<EnumVariant>,
    pub pos: Pos,
}

/// trait 定义（类型表 / 编译模式复用）
#[derive(Debug, Clone)]
pub struct TraitDef {
    pub name: String,
    pub methods: Vec<FuncDef>,
    pub pos: Pos,
}

#[derive(Debug, Clone)]
pub struct StructField {
    pub name: String,
    pub ty: TypeExpr,
    pub pos: Pos,
}

#[derive(Debug, Clone)]
pub struct EnumVariant {
    pub name: String,
    /// 载荷类型（无载荷则为空）
    pub fields: Vec<TypeExpr>,
    pub pos: Pos,
}
