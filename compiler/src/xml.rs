//! 普贤 (PuXian) XML 解析模块（M19）
//!
//! 零依赖标准库实现，与编译模式（C runtime）输出结构一致：
//! - xml_parse(xml) → dict{name, attrs, children, text} 或 null
//! - xml_escape / xml_unescape：文本与属性值实体转义
//!
//! 支持：元素/属性（单双引号）/文本/注释/CDATA/自闭合/处理指令/DOCTYPE 跳过/
//!       命名空间前缀（按普通名称）/数字与预定义实体。文本内容字节透传。

/// XML 元素节点
#[derive(Debug, Clone)]
pub struct XmlNode {
    pub name: String,
    pub attrs: Vec<(String, String)>,
    pub children: Vec<XmlChild>,
}

/// 子节点：元素或文本
#[derive(Debug, Clone)]
pub enum XmlChild {
    Elem(XmlNode),
    Text(String),
}

/// 简单递归下降解析器（基于 char 迭代，UTF-8 安全）
struct Parser {
    chars: Vec<char>,
    pos: usize,
}

impl Parser {
    fn new(input: &str) -> Self {
        Parser { chars: input.chars().collect(), pos: 0 }
    }

    fn peek(&self) -> Option<char> {
        self.chars.get(self.pos).copied()
    }

    fn next(&mut self) -> Option<char> {
        let c = self.peek();
        if c.is_some() {
            self.pos += 1;
        }
        c
    }

    fn starts_with(&self, s: &str) -> bool {
        let cs: Vec<char> = s.chars().collect();
        if self.pos + cs.len() > self.chars.len() {
            return false;
        }
        (0..cs.len()).all(|i| self.chars[self.pos + i] == cs[i])
    }

    fn eat_ws(&mut self) {
        while let Some(c) = self.peek() {
            if c == ' ' || c == '\t' || c == '\n' || c == '\r' {
                self.pos += 1;
            } else {
                break;
            }
        }
    }

    fn error(&self, msg: impl Into<String>) -> String {
        format!("XML 解析错误（位置 {}）: {}", self.pos, msg.into())
    }

    /// 跳过注释 / 处理指令 / DOCTYPE，返回 false 表示意外 EOF
    fn skip_misc(&mut self) -> Result<(), String> {
        loop {
            self.eat_ws();
            if self.starts_with("<!--") {
                // 注释：直到 -->
                let mut end = self.pos + 4;
                while end + 2 <= self.chars.len() {
                    if self.chars[end] == '-' && end + 1 < self.chars.len() && self.chars[end + 1] == '>' {
                        self.pos = end + 2;
                        break;
                    }
                    end += 1;
                }
                if !(end + 2 <= self.chars.len()) {
                    return Err(self.error("注释未闭合"));
                }
            } else if self.starts_with("<?") {
                // 处理指令：直到 ?>
                if let Some(rel) = self.find_from("?>") {
                    self.pos = rel + 2;
                } else {
                    return Err(self.error("处理指令未闭合"));
                }
            } else if self.starts_with("<!DOCTYPE") || self.starts_with("<!doctype") {
                // DOCTYPE：跳过到 >（忽略内部子集）
                let mut depth = 0usize;
                while let Some(c) = self.next() {
                    if c == '[' {
                        depth += 1;
                    } else if c == ']' {
                        depth = depth.saturating_sub(1);
                    } else if c == '>' && depth == 0 {
                        break;
                    }
                }
            } else {
                break;
            }
        }
        Ok(())
    }

    fn find_from(&self, s: &str) -> Option<usize> {
        let cs: Vec<char> = s.chars().collect();
        if cs.is_empty() {
            return None;
        }
        let mut i = self.pos;
        while i + cs.len() <= self.chars.len() {
            if (0..cs.len()).all(|k| self.chars[i + k] == cs[k]) {
                return Some(i);
            }
            i += 1;
        }
        None
    }

    /// 解析整个文档：跳过杂项 → 根元素 → 跳过尾随杂项
    fn parse_document(&mut self) -> Result<XmlNode, String> {
        self.skip_misc()?;
        if self.peek() != Some('<') {
            return Err(self.error("缺少根元素"));
        }
        let root = self.parse_element()?;
        self.skip_misc()?;
        Ok(root)
    }

    /// 解析一个元素（调用前 pos 指向 '<'）
    fn parse_element(&mut self) -> Result<XmlNode, String> {
        if self.next() != Some('<') {
            return Err(self.error("期望 '<'"));
        }
        // 元素名
        let name = self.parse_name()?;
        let attrs = self.parse_attrs()?;
        let node = XmlNode { name, attrs, children: Vec::new() };
        // 自闭合
        if self.peek() == Some('/') {
            self.next();
            if self.next() != Some('>') {
                return Err(self.error("自闭合标签格式错误"));
            }
            return Ok(node);
        }
        if self.next() != Some('>') {
            return Err(self.error("标签未以 '>' 结束"));
        }
        let children = self.parse_content(&node.name)?;
        let mut n = node;
        n.children = children;
        Ok(n)
    }

    fn parse_name(&mut self) -> Result<String, String> {
        let mut s = String::new();
        while let Some(c) = self.peek() {
            if c.is_whitespace() || c == '>' || c == '/' || c == '=' {
                break;
            }
            s.push(c);
            self.pos += 1;
        }
        if s.is_empty() {
            return Err(self.error("缺少标签名"));
        }
        Ok(s)
    }

    fn parse_attrs(&mut self) -> Result<Vec<(String, String)>, String> {
        let mut attrs = Vec::new();
        loop {
            self.eat_ws();
            match self.peek() {
                Some('>') | Some('/') => break,
                None => return Err(self.error("标签未闭合")),
                _ => {
                    let k = self.parse_name()?;
                    self.eat_ws();
                    if self.next() != Some('=') {
                        return Err(self.error(format!("属性 {} 缺少 '='", k)));
                    }
                    self.eat_ws();
                    let quote = match self.next() {
                        Some(q @ ('"' | '\'')) => q,
                        _ => return Err(self.error("属性值必须用引号括起")),
                    };
                    let mut v = String::new();
                    loop {
                        match self.next() {
                            Some(c) if c == quote => break,
                            Some(c) => v.push(c),
                            None => return Err(self.error("属性值未闭合")),
                        }
                    }
                    let dec = decode_entities(&v);
                    attrs.push((k, dec));
                }
            }
        }
        Ok(attrs)
    }

    /// 解析元素内容直到匹配的结束标签
    fn parse_content(&mut self, name: &str) -> Result<Vec<XmlChild>, String> {
        let mut children: Vec<XmlChild> = Vec::new();
        let mut text = String::new();
        loop {
            if self.pos >= self.chars.len() {
                return Err(self.error(format!("元素 <{}> 未闭合", name)));
            }
            if self.peek() == Some('<') {
                if self.starts_with("</") {
                    // 结束标签
                    if !text.is_empty() {
                        children.push(XmlChild::Text(decode_entities(&text)));
                        text.clear();
                    }
                    self.pos += 2;
                    let end_name = self.parse_name()?;
                    self.eat_ws();
                    if self.next() != Some('>') {
                        return Err(self.error("结束标签格式错误"));
                    }
                    if end_name != name {
                        return Err(self.error(format!(
                            "结束标签 </{}> 与开始标签 <{}> 不匹配",
                            end_name, name
                        )));
                    }
                    return Ok(children);
                } else if self.starts_with("<!--") {
                    // 注释
                    let mut end = self.pos + 4;
                    while end + 2 <= self.chars.len() {
                        if self.chars[end] == '-' && end + 1 < self.chars.len() && self.chars[end + 1] == '>' {
                            break;
                        }
                        end += 1;
                    }
                    if !(end + 2 <= self.chars.len()) {
                        return Err(self.error("注释未闭合"));
                    }
                    self.pos = end + 2;
                } else if self.starts_with("<![CDATA[") {
                    // CDATA：原样文本（不解码实体），作为独立文本节点
                    self.pos += 9;
                    if let Some(rel) = self.find_from("]]>") {
                        if !text.is_empty() {
                            children.push(XmlChild::Text(decode_entities(&text)));
                            text.clear();
                        }
                        let content: String = self.chars[self.pos..rel].iter().collect();
                        children.push(XmlChild::Text(content));
                        self.pos = rel + 3;
                    } else {
                        return Err(self.error("CDATA 未闭合"));
                    }
                } else if self.starts_with("<?") {
                    // 处理指令
                    if let Some(rel) = self.find_from("?>") {
                        self.pos = rel + 2;
                    } else {
                        return Err(self.error("处理指令未闭合"));
                    }
                } else {
                    // 子元素
                    if !text.is_empty() {
                        children.push(XmlChild::Text(decode_entities(&text)));
                        text.clear();
                    }
                    let child = self.parse_element()?;
                    children.push(XmlChild::Elem(child));
                }
            } else {
                text.push(self.next().unwrap());
            }
        }
    }
}

/// 解码 XML 实体：&amp; &lt; &gt; &quot; &apos; &#nn; &#xhh;
fn decode_entities(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    let chars: Vec<char> = s.chars().collect();
    let mut i = 0;
    while i < chars.len() {
        if chars[i] == '&' {
            if let Some(rel) = chars[i..].iter().position(|&c| c == ';') {
                let ent: String = chars[i + 1..i + rel].iter().collect();
                let decoded: Option<String> = match ent.as_str() {
                    "amp" => Some("&".to_string()),
                    "lt" => Some("<".to_string()),
                    "gt" => Some(">".to_string()),
                    "quot" => Some("\"".to_string()),
                    "apos" => Some("'".to_string()),
                    _ => {
                        if let Some(num) = ent.strip_prefix('#') {
                            let code = if let Some(h) = num.strip_prefix('x').or_else(|| num.strip_prefix('X')) {
                                u32::from_str_radix(h, 16).ok()
                            } else {
                                num.parse::<u32>().ok()
                            };
                            code.and_then(|c| char::from_u32(c)).map(|c| c.to_string())
                        } else {
                            None
                        }
                    }
                };
                if let Some(d) = decoded {
                    out.push_str(&d);
                    i += rel + 1;
                    continue;
                }
            }
            out.push('&');
            i += 1;
        } else {
            out.push(chars[i]);
            i += 1;
        }
    }
    out
}

/// 解析 XML 文档，返回根元素
pub fn parse(xml: &str) -> Result<XmlNode, String> {
    let mut p = Parser::new(xml);
    p.parse_document()
}

/// 转义文本中的 XML 特殊字符
pub fn escape(s: &str) -> String {
    let mut out = String::with_capacity(s.len());
    for c in s.chars() {
        match c {
            '&' => out.push_str("&amp;"),
            '<' => out.push_str("&lt;"),
            '>' => out.push_str("&gt;"),
            '"' => out.push_str("&quot;"),
            '\'' => out.push_str("&apos;"),
            _ => out.push(c),
        }
    }
    out
}

/// 反转义 XML 实体
pub fn unescape(s: &str) -> String {
    decode_entities(s)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_basic() {
        let n = parse("<root id=\"1\">hello <b>bold</b> world</root>").unwrap();
        assert_eq!(n.name, "root");
        assert_eq!(n.attrs, vec![("id".to_string(), "1".to_string())]);
        assert_eq!(n.children.len(), 3);
        match &n.children[0] {
            XmlChild::Text(t) => assert_eq!(t, "hello "),
            _ => panic!(),
        }
        match &n.children[1] {
            XmlChild::Elem(e) => {
                assert_eq!(e.name, "b");
                assert_eq!(e.children.len(), 1);
            }
            _ => panic!(),
        }
        match &n.children[2] {
            XmlChild::Text(t) => assert_eq!(t, " world"),
            _ => panic!(),
        }
    }

    #[test]
    fn test_parse_entities_cdata_selfclose() {
        let n = parse("<a x=\"a&amp;b\" y='&#65;&#x42;'><![CDATA[<raw> & 中文]]><c/></a>").unwrap();
        assert_eq!(n.attrs.len(), 2);
        assert_eq!(n.attrs[0].1, "a&b");
        assert_eq!(n.attrs[1].1, "AB");
        assert_eq!(n.children.len(), 2);
        match &n.children[0] {
            XmlChild::Text(t) => assert_eq!(t, "<raw> & 中文"),
            _ => panic!(),
        }
        match &n.children[1] {
            XmlChild::Elem(e) => assert_eq!(e.name, "c"),
            _ => panic!(),
        }
    }

    #[test]
    fn test_parse_mismatch_error() {
        assert!(parse("<a><b></a>").is_err());
        assert!(parse("<a>").is_err());
        assert!(parse("no xml").is_err());
    }

    #[test]
    fn test_escape_unescape() {
        assert_eq!(escape("<a&b>\"'\""), "&lt;a&amp;b&gt;&quot;&apos;&quot;");
        assert_eq!(unescape("&lt;a&amp;b&gt; &#65;&#x42;"), "<a&b> AB");
    }
}
