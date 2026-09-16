#!/usr/bin/env bash
# ============================================================
# M125（qg-issue 82）门：分配失败 = **请求级 5xx，进程不死**
# ------------------------------------------------------------
# 病灶（原实现）：三处 mmap 失败（slab_raw_alloc / slab_create / xmalloc 大对象）一律
#   `fprintf(stderr,"lx: 内存不足"); exit(1);` ⇒ 一个请求即可带走整个服务进程。
#   线上实证：mahesvara（8 站点同进程）整站不可达 ~3 s，journal `lx: 内存不足` →
#   `status=1` → systemd 重启；07:17 / 09:06 两轮，`NRestarts` 累计 10 次。
# 判据（五条）：
#   ① 注入失败（PX_ALLOC_FAIL_MIN）→ /big 拿到 5xx（不是连接被掐 / 进程消失）；
#   ② 真·大分配（read_file 100 GB 稀疏文件 ⇒ 一次 100 GB mmap 必被内核拒）→ /huge 5xx；
#   ③ 两种失败之后，**同一进程**仍能正常服务（/health 200、/small 200、正常 /big 200）；
#   ④ stderr 给出**分类后的真因**（出处 + 字节数 + errno），不再是光秃秃一句「内存不足」；
#   ⑤ 负控：不注入时 /big 与 /health 都必须 200（证明断言不是恒真）。
# 注：必须用**仓库自带** tools/px（PXC_HOME=仓库根 ⇒ 用仓库 runtime/）；
#     /usr/bin/px 是安装版（包根=/usr/share/puxian），其 runtime/ 与仓库不同步 —— 会测出旧行为。
# 用法：bash examples/m125_alloc_fail/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m125_alloc_fail
PX=${PX:-$ROOT/tools/px}
BIN=$ROOT/$DIR/build/alloc_fail
PAYLOAD=/tmp/m125_payload.bin
HUGE=/tmp/m125_huge.bin
THRESH=4194304            # 4 MB：高于启动期最大分配（1 MB），低于载荷（8 MB）
pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M125 验证（分配失败 → 请求级 5xx，进程不死）=="
echo "[0/5] 准备载荷"
head -c 8388609 /dev/zero | tr '\0' 'A' > "$PAYLOAD"   # 8 MB 纯 ASCII（保证 len() == 字节数）
truncate -s 100G "$HUGE"                     # 100 GB 稀疏文件（read_file 会一次要 100 GB）
echo "     payload=$(stat -c%s "$PAYLOAD") 字节（8 MB）  huge=$(stat -c%s "$HUGE") 字节（100 GB 稀疏）"

echo "[1/5] 编译（仓库 tools/px ⇒ 仓库 runtime/）"
if "$PX" build "$DIR/alloc_fail.px" > /tmp/m125_build.log 2>&1 && [ -x "$BIN" ]; then
  ok "编译成功"
else
  bad "编译失败"; tail -20 /tmp/m125_build.log | sed 's/^/     /'; exit 1
fi

run() {   # $1=标签 $2=FX 其余=env 赋值
  local tag="$1" fx="$2"; shift 2
  echo "[$tag] FX=$fx $*"
  if [ $# -gt 0 ]; then
    env FX="$fx" "$@" "$BIN" > "/tmp/m125_$tag.log" 2>&1
  else
    env FX="$fx" "$BIN" > "/tmp/m125_$tag.log" 2>&1
  fi
  local rc=$?
  grep -E "运行时错误:|处理：|px-coro|断言失败|M125 ALLOC" "/tmp/m125_$tag.log" | head -8 | sed 's/^/     /'
  if [ $rc -eq 0 ] && grep -q "M125 ALLOC-FAIL GATE PASSED" "/tmp/m125_$tag.log"; then
    ok "$tag 退出码 0 且门通过（请求级 5xx + 进程存活；旧实现此处 exit(1)）"
  else
    bad "$tag 未通过（退出码 $rc）"
  fi
}

echo "[2/5] A 注入失败（阈值 $THRESH 字节）"
run A a PX_ALLOC_FAIL_MIN=$THRESH
grep -qE "运行时错误: (内存不足|分配尺寸非法)【xmalloc" /tmp/m125_A.log && ok "stderr 给出分类真因（含 xmalloc 出处）" || bad "stderr 未给出分类真因"
grep -q "errno=" /tmp/m125_A.log && ok "stderr 带 errno" || bad "stderr 缺 errno"
grep -q "协程隔离点内 → 只终止本请求" /tmp/m125_A.log && ok "明确标注按请求级失败处理" || bad "未标注请求级处理"

echo "[3/5] B 真·大分配（read_file 100 GB ⇒ mmap 必被拒）"
run B b
grep -q "申请 10737418240" /tmp/m125_B.log && ok "stderr 报出真实申请字节数（100 GB 级）" || bad "stderr 未报出真实字节数"
grep -q "errno=12 (Cannot allocate memory)" /tmp/m125_B.log && ok "stderr 带 errno=ENOMEM 说明" || bad "stderr 缺 errno 说明"

echo "[4/5] 负控：不注入（阈值抬到 1 TB）"
run N n PX_ALLOC_FAIL_MIN=999999999999
grep -qE "运行时错误: (内存不足|分配尺寸非法)" /tmp/m125_N.log && bad "负控里出现了分配失败（阈值未生效？）" || ok "负控期无任何分配失败告警（断言非恒真）"

echo "[5/5] 回归：旧行为可复现（证明本门测的是真变化）"
grep -q "lx: 内存不足" /tmp/m125_B.log && bad "仍打印旧文案" || ok "旧文案 'lx: 内存不足' 已不再出现（新文案分类报因）"

echo
echo "== 汇总：pass=$pass fail=$fail =="
[ "$fail" = 0 ] && echo "✅ M125 verify 全绿" || echo "❌ M125 verify 有失败"
exit $fail
