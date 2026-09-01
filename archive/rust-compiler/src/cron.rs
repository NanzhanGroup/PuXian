//! M28 P1：cron 定时调度（清歌场景 B：ws-backup、自愈扫描、知识库刷新）
//!
//! 6 字段 cron 表达式：`秒 分 时 日 月 周`（周 0/7=周日，1-6=周一至周六）
//! 字段语法：`*` / `*/n` / `a,b,c` / `a-b` / `a-b/n` / 固定值
//! 语义：日与周都显式受限时（都非 *）取 **OR**（任一匹配即触发，Vixie cron 标准）；
//!       仅一个受限就只检查该字段。
//!
//! 本模块只做 **解析 + 匹配**（纯逻辑，Rust/C 双实现同一算法）；
//! 任务注册与回调执行在 builtin.rs 的 `cron(expr, fn, ...args)`（fork 解释器 +
//! 后台线程每秒 tick，clear_timer(id) 统一取消）。C 端 runtime.c 同构实现。

/// cron 表达式 → 6 字段位集合
#[derive(Debug, Clone, Default)]
pub struct CronSpec {
    pub sec: Vec<bool>,   // len 60
    pub min: Vec<bool>,   // len 60
    pub hour: Vec<bool>,  // len 24
    pub dom: Vec<bool>,   // len 32 (idx 1..=31)
    pub mon: Vec<bool>,   // len 13 (idx 1..=12)
    pub dow: Vec<bool>,   // len 7  (idx 0=Sun..6=Sat)
    /// dom 是否显式受限（非 *）
    pub dom_limited: bool,
    /// dow 是否显式受限（非 *）
    pub dow_limited: bool,
}

fn parse_field(expr: &str, min: i64, max: i64) -> Option<Vec<bool>> {
    let mut bits = vec![false; (max - min + 1) as usize];
    let mut any = false;
    for part in expr.split(',') {
        let p = part.trim();
        if p.is_empty() {
            return None;
        }
        if p == "*" {
            for v in min..=max {
                bits[(v - min) as usize] = true;
            }
            any = true;
            continue;
        }
        // 解析 [a-b]/step
        let (range, step) = match p.split('/').collect::<Vec<_>>()[..] {
            [r] => (r, 1i64),
            [r, s] => (r, s.parse::<i64>().ok()?),
            _ => return None,
        };
        if step <= 0 {
            return None;
        }
        let (lo, hi) = if range == "*" {
            (min, max)
        } else if let Some(dash) = range.find('-') {
            let a: i64 = range[..dash].trim().parse().ok()?;
            let b: i64 = range[dash + 1..].trim().parse().ok()?;
            (a, b)
        } else {
            let a: i64 = range.trim().parse().ok()?;
            (a, a)
        };
        if lo < min || hi > max || lo > hi {
            return None;
        }
        let mut v = lo;
        while v <= hi {
            bits[(v - min) as usize] = true;
            v += step;
        }
        any = true;
    }
    if !any {
        return None;
    }
    Some(bits)
}

/// 解析 cron 表达式（6 字段）。失败 → Err(msg)。
pub fn parse_cron(expr: &str) -> Result<CronSpec, String> {
    let parts: Vec<&str> = expr.split_whitespace().collect();
    if parts.len() != 6 {
        return Err(format!(
            "cron 表达式需要 6 个字段（秒 分 时 日 月 周），得到 {}: {:?}",
            parts.len(),
            expr
        ));
    }
    let sec = parse_field(parts[0], 0, 59).ok_or_else(|| format!("cron 秒字段非法: {}", parts[0]))?;
    let min = parse_field(parts[1], 0, 59).ok_or_else(|| format!("cron 分字段非法: {}", parts[1]))?;
    let hour = parse_field(parts[2], 0, 23).ok_or_else(|| format!("cron 时字段非法: {}", parts[2]))?;
    let dom_raw = parse_field(parts[3], 1, 31).ok_or_else(|| format!("cron 日字段非法: {}", parts[3]))?;
    let mon_raw = parse_field(parts[4], 1, 12).ok_or_else(|| format!("cron 月字段非法: {}", parts[4]))?;
    let dow = parse_field(parts[5], 0, 7).ok_or_else(|| format!("cron 周字段非法: {}", parts[5]))?;
    // 日/月：parse_field 用相对索引（min=1）→ 转绝对索引（1..=31 / 1..=12）
    let mut dom = vec![false; 32];
    for i in 1..=31 {
        dom[i] = dom_raw[i - 1];
    }
    let mut mon = vec![false; 13];
    for i in 1..=12 {
        mon[i] = mon_raw[i - 1];
    }
    // 周 7 = 周日 0
    let mut dow7 = vec![false; 7];
    for i in 0..8 {
        if dow[i] {
            dow7[(i % 7) as usize] = true;
        }
    }
    let dom_limited = parts[3].trim() != "*";
    let dow_limited = parts[5].trim() != "*";
    Ok(CronSpec {
        sec,
        min,
        hour,
        dom,
        mon,
        dow: dow7,
        dom_limited,
        dow_limited,
    })
}

/// 判断给定 epoch 秒是否匹配 cron（off = 本地时区偏移秒）
pub fn cron_match(spec: &CronSpec, ts: i64, off: i64) -> bool {
    let (_, mo, d, h, mi, s, wd) = crate::tztime::civil_breakdown(ts, off);
    if !spec.sec[s as usize]
        || !spec.min[mi as usize]
        || !spec.hour[h as usize]
        || !spec.mon[mo as usize]
    {
        return false;
    }
    let dom_ok = spec.dom[d as usize];
    let dow_ok = spec.dow[wd as usize];
    if spec.dom_limited && spec.dow_limited {
        dom_ok || dow_ok
    } else if spec.dom_limited {
        dom_ok
    } else if spec.dow_limited {
        dow_ok
    } else {
        true
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_parse_fields() {
        let s = parse_cron("*/5 * * * * *").unwrap();
        assert!(s.sec[0] && s.sec[5] && s.sec[10] && !s.sec[1]);
        let s = parse_cron("0 0 3 * * 1-5").unwrap();
        assert!(s.hour[3] && !s.hour[2]);
        assert!(s.dow[1] && s.dow[5] && !s.dow[6]);
        assert!(s.dow_limited && !s.dom_limited);
        let s = parse_cron("0,30 0 0 1,15 * *").unwrap();
        assert!(s.sec[0] && s.sec[30] && s.dom[1] && s.dom[15] && !s.dom[2]);
        assert!(parse_cron("bad").is_err());
        assert!(parse_cron("0 0 * * *").is_err()); // 5 字段（需 6 字段）
    }

    #[test]
    fn test_match_sec() {
        // 每 5 秒（"*/5 * * * * *"）
        let spec = parse_cron("*/5 * * * * *").unwrap();
        // 2024-03-08 09:30:45 UTC = 1709890245
        assert!(cron_match(&spec, 1709890245, 0));
        assert!(!cron_match(&spec, 1709890246, 0));
        // 每分钟第 30 秒
        let spec = parse_cron("30 * * * * *").unwrap();
        assert!(cron_match(&spec, 1709890230, 0));
        assert!(!cron_match(&spec, 1709890231, 0));
    }

    #[test]
    fn test_dom_dow_or() {
        // 日=1 且 周=周日 → OR：任一命中即触发
        let spec = parse_cron("0 0 0 1 * 0").unwrap();
        // 2024-09-01 是周日 → 命中（dow）1725148800
        assert!(cron_match(&spec, 1725148800, 0));
        // 2024-10-01 是周二 → 命中（dom）1727740800
        assert!(cron_match(&spec, 1727740800, 0));
        // 2024-09-15 是周日 → 命中（dow）1726358400
        assert!(cron_match(&spec, 1726358400, 0));
        // 2024-09-10 是周二 → 不命中 1725926400
        assert!(!cron_match(&spec, 1725926400, 0));
    }

    #[test]
    fn test_tz_match() {
        // 每天 08:00 北京时区 → UTC 00:00
        let spec = parse_cron("0 0 8 * * *").unwrap();
        // 2024-03-08 08:00:00 +08:00 = 2024-03-08 00:00:00 UTC = 1709856000
        assert!(cron_match(&spec, 1709856000, 28800));
        // UTC 07:00 不匹配（北京 15:00）
        assert!(!cron_match(&spec, 1709852400, 28800));
    }
}
