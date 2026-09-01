//! 普贤 (PuXian) 正则表达式引擎（M15，P1 缺口：正则表达式）
//!
//! 双端一致：Rust 解释器与 C 编译模式（runtime.c）共用同一套回溯算法。
//! 支持子集（文本解析 / 日志分析 / 参数抽取足够）：
//!   - 字面量、`.`（任意非换行）、`^`/`$` 锚点
//!   - 字符类 `[abc]` `[a-z0-9]` `[^...]`、预定义 `\d \D \w \W \s \S`
//!   - 量词 `* + ? {n} {n,} {n,m}`（贪婪 + 回溯）
//!   - 捕获组 `( )`（最多 9 组）、交替 `|`、转义 `\. \* \n \t` 等
//!
//! 匹配策略：候选列表法——match_node 返回所有可能的 (end, groups) 候选，
//! 顺序为贪心优先（最左、最长优先），正确支持量词回溯与捕获组。
//! 注意：`(` `)` 是 ASCII 字节级匹配（日志/ASCII 场景；UTF-8 多字节文本
//! 按字节处理，`.` 与字符类按字节判等，中文原文可透传）。

use std::collections::HashSet;

/// 捕获组槽位数（0 = 整体匹配，1..=9 为捕获组）
pub const MAX_GROUPS: usize = 10;

/// 匹配跨度
#[derive(Clone, Copy, PartialEq, Eq, Hash, Debug)]
pub struct Span(pub usize, pub usize);

/// 捕获组快照（Copy，便于候选列表克隆）
pub type Groups = [Option<Span>; MAX_GROUPS];

pub fn fresh_groups() -> Groups {
    [None; MAX_GROUPS]
}

/// 正则 AST 节点
#[derive(Debug)]
pub enum Node {
    Char(u8),
    Any,
    Class { ranges: Vec<(u8, u8)>, neg: bool },
    Seq(Vec<Node>),
    Alt(Vec<Node>),
    Rep(Box<Node>, u32, i32), // min, max（-1 = 无限）
    Group(Box<Node>, usize),
    Start,
    End,
}

/// 单个匹配候选：结束位置 + 捕获组快照
type Cand = (usize, Groups);

// ---------------- 解析（编译） ----------------

struct Parser<'a> {
    b: &'a [u8],
    pos: usize,
    groups: usize,
}

/// 编译正则表达式，返回根节点
pub fn compile(pattern: &str) -> Result<Node, String> {
    let mut p = Parser { b: pattern.as_bytes(), pos: 0, groups: 0 };
    let node = p.parse_alt()?;
    if p.pos != p.b.len() {
        return Err(format!("正则语法错误: 位置 {} 处意外的字符", p.pos));
    }
    Ok(node)
}

impl<'a> Parser<'a> {
    fn peek(&self) -> Option<u8> {
        self.b.get(self.pos).copied()
    }

    /// 交替：`seq ('|' seq)*`
    fn parse_alt(&mut self) -> Result<Node, String> {
        let mut branches = vec![self.parse_seq()?];
        while self.peek() == Some(b'|') {
            self.pos += 1;
            branches.push(self.parse_seq()?);
        }
        if branches.len() == 1 {
            Ok(branches.pop().unwrap())
        } else {
            Ok(Node::Alt(branches))
        }
    }

    /// 序列：`repeat*`（空序列 = 匹配空）
    fn parse_seq(&mut self) -> Result<Node, String> {
        let mut nodes = vec![];
        while let Some(c) = self.peek() {
            if c == b'|' || c == b')' {
                break;
            }
            nodes.push(self.parse_repeat()?);
        }
        if nodes.len() == 1 {
            Ok(nodes.pop().unwrap())
        } else {
            Ok(Node::Seq(nodes))
        }
    }

    /// 原子 + 量词
    fn parse_repeat(&mut self) -> Result<Node, String> {
        let atom = self.parse_atom()?;
        match self.peek() {
            Some(b'*') => {
                self.pos += 1;
                Ok(Node::Rep(Box::new(atom), 0, -1))
            }
            Some(b'+') => {
                self.pos += 1;
                Ok(Node::Rep(Box::new(atom), 1, -1))
            }
            Some(b'?') => {
                self.pos += 1;
                Ok(Node::Rep(Box::new(atom), 0, 1))
            }
            Some(b'{') => {
                if let Some((min, max)) = self.try_braces()? {
                    Ok(Node::Rep(Box::new(atom), min, max))
                } else {
                    Ok(atom)
                }
            }
            _ => Ok(atom),
        }
    }

    /// 尝试解析 `{n}` `{n,}` `{n,m}`；不是合法量词则回退返回 None
    fn try_braces(&mut self) -> Result<Option<(u32, i32)>, String> {
        if self.peek() != Some(b'{') {
            return Ok(None);
        }
        let save = self.pos;
        self.pos += 1;
        let mut min_s = String::new();
        while let Some(c) = self.peek() {
            if c.is_ascii_digit() {
                min_s.push(c as char);
                self.pos += 1;
            } else {
                break;
            }
        }
        if min_s.is_empty() {
            self.pos = save;
            return Ok(None);
        }
        let min: u32 = min_s.parse().map_err(|_| "正则: {n} 数字过大".to_string())?;
        let max: i32 = match self.peek() {
            Some(b'}') => {
                self.pos += 1;
                min as i32
            }
            Some(b',') => {
                self.pos += 1;
                if self.peek() == Some(b'}') {
                    self.pos += 1;
                    -1
                } else {
                    let mut max_s = String::new();
                    while let Some(c) = self.peek() {
                        if c.is_ascii_digit() {
                            max_s.push(c as char);
                            self.pos += 1;
                        } else {
                            break;
                        }
                    }
                    if self.peek() != Some(b'}') {
                        self.pos = save;
                        return Ok(None);
                    }
                    self.pos += 1;
                    max_s.parse().map_err(|_| "正则: {n,m} 数字过大".to_string())?
                }
            }
            _ => {
                self.pos = save;
                return Ok(None);
            }
        };
        if max != -1 && (max as u32) < min {
            return Err("正则: {n,m} 中 m 不能小于 n".to_string());
        }
        Ok(Some((min, max)))
    }

    /// 原子
    fn parse_atom(&mut self) -> Result<Node, String> {
        let Some(c) = self.peek() else {
            return Err("正则语法错误: 意外的结尾".to_string());
        };
        match c {
            b'(' => {
                self.pos += 1;
                let node = self.parse_alt()?;
                if self.peek() != Some(b')') {
                    return Err("正则语法错误: 缺少 ')'".to_string());
                }
                self.pos += 1;
                self.groups += 1;
                if self.groups > 9 {
                    return Err("正则: 捕获组最多 9 个".to_string());
                }
                Ok(Node::Group(Box::new(node), self.groups))
            }
            b'[' => self.parse_class(),
            b'.' => {
                self.pos += 1;
                Ok(Node::Any)
            }
            b'^' => {
                self.pos += 1;
                Ok(Node::Start)
            }
            b'$' => {
                self.pos += 1;
                Ok(Node::End)
            }
            b'\\' => self.parse_escape(),
            b')' => Err("正则语法错误: 意外的 ')'".to_string()),
            _ => {
                self.pos += 1;
                Ok(Node::Char(c))
            }
        }
    }

    /// 字符类 `[...]`
    fn parse_class(&mut self) -> Result<Node, String> {
        self.pos += 1; // '['
        let mut neg = false;
        if self.peek() == Some(b'^') {
            neg = true;
            self.pos += 1;
        }
        let mut ranges: Vec<(u8, u8)> = vec![];
        let mut first = true;
        loop {
            let Some(c) = self.peek() else {
                return Err("正则语法错误: 字符类缺少 ']'".to_string());
            };
            if c == b']' && !first {
                self.pos += 1;
                break;
            }
            first = false;
            let (rs, is_neg) = self.parse_class_elem()?;
            // 仅单字符且非取反子类时支持 `a-z` 范围语法
            if rs.len() == 1 && !is_neg
                && self.peek() == Some(b'-')
                && self.b.get(self.pos + 1).is_some()
                && self.b.get(self.pos + 1) != Some(&b']')
            {
                self.pos += 1; // '-'
                let (rs2, is_neg2) = self.parse_class_elem()?;
                if rs2.len() == 1 && !is_neg2 {
                    let lo = rs[0].0;
                    let hi = rs2[0].1;
                    if hi < lo {
                        return Err("正则: 字符范围 hi < lo".to_string());
                    }
                    ranges.push((lo, hi));
                } else {
                    return Err("正则: 字符范围右端不能是转义类".to_string());
                }
            } else {
                ranges.extend(rs);
            }
        }
        if ranges.is_empty() {
            return Err("正则语法错误: 空字符类".to_string());
        }
        Ok(Node::Class { ranges, neg })
    }

    /// 字符类元素：单字符、普通转义、或 `\d \w \s` 子类（返回范围集合 + 是否取反）
    fn parse_class_elem(&mut self) -> Result<(Vec<(u8, u8)>, bool), String> {
        let Some(c) = self.peek() else {
            return Err("正则语法错误: 字符类提前结束".to_string());
        };
        if c == b'\\' {
            self.pos += 1;
            let Some(e) = self.peek() else {
                return Err("正则语法错误: 转义提前结束".to_string());
            };
            self.pos += 1;
            let digits = || vec![(b'0', b'9')];
            let word = || vec![(b'0', b'9'), (b'A', b'Z'), (b'a', b'z'), (b'_', b'_')];
            let space = || {
                vec![
                    (b' ', b' '),
                    (b'\t', b'\t'),
                    (b'\n', b'\n'),
                    (b'\r', b'\r'),
                    (b'\x0b', b'\x0b'),
                    (b'\x0c', b'\x0c'),
                ]
            };
            return Ok(match e {
                b'd' => (digits(), false),
                b'w' => (word(), false),
                b's' => (space(), false),
                // 类内 \D \W \S 按字面字母处理（POSIX 语义，避免类内取反歧义）
                _ => {
                    let ch = match e {
                        b'n' => b'\n',
                        b't' => b'\t',
                        b'r' => b'\r',
                        b'0' => 0,
                        b'f' => 0x0c,
                        b'v' => 0x0b,
                        _ => e,
                    };
                    (vec![(ch, ch)], false)
                }
            });
        }
        self.pos += 1;
        Ok((vec![(c, c)], false))
    }

    /// 转义
    fn parse_escape(&mut self) -> Result<Node, String> {
        self.pos += 1; // '\\'
        let Some(e) = self.peek() else {
            return Err("正则语法错误: 转义字符缺失".to_string());
        };
        self.pos += 1;
        let digits = || vec![(b'0', b'9')];
        let word = || vec![(b'0', b'9'), (b'A', b'Z'), (b'a', b'z'), (b'_', b'_')];
        let space = || vec![(b' ', b' '), (b'\t', b'\t'), (b'\n', b'\n'), (b'\r', b'\r'), (b'\x0b', b'\x0b'), (b'\x0c', b'\x0c')];
        Ok(match e {
            b'd' => Node::Class { ranges: digits(), neg: false },
            b'D' => Node::Class { ranges: digits(), neg: true },
            b'w' => Node::Class { ranges: word(), neg: false },
            b'W' => Node::Class { ranges: word(), neg: true },
            b's' => Node::Class { ranges: space(), neg: false },
            b'S' => Node::Class { ranges: space(), neg: true },
            b'n' => Node::Char(b'\n'),
            b't' => Node::Char(b'\t'),
            b'r' => Node::Char(b'\r'),
            b'0' => Node::Char(0),
            b'f' => Node::Char(0x0c),
            b'v' => Node::Char(0x0b),
            b'.' | b'*' | b'+' | b'?' | b'(' | b')' | b'[' | b']' | b'{' | b'}' | b'|' | b'^'
            | b'$' | b'\\' | b'/' => Node::Char(e),
            _ => return Err(format!("正则语法错误: 未知转义 \\{}", e as char)),
        })
    }
}

// ---------------- 匹配（候选列表回溯） ----------------

/// 返回节点从 pos 开始的所有匹配候选（贪心优先排序）
fn match_node(node: &Node, text: &[u8], pos: usize, groups: &Groups) -> Vec<Cand> {
    match node {
        Node::Char(c) => {
            if pos < text.len() && text[pos] == *c {
                vec![(pos + 1, *groups)]
            } else {
                vec![]
            }
        }
        Node::Any => {
            if pos < text.len() && text[pos] != b'\n' {
                vec![(pos + 1, *groups)]
            } else {
                vec![]
            }
        }
        Node::Class { ranges, neg } => {
            if pos < text.len() {
                let ch = text[pos];
                let hit = ranges.iter().any(|&(lo, hi)| ch >= lo && ch <= hi);
                if hit != *neg {
                    vec![(pos + 1, *groups)]
                } else {
                    vec![]
                }
            } else {
                vec![]
            }
        }
        Node::Start => {
            if pos == 0 {
                vec![(pos, *groups)]
            } else {
                vec![]
            }
        }
        Node::End => {
            if pos == text.len() {
                vec![(pos, *groups)]
            } else {
                vec![]
            }
        }
        Node::Alt(branches) => {
            let mut out = vec![];
            for b in branches {
                out.extend(match_node(b, text, pos, groups));
            }
            out
        }
        Node::Group(child, idx) => {
            let mut g = *groups;
            g[*idx] = Some(Span(pos, pos));
            let mut out = vec![];
            for (end, mut gg) in match_node(child, text, pos, &g) {
                gg[*idx] = Some(Span(pos, end));
                out.push((end, gg));
            }
            out
        }
        Node::Rep(child, min, max) => {
            // BFS：out[i] = 匹配 i 次后的候选；贪心优先（次数多者在前）
            let mut out: Vec<(usize, Groups, usize)> = vec![(pos, *groups, 0)];
            let mut frontier: Vec<(usize, Groups)> = vec![(pos, *groups)];
            let mut level = 0;
            loop {
                if *max >= 0 && level >= *max as usize {
                    break;
                }
                let mut next: Vec<(usize, Groups)> = vec![];
                for (p, g) in &frontier {
                    for (end, gg) in match_node(child, text, *p, g) {
                        if end > *p {
                            next.push((end, gg));
                        }
                    }
                }
                if next.is_empty() {
                    break;
                }
                // 去重
                let mut seen: HashSet<(usize, Groups)> = HashSet::new();
                next.retain(|(e, g)| seen.insert((*e, *g)));
                level += 1;
                for (e, g) in &next {
                    out.push((*e, *g, level));
                }
                frontier = next;
            }
            // 贪心优先：匹配次数多者在前（稳定排序保持同级内部顺序）
            out.sort_by(|a, b| b.2.cmp(&a.2));
            out.into_iter()
                .filter(|(_, _, cnt)| *cnt >= *min as usize)
                .map(|(e, g, _)| (e, g))
                .collect()
        }
        Node::Seq(nodes) => {
            let mut cur: Vec<Cand> = vec![(pos, *groups)];
            for n in nodes {
                let mut next: Vec<Cand> = vec![];
                for (p, g) in &cur {
                    next.extend(match_node(n, text, *p, g));
                }
                if next.is_empty() {
                    return vec![];
                }
                cur = next;
            }
            cur
        }
    }
}

/// 匹配结果
#[derive(Debug, Clone)]
pub struct MatchInfo {
    pub start: usize,
    pub end: usize,
    /// groups[0] = 整体匹配跨度，groups[1..] = 捕获组（None 表示未参与）
    pub groups: Vec<Option<Span>>,
}

/// 构造 MatchInfo：groups[0] 填整体跨度
fn to_info(start: usize, end: usize, g: Groups) -> MatchInfo {
    let mut gv = g.to_vec();
    if gv.len() < MAX_GROUPS {
        gv.resize(MAX_GROUPS, None);
    }
    gv[0] = Some(Span(start, end));
    MatchInfo { start, end, groups: gv }
}

/// 已编译正则
pub struct Regex {
    root: Node,
}

impl Regex {
    pub fn new(pattern: &str) -> Result<Regex, String> {
        Ok(Regex { root: compile(pattern)? })
    }

    fn first_from(&self, text: &[u8], start: usize) -> Option<Cand> {
        let g0 = fresh_groups();
        match_node(&self.root, text, start, &g0).into_iter().next()
    }

    /// 整体匹配（隐含 ^...$），返回第一个完整匹配
    pub fn full_match(&self, text: &str) -> Option<MatchInfo> {
        let b = text.as_bytes();
        let g0 = fresh_groups();
        for (end, g) in match_node(&self.root, b, 0, &g0) {
            if end == b.len() {
                return Some(to_info(0, end, g));
            }
        }
        None
    }

    /// 任意位置搜索，返回最左（同位置贪心优先）的第一个匹配
    pub fn search(&self, text: &str) -> Option<MatchInfo> {
        let b = text.as_bytes();
        for start in 0..=b.len() {
            let g0 = fresh_groups();
            if let Some((end, g)) = match_node(&self.root, b, start, &g0).into_iter().next() {
                return Some(to_info(start, end, g));
            }
        }
        None
    }

    /// 全部非重叠匹配（空匹配时前进 1，避免死循环）
    pub fn find_all(&self, text: &str) -> Vec<MatchInfo> {
        let b = text.as_bytes();
        let mut out = vec![];
        let mut pos = 0;
        while pos <= b.len() {
            let mut found: Option<(usize, usize, Groups)> = None;
            for start in pos..=b.len() {
                if let Some((end, g)) = self.first_from(b, start) {
                    found = Some((start, end, g));
                    break;
                }
            }
            let Some((s, e, g)) = found else { break };
            out.push(to_info(s, e, g));
            pos = if e == s { s + 1 } else { e };
        }
        out
    }

    /// 替换所有匹配；repl 支持 $0-$9 捕获组引用、$$ 字面 $（未匹配组替换为空）
    pub fn replace(&self, text: &str, repl: &str) -> String {
        let b = text.as_bytes();
        let mut out: Vec<u8> = Vec::new();
        let mut pos = 0;
        while pos <= b.len() {
            let mut found: Option<(usize, usize, Groups)> = None;
            for start in pos..=b.len() {
                if let Some((end, g)) = self.first_from(b, start) {
                    found = Some((start, end, g));
                    break;
                }
            }
            let Some((s, e, mut g)) = found else {
                out.extend_from_slice(&b[pos..]);
                break;
            };
            g[0] = Some(Span(s, e)); // $0 = 整体匹配
            out.extend_from_slice(&b[pos..s]);
            out.extend_from_slice(&expand_repl(repl, &g, b));
            pos = if e == s { s + 1 } else { e };
            if e == s && pos <= b.len() {
                out.push(b[s]); // 空匹配：保留原字符（标准 re.sub 语义）
            }
        }
        String::from_utf8_lossy(&out).into_owned()
    }

    /// 按模式分割（分隔符被丢弃；忽略空匹配）
    pub fn split(&self, text: &str) -> Vec<String> {
        let b = text.as_bytes();
        let mut out = vec![];
        let mut pos = 0;
        while pos <= b.len() {
            let mut found: Option<(usize, usize)> = None;
            for start in pos..=b.len() {
                if let Some((end, _)) = self.first_from(b, start) {
                    if end > start {
                        found = Some((start, end));
                        break;
                    }
                }
            }
            let Some((s, e)) = found else { break };
            out.push(String::from_utf8_lossy(&b[pos..s]).into_owned());
            pos = e;
        }
        out.push(String::from_utf8_lossy(&b[pos..]).into_owned());
        out
    }
}

/// 展开替换串中的 $0-$9 / $$（groups 中 Span 为文本绝对偏移，可直接切片原文）
fn expand_repl(repl: &str, groups: &Groups, text: &[u8]) -> Vec<u8> {
    let rb = repl.as_bytes();
    let mut out = Vec::new();
    let mut i = 0;
    while i < rb.len() {
        if rb[i] == b'$' && i + 1 < rb.len() {
            let c = rb[i + 1];
            if c == b'$' {
                out.push(b'$');
                i += 2;
                continue;
            }
            if c.is_ascii_digit() {
                let idx = (c - b'0') as usize;
                if idx < MAX_GROUPS {
                    if let Some(Span(s, e)) = groups[idx] {
                        if s <= e && e <= text.len() {
                            out.extend_from_slice(&text[s..e]);
                        }
                    }
                }
                i += 2;
                continue;
            }
        }
        out.push(rb[i]);
        i += 1;
    }
    out
}

#[cfg(test)]
mod tests {
    use super::*;

    fn full(pattern: &str, text: &str) -> bool {
        Regex::new(pattern).unwrap().full_match(text).is_some()
    }

    fn search_groups(pattern: &str, text: &str) -> Vec<Option<(usize, usize)>> {
        Regex::new(pattern)
            .unwrap()
            .search(text)
            .map(|m| m.groups.iter().map(|s| s.map(|x| (x.0, x.1))).collect())
            .unwrap_or_default()
    }

    #[test]
    fn literal_and_anchor() {
        assert!(full("abc", "abc"));
        assert!(!full("abc", "abx"));
        assert!(full("^abc$", "abc"));
        assert!(!full("^abc$", "xabc"));
        assert!(full("a.c", "abc"));
        assert!(full("a.c", "a\nc") == false); // . 不匹配换行
    }

    #[test]
    fn char_class() {
        assert!(full("[a-z]+", "hello"));
        assert!(!full("[a-z]+", "Hello"));
        assert!(full("[^0-9]+", "abc!"));
        assert!(!full("[^0-9]+", "a1b"));
        assert!(full("[a-z0-9_]+", "var_42"));
        assert!(full("\\d+", "12345"));
        assert!(full("\\d+", "12a45") == false);
        assert!(full("\\w+", "abc_123"));
        assert!(full("\\s+", " \t\n"));
    }

    #[test]
    fn quantifier_and_backtrack() {
        assert!(full("a+", "aaa"));
        assert!(full("a{2,3}", "aaaa") == false); // 最多 3 个
        assert!(full("a{2,3}", "aaa"));
        assert!(full("a{2,}", "aaaa"));
        assert!(full("colou?r", "color"));
        assert!(full("colou?r", "colour"));
        // 量词回溯：交替分支选择让整体成功
        assert!(full("(a|ab)+", "ab"));
        assert!(full("(a|ab)+", "abab"));
        assert!(full("a*ab", "aaab"));
    }

    #[test]
    fn group_and_alt() {
        assert!(full("(ab)+", "ababab"));
        assert!(full("cat|dog", "cat"));
        assert!(full("cat|dog", "dog"));
        assert!(!full("cat|dog", "cow"));
        let g = search_groups("(\\w+)@(\\w+)", "mail: foo@bar.com");
        assert_eq!(g[0], Some((6, 13)));
        assert_eq!(g[1], Some((6, 9)));
        assert_eq!(g[2], Some((10, 13)));
    }

    #[test]
    fn find_replace_split() {
        let r = Regex::new("\\d+").unwrap();
        let all = r.find_all("a1b22c333");
        assert_eq!(all.len(), 3);
        assert_eq!((all[0].start, all[0].end), (1, 2));
        assert_eq!((all[2].start, all[2].end), (6, 9));
        assert_eq!(r.replace("a1b22", "[#]"), "a[#]b[#]");
        let g = Regex::new("(\\w+)@(\\w+)").unwrap();
        assert_eq!(g.replace("foo@bar x@y", "$2.$1"), "bar.foo y.x");
        let s = Regex::new("[,\\s]+").unwrap();
        assert_eq!(s.split("a, b  c"), vec!["a", "b", "c"]);
        assert_eq!(s.split(",a,"), vec!["", "a", ""]);
    }

    #[test]
    fn empty_match_and_chinese() {
        let r = Regex::new("x*").unwrap();
        assert_eq!(r.find_all("ab").len(), 3); // 空匹配：a/b/末尾
        // 中文按字节透传，\d 提取数字仍可用
        let r = Regex::new("价格\\d+").unwrap();
        let m = r.search("苹果价格12元").unwrap();
        assert_eq!(&"苹果价格12元"[m.start..m.end], "价格12");
    }

    #[test]
    fn compile_errors() {
        assert!(Regex::new("(ab").is_err());
        assert!(Regex::new("[abc").is_err());
        assert!(Regex::new("a{3,2}").is_err());
        assert!(Regex::new("\\q").is_err());
        assert!(Regex::new("((((((((((a))))))))))").is_err()); // 10 组超限
    }
}
