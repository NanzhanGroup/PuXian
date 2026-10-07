#!/usr/bin/env bash
# ============================================================
# M279 门 · stdlib 纯函数族「逐位置 × 边界/错类型」+ 结构性死循环
# ------------------------------------------------------------
# 由来（真缺陷 · 高危）：`std.collections.chunk(items, n)` 在 `n <= 0` 时
#   内层 `while j < n and i < len(items)` **恒假** ⇒ 推进语句（唯一 `i += 1` 处）
#   永不执行 ⇒ 外层 `while i < len(items)` **永不推进** ⇒ 死循环 + `result` 无限增长。
#   实测（修前）：解释轨挂住 60s；限内存 300MB 时 VM/C 轨报
#   「分配尺寸非法【slab_create】申请 192 字节」/「内存不足【xmalloc 大对象】申请 16777216 字节」
#   —— **错误信息指不到真因**（用户看不到"你传了 0"）。
#
# 判据（三层，每层独立有牙）：
#   [1] 静态：结构性死循环扫描器（含自证 8 条 + 豁免表 + 过期判据）
#   [2] 动态：`chunk(xs, 0/-1)` 三轨必须**响亮且限时返回**（超时=死循环 ⇒ 判红）
#   [3] 三轨对拍：stdlib 纯函数族 × 边界/错类型 ⇒ 分叉 0 + 契约表双向
# ============================================================
set -u
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M279_W:-/tmp/m279}"
NEG_SKIP="${NEG_SKIP:-0}"
mkdir -p "$W"
PASS=0; FAIL=0
ok()   { echo "  ✅ $1"; PASS=$((PASS+1)); }
bad()  { echo "  ❌ $1"; FAIL=$((FAIL+1)); }
chk()  { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }

echo "══ M279 · stdlib 纯函数族边界面 + 结构性死循环 ══"

# ── [1] 静态扫描器 ──────────────────────────────────────────
echo "[1] 结构性死循环静态扫描"
SC="$ROOT/selfhost/check_stdlib_loops.py"
if [ ! -f "$SC" ]; then bad "[1] 缺 $SC"; else
  o=$(timeout 120 python3 "$SC" --self-test 2>&1 | tail -1)
  case "$o" in *"8 通过 / 0 失败"*) ok "[1a] 扫描器自证 8/0";; *) bad "[1a] 扫描器自证：$o";; esac
  AL="$ROOT/selfhost/stdlib_loops.allow"
  so=$(timeout 120 python3 "$SC" --root "$ROOT" --allow "$AL" --json "$W/loopscan.json" 2>&1)
  echo "$so" | tail -3 > "$W/loopscan.txt"
  v=$(python3 -c "import json;d=json.load(open('$W/loopscan.json'));print(len(d['violations']))" 2>/dev/null || echo ERR)
  wv=$(python3 -c "import json;d=json.load(open('$W/loopscan.json'));print(d['waived'])" 2>/dev/null || echo ERR)
  st=$(python3 -c "import json;d=json.load(open('$W/loopscan.json'));print(len(d['stale']))" 2>/dev/null || echo ERR)
  chk "[1b] stdlib 结构性死循环违例" "$v" "0"
  chk "[1c] 豁免生效条数" "$wv" "2"
  chk "[1d] 豁免过期条数" "$st" "0"
  case "$so" in *M279-LOOPSCAN-OK*) ok "[1e] 扫描器结论行 OK";; *) bad "[1e] 扫描器结论行：$(echo "$so"|tail -1)";; esac
fi

# ── [2] 动态：chunk 边界必须限时响亮 ────────────────────────
echo "[2] chunk(n<=0) 动态判据（三轨 × 限时）"
CH="$W/chk.px"
cat > "$CH" <<'PXEOF'
import std.collections
def main():
    var a = args()
    var c = a[1]
    if c == "zero":
        print("R", chunk([1,2,3], 0))
    else:
        if c == "neg":
            print("R", chunk([1,2,3], -1))
        else:
            if c == "ok":
                print("R", chunk([1,2,3,4,5], 2))
            else:
                print("R", chunk([], 2))
PXEOF
mkdir -p "$W/build"
VM="$W/build/chk_vm"; CC="$W/build/chk_c"
( cd "$W" && timeout 300 "$ROOT/tools/px" build chk.px > "$W/build_vm.log" 2>&1 ) && cp -f "$W/build/chk" "$VM" 2>/dev/null
( cd "$W" && PX_BUILD_ENGINE=c timeout 300 "$ROOT/tools/px" build chk.px > "$W/build_c.log" 2>&1 ) && cp -f "$W/build/chk" "$CC" 2>/dev/null
if [ ! -x "$VM" ] || [ ! -x "$CC" ]; then
  bad "[2] 构建未产出（见 $W/build_vm.log / $W/build_c.log）"
else
  ok "[2a] 两轨构建成功"
  run1() { bash -c "ulimit -v 500000; timeout 8 '$1' $2" 2>&1; }
  for tr in "vm:$VM" "c:$CC"; do
    nm="${tr%%:*}"; bin="${tr#*:}"
    for cs in zero neg; do
      o=$(run1 "$bin" "$cs"); rc=$?
      case "$o" in *"chunk 的 n 需要正整数"*) ok "[2b-$nm-$cs] 响亮且限时（rc=$rc）";;
        *) bad "[2b-$nm-$cs] 未按契约报错（rc=$rc）：$(echo "$o"|head -1)";; esac
      [ "$rc" = "124" ] && bad "[2b-$nm-$cs] **超时 = 死循环**"
    done
    o=$(run1 "$bin" ok); rc=$?
    case "$o" in *"[[1, 2], [3, 4], [5]]"*) ok "[2c-$nm] 合法路径不变";;
      *) bad "[2c-$nm] 合法路径变了（rc=$rc）：$(echo "$o"|head -1)";; esac
  done
fi

# ── [3] 三轨对拍：stdlib 边界/错类型（契约表）────────────────
echo "[3] 三轨对拍 + 契约表双向"
# ⚠ 自足化（M279 · 由 chain 第 ⑧ 步判红抓出）：
#   three_tracks.py 按 `--work "$W"` 找 `drv.px`，而它在本门目录（$HERE）里 ——
#   修前没有任何人把它放进 $W ⇒ **人工单跑（预置了 $W）绿、chain 里跑红**。
#   判据：**门必须自足** —— 语料由门自己就位，不依赖调用方预置。
cp -f "$HERE/drv.px" "$W/drv.px"
python3 "$HERE/three_tracks.py" --root "$ROOT" --work "$W" --cases "$HERE/MODEL.tsv" \
        --pxi "$ROOT/bootstrap/pxi" --json "$W/three.json" > "$W/three.txt" 2>&1
trc=$?
tail -4 "$W/three.txt" | sed 's/^/     /'
case $trc in
  0) ok "[3a] 三轨分叉 0";;
  *) bad "[3a] 三轨对拍 rc=$trc（见 $W/three.txt）";;
esac
d=$(python3 -c "import json;d=json.load(open('$W/three.json'));print(len(d.get('diverged',[])))" 2>/dev/null || echo ERR)
chk "[3b] 分叉条数" "$d" "0"
m2=$(python3 -c "import json;d=json.load(open('$W/three.json'));print(len(d.get('model_mismatch',[])))" 2>/dev/null || echo ERR)
chk "[3c] 契约表不符条数（双向）" "$m2" "0"

# ── [4] 负控 ────────────────────────────────────────────────
if [ "$NEG_SKIP" = "1" ]; then
  echo "[4] 负控（--neg-skip：CI 档跳过）"
else
  echo "[4] 负控 A/B/C"
  cp -f "$ROOT/stdlib/collections.px" "$W/coll.pre"
  # NC-A：忠实撤回守卫（退回 M279 修前源码）⇒ [2] 必须.red
  python3 - "$ROOT/stdlib/collections.px" <<'PYEOF'
import re,sys
p=sys.argv[1]; s=open(p,encoding='utf-8').read()
i=s.find('def chunk'); j=s.find('\n## ',i)
blk=s[i:j if j>0 else len(s)]
new='''def chunk(items, n):
    result = []
    var i = 0
    while i < len(items):
        var part = []
        var j = 0
        while j < n and i < len(items):
            part.append(items[i])
            i += 1
            j += 1
        result.append(part)
    return result
'''
open(p,'w',encoding='utf-8').write(s[:i]+new+s[j if j>0 else len(s):])
PYEOF
  o=$(timeout 60 python3 "$SC" --root "$ROOT" --allow "$AL" 2>&1 | tail -1)
  case "$o" in *FAIL*) ok "[4A] 撤回守卫 ⇒ 扫描器判红";; *) bad "[4A] 撤回守卫后扫描器仍绿：$o";; esac
  cp -f "$W/coll.pre" "$ROOT/stdlib/collections.px"
  # NC-B：把豁免表理由清空 ⇒ 该条必须**不再算豁免**（纪律：理由为空即无效）
  printf 'stdlib/html.px:175 \n' > "$W/allow_empty"
  o=$(timeout 60 python3 "$SC" --root "$ROOT" --allow "$W/allow_empty" 2>&1 | tail -1)
  case "$o" in *FAIL*) ok "[4B] 豁免理由为空 ⇒ 判红（不计豁免）";; *) bad "[4B] 空理由竟被豁免：$o";; esac
  # NC-C：判据自伤 ⇒ [1b] 必须变绿（证明那道红来自判据本身）
  o=$(timeout 60 python3 "$SC" --self-test 2>&1 | grep -c '⑥ 判据自伤')
  chk "[4C] 判据自伤负控在位" "$o" "1"
  cmp -s "$W/coll.pre" "$ROOT/stdlib/collections.px" && ok "[4D] 源码逐字节还原" || bad "[4D] 源码未还原"
fi

echo ""
echo "══ 汇总：$PASS 通过 / $FAIL 失败 ══"
[ "$FAIL" = "0" ] && { echo "M279-VERIFY-OK"; exit 0; } || { echo "M279-VERIFY-FAIL"; exit 1; }
