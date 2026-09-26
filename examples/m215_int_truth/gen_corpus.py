#!/usr/bin/env python3
# ============================================================
# M215 门 · 语料生成器（**单一事实源**）
# ------------------------------------------------------------
# 本脚本是 DOMAIN 的唯一声明处，同时产出：
#   · corpus.px   —— 被测语料（三轨各跑一遍，输出必须与真值逐字节一致）
#   · domain.txt  —— (n d) 逐对清单（给两套**彼此独立**的真值实现读）
#   另打印 DOMAIN 的 .px 列表文本，供漂移检测（verify.sh 比对 corpus.px 里的
#   DOMAIN 是否与本脚本一致 ⇒ 防止"生成器改了、入库语料没重生成"）。
#
# ⚠️ 用 `@DOMAIN@` 占位替换而不是 `%` 格式化 —— 语料里到处是 `%` 运算符，
#    用 `%` 格式化就必须逐个写成 `%%`，极易漏（首版就在注释里的 `%` 上翻车）。
#
# 值域选取理由：缺陷 305 只在 `n - r` 溢出时发作，即 **n 逼近 INT64_MIN**；
#   故值域以 INT64_MIN 邻域为主，同时纳入常规值与全符号组合（正/负/零/±1/±大数），
#   以及 INT64_MIN/2、INT64_MAX/2 等"半个量程"值（覆盖 q 的量级分界）。
#   缺陷 307（>2^53 的比较退化）需要的巨大值也在其中（2^53 附近另有专门用例）。
# ============================================================
import sys

DOMAIN = [
    -9223372036854775808,   # INT64_MIN
    -9223372036854775807,
    -9223372036854775806,
    -9223372036854775805,
    -9223372036854775803,
    -9223372036854775801,
    -4611686018427387904,   # INT64_MIN/2
    -4611686018427387903,
    -1000000000000000001,
    -1000000007,
    -9007199254740993,      # -(2^53+1)：缺陷 307 的判据点
    -9007199254740992,      # -2^53
    -17, -7, -3, -2, -1, 0, 1, 2, 3, 7, 17,
    9007199254740992,       # 2^53
    9007199254740993,       # 2^53+1
    1000000007,
    1000000000000000001,
    4611686018427387903,
    4611686018427387904,    # 2^62
    9223372036854775806,
    9223372036854775807,    # INT64_MAX
]

DOMAIN_PX = "[" + ", ".join(("MINT" if v == -9223372036854775808 else str(v)) for v in DOMAIN) + "]"
PLACEHOLDER = "@DOMAIN@"
# ⚠️ INT64_MIN **不能写成字面量**：词法器按正数解析 `9223372036854775808` ⇒
#   `E1004 无效整数`（与 Go 同款限制：`x := -9223372036854775808` 亦报常量溢出）。
#   故 .px 侧用 `MINT` 这个由 `-9223372036854775807 - 1` 求出的绑定代指。
#   该口径已登记进速查表（事实 240）—— 这是**文档缺口**，不是缺陷。

CORPUS = '''# ============================================================
# M215 门 · 整数运算符的**逐值真值对拍**（第 94 轮 · 缺陷 305 / 306 / 307）
# ------------------------------------------------------------
# ⚠️ 本门存在的理由（先说清"为什么需要一道新门"）：
#   缺陷 305/306 的病灶在**运行时原语内部**，且 `px_idiv`（runtime.c）与 `pxc_vm`
#   的 `PXOP_IDIV` 快路径**各写了一遍同样的公式** ⇒ **三轨跑的是同一份语义**，
#   输出**逐字节一致**。任何「三轨对拍门」按定义看不见它 —— 必须引入**独立真值**。
#   （缺陷 307 恰好相反：那是**三轨分叉**，但老门里没有一道拿大整数比过。）
#   ⇒ 判据 = 三轨彼此逐字节一致 **且** 与 `truth/` 的独立真值逐字节一致。
#
# ⚠️ **为什么叉积里排除了 (INT64_MIN, -1) 这一对**：
#   该对是**唯一**"真商不可表示"的（真商 = +2^63），也是修前 **SIGFPE + core** 的崩点。
#   它由**段 B 专管**（trap1/trap2，定向断言字面值）。若留在段 A 的叉积里，那么
#   "撤回 306 短路"这道负控会让进程在**段 A 中途**崩掉，而 C 的 stdout 是**块缓冲**
#   （静态件不能用 stdbuf 关缓冲）⇒ 段 A 的输出整块丢失、无法与"撤回 305"那道负控解耦。
#   ⇒ 设计取舍：**牺牲 1/930 的叉积覆盖，换取两道负控的独立性**（那一对的覆盖没丢，在段 B）。
#
# 语料由 gen_corpus.py **生成**（单一事实源），请勿手改：
#   python3 gen_corpus.py --out .
# ============================================================

# 值域（与 gen_corpus.py 的 DOMAIN 必须逐字一致；verify.sh 会做漂移检测）
# MINT = INT64_MIN：不能写字面量（E1004），只能算出来（见 gen_corpus.py 注释）
let MINT = -9223372036854775807 - 1
let domain = @DOMAIN@

let mn = MINT
let mx = 9223372036854775807

# ---------- 段 A：逐对 `//` 与 `%`（缺陷 305 · 商溢出） ----------
print("== A-BEGIN ==")
for n in domain:
    for d in domain:
        if d != 0 and not (n == mn and d == -1):
            print("n=" + str(n) + " d=" + str(d) + " q=" + str(n // d) + " r=" + str(n % d))
print("== A-END ==")

# ---------- 段 B：**陷阱回归**（缺陷 306：修前 SIGFPE + core dump） ----------
# 修前 trap1/trap2 会让进程**直接崩掉**（rc=136、core dumped）⇒ 能跑到就说明崩点已收口。
print("== B-BEGIN ==")
print("trap1=" + str(mn // -1))          # INT64_MIN // -1  => 回绕 INT64_MIN（Go 规范）
print("trap2=" + str(mn % -1))           # INT64_MIN % -1   => 0
print("trap3=" + str(-7 // mn))          # -7 // INT64_MIN  => 1
print("trap4=" + str(-7 % mn))           # -7 % INT64_MIN   => 9223372036854775801
print("trap5=" + str(mx // -1))          # INT64_MAX // -1  => -INT64_MAX（本不崩）
print("== B-END ==")

# ---------- 段 D：**比较真值**（缺陷 307：解释轨 int/int 退化为 double） ----------
# 6 个运算符 × 702 对。整数比较是**精确**的（没有浮点尾数位）—— 这正是判据面：
#   `9223372036854775806 < 9223372036854775807` 必须是 true（修前解释轨给 false）。
print("== D-BEGIN ==")
for n in domain:
    for d in domain:
        if d != 0 and not (n == mn and d == -1):
            print("n=" + str(n) + " d=" + str(d) + " lt=" + str(n < d) + " le=" + str(n <= d) + " gt=" + str(n > d) + " ge=" + str(n >= d) + " eq=" + str(n == d) + " ne=" + str(n != d))
print("== D-END ==")

# ---------- 段 C：**不变量自断言**（不依赖真值文件也能自证） ----------
# 判据（欧几里得余数的定义）：0 <= r < |d|。
#   ⚠️ `|d|` 对 d == INT64_MIN 不可表示为正 int64 => 该分支按"任何非负 int64 都 < 2^63"判真。
#   ⚠️ 代数不变量 `a == q*b + r` **不在此断言** —— 边界上 `q*b` 本身会溢出，
#      该公理由 truth/ 的两套独立实现（Go 量级域 + Python bignum）负责自证。
print("== C-BEGIN ==")
var bad = 0
var cnt = 0
for n in domain:
    for d in domain:
        if d != 0 and not (n == mn and d == -1):
            let q = n // d
            let r = n % d
            cnt = cnt + 1
            var rng = r >= 0
            if rng:
                if d > 0:
                    rng = r < d
                elif d == mn:
                    rng = true
                else:
                    rng = r < (0 - d)
            if not rng:
                bad = bad + 1
                print("RNG-VIOL n=" + str(n) + " d=" + str(d) + " q=" + str(q) + " r=" + str(r))
print("== C-END ==")
print("M215-INV-BAD=" + str(bad) + " CNT=" + str(cnt))
print("M215-DONE")
'''


def main():
    out = "."
    args = sys.argv[1:]
    for i, a in enumerate(args):
        if a == "--out" and i + 1 < len(args):
            out = args[i + 1]
    text = CORPUS.replace(PLACEHOLDER, DOMAIN_PX)
    with open(out + "/corpus.px", "w") as f:
        f.write(text)
    with open(out + "/domain.txt", "w") as f:
        for n in DOMAIN:
            for d in DOMAIN:
                if d != 0 and not (n == -9223372036854775808 and d == -1):
                    f.write("%d %d\n" % (n, d))
    print("DOMAIN_PX=%s" % DOMAIN_PX)
    print("pairs=%d (叉积 %d 减去 1 对专管)" % (len(DOMAIN) * (len(DOMAIN) - 1) - 1, len(DOMAIN) * (len(DOMAIN) - 1)))
    print("wrote %s/corpus.px and %s/domain.txt" % (out, out))


if __name__ == "__main__":
    main()
