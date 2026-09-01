//! 普贤 (PuXian) 零依赖 JSON 值 + 解析 + 序列化
//! M7 协议层基础：LSP / MCP 均使用 JSON-RPC 2.0 over stdio

use std::fmt::Write;

/// JSON 值（对象保留插入顺序，用 Vec<(String, Json)> 而非 HashMap）
#[derive(Debug, Clone, PartialEq)]
pub enum Json {
    Null,
    Bool(bool),
    Int(i64),
    Float(f64),
    Str(String),
    Arr(Vec<Json>),
    Obj(Vec<(String, Json)>),
}

impl Json {
    pub fn obj() -> Json {
        Json::Obj(Vec::new())
    }
    pub fn arr() -> Json {
        Json::Arr(Vec::new())
    }
    /// 插入/更新对象字段（保留首次插入顺序；已存在则更新值）
    pub fn set(&mut self, key: &str, v: Json) {
        if let Json::Obj(entries) = self {
            if let Some(e) = entries.iter_mut().find(|(k, _)| k == key) {
                e.1 = v;
                return;
            }
            entries.push((key.to_string(), v));
        }
    }
    pub fn push(&mut self, v: Json) {
        if let Json::Arr(items) = self {
            items.push(v);
        }
    }
    /// 取对象字段
    pub fn get(&self, key: &str) -> Option<&Json> {
        match self {
            Json::Obj(entries) => entries.iter().find(|(k, _)| k == key).map(|(_, v)| v),
            _ => None,
        }
    }
    pub fn get_mut(&mut self, key: &str) -> Option<&mut Json> {
        match self {
            Json::Obj(entries) => entries.iter_mut().find(|(k, _)| k == key).map(|(_, v)| v),
            _ => None,
        }
    }
    /// 取对象字段（字符串）
    pub fn get_str(&self, key: &str) -> Option<&str> {
        match self.get(key) {
            Some(Json::Str(s)) => Some(s),
            _ => None,
        }
    }
    pub fn is_null(&self) -> bool {
        matches!(self, Json::Null)
    }
    pub fn as_str(&self) -> Option<&str> {
        match self {
            Json::Str(s) => Some(s),
            _ => None,
        }
    }
}

// ==================== 解析 ====================

struct Parser<'a> {
    bytes: &'a [u8],
    idx: usize,
}

impl<'a> Parser<'a> {
    fn skip_ws(&mut self) {
        while self.idx < self.bytes.len() && matches!(self.bytes[self.idx], b' ' | b'\t' | b'\n' | b'\r') {
            self.idx += 1;
        }
    }
    fn peek(&self) -> Option<u8> {
        self.bytes.get(self.idx).copied()
    }
    fn expect(&mut self, b: u8) -> Result<(), String> {
        if self.peek() == Some(b) {
            self.idx += 1;
            Ok(())
        } else {
            Err(format!("期望 '{}'，位置 {}", b as char, self.idx))
        }
    }
    fn parse_value(&mut self) -> Result<Json, String> {
        self.skip_ws();
        match self.peek() {
            Some(b'{') => {
                self.idx += 1;
                let mut entries = Vec::new();
                self.skip_ws();
                if self.peek() == Some(b'}') {
                    self.idx += 1;
                    return Ok(Json::Obj(entries));
                }
                loop {
                    self.skip_ws();
                    let k = self.parse_string()?;
                    self.skip_ws();
                    self.expect(b':')?;
                    let v = self.parse_value()?;
                    entries.push((k, v));
                    self.skip_ws();
                    match self.peek() {
                        Some(b',') => { self.idx += 1; }
                        Some(b'}') => { self.idx += 1; break; }
                        _ => return Err(format!("对象解析失败，位置 {}", self.idx)),
                    }
                }
                Ok(Json::Obj(entries))
            }
            Some(b'[') => {
                self.idx += 1;
                let mut items = Vec::new();
                self.skip_ws();
                if self.peek() == Some(b']') {
                    self.idx += 1;
                    return Ok(Json::Arr(items));
                }
                loop {
                    let v = self.parse_value()?;
                    items.push(v);
                    self.skip_ws();
                    match self.peek() {
                        Some(b',') => { self.idx += 1; }
                        Some(b']') => { self.idx += 1; break; }
                        _ => return Err(format!("数组解析失败，位置 {}", self.idx)),
                    }
                }
                Ok(Json::Arr(items))
            }
            Some(b'"') => Ok(Json::Str(self.parse_string()?)),
            Some(b't') => { self.expect_lit("true")?; Ok(Json::Bool(true)) }
            Some(b'f') => { self.expect_lit("false")?; Ok(Json::Bool(false)) }
            Some(b'n') => { self.expect_lit("null")?; Ok(Json::Null) }
            Some(c) if c == b'-' || c.is_ascii_digit() => self.parse_number(),
            _ => Err(format!("无法解析 JSON，位置 {}", self.idx)),
        }
    }
    fn expect_lit(&mut self, lit: &str) -> Result<(), String> {
        for ch in lit.bytes() {
            if self.peek() != Some(ch) {
                return Err(format!("期望 '{}'", lit));
            }
            self.idx += 1;
        }
        Ok(())
    }
    fn parse_string(&mut self) -> Result<String, String> {
        self.expect(b'"')?;
        // 按原始字节累积，最后统一 UTF-8 解码：修复多字节字符（如中文）被
        // 逐字节 `c as char` 转成 latin-1 字符的乱码问题
        let mut bytes: Vec<u8> = Vec::new();
        while let Some(c) = self.peek() {
            self.idx += 1;
            match c {
                b'"' => {
                    return String::from_utf8(bytes)
                        .map_err(|_| "字符串包含非法 UTF-8 字节".to_string());
                }
                b'\\' => {
                    let esc = self.peek().ok_or_else(|| "字符串转义不完整".to_string())?;
                    self.idx += 1;
                    match esc {
                        b'"' => bytes.push(b'"'),
                        b'\\' => bytes.push(b'\\'),
                        b'/' => bytes.push(b'/'),
                        b'n' => bytes.push(b'\n'),
                        b't' => bytes.push(b'\t'),
                        b'r' => bytes.push(b'\r'),
                        b'b' => bytes.push(0x08),
                        b'f' => bytes.push(0x0c),
                        b'u' => {
                            if self.idx + 4 > self.bytes.len() {
                                return Err("\\u 转义不完整".to_string());
                            }
                            let hex = std::str::from_utf8(&self.bytes[self.idx..self.idx + 4])
                                .map_err(|_| "\\u 非法".to_string())?;
                            let code = u32::from_str_radix(hex, 16)
                                .map_err(|_| "\\u 非法".to_string())?;
                            self.idx += 4;
                            if let Some(ch) = char::from_u32(code) {
                                let mut tmp = [0u8; 4];
                                bytes.extend_from_slice(ch.encode_utf8(&mut tmp).as_bytes());
                            }
                        }
                        _ => return Err(format!("非法转义 \\{}", esc as char)),
                    }
                }
                _ => bytes.push(c),
            }
        }
        Err("字符串未闭合".to_string())
    }
    fn parse_number(&mut self) -> Result<Json, String> {
        let start = self.idx;
        if self.peek() == Some(b'-') { self.idx += 1; }
        while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) { self.idx += 1; }
        let mut is_float = false;
        if self.peek() == Some(b'.') {
            is_float = true;
            self.idx += 1;
            while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) { self.idx += 1; }
        }
        if matches!(self.peek(), Some(b'e') | Some(b'E')) {
            is_float = true;
            self.idx += 1;
            if matches!(self.peek(), Some(b'+') | Some(b'-')) { self.idx += 1; }
            while self.peek().map(|c| c.is_ascii_digit()).unwrap_or(false) { self.idx += 1; }
        }
        let txt = std::str::from_utf8(&self.bytes[start..self.idx]).map_err(|_| "数字非法".to_string())?;
        if is_float {
            txt.parse::<f64>().map(Json::Float).map_err(|_| format!("数字非法: {}", txt))
        } else if let Ok(i) = txt.parse::<i64>() {
            Ok(Json::Int(i))
        } else {
            txt.parse::<f64>().map(Json::Float).map_err(|_| format!("数字非法: {}", txt))
        }
    }
}

/// 解析 JSON 文本
pub fn parse(s: &str) -> Result<Json, String> {
    let mut p = Parser { bytes: s.as_bytes(), idx: 0 };
    let v = p.parse_value()?;
    p.skip_ws();
    if p.idx != s.len() {
        return Err(format!("JSON 尾部多余内容，位置 {}", p.idx));
    }
    Ok(v)
}

// ==================== 序列化 ====================

/// 序列化 JSON 值为紧凑字符串
pub fn stringify(v: &Json) -> String {
    let mut out = String::new();
    write_json(v, &mut out);
    out
}

fn write_json(v: &Json, out: &mut String) {
    match v {
        Json::Null => out.push_str("null"),
        Json::Bool(b) => out.push_str(if *b { "true" } else { "false" }),
        Json::Int(i) => { let _ = write!(out, "{}", i); }
        Json::Float(f) => {
            if f.is_finite() && f.fract() == 0.0 && f.abs() < 1e15 {
                let _ = write!(out, "{:.1}", f);
            } else {
                let _ = write!(out, "{}", f);
            }
        }
        Json::Str(s) => { write_str(s, out); }
        Json::Arr(items) => {
            out.push('[');
            for (i, item) in items.iter().enumerate() {
                if i > 0 { out.push(','); }
                write_json(item, out);
            }
            out.push(']');
        }
        Json::Obj(entries) => {
            out.push('{');
            for (i, (k, val)) in entries.iter().enumerate() {
                if i > 0 { out.push(','); }
                write_str(k, out);
                out.push(':');
                write_json(val, out);
            }
            out.push('}');
        }
    }
}

fn write_str(s: &str, out: &mut String) {
    out.push('"');
    for c in s.chars() {
        match c {
            '"' => out.push_str("\\\""),
            '\\' => out.push_str("\\\\"),
            '\n' => out.push_str("\\n"),
            '\r' => out.push_str("\\r"),
            '\t' => out.push_str("\\t"),
            '\u{8}' => out.push_str("\\b"),
            '\u{c}' => out.push_str("\\f"),
            c if (c as u32) < 0x20 => {
                let _ = write!(out, "\\u{:04x}", c as u32);
            }
            c => out.push(c),
        }
    }
    out.push('"');
}

/// 美化输出（2 空格缩进），用于调试
pub fn stringify_pretty(v: &Json) -> String {
    let mut out = String::new();
    write_pretty(v, 0, &mut out);
    out
}

fn write_pretty(v: &Json, indent: usize, out: &mut String) {
    let pad = "  ".repeat(indent);
    match v {
        Json::Arr(items) if !items.is_empty() => {
            out.push_str("[\n");
            for (i, item) in items.iter().enumerate() {
                out.push_str(&"  ".repeat(indent + 1));
                write_pretty(item, indent + 1, out);
                if i + 1 < items.len() { out.push(','); }
                out.push('\n');
            }
            out.push_str(&pad);
            out.push(']');
        }
        Json::Obj(entries) if !entries.is_empty() => {
            out.push_str("{\n");
            for (i, (k, val)) in entries.iter().enumerate() {
                out.push_str(&"  ".repeat(indent + 1));
                write_str(k, out);
                out.push_str(": ");
                write_pretty(val, indent + 1, out);
                if i + 1 < entries.len() { out.push(','); }
                out.push('\n');
            }
            out.push_str(&pad);
            out.push('}');
        }
        _ => write_json(v, out),
    }
}

// ==================== 测试 ====================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_basic() {
        let v = parse(r#"{"a": 1, "b": [true, null, "x"], "c": {"d": 2.5}}"#).unwrap();
        assert_eq!(v.get("a"), Some(&Json::Int(1)));
        assert_eq!(v.get("b").unwrap().as_str(), None);
        assert_eq!(v.get("c").unwrap().get("d"), Some(&Json::Float(2.5)));
    }

    #[test]
    fn test_parse_escapes() {
        let v = parse(r#""a\nb\t\"c\"\\""#).unwrap();
        assert_eq!(v.as_str(), Some("a\nb\t\"c\"\\"));
    }

    #[test]
    fn test_parse_unicode() {
        let v = parse(r#""\u4e2d\u6587""#).unwrap();
        assert_eq!(v.as_str(), Some("中文"));
    }

    #[test]
    fn test_parse_utf8_direct() {
        // 直接 UTF-8 中文（M17 修复：此前多字节被逐字节转 latin-1 乱码）
        let v = parse("{\"name\": \"东月\"}").unwrap();
        assert_eq!(v.get("name"), Some(&Json::Str("东月".to_string())));
        let v2 = parse("\"你好，世界\"").unwrap();
        assert_eq!(v2.as_str(), Some("你好，世界"));
        // \u 转义与直接 UTF-8 混合
        let v3 = parse(r#""\u4e1c\u6708月""#).unwrap();
        assert_eq!(v3.as_str(), Some("东月月"));
    }

    #[test]
    fn test_roundtrip() {
        let src = r#"{"name":"px","ver":0.1,"ok":true,"nil":null,"arr":[1,2,3]}"#;
        let v = parse(src).unwrap();
        let out = stringify(&v);
        let v2 = parse(&out).unwrap();
        assert_eq!(v, v2);
    }

    #[test]
    fn test_stringify_escape() {
        let v = Json::Str("a\"b\nc".to_string());
        assert_eq!(stringify(&v), r#""a\"b\nc""#);
    }

    #[test]
    fn test_obj_order_preserved() {
        let v = parse(r#"{"z":1,"a":2,"m":3}"#).unwrap();
        if let Json::Obj(entries) = v {
            let keys: Vec<&str> = entries.iter().map(|(k, _)| k.as_str()).collect();
            assert_eq!(keys, vec!["z", "a", "m"]);
        } else {
            panic!("期望对象");
        }
    }

    #[test]
    fn test_set_get() {
        let mut v = Json::obj();
        v.set("a", Json::Int(1));
        v.set("b", Json::Str("s".into()));
        v.set("a", Json::Int(2)); // 更新
        assert_eq!(v.get("a"), Some(&Json::Int(2)));
        assert_eq!(v.get_str("b"), Some("s"));
        assert!(v.get("c").is_none());
    }
}
