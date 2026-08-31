//! M28 P1：时间 / 时区（清歌场景 B 软缺口：日志时间戳、到期计算、TZ 转换）
//!
//! 设计约束：双模式（Rust 解释器 / C 编译）**逐字节一致** → 不依赖系统时区数据库，
//! 全部用纯整数民用日历算法（Howard Hinnant 的 days_from_civil / civil_from_days），
//! C 端 runtime.c 照抄同一算法。时区仅支持 **UTC + 固定偏移**（+HH:MM / -HH:MM），
//! 满足日志/到期/调度需求；命名的 IANA 时区表不在本里程碑范围。
//!
//! API：
//! - `time_format(ts, fmt[, tz]) -> str`：格式化 epoch 秒。tz 默认 "UTC"；支持 "+08:00" / "-05:30"。
//!   指令：%Y %y %m %d %H %M %S %j %a %A %b %B %p %z %Z %%
//! - `time_parse(str, fmt[, tz]) -> int|null`：按 fmt 解析 → epoch 秒（本地时区按 tz 解释）；
//!   解析失败返回 null（不抛错）。支持 %Y %m %d %H %M %S %z。
//! - `tz_offset(tz) -> int`：时区偏移秒（"UTC" → 0；"+08:00" → 28800）

/// 时区偏移：解析 "UTC" / "Z" / "+HH:MM" / "-HH:MM" / "+HHMM" / "+HH" → 秒。
/// 非法返回 None。
pub fn tz_offset(tz: &str) -> Option<i64> {
    let t = tz.trim();
    if t.is_empty() || t.eq_ignore_ascii_case("utc") || t == "Z" {
        return Some(0);
    }
    let bytes = t.as_bytes();
    if bytes.len() < 2 || (bytes[0] != b'+' && bytes[0] != b'-') {
        return None;
    }
    let sign: i64 = if bytes[0] == b'-' { -1 } else { 1 };
    let rest = &t[1..];
    let (hh, mm): (i64, i64) = if let Some(ci) = rest.find(':') {
        let h = rest[..ci].parse::<i64>().ok()?;
        let m = rest[ci + 1..].parse::<i64>().ok()?;
        (h, m)
    } else if rest.len() == 4 {
        let h = rest[..2].parse::<i64>().ok()?;
        let m = rest[2..].parse::<i64>().ok()?;
        (h, m)
    } else if rest.len() == 2 || rest.len() == 1 {
        let h = rest.parse::<i64>().ok()?;
        (h, 0)
    } else {
        return None;
    };
    if !(0..=23).contains(&hh) || !(0..=59).contains(&mm) {
        return None;
    }
    Some(sign * (hh * 3600 + mm * 60))
}

// ---- 民用日历算法（Howard Hinnant, http://howardhinnant.github.io/date_algorithms.html）----

/// days_from_civil(y, m, d) → 1970-01-01 起的天数（负数表示更早）
pub fn days_from_civil(y: i64, m: i64, d: i64) -> i64 {
    let y = if m <= 2 { y - 1 } else { y };
    let era = if y >= 0 { y } else { y - 399 } / 400;
    let yoe = y - era * 400; // [0, 399]
    let mp = (m + 9) % 12; // [0, 11]
    let doy = (153 * mp + 2) / 5 + d - 1; // [0, 365]
    let doe = yoe * 365 + yoe / 4 - yoe / 100 + doy; // [0, 146096]
    era * 146097 + doe - 719468
}

/// civil_from_days(z) → (y, m, d)
pub fn civil_from_days(z: i64) -> (i64, i64, i64) {
    let z = z + 719468;
    let era = if z >= 0 { z } else { z - 146096 } / 146097;
    let doe = z - era * 146097; // [0, 146096]
    let yoe = (doe - doe / 1460 + doe / 36524 - doe / 146096) / 365; // [0, 399]
    let y = yoe + era * 400;
    let doy = doe - (365 * yoe + yoe / 4 - yoe / 100); // [0, 365]
    let mp = (5 * doy + 2) / 153; // [0, 11]
    let d = doy - (153 * mp + 2) / 5 + 1; // [1, 31]
    let m = if mp < 10 { mp + 3 } else { mp - 9 }; // [1, 12]
    (if m <= 2 { y + 1 } else { y }, m, d)
}

/// 从 epoch 秒拆出（本地钟面, 按 tz 偏移）→ (y, mo, d, h, mi, s, 星期几)
/// weekday: 0=周日 1=周一 ... 6=周六（Unix 历 1970-01-01 是周四 → 4）
pub fn civil_breakdown(ts: i64, off: i64) -> (i64, i64, i64, i64, i64, i64, i64) {
    let local = ts + off;
    let days = local.div_euclid(86400);
    let secs = local.rem_euclid(86400);
    let (y, m, d) = civil_from_days(days);
    let h = secs / 3600;
    let mi = (secs % 3600) / 60;
    let s = secs % 60;
    // 1970-01-01 = 周四(4)；days=0 → 4
    let wd = (4 + days).rem_euclid(7);
    (y, m, d, h, mi, s, wd)
}

const WEEKDAYS_SHORT: [&str; 7] = ["Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"];
const WEEKDAYS_FULL: [&str; 7] = [
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday",
];
const MONTHS_SHORT: [&str; 12] = [
    "Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec",
];
const MONTHS_FULL: [&str; 12] = [
    "January", "February", "March", "April", "May", "June", "July", "August", "September",
    "October", "November", "December",
];

/// 年日（1-366）
fn day_of_year(y: i64, m: i64, d: i64) -> i64 {
    days_from_civil(y, m, d) - days_from_civil(y, 1, 1) + 1
}

fn pad2(v: i64) -> String {
    format!("{:02}", v)
}

/// 格式化 epoch 秒 → 字符串。fmt 指令见模块头。
pub fn time_format(ts: i64, fmt: &str, tz: &str) -> String {
    let off = tz_offset(tz).unwrap_or(0);
    let (y, mo, d, h, mi, s, wd) = civil_breakdown(ts, off);
    let sign = if off < 0 { "-" } else { "+" };
    let aoff = off.abs();
    let mut out = String::new();
    let b = fmt.as_bytes();
    let mut i = 0;
    while i < b.len() {
        if b[i] == b'%' && i + 1 < b.len() {
            i += 1;
            match b[i] {
                b'Y' => out.push_str(&format!("{:04}", y)),
                b'y' => out.push_str(&format!("{:02}", y.rem_euclid(100))),
                b'm' => out.push_str(&pad2(mo)),
                b'd' => out.push_str(&pad2(d)),
                b'H' => out.push_str(&pad2(h)),
                b'M' => out.push_str(&pad2(mi)),
                b'S' => out.push_str(&pad2(s)),
                b'j' => out.push_str(&format!("{:03}", day_of_year(y, mo, d))),
                b'a' => out.push_str(WEEKDAYS_SHORT[wd as usize]),
                b'A' => out.push_str(WEEKDAYS_FULL[wd as usize]),
                b'b' | b'h' => out.push_str(MONTHS_SHORT[(mo - 1) as usize]),
                b'B' => out.push_str(MONTHS_FULL[(mo - 1) as usize]),
                b'p' => out.push_str(if h < 12 { "AM" } else { "PM" }),
                b'z' => out.push_str(&format!("{}{:02}{:02}", sign, aoff / 3600, (aoff % 3600) / 60)),
                b'Z' => {
                    if off == 0 {
                        out.push_str("UTC");
                    } else {
                        out.push_str(&format!("{}{:02}{:02}", sign, aoff / 3600, (aoff % 3600) / 60));
                    }
                }
                b'%' => out.push('%'),
                b'I' => {
                    let h12 = h % 12;
                    out.push_str(&pad2(if h12 == 0 { 12 } else { h12 }));
                }
                _ => {
                    out.push('%');
                    out.push(b[i] as char);
                }
            }
        } else {
            out.push(b[i] as char);
        }
        i += 1;
    }
    out
}

/// 解析 epoch 秒的钟面各字段 → (y,mo,d,h,mi,s)（tz 偏移解释）
fn parse_breakdown(ts: i64, off: i64) -> (i64, i64, i64, i64, i64, i64) {
    let (y, mo, d, h, mi, s, _) = civil_breakdown(ts, off);
    (y, mo, d, h, mi, s)
}

/// 按 fmt 解析字符串 → epoch 秒（tz 解释本地钟面）；失败返回 None。
/// 支持指令：%Y %y %m %d %H %I %M %S %z（其余当作字面量跳过匹配）。
/// 未指定字段取默认：%Y 取 1970、%m/%d 取 1、%H/%M/%S 取 0。
pub fn time_parse(s: &str, fmt: &str, tz: &str) -> Option<i64> {
    let off = tz_offset(tz).unwrap_or(0);
    let sb = s.as_bytes();
    let fb = fmt.as_bytes();
    let mut si = 0usize;
    let mut fi = 0usize;
    let mut year: i64 = 1970;
    let mut mon: i64 = 1;
    let mut day: i64 = 1;
    let mut hour: i64 = 0;
    let mut minute: i64 = 0;
    let mut sec: i64 = 0;
    let mut parsed_z: Option<i64> = None;
    while fi < fb.len() {
        if fb[fi] == b'%' && fi + 1 < fb.len() {
            fi += 1;
            match fb[fi] {
                b'Y' => {
                    let (v, ni) = read_int(&sb, si, 4)?;
                    year = v;
                    si = ni;
                }
                b'y' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    year = if v < 69 { 2000 + v } else { 1900 + v };
                    si = ni;
                }
                b'm' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    mon = v;
                    si = ni;
                }
                b'd' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    day = v;
                    si = ni;
                }
                b'H' | b'I' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    hour = v;
                    si = ni;
                }
                b'M' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    minute = v;
                    si = ni;
                }
                b'S' => {
                    let (v, ni) = read_int(&sb, si, 2)?;
                    sec = v;
                    si = ni;
                }
                b'z' | b'Z' => {
                    // 读取 +HH:MM / -HH:MM / +HHMM / UTC
                    let tzpart = &s[si.min(s.len())..];
                    if tzpart.len() >= 3 && (tzpart.starts_with('+') || tzpart.starts_with('-')) {
                        let bytes = tzpart.as_bytes();
                        let end = if bytes.len() > 3 && bytes[3] == b':' {
                            si + 6
                        } else {
                            si + 5
                        };
                        if end <= s.len() {
                            parsed_z = tz_offset(&s[si..end]);
                            si = end;
                        }
                    } else if tzpart.starts_with("UTC") {
                        parsed_z = Some(0);
                        si += 3;
                    }
                }
                _ => {
                    // 未知指令：当作单字符字面量
                    if si < sb.len() && sb[si] == fb[fi] {
                        si += 1;
                    }
                }
            }
        } else {
            // 字面量：跳过空格需匹配，其余要求相同字符
            if fb[fi] == b' ' {
                while si < sb.len() && sb[si] == b' ' {
                    si += 1;
                }
            } else {
                if si >= sb.len() || sb[si] != fb[fi] {
                    return None;
                }
                si += 1;
            }
        }
        fi += 1;
    }
    if mon < 1 || mon > 12 || day < 1 || day > 31 || hour > 23 || minute > 59 || sec > 60 {
        return None;
    }
    let use_off = parsed_z.unwrap_or(off);
    let days = days_from_civil(year, mon, day);
    let utc = days * 86400 + hour * 3600 + minute * 60 + sec - use_off;
    // 校验日真实存在（round-trip）
    let (y2, m2, d2) = civil_from_days(days);
    if y2 != year || m2 != mon || d2 != day {
        return None;
    }
    Some(utc)
}

/// 读取字符串中连续数字（最多 max 位），返回 (值, 结束下标)
fn read_int(b: &[u8], start: usize, max: usize) -> Option<(i64, usize)> {
    let mut i = start;
    while i < b.len() && b[i].is_ascii_digit() && (i - start) < max {
        i += 1;
    }
    if i == start {
        return None;
    }
    let v: i64 = std::str::from_utf8(&b[start..i]).ok()?.parse().ok()?;
    Some((v, i))
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tz_offset() {
        assert_eq!(tz_offset("UTC"), Some(0));
        assert_eq!(tz_offset("+08:00"), Some(28800));
        assert_eq!(tz_offset("-05:30"), Some(-19800));
        assert_eq!(tz_offset("+0800"), Some(28800));
        assert_eq!(tz_offset("+8"), Some(28800));
        assert_eq!(tz_offset("bogus"), None);
    }

    #[test]
    fn test_format_utc() {
        // 1970-01-01 00:00:00 UTC = 0（周四）
        assert_eq!(time_format(0, "%Y-%m-%d %H:%M:%S", "UTC"), "1970-01-01 00:00:00");
        assert_eq!(time_format(0, "%a %A", "UTC"), "Thu Thursday");
        assert_eq!(time_format(0, "%b %B", "UTC"), "Jan January");
        assert_eq!(time_format(0, "%j", "UTC"), "001");
        assert_eq!(time_format(0, "%z %Z", "UTC"), "+0000 UTC");
        // 2024-03-08 09:30:45 UTC = 1709890245
        assert_eq!(
            time_format(1709890245, "%Y-%m-%d %H:%M:%S %a", "UTC"),
            "2024-03-08 09:30:45 Fri"
        );
    }

    #[test]
    fn test_format_tz() {
        // 1709890245 UTC → +08:00 = 17:30:45
        assert_eq!(
            time_format(1709890245, "%Y-%m-%d %H:%M:%S %z", "+08:00"),
            "2024-03-08 17:30:45 +0800"
        );
        // -05:30 → 04:00:45
        assert_eq!(
            time_format(1709890245, "%H:%M:%S", "-05:30"),
            "04:00:45"
        );
    }

    #[test]
    fn test_parse_roundtrip() {
        assert_eq!(time_parse("2024-03-08 09:30:45", "%Y-%m-%d %H:%M:%S", "UTC"), Some(1709890245));
        assert_eq!(time_parse("2024-03-08 17:30:45", "%Y-%m-%d %H:%M:%S", "+08:00"), Some(1709890245));
        // 带 %z
        assert_eq!(
            time_parse("2024-03-08 17:30:45 +0800", "%Y-%m-%d %H:%M:%S %z", "UTC"),
            Some(1709890245)
        );
        assert_eq!(time_parse("bad", "%Y-%m-%d", "UTC"), None);
    }
}
