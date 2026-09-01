//! 普贤 (PuXian) 文档生成器（px doc）
//!
//! 从 `##` 文档注释（紧随 def/struct/enum/trait 定义之前）提取生成 Markdown。
//! 用法：px doc <file.px> [--output out.md]  默认输出到 stdout。
//!
//! 约定：文档注释必须以 `##` 开头且紧跟被注释的顶层定义（中间无空语句）。

use crate::lexer::Lexer;
use crate::token::TokenKind;

/// 生成的 Markdown 文档
pub fn generate_doc(src: &str, file_name: &str) -> Result<String, String> {
    let tokens = Lexer::new_with_comments(src)
        .tokenize()
        .map_err(|e| e.to_string())?;

    let mut md = String::new();
    md.push_str(&format!("# {} API 文档\n\n", file_name));
    md.push_str("> 由 `px doc` 自动生成。\n\n");

    let mut funcs: Vec<(String, String)> = Vec::new(); // (签名, 文档)
    let mut types: Vec<(String, String)> = Vec::new(); // (签名, 文档)
    let mut pending_doc: Vec<String> = Vec::new();

    let mut i = 0usize;
    while i < tokens.len() {
        let tok = &tokens[i];
        match &tok.kind {
            TokenKind::Comment(text) if is_doc_comment(text) => {
                // 累积文档注释（连续 ## 行合并；块注释中的 ## 也接受）
                pending_doc.push(clean_doc_line(text));
                i += 1;
            }
            TokenKind::Def
            | TokenKind::Struct
            | TokenKind::Enum
            | TokenKind::Trait => {
                let kind = tok.kind.clone();
                let doc = if pending_doc.is_empty() {
                    String::new()
                } else {
                    pending_doc.join("\n")
                };
                pending_doc.clear();

                // 提取签名：从当前 token 到定义主体前的 token 序列
                let mut j = i;
                let mut depth = 0i32;
                let mut sig_tokens: Vec<String> = Vec::new();
                while j < tokens.len() {
                    let t = &tokens[j];
                    match &t.kind {
                        TokenKind::Newline | TokenKind::Indent | TokenKind::Dedent | TokenKind::Eof => {
                            if depth == 0 {
                                break;
                            }
                        }
                        _ => {}
                    }
                    match &t.kind {
                        TokenKind::LParen | TokenKind::LBracket | TokenKind::LBrace => depth += 1,
                        TokenKind::RParen | TokenKind::RBracket | TokenKind::RBrace => depth -= 1,
                        _ => {}
                    }
                    sig_tokens.push(render_sig_token(&t.kind));
                    if depth < 0 {
                        break;
                    }
                    if depth == 0 && matches!(t.kind, TokenKind::Newline) {
                        break;
                    }
                    j += 1;
                }
                let sig = join_sig(&sig_tokens);

                match kind {
                    TokenKind::Def => funcs.push((sig, doc)),
                    _ => types.push((sig, doc)),
                }
                i = j;
                continue;
            }
            TokenKind::Newline | TokenKind::Indent | TokenKind::Dedent | TokenKind::Eof => {
                // 行结构 token 不打断待定文档注释
            }
            _ => {
                // 其他 token 打断待注释
                pending_doc.clear();
            }
        }
        i += 1;
    }

    // ---- 输出 ----
    if !funcs.is_empty() {
        md.push_str("## 函数\n\n");
        for (sig, doc) in &funcs {
            md.push_str(&format!("### `{}`\n\n", sig));
            if doc.is_empty() {
                md.push_str("_无文档注释。_\n\n");
            } else {
                md.push_str(doc);
                md.push_str("\n\n");
            }
        }
    }
    if !types.is_empty() {
        md.push_str("## 类型\n\n");
        for (sig, doc) in &types {
            md.push_str(&format!("### `{}`\n\n", sig));
            if doc.is_empty() {
                md.push_str("_无文档注释。_\n\n");
            } else {
                md.push_str(doc);
                md.push_str("\n\n");
            }
        }
    }
    if funcs.is_empty() && types.is_empty() {
        md.push_str("_未发现带 `##` 文档注释的顶层定义。_\n");
    }

    Ok(md)
}

fn is_doc_comment(text: &str) -> bool {
    let t = text.trim_start();
    t.starts_with("##") && !t.starts_with("###")
}

fn clean_doc_line(text: &str) -> String {
    let t = text.trim_start();
    // 去掉前导 ##（可多个，最多保留一个 # 作为 markdown 层级）
    let stripped = t.trim_start_matches('#');
    stripped.trim_start().to_string()
}

fn render_sig_token(kind: &TokenKind) -> String {
    match kind {
        TokenKind::Ident(v) => v.clone(),
        TokenKind::Int(v) => v.to_string(),
        TokenKind::Float(v) => format!("{}", v),
        TokenKind::Str(v) => format!("\"{}\"", v),
        k => k.to_string(),
    }
}

/// 按签名规则拼接 token 文本：
///   `(` 前/后无空格、`)` 前无空格、`,` 前无空格后加空格、`:` 前无空格后加空格、`->` 两侧空格
fn join_sig(tokens: &[String]) -> String {
    let mut out = String::new();
    for (i, t) in tokens.iter().enumerate() {
        let cur = t.as_str();
        let need_space = if i == 0 {
            false
        } else {
            let prev = tokens[i - 1].as_str();
            match cur {
                ")" | "," | ":" | "(" => false,
                _ => match prev {
                    "(" => false,
                    "," | ":" | "->" => true,
                    _ => true,
                },
            }
        };
        if need_space {
            out.push(' ');
        }
        out.push_str(cur);
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_doc_basic() {
        let src = "## 计算斐波那契\n## 参数 n：序号\ndef fib(n: int) -> int:\n    return n\n\n## 点类型\nstruct Point:\n    x: int\n    y: int\n";
        let md = generate_doc(src, "test.px").unwrap();
        assert!(md.contains("# test.px API 文档"));
        assert!(md.contains("## 函数"));
        assert!(md.contains("### `def fib(n : int) -> int :`") || md.contains("### `def fib(n: int) -> int:`"));
        assert!(md.contains("计算斐波那契"));
        assert!(md.contains("## 类型"));
        assert!(md.contains("struct Point"));
    }

    #[test]
    fn test_doc_no_comments() {
        let src = "def f():\n    return 1\n";
        let md = generate_doc(src, "a.px").unwrap();
        assert!(md.contains("无文档注释"));
    }
}
