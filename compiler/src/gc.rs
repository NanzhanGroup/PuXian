//! 普贤 (PuXian) 解释器循环引用回收（M22 追踪式 GC）
//! 背景：解释器值系统用 Arc<Mutex<...>> 表示共享容器（list/dict/struct/chan），
//! 循环引用（如 a.append(a)、d["self"]=d、chan 传自己）使 Arc 计数永不为 0 → 泄漏。
//! 方案：全局注册表存所有容器的 Weak 引用；周期性运行"额外计数根检测 + 标记清扫"：
//!   1) strong = Weak::strong_count（含外部持有者：栈/全局/线程/Rc 闭包环境）
//!   2) indegree = 注册表内其他容器指向本容器的边数
//!   3) extra = strong - indegree；extra > 0 → 存在注册表外持有者 → 活根
//!   4) 从活根 BFS 沿边标记可达；未标记者即纯循环垃圾 → clear() 清空内容断环 → Arc 自然释放
//! 无需扫描线程栈：任何外部持有（主栈/协程栈/定时器/闭包 Rc 环境）都体现为 extra ≥ 1，
//! 天然保护；唯一竞态是"快照后线程新持有 → 清空后读到空容器"（无 use-after-free，良性）。
//! 并发安全：仅当无 spawn 线程活跃（ACTIVE_SPAWNS == 0）时收集，主线程单线程阶段无竞态。

use std::collections::{HashMap, VecDeque};
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{Arc, Mutex, OnceLock, Weak};

use crate::value::ChanState;
use crate::value::Value;

/// 活跃 spawn 线程计数（interp.rs spawn 前后增减；>0 时跳过收集）
pub static ACTIVE_SPAWNS: AtomicI64 = AtomicI64::new(0);

/// 收集阈值：注册容器数累计增量超过该值且无活跃 spawn 时触发一次收集
const GC_TRIGGER: i64 = 20000;

// ==================== 注册表 ====================

#[derive(Clone)]
enum GcWeak {
    List(Weak<Mutex<Vec<Value>>>),
    Dict(Weak<Mutex<HashMap<String, Value>>>),
    Struct(Weak<Mutex<HashMap<String, Value>>>),
    Chan(Weak<ChanState>),
}

struct Entry {
    weak: GcWeak,
    strong: usize,
    indegree: usize,
    reachable: bool,
}

fn registry() -> &'static Mutex<HashMap<usize, Entry>> {
    static M: OnceLock<Mutex<HashMap<usize, Entry>>> = OnceLock::new();
    M.get_or_init(|| Mutex::new(HashMap::new()))
}

/// 累计注册数（触发阈值用）
static ALLOC_COUNT: AtomicI64 = AtomicI64::new(0);

/// 容器地址（注册键）：Arc 指向的堆地址
fn weak_addr(w: &GcWeak) -> usize {
    match w {
        GcWeak::List(a) => std::sync::Weak::as_ptr(a) as usize,
        GcWeak::Dict(a) => std::sync::Weak::as_ptr(a) as usize,
        GcWeak::Struct(a) => std::sync::Weak::as_ptr(a) as usize,
        GcWeak::Chan(a) => std::sync::Weak::as_ptr(a) as usize,
    }
}

/// 任意 Value 指向的注册容器地址（Func/Env 不入注册表 → None）
pub fn value_addr(v: &Value) -> Option<usize> {
    match v {
        Value::List(a) => Some(std::sync::Arc::as_ptr(a) as usize),
        Value::Dict(a) => Some(std::sync::Arc::as_ptr(a) as usize),
        Value::Chan(a) => Some(std::sync::Arc::as_ptr(a) as usize),
        Value::StructInstance { fields, .. } => Some(std::sync::Arc::as_ptr(fields) as usize),
        _ => None,
    }
}

fn insert(weak: GcWeak) {
    // 惰性清理 + 覆盖死条目（地址可能被新 Arc 复用）
    let addr = weak_addr(&weak);
    let mut reg = registry().lock().unwrap();
    if let Some(old) = reg.get(&addr) {
        if old.weak.is_dead() {
            reg.insert(
                addr,
                Entry {
                    weak,
                    strong: 0,
                    indegree: 0,
                    reachable: false,
                },
            );
        }
        // 同地址活条目理论不可达（两个活 Arc 不可能同址）
        return;
    }
    reg.insert(
        addr,
        Entry {
            weak,
            strong: 0,
            indegree: 0,
            reachable: false,
        },
    );
}

/// 阈值触发：无活跃 spawn 时收集；返回是否执行了收集
fn maybe_collect() -> bool {
    if ALLOC_COUNT.fetch_add(1, Ordering::Relaxed) + 1 >= GC_TRIGGER {
        ALLOC_COUNT.store(0, Ordering::Relaxed);
        if ACTIVE_SPAWNS.load(Ordering::SeqCst) == 0 {
            collect();
            return true;
        }
    }
    false
}

pub fn register_list(arc: &Arc<Mutex<Vec<Value>>>) {
    maybe_collect();
    insert(GcWeak::List(Arc::downgrade(arc)));
}
pub fn register_dict(arc: &Arc<Mutex<HashMap<String, Value>>>) {
    maybe_collect();
    insert(GcWeak::Dict(Arc::downgrade(arc)));
}
pub fn register_struct(arc: &Arc<Mutex<HashMap<String, Value>>>) {
    maybe_collect();
    insert(GcWeak::Struct(Arc::downgrade(arc)));
}
pub fn register_chan(arc: &Arc<ChanState>) {
    maybe_collect();
    insert(GcWeak::Chan(Arc::downgrade(arc)));
}

// ==================== 收集 ====================

impl GcWeak {
    fn is_dead(&self) -> bool {
        match self {
            GcWeak::List(w) => w.strong_count() == 0,
            GcWeak::Dict(w) => w.strong_count() == 0,
            GcWeak::Struct(w) => w.strong_count() == 0,
            GcWeak::Chan(w) => w.strong_count() == 0,
        }
    }

    fn strong_count(&self) -> usize {
        match self {
            GcWeak::List(w) => w.strong_count(),
            GcWeak::Dict(w) => w.strong_count(),
            GcWeak::Struct(w) => w.strong_count(),
            GcWeak::Chan(w) => w.strong_count(),
        }
    }

    /// 枚举子容器地址（不 clone Value；锁内遍历；仅统计注册表内地址）
    fn child_addrs(&self, reg: &HashMap<usize, Entry>) -> Vec<usize> {
        let mut out = Vec::new();
        match self {
            GcWeak::List(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(g) = m.lock() {
                        for v in g.iter() {
                            if let Some(a) = value_addr(v) {
                                if reg.contains_key(&a) {
                                    out.push(a);
                                }
                            }
                        }
                    }
                }
            }
            GcWeak::Dict(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(g) = m.lock() {
                        for v in g.values() {
                            if let Some(a) = value_addr(v) {
                                if reg.contains_key(&a) {
                                    out.push(a);
                                }
                            }
                        }
                    }
                }
            }
            GcWeak::Struct(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(g) = m.lock() {
                        for v in g.values() {
                            if let Some(a) = value_addr(v) {
                                if reg.contains_key(&a) {
                                    out.push(a);
                                }
                            }
                        }
                    }
                }
            }
            GcWeak::Chan(w) => {
                if let Some(c) = w.upgrade() {
                    if let Ok(inner) = c.inner.lock() {
                        for v in inner.buf.iter() {
                            if let Some(a) = value_addr(v) {
                                if reg.contains_key(&a) {
                                    out.push(a);
                                }
                            }
                        }
                    }
                }
            }
        }
        out
    }

    /// 清空内容断环（仅对垃圾容器调用）
    fn clear(&self) {
        match self {
            GcWeak::List(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(mut g) = m.lock() {
                        g.clear();
                    }
                }
            }
            GcWeak::Dict(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(mut g) = m.lock() {
                        g.clear();
                    }
                }
            }
            GcWeak::Struct(w) => {
                if let Some(m) = w.upgrade() {
                    if let Ok(mut g) = m.lock() {
                        g.clear();
                    }
                }
            }
            GcWeak::Chan(w) => {
                if let Some(c) = w.upgrade() {
                    if let Ok(mut inner) = c.inner.lock() {
                        inner.buf.clear();
                    }
                }
            }
        }
    }
}

/// 手动/阈值触发收集：仅无活跃 spawn 线程时执行（返回是否执行）
pub fn collect() -> bool {
    if ACTIVE_SPAWNS.load(Ordering::SeqCst) != 0 {
        return false;
    }
    let mut reg = registry().lock().unwrap();
    if reg.is_empty() {
        return true;
    }
    // 1. 清理死条目 + 快照 strong
    let mut dead: Vec<usize> = Vec::new();
    for (&addr, e) in reg.iter_mut() {
        if e.weak.strong_count() == 0 {
            dead.push(addr);
        } else {
            e.strong = e.weak.strong_count();
            e.indegree = 0;
            e.reachable = false;
        }
    }
    for d in &dead {
        reg.remove(d);
    }
    if reg.is_empty() {
        return true;
    }
    // 2. 计算入度
    let addrs: Vec<usize> = reg.keys().cloned().collect();
    for addr in &addrs {
        let children = reg.get(addr).unwrap().weak.child_addrs(&reg);
        for c in children {
            if let Some(e) = reg.get_mut(&c) {
                e.indegree += 1;
            }
        }
    }
    // 3. 活根 = extra > 0（存在注册表外持有者）
    let mut queue: VecDeque<usize> = VecDeque::new();
    for (&addr, e) in reg.iter() {
        if e.strong > e.indegree {
            queue.push_back(addr);
        }
    }
    // 4. BFS 标记可达
    while let Some(addr) = queue.pop_front() {
        if let Some(e) = reg.get_mut(&addr) {
            if e.reachable {
                continue;
            }
            e.reachable = true;
        }
        let children = reg.get(&addr).unwrap().weak.child_addrs(&reg);
        for c in children {
            if let Some(e) = reg.get(&c) {
                if !e.reachable {
                    queue.push_back(c);
                }
            }
        }
    }
    // 5. 清扫：未标记 = 纯循环垃圾 → 断环 → 移出注册表
    let garbage: Vec<usize> = reg
        .iter()
        .filter(|(_, e)| !e.reachable)
        .map(|(&a, _)| a)
        .collect();
    for g in &garbage {
        if let Some(e) = reg.get(g) {
            e.weak.clear();
        }
        reg.remove(g);
    }
    true
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::collections::HashMap;

    #[test]
    fn test_cycle_collect_list_self() {
        // a = []; a.append(a) → 循环；drop a → 收集后应被回收
        let a = Arc::new(Mutex::new(Vec::<Value>::new()));
        register_list(&a);
        {
            let mut g = a.lock().unwrap();
            g.push(Value::List(a.clone()));
        }
        assert_eq!(std::sync::Arc::strong_count(&a), 2); // 外部 + 自引用
        // 模拟 a 离开作用域
        drop(a);
        let mut reg = registry().lock().unwrap();
        reg.retain(|_, e| !e.weak.is_dead());
        drop(reg);
        collect();
        let reg = registry().lock().unwrap();
        assert!(reg.is_empty(), "循环引用应被回收");
    }

    #[test]
    fn test_cycle_collect_dict_self() {
        let d = Arc::new(Mutex::new(HashMap::<String, Value>::new()));
        register_dict(&d);
        {
            let mut g = d.lock().unwrap();
            g.insert("self".into(), Value::Dict(d.clone()));
        }
        drop(d);
        let mut reg = registry().lock().unwrap();
        reg.retain(|_, e| !e.weak.is_dead());
        drop(reg);
        collect();
        let reg = registry().lock().unwrap();
        assert!(reg.is_empty(), "dict 循环引用应被回收");
    }

    #[test]
    fn test_live_object_not_collected() {
        let a = Arc::new(Mutex::new(Vec::<Value>::new()));
        register_list(&a);
        // 外部引用保持 → strong=1, indegree=0, extra=1 → 活
        collect();
        let reg = registry().lock().unwrap();
        assert!(reg.contains_key(&(std::sync::Arc::as_ptr(&a) as usize)), "活对象不应被回收");
    }
}
