//! M31 沙箱安全：`sandbox_enter(opts)` —— 内存限制 / 危险函数禁限 / 权限降级
//!
//! 进程级沙箱（解释器 Rust 与编译 C 同一语义，双模式一致）：
//! - `memory_mb`：内存上限。`setrlimit(RLIMIT_AS)` = 当前 VSS + memory_mb（新增上限），
//!   防止脚本无限分配内存拖垮宿主（pxdb / WebServer 多租户场景）。
//! - `deny`：禁用的内置函数名列表（如 `["os_spawn", "os_kill", "write_file"]`），
//!   之后调用这些函数报错 `沙箱：函数 X 已被禁用`（R2201）。
//! - `drop_priv`：root 降权到 nobody(uid/gid 65534)。非 root 请求降权 → 返回 false（不生效）。
//!
//! 说明：deny 检查在解释器 `call_builtin` 入口 / C 运行时 `px_call` native 分派处统一执行，
//! 覆盖解释器模式与编译模式全部内置函数调用路径。

use std::collections::HashSet;
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::{Mutex, OnceLock};

fn deny_set() -> &'static Mutex<HashSet<String>> {
    static M: OnceLock<Mutex<HashSet<String>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashSet::new()))
}

static ACTIVE: AtomicBool = AtomicBool::new(false);

/// 沙箱是否已激活（deny 表已设置）
pub fn sandbox_active() -> bool {
    ACTIVE.load(Ordering::Relaxed)
}

/// 内置函数调用前的沙箱检查：函数名在 deny 表 → Err（调用方转为 R2201 报错）
pub fn check(name: &str) -> Result<(), String> {
    if !ACTIVE.load(Ordering::Relaxed) {
        return Ok(());
    }
    if deny_set().lock().unwrap().contains(name) {
        return Err(format!("沙箱：函数 {} 已被禁用", name));
    }
    Ok(())
}

/// 当前进程虚拟地址空间（VSS）字节数（/proc/self/statm 第 1 字段 × 页大小）
fn current_vss_bytes() -> u64 {
    if let Ok(s) = std::fs::read_to_string("/proc/self/statm") {
        if let Some(f) = s.split_whitespace().next() {
            if let Ok(pages) = f.parse::<u64>() {
                return pages.saturating_mul(4096);
            }
        }
    }
    0
}

/// 执行沙箱进入。参数已在 builtin 层解析校验：
/// - `memory_mb`：新增内存上限（MB）
/// - `deny`：禁用的内置函数名
/// - `drop_priv`：是否降权到 nobody
/// 返回 Ok(true) 成功 / Ok(false) 环境不满足（非 root 请求降权）/ Err 失败（含文案）
pub fn enter(memory_mb: Option<u64>, deny: &[String], drop_priv: bool) -> Result<bool, String> {
    // drop_priv 前置检查：非 root 且请求降权 → 直接 false（不做任何修改，可被调用方感知）
    if drop_priv && unsafe { libc::geteuid() } != 0 {
        return Ok(false);
    }
    // 1. 内存限制：RLIMIT_AS = 当前 VSS + memory_mb
    if let Some(mb) = memory_mb {
        let vss = current_vss_bytes();
        let limit = vss.saturating_add(mb.saturating_mul(1024 * 1024));
        let rl = libc::rlimit {
            rlim_cur: limit,
            rlim_max: limit,
        };
        let rc = unsafe { libc::setrlimit(libc::RLIMIT_AS, &rl) };
        if rc != 0 {
            return Err(format!("沙箱：设置内存上限失败（memory_mb={}）", mb));
        }
    }
    // 2. 危险函数禁限：填充 deny 表并激活
    if !deny.is_empty() {
        let mut s = deny_set().lock().unwrap();
        for n in deny {
            s.insert(n.clone());
        }
    }
    ACTIVE.store(true, Ordering::Relaxed);
    // 3. 权限降级：root → nobody(65534)。setgid 先于 setuid（setuid 后无法再改组）。
    if drop_priv {
        unsafe {
            libc::setgid(65534);
            libc::setuid(65534);
        }
    }
    Ok(true)
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_check_disabled() {
        enter(None, &["os_spawn".to_string()], false).unwrap();
        assert!(sandbox_active());
        assert!(check("os_spawn").is_err());
        assert!(check("os_pid").is_ok());
    }

    #[test]
    fn test_enter_memory_limit() {
        // memory_mb 会设置进程级 RLIMIT_AS（影响整个测试进程），这里用大值验证不 panic
        // 且返回 Ok(true)；不验证真实超限（会 OOM 进程）
        let r = enter(Some(8192), &[], false);
        assert!(r.is_ok());
    }
}
