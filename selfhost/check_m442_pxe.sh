#!/usr/bin/env bash
# ============================================================
# selfhost/check_m442_pxe.sh —— .pxe 序列化模块（M442 阶段2）判据
# ------------------------------------------------------------
# 判据面：格式契约 / 三轨等价 / 序列化覆盖（闭包·结构体·内嵌 NUL）/ 安全边界负控。
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

echo
echo "== m442_pxe: PASS=$PASS FAIL=$FAIL =="
[ "$FAIL" -eq 0 ] || exit 1
exit 0
