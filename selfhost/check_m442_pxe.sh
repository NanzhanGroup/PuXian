#!/usr/bin/env bash
# ============================================================
# selfhost/check_m442_pxe.sh —— .pxe 序列化模块（M442 阶段2）判据
# ------------------------------------------------------------
# 判据面：格式契约 / 三轨等价 / 序列化覆盖（闭包·结构体·内嵌 NUL）/ 安全边界负控。+ B 自举边界
#   B 自举边界（M442-S3 · 2026-11-24 CI 红）：runtime 带 main 的宿主唯一（pxvm.c）·
#     native_bootstrap.sh 排除表在 · 负控「抠掉排除分支」⇒ 自举就地 rc=3（证明判据有牙）
# 纪律（每条都要能判红；负控必须有前提自证）：
#   S 形状：pxe.c 有 V1..V6 · 头长 96 与 TRAILER 40 两侧一致 · op 码镜像双向无缺无多
#   P 正向：parity 逐字段 · 三轨等价（解释 / 静态 VM-C / .pxe+pxvm）· 体积 < 静态 1%
#   N 负控：9 条定点畸变（改一字节 / 版本 / total / 跳转越界 / LOADK 越界 /
#          modname 越界 / 截尾 / magic），**改完重算校验和**才算测到 V3/V4/V5；
#          跳转越界那条必须先自证「样例确实有跳转指令」，否则判据是空跑
# 用法：check_m442_pxe.sh
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u
ROOT="$(cd "$(dirname "$0")/.." && pwd)"
PX="$ROOT/tools/px"
RT="$ROOT/runtime"
PASS=0; FAIL=0
ok()  { echo "  PASS $1"; PASS=$((PASS+1)); }
bad() { echo "  FAIL $1"; FAIL=$((FAIL+1)); }
env_bad() { echo "❌ 环境不成立：$1" >&2; exit 3; }

[ -f "$RT/pxe.c" ] || env_bad "缺 runtime/pxe.c"
[ -f "$RT/pxvm.c" ] || env_bad "缺 runtime/pxvm.c"
[ -f "$ROOT/selfhost/bc_pxe.px" ] || env_bad "缺 bc_pxe.px"
command -v python3 >/dev/null || env_bad "缺 python3"

TMP="$(mktemp -d "${TMPDIR:-/tmp}/m442.XXXXXX")" || env_bad "mktemp 失败"
trap 'rm -rf "$TMP"' EXIT
cd "$ROOT"

echo "── S 形状与契约"
# S1 校验层齐全
miss=""
for v in V1 V2 V3 V4 V5 V6; do grep -q "$v" "$RT/pxe.c" || miss="$miss $v"; done
[ -z "$miss" ] && ok "S1 loader 含 V1..V6 校验层" || bad "S1 缺校验层：$miss"
# S2 头长/TRAILER 两侧一致（序列化器 ⇄ 加载器）
hdr_c="$(grep -oE '^#define PXE_HDR +[0-9]+' "$RT/pxe.h" | grep -oE '[0-9]+$')"
tr_c="$(grep -oE '^#define PXE_TRAILER +[0-9]+' "$RT/pxe.c" | grep -oE '[0-9]+$')"
hdr_py="$(grep -oE 'let o_K = [0-9]+' "$ROOT/selfhost/bc_pxe.px" | grep -oE '[0-9]+$')"
tr_py="$(grep -oE 'let total = payload \+ [0-9]+' "$ROOT/selfhost/bc_pxe.px" | grep -oE '[0-9]+$')"
if [ "${hdr_c:-x}" = "${hdr_py:-y}" ] && [ "${tr_c:-x}" = "${tr_py:-y}" ]; then
    ok "S2 头长/尾长两侧一致（HDR=$hdr_c TRAILER=$tr_c）"
else
    bad "S2 头/尾不一致：pxe.h HDR=${hdr_c:-?} pxvm TRAILER=${tr_c:-?} · bc_pxe o_K=${hdr_py:-?} +${tr_py:-?}"
fi
# S3 op 码镜像双向（vm.h ⇄ bc_pxe.px）
grep -oE '^#define PXOP_[A-Z0-9_]+ +[0-9]+' "$RT/vm.h" | awk '{print $2" "$3}' | sort > "$TMP/vmops.txt"
grep -oE 'if op == "[A-Z0-9_]+"|return [0-9]+' "$ROOT/selfhost/bc_pxe.px" >/dev/null 2>&1 || true
python3 - "$ROOT/selfhost/bc_pxe.px" "$TMP/vmops.txt" "$TMP" <<'PY'
import re, sys
src = open(sys.argv[1], encoding="utf-8").read()
m = re.search(r"def pxe_op_num\(op\):(.*?)\n# -", src, re.S)
if not m:
    print("E: 找不到 pxe_op_num")
    sys.exit(1)
body = m.group(1)
pairs = re.findall(r'if op == "([A-Z0-9_]+)":\s*\n\s*return (\d+)', body)
pxe = {n: int(v) for n, v in pairs}
vm = {}
for ln in open(sys.argv[2]):
    n, v = ln.split()
    vm[n.replace("PXOP_", "")] = int(v)
miss = [k for k in vm if k not in pxe]
extra = [k for k in pxe if k not in vm]
diff = [k for k in vm if k in pxe and vm[k] != pxe[k]]
print("  mirror vm=%d pxe=%d miss=%s extra=%s diff=%s" % (len(vm), len(pxe), miss, extra, diff))
sys.exit(0 if not (miss or extra or diff) else 1)
PY
if [ $? -eq 0 ]; then ok "S3 op 码镜像双向一致"; else bad "S3 op 码镜像不一致"; fi

echo "── 夹具（样例 + 静态/解释轨产物）"
mkdir -p "$TMP/s" && cd "$TMP/s"
cat > h3.px <<'EOF'
def sign(x):
    if x > 0:
        return "pos"
    else:
        return "neg"

def main():
    print(sign(3) + "," + sign(-3))
    return 0
EOF
cat > t2.px <<'EOF'
def add(a, b):
    return a + b

def main():
    var i = 0
    var s = 0
    while i < 10:
        s = s + i
        i = i + 1
    print(str(s) + " " + str(add(1.5, 0.75)) + " " + str(true) + " " + str(null))
    return 0
EOF
cat > st.px <<'EOF'
struct Point:
    x: int
    y: int

def main():
    let p = Point(3, 4)
    print(str(p.x) + "," + str(p.y))
    return 0
EOF
cat > cl.px <<'EOF'
def counter():
    var n = 0
    def inc():
        n = n + 1
        return n
    return inc

def main():
    let c = counter()
    print(str(c()) + " " + str(c()) + " " + str(c()))
    return 0
EOF
cat > nul.px <<'EOF'
def main():
    let s = "a\u{0}b"
    print(str(len(s)))
    print(s)
    return 0
EOF
cp "$ROOT/tools/pxslice.px" pxslice.px
SAMPLES="h3 t2 st cl nul pxslice"

echo "── 构建（bc_cli / pxvm / 静态产物 / 解释轨）"
cd "$ROOT"
"$PX" build "$ROOT/selfhost/bc_cli.px" >"$TMP/bccli.log" 2>&1 || env_bad "bc_cli 构建失败（$(tail -2 "$TMP/bccli.log" | tr '\n' ' ')）"
BC="$ROOT/selfhost/build/bc_cli"
[ -x "$BC" ] || env_bad "缺 $BC"
cd "$TMP/s"
bash "$ROOT/selfhost/build_pxvm.sh" "$TMP/pxvm" >"$TMP/pxvm.log" 2>&1 || env_bad "pxvm 构建失败（$(tail -2 "$TMP/pxvm.log" | tr '\n' ' ')）"
ok "构建 pxvm（$(stat -c %s "$TMP/pxvm") B）"

echo "── P 正向判据"
pfail=0
for s in $SAMPLES; do
    "$BC" "$s.px" > "$s.dump" 2>&1 || { bad "P0 dump 失败 $s"; pfail=1; continue; }
    "$BC" --emit-pxe "$s.px" > "$s.pxe.log" 2>&1 || { bad "P0 emit-pxe 失败 $s"; pfail=1; continue; }
    [ -f "$s.pxe" ] || { bad "P0 未产出 $s.pxe"; pfail=1; continue; }
done
[ $pfail -eq 0 ] && ok "P0 6 样例 dump + .pxe 全部产出"

for s in $SAMPLES; do
    if python3 "$ROOT/selfhost/m442_parity.py" "$s.dump" "$s.pxe" > "$s.parity" 2>&1; then
        ok "P1 parity 逐字段一致 $s（$(sed -n 's/^parity2: OK（\(.*\)）$/\1/p' "$s.parity")）"
    else
        bad "P1 parity 不一致 $s —— $(tail -2 "$s.parity" | tr '\n' ' ')"
    fi
done

# P2 三轨等价：解释轨 / 静态 VM-C 轨 / .pxe+pxvm 轨（stdout + 退出码全等）
# 注意：用文件 + cmp 比对，**不要用 $(...) 装 stdout** —— bash 会丢弃 NUL 字节，
#   那样 nul.px 这条判据就被悄悄弱化（实测踩到 warning: ignored null byte）。
for s in $SAMPLES; do
    "$PX" build "$s.px" >/dev/null 2>&1 || { bad "P2 静态构建失败 $s"; continue; }
    "$TMP/s/build/$s" > "$TMP/$s.out.static" 2>&1; r_static=$?
    echo "$r_static" > "$TMP/$s.rc.static"
    "$TMP/pxvm" "$s.pxe" > "$TMP/$s.out.pxe" 2>&1; r_pxe=$?
    if cmp -s "$TMP/$s.out.static" "$TMP/$s.out.pxe" && [ "$r_static" = "$r_pxe" ]; then
        ok "P2 静态 ⇄ .pxe 等价 $s（rc=$r_static）"
    else
        bad "P2 不等价 $s：static[$(head -c 80 "$TMP/$s.out.static")]rc=$r_static vs pxe[$(head -c 80 "$TMP/$s.out.pxe")]rc=$r_pxe"
    fi
done
# P2b 解释轨（px run）与静态轨一致 —— 只在能解释执行的小样例上做
for s in h3 t2; do
    "$PX" run "$s.px" > "$TMP/$s.out.interp" 2>&1; r_i=$?
    r_s="$(cat "$TMP/$s.rc.static" 2>/dev/null || echo x)"
    if cmp -s "$TMP/$s.out.interp" "$TMP/$s.out.static" && [ "$r_i" = "$r_s" ]; then
        ok "P2b 解释轨 ⇄ 静态轨一致 $s"
    else
        bad "P2b 解释轨不等价 $s：[$(head -c 80 "$TMP/$s.out.interp")]rc=$r_i vs static rc=$r_s"
    fi
done

# P3 闭包 upvals 真序列化（nup>0 且三轨一致已在 P2 覆盖，这里查元数据）
if "$TMP/pxvm" --info cl.pxe | grep -q "nup=1"; then
    ok "P3 闭包 upvals 已序列化（cl: nup=1）"
else
    bad "P3 闭包 upvals 丢失（cl 未出现 nup=1）"
fi
# P4 结构体元数据
if "$TMP/pxvm" --info st.pxe | grep -q "nstructs=1"; then
    if "$TMP/pxvm" --info st.pxe | grep -qE "S\[0\] Point nfields=2 fnames=\[x,y\]"; then
        ok "P4 结构体元数据（Point x,y）"
    else
        bad "P4 结构体字段名不符：$("$TMP/pxvm" --info st.pxe | grep 'S\[' | head -1)"
    fi
else
    bad "P4 结构体段缺失（nstructs≠1）"
fi
# P5 体积：.pxe 必须远小于静态产物（<1%）
st_size=$(stat -c %s "$TMP/s/build/pxslice")
pxe_size=$(stat -c %s pxslice.pxe)
pct=$(( pxe_size * 10000 / st_size ))   # 万分比
if [ "$pct" -lt 100 ]; then
    ok "P5 体积 pxslice.pxe=${pxe_size}B vs 静态 ${st_size}B（$((pct/100)).$((pct%100))%）"
else
    bad "P5 体积未收敛：pxslice.pxe=${pxe_size}B / 静态 ${st_size}B"
fi

echo "── Q 阶段3：编译器正主发射 + px build --pxe + .pxecache"
# Q0 形状：compiler.px（自举正主）含两条接线（少一条整条用户路径就断）
qmiss=""
grep -q -- '--emit-pxe' "$ROOT/selfhost/compiler.px" || qmiss="$qmiss emit-pxe"
grep -q -- '--out' "$ROOT/selfhost/compiler.px" || qmiss="$qmiss out"
[ -z "$qmiss" ] && ok "Q0 compiler.px 含 --emit-pxe / --out 接线" || bad "Q0 compiler.px 缺：$qmiss"

# Q1 入库编译器（bootstrap/pxc_vm）真的支持 --emit-pxe —— 不支持 = 没重烘
if "$ROOT/bootstrap/pxc_vm" --emit-pxe --out "$TMP/probe.pxe" "$TMP/s/h3.px" >/dev/null 2>&1 \
   && [ -s "$TMP/probe.pxe" ]; then
    ok "Q1 bootstrap/pxc_vm 支持 --emit-pxe（$(stat -c %s "$TMP/probe.pxe")B）"
else
    bad "Q1 bootstrap/pxc_vm 不支持 --emit-pxe（⇒ ./selfhost/rebake_bin.sh --entries=pxc,pxc_vm 或 --rebake-all）"
fi

# Q2 两入口产物逐字节一致（bc_cli 与编译器正主走同一 bc_pxe_emit）
cd "$TMP/s"
for s in $SAMPLES; do
    if ! "$ROOT/bootstrap/pxc_vm" --emit-pxe --out "$TMP/c_$s.pxe" "$s.px" >/dev/null 2>&1; then
        bad "Q2 compiler 发射失败 $s"; continue
    fi
    if ! "$BC" --emit-pxe --out "$TMP/b_$s.pxe" "$s.px" >/dev/null 2>&1; then
        bad "Q2 bc_cli 发射失败 $s"; continue
    fi
    if cmp -s "$TMP/c_$s.pxe" "$TMP/b_$s.pxe"; then
        ok "Q2 两入口产物一致 $s（$(stat -c %s "$TMP/c_$s.pxe")B）"
    else
        bad "Q2 两入口产物不一致 $s"
    fi
done

# Q3 `px build --pxe` 端到端：产物落 <src>/build/<base>.pxe，pxvm 执行输出 = 静态产物
if "$PX" build --pxe h3.px > "$TMP/q3.log" 2>&1 && [ -f "$TMP/s/build/h3.pxe" ]; then
    "$TMP/pxvm" "$TMP/s/build/h3.pxe" > "$TMP/q3.out" 2>&1
    if cmp -s "$TMP/q3.out" "$TMP/h3.out.static"; then
        ok "Q3 px build --pxe 端到端（$(stat -c %s "$TMP/s/build/h3.pxe")B · 输出=静态）"
    else
        bad "Q3 px build --pxe 产物输出 ≠ 静态产物"
    fi
else
    bad "Q3 px build --pxe 失败：$(tail -2 "$TMP/q3.log" 2>/dev/null | tr '\n' ' ')"
fi

# Q4 .pxecache 二次命中（判据：verbose 里出现「缓存命中」）
PX_BUILD_VERBOSE=1 "$PX" build --pxe h3.px > "$TMP/q4.log" 2>&1
if grep -q '缓存命中' "$TMP/q4.log"; then
    ok "Q4 .pxecache 二次命中"
else
    bad "Q4 .pxecache 未命中：$(tail -2 "$TMP/q4.log" | tr '\n' ' ')"
fi

# Q5 改源码 ⇒ 缓存必须失效（与 M441 缺陷 924 同族：键要覆盖所有影响产物的输入）
#   ⚠️ 2026-11-24 实测（M442 补 2 · 缺陷 928）：本条原来用**固定文本** q5-new ⇒ 源码 sha 进了
#     缓存键，本机**第二次**跑同一门就「缓存命中」判红 = 假红（CI 每次冷缓存所以看不出来）。
#     改为：每次跑用唯一后缀，判据拆成两段 —— ①首版（唯一内容）建一次 ②同一文件改内容再建
#     **必须不命中**。前提自证：两版内容 sha 必须不同，否则这条是空跑。
S5="$(date +%s%N)-$$"
printf 'def main():\n    print("q5-a-%s")\n    return 0\n' "$S5" > "$TMP/s/q5.px"
S5A="$(sha256sum "$TMP/s/q5.px" | cut -c1-16)"
if PX_BUILD_VERBOSE=1 "$PX" build --pxe q5.px > "$TMP/q5a.log" 2>&1; then
    printf 'def main():\n    print("q5-b-%s")\n    return 0\n' "$S5" > "$TMP/s/q5.px"
    S5B="$(sha256sum "$TMP/s/q5.px" | cut -c1-16)"
    if [ "$S5A" = "$S5B" ]; then
        bad "Q5 前提不成立：两版内容 sha 相同（$S5A）⇒ 判据空跑"
    elif PX_BUILD_VERBOSE=1 "$PX" build --pxe q5.px > "$TMP/q5.log" 2>&1; then
        if grep -q '缓存命中' "$TMP/q5.log"; then
            bad "Q5 改了源码仍命中缓存（键漏了源码）：$S5A → $S5B"
        else
            ok "Q5 改源码 ⇒ 缓存失效（$S5A → $S5B）"
        fi
    else
        bad "Q5 改源码后构建失败：$(tail -2 "$TMP/q5.log" | tr '\n' ' ')"
    fi
else
    bad "Q5 首版构建失败：$(tail -2 "$TMP/q5a.log" | tr '\n' ' ')"
fi

# Q6 `px run *.pxe` 走 pxvm（用户面入口），输出/退出码与静态产物全等
"$PX" run "$TMP/s/build/h3.pxe" > "$TMP/q6.out" 2>&1; r_q6=$?
r_static_h3="$(cat "$TMP/h3.rc.static" 2>/dev/null || echo x)"
if cmp -s "$TMP/q6.out" "$TMP/h3.out.static" && [ "$r_q6" = "$r_static_h3" ]; then
    ok "Q6 px run *.pxe 走 pxvm（rc=$r_q6）"
else
    bad "Q6 px run *.pxe 不等价：[$(head -c 60 "$TMP/q6.out")] rc=$r_q6 vs static rc=$r_static_h3"
fi

# Q7 pxvm 入库且双指纹与当前源码一致（.pxe 的唯一执行者，不能是旧件）
if "$ROOT/selfhost/rebake_bin.sh" --check-pxvm > "$TMP/q7.log" 2>&1; then
    ok "Q7 pxvm 入库 · 双指纹与当前源码一致"
else
    bad "Q7 pxvm 来源门红：$(tail -2 "$TMP/q7.log" | tr '\n' ' ')"
fi

echo "── B 自举边界：runtime 库 ⇄ 宿主程序（M442-S3 · 2026-11-24 CI 红）"
# 为什么有这一块（2026-11-24 实测 · CI run 45311642994 @ b048dfd）：
#   pxvm.c 是 `.pxe` **宿主程序**（自带 main），不属于 runtime 库。native_bootstrap.sh 的
#   `ls runtime/*.c` 把它顺带收进自举 runtime 清单 ⇒ 与 golden/compiler.c 的 main 撞
#   「multiple definition of `main`」⇒ 原生档（aarch64 runner）与交叉档（x86_64→aarch64）
#   两条自举步同时红；而本地全量门**不含自举**（CI 独占）⇒ 「本地绿、CI 红」。
HOSTS="$(grep -lE '(^|[^A-Za-z0-9_])int[[:space:]]+main[[:space:]]*\(' "$RT"/*.c 2>/dev/null \
         | xargs -r -n1 basename | sort | tr '\n' ' ')"
if [ "$HOSTS" = "pxvm.c " ]; then
    ok "B1 runtime 中带 main 的宿主程序集合 = {pxvm.c}"
else
    bad "B1 runtime 中带 main 的宿主程序集合 = [$HOSTS]（期望仅 pxvm.c；新增宿主须同批改自举排除表与本判据）"
fi
NB="$ROOT/selfhost/native_bootstrap.sh"
if grep -qE '^[[:space:]]*pxvm\.c\)[[:space:]]*$' "$NB"; then
    ok "B2 native_bootstrap.sh 的 runtime 清单显式排除 pxvm.c"
else
    bad "B2 native_bootstrap.sh 未排除 pxvm.c（自举链接期必 dup main）"
fi
# B3 判据有牙（负控）：把排除分支抠掉 ⇒ ①B2 的 grep 必失配 ②自举脚本必须**就地** rc=3
#   前提自证：先证明拷贝件里真的没有那两行了，否则这条是空跑。
B3D="$(mktemp -d "${TMPDIR:-/tmp}/m442b3.XXXXXX")"; mkdir -p "$B3D/selfhost"
ln -s "$RT" "$B3D/runtime"
ln -s "$ROOT/selfhost/golden" "$B3D/selfhost/golden"   # 脚本前置检查要求基准存在，否则到不了 guard
ln -s "$ROOT/selfhost/compiler.px" "$B3D/selfhost/compiler.px"
python3 - "$NB" "$B3D/selfhost/nb.sh" <<'PY'
import re, sys
s = open(sys.argv[1], encoding='utf-8').read()
s2 = re.sub(r'\n[ \t]*pxvm\.c\)[ \t]*\n[ \t]*continue ;;', '', s, count=1)
if s2 == s:
    print("E: 没找到 pxvm.c 排除分支"); sys.exit(1)
open(sys.argv[2], 'w', encoding='utf-8').write(s2)
PY
if [ $? -ne 0 ]; then
    bad "B3 负控前提不成立（抠不掉排除分支）"
elif grep -qE '^[[:space:]]*pxvm\.c\)[[:space:]]*$' "$B3D/selfhost/nb.sh"; then
    bad "B3 负控前提不成立（排除分支仍在）"
elif bash "$B3D/selfhost/nb.sh" --no-prove --jobs 8 --outdir "$B3D/out" > "$B3D/b3.log" 2>&1; then
    bad "B3 抠掉排除分支后自举仍 rc=0（判据无牙）"
elif grep -q "没被列入宿主排除表" "$B3D/b3.log"; then
    ok "B3 判据有牙：抠掉排除分支 ⇒ 自举就地 rc=3（guard 命中）"
else
    bad "B3 失败原因不是 guard：$(tail -2 "$B3D/b3.log" | tr '\n' ' ')"
fi
rm -rf "$B3D"

echo "── N 负控（loader 必须拒收 / 判据必须变红）"
if python3 "$ROOT/selfhost/m442_negctl.py" "$TMP/pxvm" h3.pxe > "$TMP/neg.log" 2>&1; then
    ok "N1 九条定点畸变全被拒收（h3）"
else
    bad "N1 负控未全红 —— $(grep -E 'FAIL|前提' "$TMP/neg.log" | tr '\n' ' ')"
fi
sed -n 's/^   └ 前提自证：\(.*\)$/  INFO 前提自证：\1/p' "$TMP/neg.log"

# N2 校验和真的是必要条件：把 trailer 的 sha 改一字节 ⇒ 必须 rc=6
python3 - "$TMP/pxvm" h3.pxe "$TMP/n2.pxe" <<'PY'
import shutil, subprocess, sys
src, dst = sys.argv[2], sys.argv[3]
shutil.copyfile(src, dst)
b = bytearray(open(dst, "rb").read())
b[-20] ^= 0xFF
open(dst, "wb").write(b)
rc = subprocess.run([sys.argv[1], dst], capture_output=True).returncode
print("sha 破坏 rc=%d" % rc)
sys.exit(0 if rc == 6 else 1)
PY
if [ $? -eq 0 ]; then ok "N2 校验和破坏必须拒收（rc=6）"; else bad "N2 校验和破坏未被拒收"; fi

# N3 判据自测（parity 有牙）：篡改一条指令的 c 操作数并**重算校验和** ⇒ parity 必须判红
python3 - "$ROOT/selfhost/m442_parity.py" t2.dump t2.pxe "$TMP/t2_bad.pxe" <<'PY'
import hashlib, struct, subprocess, sys, zlib
parity, dump, src, dst = sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4]
b = bytearray(open(src, "rb").read())
nK, nN, nG = struct.unpack_from("<I", b, 8)[0], struct.unpack_from("<I", b, 12)[0], struct.unpack_from("<I", b, 16)[0]
nf, nst, ninst = struct.unpack_from("<I", b, 20)[0], struct.unpack_from("<I", b, 28)[0], struct.unpack_from("<I", b, 32)[0]
oF = 96 + nK * 24 + nN * 4 + nG * 4
oFU = oF + nf * 32
nup = sum(struct.unpack_from("<I", b, oF + i * 32 + 24)[0] for i in range(nf))
oT = oFU + nup * 4
nft = sum(struct.unpack_from("<I", b, oT + i * 12 + 4)[0] for i in range(nst))
oC = oT + nst * 12 + nft * 4
# 找一条非跳转指令，改 c 字段（不破坏结构合法性）
mutated = False
for i in range(nf):
    bcoff = struct.unpack_from("<I", b, oF + i * 32 + 16)[0]
    nbc = struct.unpack_from("<I", b, oF + i * 32 + 20)[0]
    for j in range(nbc):
        q = oC + (bcoff + j) * 8
        if b[q] not in (46, 47, 48, 77):
            c = struct.unpack_from("<H", b, q + 6)[0]
            struct.pack_into("<H", b, q + 6, (c + 1) % 65536)
            mutated = True
            break
    if mutated:
        break
if not mutated:
    print("E: 找不到可篡改的非跳转指令")
    sys.exit(2)
payload = len(b) - 40
struct.pack_into("<I", b, payload, zlib.crc32(bytes(b[:payload])) & 0xFFFFFFFF)
b[payload + 4:payload + 36] = hashlib.sha256(bytes(b[:payload])).digest()
open(dst, "wb").write(b)
rc = subprocess.run([sys.executable, parity, dump, dst], capture_output=True).returncode
print("parity(bad)=%d" % rc)
sys.exit(0 if rc == 1 else 1)
PY
if [ $? -eq 0 ]; then ok "N3 parity 有牙（篡改指令后校验和合法仍判红）"; else bad "N3 parity 未判红（判据无效）"; fi

# N10 判据有牙（pxvm 来源门）：篡改 PXVMFP ⇒ --check-pxvm 必红；复原 ⇒ 复绿
#   前提自证：先证明「文件里真有 PXVMFP 标记且替换真的发生」，否则这条是空跑。
QV="$ROOT/bootstrap/pxvm"
if [ -x "$QV" ]; then
    cp -p "$QV" "$TMP/pxvm.orig"
    python3 - "$QV" > "$TMP/n10.mut" 2>&1 <<'PY'
import re, sys
p = sys.argv[1]
d = open(p, "rb").read()
m = re.search(rb"PXVMFP-[0-9a-f]{16}", d)
if not m:
    print("E: 无 PXVMFP 标记")
    sys.exit(3)
d = d[:m.start()] + b"PXVMFP-0000000000000000" + d[m.end():]
open(p, "wb").write(d)
print("mutated at %d" % m.start())
sys.exit(0)
PY
    mutrc=$?
    if [ "$mutrc" = 0 ]; then
        if "$ROOT/selfhost/rebake_bin.sh" --check-pxvm > "$TMP/n10.log" 2>&1; then
            bad "N10 篡改 PXVMFP 后 --check-pxvm 仍绿（判据无牙）"
        else
            ok "N10 篡改 PXVMFP ⇒ --check-pxvm 必红（判据有牙）"
        fi
    else
        bad "N10 前提不成立：$(cat "$TMP/n10.mut" | tr '\n' ' ')"
    fi
    cp -p "$TMP/pxvm.orig" "$QV"
    if "$ROOT/selfhost/rebake_bin.sh" --check-pxvm >/dev/null 2>&1; then
        ok "N10b 复原后 --check-pxvm 复绿（负控未污染）"
    else
        bad "N10b 复原失败：bootstrap/pxvm 被污染"
    fi
else
    bad "N10 环境：缺 bootstrap/pxvm，无法构成负控"
fi

# N11 .pxecache 与静态产物缓存**不串味**（同一源码两条轨）
#   为什么：两轨共用 BUILD_CACHE，若 key 不分桶，`build --pxe` 可能拿到静态二进制
if [ -f "$TMP/s/build/h3.pxe" ]; then
    pxe_magic="$(head -c 4 "$TMP/s/build/h3.pxe")"
    if [ "$pxe_magic" = "PXE1" ] && [ -f "$TMP/s/build/h3" ] \
       && [ "$(stat -c %s "$TMP/s/build/h3")" -gt 1000000 ]; then
        ok "N11 两轨产物不串味（.pxe=PXE1 $(stat -c %s "$TMP/s/build/h3.pxe")B vs 静态 $(stat -c %s "$TMP/s/build/h3")B）"
    else
        bad "N11 分桶异常（magic=[$pxe_magic] 或缺静态产物）"
    fi
else
    bad "N11 缺 .pxe 产物，无法验证分桶"
fi

echo
echo "== m442_pxe: PASS=$PASS FAIL=$FAIL =="
[ "$FAIL" -eq 0 ] || exit 1
exit 0
