#!/usr/bin/env python3
# ============================================================
# M196 判据 [S7]：「同码文案」的三轨单一真相（静态对拍 + 棘轮基线）
# ------------------------------------------------------------
# 口径（docs/ERROR_CODES.md §6.7）：
#   **同一操作**在两边的报错**正文必须逐字相同** —— 解释轨的守卫**先于**原生触发，
#   所以解释轨的每条文案都必须是「原生侧也会给的那一条」，否则**同一坏输入三轨两文**。
# 取两边所有 `px_error("R####: <msg>")`（runtime/*.c）与 `i_r####("<msg>")`（selfhost/*.px），
# 按「**首 token**（函数名）」配对，逐条判断「解释轨文案 ⇄ 原生文案」是否等价。
#   等价规则（三条，逐条放宽）：
#     ① 逐字相同；
#     ② 解释轨是**拼接式**（`"前缀" + 值`）⇒ 与原生侧的**格式串模板**等价：
#        模板里的 `%xxx` 换成通配，要求解释轨字面量是模板的**前缀**，余下只有格式串/标点；
#     ③ 模板**以格式串开头**（如 `%s 的 %s 需要整数，实际是 %s`）⇒ 先用函数名实例化第一个
#        `%s` 再套用 ②。
# 棘轮：BASELINE 记录「**已判定为真差异**」的 (函数 → 解释轨文案清单)；
#   当前差异 ⊆ 基线 ⇒ 绿；出现基线外的新差异 ⇒ **判红**；基线里已消失的条目 ⇒ ℹ️ 提示删除。
# 用法：python3 scan_msgs.py [--root <仓库根>] [--list]   退出码 0=绿 1=红
# ============================================================
import collections, os, re, sys

ROOT = "/data/code/puxian"
SHOW = False
for i, a in enumerate(sys.argv):
    if a == "--root" and i + 1 < len(sys.argv):
        ROOT = sys.argv[i + 1]
    if a == "--list":
        SHOW = True

NAT = re.compile(r'px_error\(\s*"(R\d{4}):\s*([^"]*)"')
INT = re.compile(r'i_r(\d{4})\(\s*"([^"]*)"')
SPEC = re.compile(r'%[-0-9.]*[a-zA-Z]+')
TAIL = re.compile(r'^[\s，,：:（）()【】\[\]…。]*(' + SPEC.pattern + r'|[0-9]+)?[\s，,：:（）()【】\[\]…。]*$')

# ── 棘轮基线（M196 建立）：**已判定为真差异**的函数 → 解释轨独有的文案（无格式串部分）
#    ⚠️ 每条都必须是「真的不同」或「已核实为等价但静态器看不见」——见 FMT_OK 的理由。
BASELINE = {}        # M196 收口后**已清空**：此后任何新增的「解释轨独有文案」立即判红
# ── FMT_OK：**已核实等价、但静态器看不见**的条目（每条必须给理由 + 由门的动态判据钉住）──
#   背景：原生侧有些文案由**公共 helper 的格式串模板**产出（`math_num` / `px_arg_int`），
#   模板里的 `%s` 在运行期填入**函数名**/**参数名**，而解释轨是**字符串拼接** ——
#   静态器无法把 `range 的 end 需要整数，实际是` 与模板 `%s 的 %s 需要整数，实际是 %s` 对上。
FMT_OK = {
    "range": {
        "msgs": {"range 的 start 需要整数，实际是", "range 的 end 需要整数，实际是",
                 "range 的 step 需要整数，实际是"},
        "why": "原生侧由 `px_arg_int(args[i], \"range\", \"start\"/\"end\"/\"step\")` 产出，"
               "模板 `%s 的 %s 需要整数，实际是 %s`（**第二个** %s 是参数名，静态器无法实例化）"
               " —— 实测三轨同文（门 [2] 的 f3/f4/f5 钉住）",
    },
    "atan2": {
        "msgs": {"atan2 参数必须是数字，实际是"},
        "why": "原生侧由 `math_num(v, \"atan2\")` 产出，模板 `%s 参数必须是数字，实际是 %s`"
               "（以格式串**开头** ⇒ head 为空、静态器看不见）—— 实测三轨同文（门 [2] 的 f6 钉住）",
    },
}


def head(msg):
    m = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)", msg.strip())
    return m.group(1) if m else ""


def collect_native():
    out = collections.defaultdict(set)
    for f in sorted(os.listdir(os.path.join(ROOT, "runtime"))):
        if not f.endswith(".c"):
            continue
        for ln, line in enumerate(open(os.path.join(ROOT, "runtime", f), encoding="utf-8"), 1):
            st = line.lstrip()
            if st.startswith("//") or st.startswith("*") or st.startswith("/*"):
                continue
            for m in NAT.finditer(line):
                out[head(m.group(2))].add((m.group(1), m.group(2).strip(), "%s:%d" % (f, ln)))
    return out


def collect_interp():
    out = collections.defaultdict(set)
    for f in ("selfhost/ibuiltin.px", "selfhost/icall.px", "selfhost/ival.px",
              "selfhost/iexpr.px", "selfhost/istmt.px"):
        p = os.path.join(ROOT, f)
        if not os.path.exists(p):
            continue
        for ln, line in enumerate(open(p, encoding="utf-8"), 1):
            if line.lstrip().startswith("#"):
                continue
            for m in INT.finditer(line):
                out[head(m.group(2))].add((m.group(1), m.group(2).strip(), "%s:%d" % (f, ln)))
    return out


def covered(t, n, fn):
    """解释轨字面量 t ⇄ 原生消息 n（可能含格式串）是否等价"""
    if n == t:
        return True
    cands = [n]
    if n.startswith("%s"):                    # 规则 ③：模板以格式串开头 ⇒ 用函数名实例化
        cands.append(SPEC.sub(fn, n, count=1))
    for c in cands:
        if c.startswith(t) and TAIL.match(c[len(t):]):
            return True
    return False


def compute():
    nat, itr = collect_native(), collect_interp()
    common = sorted((set(nat) & set(itr)) - {""})
    cur = {}
    # 全局**格式串模板**（head 为空，如 `%s 参数必须是数字，实际是 %s` —— 由 math_num 等
    #   「把函数名当参数」的公共 helper 产出）⇒ 用函数名实例化后对所有函数都适用。
    global_tmpl = sorted(set(x[1] for x in nat.get("", set())))
    for fn in common:
        ns = [x[1] for x in nat[fn]] + global_tmpl
        miss = []
        for x in sorted(itr[fn]):
            t = x[1]
            if not any(covered(t, n, fn) for n in ns):
                miss.append(t)
        ok = set(FMT_OK.get(fn, {}).get("msgs", set()))
        miss = [m for m in miss if m not in ok]
        if miss:
            cur[fn] = miss
    return common, cur


def main():
    common, cur = compute()
    if SHOW or BASELINE is None:
        print("两边都有文案的函数 %d · **不一致** %d\n" % (len(common), len(cur)))
        for fn in sorted(cur):
            print("── %-20s %s" % (fn, " | ".join(cur[fn])))
        return []
    fails, notes = [], []
    base = {k: set(v) for k, v in BASELINE.items()}
    for fn, msgs in sorted(cur.items()):
        allowed = base.get(fn, set())
        for msg in msgs:
            if msg not in allowed:
                fails.append("函数 %s 出现**基线外**的解释轨独有文案「%s」—— §6.7：同一操作必须逐字相同"
                             "（确需豁免请在 BASELINE 里加该条并给理由）" % (fn, msg))
    for fn in sorted(base):
        if fn not in cur:
            notes.append("%s 已完全对齐 ⇒ 可从 BASELINE 删除（棘轮只许减少）" % fn)
        else:
            for msg in sorted(base[fn] - set(cur[fn])):
                notes.append("%s 的「%s」已对齐 ⇒ 可从 BASELINE 删除" % (fn, msg))
    print("── [S7] 文案对拍：可比函数 %d · 当前不一致 %d · 基线函数 %d · 新增 0/%d ──"
          % (len(common), len(cur), len(base), len(fails)))
    for n in notes:
        print("   ℹ️", n)
    return fails


if __name__ == "__main__":
    f = main()
    for x in f:
        print("❌", x)
    sys.exit(1 if f else 0)
