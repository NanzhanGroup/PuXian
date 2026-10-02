# 表元素指针契约（`docs/TABLE_PTR_CONTRACT.md`）

> 立此文档于 **M242（第 119 轮）**，起因是 M240（缺陷 399）那次真实崩溃 ——
> 崩点在 `px_pxpend_enter`，根因却是「**表在锁外被搬走了**」。
> 本轮把那个教训从「一个缺陷的修复」升格为**一类结构的判据**。

---

## 1 问题：`xrealloc` 增长的表，元素地址**跨锁即失效**

`runtime/` 里有三张**按 fd 索引**的表：

| 表 | 扩容函数 | 互斥锁 |
|---|---|---|
| `g_conns`（连接上下文） | `px_evc_ensure` | **`g_conn_mu`**（注意：不是 `g_conns_mu`） |
| `g_hpend`（handler 挂起项） | `http_pend_ensure` | `g_hpend_mu` |
| `g_pxpend`（px_serve 挂起项） | `px_pxpend_ensure` | `g_pxpend_mu` |

三者都是 `xrealloc(g_X, ncap * sizeof(...))` **几何增长**。关键事实：

* 数组 **> 128 KB** 时 glibc 的 `realloc` 走 **`mmap`**（而不是原地 `brk` 扩展）
  ⇒ **整块搬家 + 旧块 `munmap`**；
* 这个动作由**任意线程**、在它自己的锁内、以**更大的 fd** 触发；
* 于是**别人手里那个 `&g_X[fd]`** 就成了野指针。

后果分两种，**后者更危险**：

| 动作 | 表现 |
|---|---|
| **读**它 | `SEGV`（崩得快，容易查） |
| **写**它 | **静默堆破坏** —— 写进已 `munmap`/被复用的内存，症状在别处、很久以后才发作 |

> M240 实测：`px_pxpend_enter` 在**锁外**做 TLS 握手（可能几百毫秒），期间任何线程
> 以更大 fd 进来就把它手里的 `e` 变成野指针 ⇒ 读 `+4` 偏移 SEGV / 写 `e->conn` 静默破坏。

**对照：固定表不受影响。** `g_sse_conns`（一次性 `xcalloc` + `inited` 早退）、
`g_px_buses`（`PX_BUS_MAX` 定长）、`g_threads`（注释明说「此后指针恒定」）、
`g_ops`（`static const` 编译期常量数组）—— 它们的元素地址**恒定**，返回指针是安全的。

---

## 2 契约

**凡「会搬家的表」的元素地址外泄口，必须登记契约。**

| 契约 | 含义 |
|---|---|
| `locked-only` | 调用方**必须**持 `<表>_mu`；需要**跨锁**取得字段时，一律用**专用访问器**（锁内重新解析后**按值**返回），**不得**把指针带出锁 |
| `null-only` | 可跨锁使用，但调用方**只准判空**，**不得**解引用 |

登记表：`examples/m242_table_ptr/CONTRACT.tsv`（**表 / 访问器 / 契约 / 互斥锁 / 依据**）。

**跨锁取值怎么做**（`g_pxpend` 的既有范例）：

```c
// ✅ 锁内重新解析，按值返回
static PxConn* px_pxpend_conn_of(int fd) {
    PxConn* c = NULL;
    pthread_mutex_lock(&g_pxpend_mu);
    PxPend* e = px_pxpend_ctx(fd);
    if (e) c = e->conn;
    pthread_mutex_unlock(&g_pxpend_mu);
    return c;
}
// ❌ 反面：锁内取指针、解锁后解引用
```

---

## 3 审计器

```
python3 selfhost/table_ptr_audit.py --root .        # 人工可读
python3 selfhost/table_ptr_audit.py --root . --json
python3 selfhost/table_ptr_audit.py --self-test     # 14 锚点（含反向判据）
```

**判据三层**（全部**源码派生**，不手抄名单）：

1. **会搬家的表** ← 从 `xrealloc(g_X, …)` 派生（实测 8 张）；
2. **外泄点** ← 三种形状：
   * 形状 1 `return &g_X[...]`（6 处）
   * 形状 2 `return <accessor>(...)`（当前 0 处）
   * 形状 3 `T* v = <accessor>(...); … return v;` ← ⚠️ **盲区形状**，
     只扫形状 1 的扫描器**看不见它**（M242 缺陷 403 就在此）
3. **契约核验** ← `locked-only` 的每个调用点必须被 `lock(&<表>_mu)` … `unlock` 夹住。

**退出码**：`0` 干净 · `1` 违例 · `3` **判据自身失效**（契约表缺失 / 派生集过小）——
最后这条是刻意的：**拒绝静默通过**。

---

## 4 ⚠️ 两个「判据自己的坑」（M242 缺陷 404 / 405）

写判据时踩的，记在这里因为它们**看起来都完全合理**：

### 4.1 锁名**不得由表名推断**

`g_conns` 表的锁叫 **`g_conn_mu`**（少一个 `s`）。
首版按 `表名 + '_mu'` 推断 ⇒ 生成 `g_conns_mu` ⇒ 永远匹配不到 ⇒
**把 11 个完全正确的调用点全报成「未持锁」**。

⇒ 契约表**显式登记锁名**，并校验它在源码里真实存在（`static pthread_mutex_t <名> =`）。

### 4.2 分支内早退的 `unlock` **不算解锁**

```c
pthread_mutex_lock(&g_conn_mu);
if (px_evc_ensure(fd) != 0) { pthread_mutex_unlock(&g_conn_mu); return -1; }
PxConnCtx* c = px_evc_ctx(fd);      /* 主路径：仍然持锁 */
```

行级扫描往上找，先看到那个 `unlock` ⇒ 判「锁外」= 假阳。
⇒ 判据：**含 `if (` 或 `return` 的 unlock 行跳过**，继续往上找。
（保守方向不变：真「锁外调用」往上只会遇到裸 `unlock` 或函数顶格 `}` ⇒ 仍判红。）

> 教训与 M212/M213 同款：**判据报到「找不到」时，先怀疑判据。**
> 这两条假阳合计 **12 项**，若照单全收就会去「修」一个并不存在的缺陷。

---

## 5 覆盖边界（如实）

* **形状 2** 当前 0 处 ⇒ 判据在位但**无样本**（未经实测）。
* **`null-only`** 契约档本表为空 ⇒ 「调用方只准判空」这条核验**未实现**（留待有样本时）。
* 审计器只覆盖 `runtime/*.c`；`tools/` 与 `stdlib/` 不在此面内（它们不含本类 fd 索引表）。
* 行级持锁分析**不理解控制流**（见 §4.2）—— 它对本仓「访问器调用点均在锁内」的既有风格够用，
  但**不是**通用的数据流分析（M235s1 的「分配器配对」通用审计器就因此放弃过）。
