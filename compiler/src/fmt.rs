//! 普贤 (PuXian) 代码格式化器（px fmt）
//!
//! 基于 token 流重建源码：
//!   - 统一缩进为 4 空格
//!   - 运算符两侧空格（一元紧贴）、逗号/冒号后空格、括号内无空格
//!   - 函数调用 `f(` 无空格、关键字 `if (` 有空格、`fn(` 无空格
//!   - 注释保留（行内注释前补两空格；行首注释按原列对齐）
//!   - 字符串统一重转义为双引号（保留语义，原始字面量形式规范化）
//!   - 连续空行压缩为 1 个；行尾无多余空格；文件末尾单个换行
//!
//! 保证：格式化后重新词法分析，token 序列语义等价（Int/Float/Str 值不变）。

use crate::lexer::Lexer;
use crate::token::{TokenKind};

/// 格式化源码；失败返回词法错误
pub fn format(src: &str) -> Result<String, String> {
    let tokens = Lexer::new_with_comments(src)
        .tokenize()
        .map_err(|e| e.to_string())?;

    let mut out = String::new();
    let mut level: usize = 0;
    let mut at_line_start = true;
    let mut prev: Option<TokenKind> = None;
    // 占位：文件开头/行首前视为 Newline（一元上下文），保证首个 token 后有空格
    let mut prev2: Option<TokenKind> = Some(TokenKind::Newline);

    for tok in &tokens {
        let kind = tok.kind.clone();
        match &kind {
            TokenKind::Eof => break,
            TokenKind::Newline => {
                trim_trailing_spaces(&mut out);
                out.push('\n');
                at_line_start = true;
                if let Some(p) = prev.take() {
                    prev2 = Some(p);
                }
                prev = Some(kind);
                continue;
            }
            TokenKind::Indent => {
                level += 1;
                if !at_line_start {
                    out.push('\n');
                }
                at_line_start = true;
                if let Some(p) = prev.take() {
                    prev2 = Some(p);
                }
                prev = Some(kind);
                continue;
            }
            TokenKind::Dedent => {
                level = level.saturating_sub(1);
                if !at_line_start {
                    out.push('\n');
                }
                at_line_start = true;
                if let Some(p) = prev.take() {
                    prev2 = Some(p);
                }
                prev = Some(kind);
                continue;
            }
            TokenKind::Comment(text) => {
                if at_line_start {
                    // 行首注释：按原列缩进对齐（块注释可跨行，原文保留）
                    let col = tok.pos.col;
                    if col > 1 {
                        for _ in 0..col - 1 {
                            out.push(' ');
                        }
                    }
                    out.push_str(text);
                    at_line_start = false;
                } else {
                    // 行内注释：前补两个空格
                    trim_trailing_spaces(&mut out);
                    out.push_str("  ");
                    out.push_str(text);
                    at_line_start = false;
                }
                if let Some(p) = prev.take() {
                    prev2 = Some(p);
                }
                prev = Some(kind);
                continue;
            }
            _ => {}
        }

        // 普通 token
        if at_line_start {
            for _ in 0..level {
                out.push_str("    ");
            }
            at_line_start = false;
        } else if let (Some(p), Some(p2)) = (&prev, &prev2) {
            if needs_space(p2, p, &kind) {
                out.push(' ');
            }
        }

        out.push_str(&render(&kind));
        if let Some(p) = prev.take() {
            prev2 = Some(p);
        }
        prev = Some(kind);
    }

    // 压缩连续空行：最多保留 1 个空行
    let mut lines: Vec<String> = Vec::new();
    let mut blank_run = 0usize;
    for line in out.split('\n') {
        if line.trim().is_empty() {
            blank_run += 1;
            if blank_run > 1 {
                continue;
            }
            lines.push(String::new());
        } else {
            blank_run = 0;
            lines.push(line.to_string());
        }
    }
    // 去掉末尾空行，保证单个换行
    while lines.last().map(|l| l.trim().is_empty()).unwrap_or(false) {
        lines.pop();
    }
    let mut result = lines.join("\n");
    result.push('\n');
    Ok(result)
}

/// 判断三个相邻 token（prev2, prev, cur）之间是否需要空格
fn needs_space(prev2: &TokenKind, prev: &TokenKind, cur: &TokenKind) -> bool {
    use TokenKind::*;

    // 行结构 token 不在此处理（上面已 continue）
    // 当前 token 前不加空格的情况（左紧贴）
    match cur {
        RParen | RBracket | RBrace | Dot | OptionalChain | Comma | Colon | Bang => return false,
        _ => {}
    }
    // 前一个 token 后不加空格的情况（右紧贴）
    match prev {
        LParen | LBracket | LBrace | Dot | OptionalChain | BitNot => return false,
        _ => {}
    }
    // 一元 +/- 紧贴：前面是运算符/左括号/逗号/关键字/行首时
    if matches!(cur, Minus | Plus) && is_unary_context(prev) {
        return false;
    }
    // 一元 +/- 后紧贴操作数（如 add(-1, 1) 的 -1）
    if matches!(prev, Minus | Plus) && is_unary_context(prev2) {
        return false;
    }
    // 函数调用 f( / 下标 a[ 无空格
    if matches!(prev, Ident(_)) && matches!(cur, LParen | LBracket) {
        return false;
    }
    // 值后紧跟左括号（如 ) ( 链式调用）无空格
    if matches!(
        prev,
        Int(_) | Float(_) | Str(_) | RParen | RBracket | RBrace | True | False | Null
    ) && matches!(cur, LParen | LBracket)
    {
        return false;
    }
    // fn(x) 无空格；其他关键字 + ( 有空格（if (、while (、chan (）
    if matches!(prev, Fn) && matches!(cur, LParen) {
        return false;
    }
    // not 与操作数之间要空格（cur == Not 时前加空格；Not 后加空格由下一次判断）
    if matches!(cur, Not) {
        return true;
    }
    // 其他默认加空格
    true
}

/// 判断 prev 是否构成一元 +/- 的上下文（即 cur 是操作数，非二元运算）
fn is_unary_context(prev: &TokenKind) -> bool {
    use TokenKind::*;
    matches!(
        prev,
        Plus | Minus
            | Star
            | Slash
            | IntDiv
            | Mod
            | Pow
            | Eq
            | Ne
            | Lt
            | Le
            | Gt
            | Ge
            | And
            | Or
            | Not
            | BitAnd
            | BitOr
            | BitXor
            | Shl
            | Shr
            | ShrU
            | Assign
            | PlusAssign
            | MinusAssign
            | StarAssign
            | SlashAssign
            | IntDivAssign
            | ModAssign
            | PowAssign
            | BitAndAssign
            | BitOrAssign
            | BitXorAssign
            | ShlAssign
            | ShrAssign
            | ShrUAssign
            | Pipe
            | NullCoalesce
            | Arrow
            | LParen
            | LBracket
            | LBrace
            | Comma
            | Colon
            | Newline
            | Indent
            | Dedent
            | Let
            | Var
            | Const
            | Return
            | If
            | Elif
            | Else
            | For
            | While
            | In
            | Def
            | Fn
            | Match
            | Case
            | Spawn
            | Chan
            | Select
            | Send
            | Recv
            | Import
            | From
            | As
            | Pub
            | Capture
    )
}

/// 渲染 token 文本
fn render(kind: &TokenKind) -> String {
    use TokenKind::*;
    match kind {
        Int(v) => v.to_string(),
        Float(v) => {
            let s = format!("{}", v);
            // 整数值的浮点必须保留小数点（否则重 lex 变 Int，语义改变）
            if !s.contains('.') && !s.contains('e') && !s.contains('E') {
                format!("{}.0", s)
            } else {
                s
            }
        }
        Str(v) => escape_str(v),
        Ident(v) => v.clone(),
        k => k.to_string(),
    }
}

/// 字符串重转义为双引号字面量
fn escape_str(s: &str) -> String {
    let mut out = String::from("\"");
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            c if (c as u32) < 0x20 => out.push_str(&format!("\\u{{{:x}}}", c as u32)),
            c => out.push(c),
        }
    }
    out.push('"');
    out
}

fn trim_trailing_spaces(out: &mut String) {
    while out.ends_with(' ') {
        out.pop();
    }
}

// ==================== M25：px fmt --diff（unified diff） ====================
// 与 gofmt -d / black --diff 同语义：不写回文件，打印统一格式差异（供 CI/人工审查）。

/// 行级 LCS 最长公共子序列（O(n*m)，格式化场景行数有限，足够）
fn lcs_table(a: &[&str], b: &[&str]) -> Vec<Vec<usize>> {
    let n = a.len();
    let m = b.len();
    let mut dp = vec![vec![0usize; m + 1]; n + 1];
    for i in (0..n).rev() {
        for j in (0..m).rev() {
            dp[i][j] = if a[i] == b[j] {
                dp[i + 1][j + 1] + 1
            } else {
                dp[i + 1][j].max(dp[i][j + 1])
            };
        }
    }
    dp
}

/// 生成 unified diff（@@ hunk 头 + 上下文 3 行）。无差异返回空串。
pub fn unified_diff(orig: &str, new: &str) -> String {
    let a: Vec<&str> = orig.lines().collect();
    let b: Vec<&str> = new.lines().collect();
    if a == b {
        return String::new();
    }
    let dp = lcs_table(&a, &b);
    // 回溯得到 op 序列：0=同, 1=删, 2=增
    let mut ops: Vec<(u8, &str)> = Vec::new();
    let (mut i, mut j) = (0usize, 0usize);
    while i < a.len() && j < b.len() {
        if a[i] == b[j] {
            ops.push((0, a[i]));
            i += 1;
            j += 1;
        } else if dp[i + 1][j] >= dp[i][j + 1] {
            ops.push((1, a[i]));
            i += 1;
        } else {
            ops.push((2, b[j]));
            j += 1;
        }
    }
    while i < a.len() {
        ops.push((1, a[i]));
        i += 1;
    }
    while j < b.len() {
        ops.push((2, b[j]));
        j += 1;
    }

    // 变化点分组：间隔（连续同行数）≥ 2*ctx 则拆成独立 hunk
    let ctx = 3usize;
    let changes: Vec<usize> = ops
        .iter()
        .enumerate()
        .filter(|(_, op)| op.0 != 0)
        .map(|(k, _)| k)
        .collect();
    let mut out = String::new();
    out.push_str("--- original\n");
    out.push_str("+++ formatted\n");
    let mut gi = 0usize;
    while gi < changes.len() {
        let mut gj = gi + 1;
        while gj < changes.len() && changes[gj] - changes[gj - 1] < 2 * ctx + 1 {
            gj += 1;
        }
        let first = changes[gi];
        let last = changes[gj - 1];
        let start = first.saturating_sub(ctx);
        let end = (last + ctx + 1).min(ops.len());
        emit_hunk(&mut out, &ops, start, end);
        gi = gj;
    }
    out
}

fn emit_hunk(out: &mut String, ops: &[(u8, &str)], start: usize, end: usize) {
    // 计算 hunk 起始行号（1-based）：遍历 start 之前的 op
    let (mut o_line, mut n_line) = (1usize, 1usize);
    for op in ops.iter().take(start) {
        match op.0 {
            0 => {
                o_line += 1;
                n_line += 1;
            }
            1 => o_line += 1,
            _ => n_line += 1,
        }
    }
    // 统计 hunk 内行数（含上下文）
    let (mut o_count, mut n_count) = (0usize, 0usize);
    for op in ops.iter().take(end).skip(start) {
        match op.0 {
            0 => {
                o_count += 1;
                n_count += 1;
            }
            1 => o_count += 1,
            _ => n_count += 1,
        }
    }
    out.push_str(&format!(
        "@@ -{},{} +{},{} @@\n",
        o_line,
        o_count.max(1),
        n_line,
        n_count.max(1)
    ));
    for (_, op) in ops.iter().enumerate().take(end).skip(start) {
        match op.0 {
            0 => out.push_str(&format!(" {}\n", op.1)),
            1 => out.push_str(&format!("-{}\n", op.1)),
            _ => out.push_str(&format!("+{}\n", op.1)),
        }
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use crate::lexer::Lexer;

    /// 提取语义 token 序列（忽略位置、注释、行结构）
    fn semantic_tokens(src: &str) -> Vec<String> {
        Lexer::new(src)
            .tokenize()
            .unwrap()
            .into_iter()
            .filter(|t| {
                !matches!(
                    t.kind,
                    TokenKind::Newline
                        | TokenKind::Indent
                        | TokenKind::Dedent
                        | TokenKind::Comment(_)
                        | TokenKind::Eof
                )
            })
            .map(|t| match &t.kind {
                TokenKind::Int(v) => format!("int:{}", v),
                TokenKind::Float(v) => format!("float:{}", v),
                TokenKind::Str(v) => format!("str:{}", v),
                TokenKind::Ident(v) => format!("id:{}", v),
                k => k.to_string(),
            })
            .collect()
    }

    #[test]
    fn test_fmt_basic() {
        let src = "def fib(n:int)->int:\n    if n<=1:\n        return n\n    else:\n        return fib(n-1)+fib(n-2)\n";
        let out = format(src).unwrap();
        assert!(out.contains("def fib(n: int) -> int:"));
        assert!(out.contains("if n <= 1:"));
        assert!(out.contains("fib(n - 1) + fib(n - 2)"));
        // 语义等价
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_float_preserved() {
        let src = "let a = 1.0\nlet b = 1.25\nprint(a, b)\n";
        let out = format(src).unwrap();
        assert!(out.contains("1.0"), "整数值浮点必须保留小数点: {}", out);
        assert!(out.contains("1.25"));
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_comment_kept() {
        let src = "# 头部注释\ndef f():\n    # 行首注释\n    let x = 1  # 行内注释\n    return x\n";
        let out = format(src).unwrap();
        assert!(out.contains("# 头部注释"));
        assert!(out.contains("# 行首注释"));
        assert!(out.contains("# 行内注释"));
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_call_and_keyword() {
        let src = "def main():\n    if (x > 0):\n        print(to_upper(\"hi\"))\n";
        let out = format(src).unwrap();
        // 函数调用无空格
        assert!(out.contains("print(to_upper(\"hi\"))"), "{}", out);
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_blank_lines_compressed() {
        let src = "let a = 1\n\n\n\nlet b = 2\n";
        let out = format(src).unwrap();
        let blank = out.split('\n').filter(|l| l.trim().is_empty()).count();
        assert!(blank <= 1, "连续空行应压缩: {:?}", out);
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_pipe_and_dict() {
        let src = "let d={\"name\":\"px\",\"v\":1}\nlet s = \"hello\" |> to_upper()\nprint(d[\"name\"], s)\n";
        let out = format(src).unwrap();
        assert!(out.contains("{\"name\": \"px\", \"v\": 1}"), "{}", out);
        assert!(out.contains("|> to_upper()"), "{}", out);
        assert_eq!(semantic_tokens(src), semantic_tokens(&out));
    }

    #[test]
    fn test_fmt_idempotent() {
        let src = "def main():\n    let nums=[3,1,4]\n    for i in nums:\n        print(i)\n";
        let once = format(src).unwrap();
        let twice = format(&once).unwrap();
        assert_eq!(once, twice, "fmt 应幂等");
    }

    #[test]
    fn test_diff_empty_when_same() {
        let src = "let a = 1\nlet b = 2\n";
        assert_eq!(unified_diff(src, src), "", "相同内容无 diff");
    }

    #[test]
    fn test_diff_basic_hunks() {
        let orig = "line1\nline2\nline3\nbad\nline5\nline6\nline7\n";
        let new = "line1\nline2\nline3\ngood\nline5\nline6\nline7\n";
        let d = unified_diff(orig, new);
        assert!(d.starts_with("--- original\n+++ formatted\n"), "{}", d);
        assert!(d.contains("-bad\n"), "应含删除行: {}", d);
        assert!(d.contains("+good\n"), "应含新增行: {}", d);
        assert!(d.contains("@@"), "应含 hunk 头: {}", d);
    }
}
