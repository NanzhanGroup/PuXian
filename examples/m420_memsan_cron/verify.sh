#!/bin/bash
# ============================================================
# m420_memsan_cron —— cron 线程的 job 分配必须 xmalloc/xfree 配对（ASan 实测）
# ------------------------------------------------------------
# 缺陷 844（M420 实测，2026-11-19）：
#   bi_cron 用 malloc 分配 CronJob 与 args，但 cron_thread 退出时用 xfree 释放。
#   xfree 只认 xmalloc 的块：slab_find 未命中 → 读 *(size_t*)(p-8)（越界读）→
#   munmap((char*)p-8, total)（对非 mmap 地址是未定义行为）。
#   ASan: heap-buffer-overflow READ of size 8 at runtime.c:1315 in xfree ← cron_thread。
#   常规构建下这 8 字节读恰好落在 chunk header 上 → 静默通过（292 门全绿也发现不了）。
#   产品契约：spawn_thread(17197) / timer_thread(17378) 都是 xmalloc/xfree，cron 是唯一例外。
#
# 门结构：
#   [1] 静态判据：CronJob/args 的分配是 xmalloc；失败路劲释放是 xfree
#   [2] 静态负控：复原成 malloc/free 的片段 ⇒ 检查器必须判红
#   [3] 工具正控：C 夹具故意 heap 越界 ⇒ ASan 必报（证明 sanitizer 真在工作，不是没生效的假绿）
#   [4] 动态正控：ASan 构建 m28_cron + 真跑 ⇒ rc==0 且 stderr 无 AddressSanitizer
#   [5] 前提自证：动态二进制里确实链了 ASan（读符号）
# 环境缺失（无 gcc / 无 libasan）⇒ rc=3（绝不降级成绿）
# ============================================================
set -uo pipefail

. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败" >&2; exit 2; }

HERE=$(cd "$(dirname "$0")" && pwd)
RT=$(cd "$HERE/../.." && pwd)
RC="$RT/runtime/runtime.c"
SRC="$RT/examples/m28_cron.px"
pass=0; fail=0
ok()  { pass=$((pass+1)); echo "PASS $*"; }
bad() { fail=$((fail+1)); echo "FAIL $*"; }

# ---------- 检查器：cron 的 job 分配器 ----------
# 输出 ALLOC=<函数名> 与 FREEFAIL=<函数名>
check_cron_alloc() {
  awk '
    # 只认「分配点」：行内必须出现 alloc( 调用；`CronJob* job = (CronJob*)p;` 是从参数取，不算
    /CronJob\* job = \(CronJob\*\)/ && /alloc[[:space:]]*\(/ {
      if ($0 ~ /xmalloc/) printf "ALLOC job=xmalloc pos=%d\n", NR
      else                printf "ALLOC job=OTHER pos=%d\n", NR
    }
    /job->args = \(LXValue\*\)/ && /alloc[[:space:]]*\(/ {
      if ($0 ~ /xmalloc/) printf "ALLOC args=xmalloc pos=%d\n", NR
      else                printf "ALLOC args=OTHER pos=%d\n", NR
    }
    # pthread_create 失败路径：紧邻的两次释放必须是 xfree
    /pthread_create\(&t, NULL, cron_thread, job\)/ { create_line = NR }
    /\<free\(job\);/ || /\<free\(job->args\);/ {
      if (create_line > 0 && NR > create_line && NR - create_line < 20)
        printf "FREEFAIL other pos=%d\n", NR
    }
    /\<xfree\(job\);/ { if (create_line > 0 && NR > create_line && NR - create_line < 20) printf "FREEFAIL xfree pos=%d\n", NR }
  ' "$1"
}

# [1] 真源码静态判据
out=$(check_cron_alloc "$RC")
echo "  [1] 真源码："
printf '%s\n' "$out" | sed 's/^/      /'
alloc_ok=$(printf '%s\n' "$out" | grep -c 'ALLOC \(job\|args\)=xmalloc' || true)
alloc_bad=$(printf '%s\n' "$out" | grep -c 'ALLOC \(job\|args\)=OTHER' || true)
freefail_bad=$(printf '%s\n' "$out" | grep -c 'FREEFAIL other' || true)
if [ "$alloc_ok" -ge 2 ] && [ "$alloc_bad" -eq 0 ] && [ "$freefail_bad" -eq 0 ]; then
  ok "[1] cron 的 job/args 均 xmalloc，失败路径 xfree（与 spawn/timer 同款）"
else
  bad "[1] cron 存在 malloc 与 xfree 错配（缺陷 844 回归）"
fi

# [2] 静态负控
tmpd=$(mktemp -d /tmp/m420-memsan-neg.XXXXXX) || { echo "M420-MEMSAN-ENV rc=3"; exit 3; }
cat > "$tmpd/neg.c" <<'NEG'
LXValue bi_cron_bad(LXValue* args, int nargs, void* ctx) {
    CronJob* job = (CronJob*)malloc(sizeof(CronJob));
    job->args = (LXValue*)malloc(sizeof(LXValue) * 1);
    pthread_create(&t, NULL, cron_thread, job);
    if (0) { free(job->args); free(job); }
    return px_null();
}
NEG
negout=$(check_cron_alloc "$tmpd/neg.c")
neg_bad=$(printf '%s\n' "$negout" | grep -c '=OTHER\|FREEFAIL other' || true)
echo "  [2] 负控（复原 malloc/free）：bad=$neg_bad"
if [ "$neg_bad" -ge 2 ]; then
  ok "[2] 负控红：复原成 malloc/free 时检查器判红（前提自证：检查器真在看分配器名）"
else
  bad "[2] 负控不红 —— 检查器没有牙"
fi
rm -rf "$tmpd"

# ---------- sanitizer 环境 ----------
CC=gcc
command -v $CC >/dev/null 2>&1 || { echo "M420-MEMSAN-ENV rc=3（缺 gcc）"; exit 3; }
asan_lib=$($CC -print-file-name=libasan.so 2>/dev/null)
if [ ! -e "$asan_lib" ] && ! echo 'int main(){return 0;}' | $CC -fsanitize=address -x c - -o /tmp/m420-asan-probe.$$ >/dev/null 2>&1; then
  echo "M420-MEMSAN-ENV rc=3（缺 ASan 运行时）"; exit 3
fi

WRAP=/tmp/m420-cc-asan   # 固定路径：cache key 含 cc 路径 ⇒ 固定才能复用已插桩的 runtime .o
cat > "$WRAP" <<'WEOF'
#!/bin/bash
args=()
for a in "$@"; do [ "$a" = "-static" ] && continue; args+=("$a"); done
exec gcc "${args[@]}" -fsanitize=address -fno-omit-frame-pointer -g
WEOF
chmod +x "$WRAP"

# [3] 工具正控：故意越界必须被 ASan 抓到
cat > /tmp/m420-asan-pos.$$.c <<'CEOF'
#include <stdlib.h>
int main(void){ char* p = (char*)malloc(4); p[8] = 1; free(p); return 0; }
CEOF
"$WRAP" -o /tmp/m420-asan-pos.$$ /tmp/m420-asan-pos.$$.c 2>/dev/null
ASAN_OPTIONS=detect_leaks=0 /tmp/m420-asan-pos.$$ >/dev/null 2>/tmp/m420-asan-pos.$$.err
if grep -q 'AddressSanitizer' /tmp/m420-asan-pos.$$.err; then
  ok "[3] 工具正控：ASan 抓到故意越界（sanitizer 真在工作）"
else
  bad "[3] 工具正控失败 —— ASan 没抓，[4] 的绿不可信"
fi

# [4] 动态正控：ASan 构建 + 真跑
[ -f "$SRC" ] || { echo "M420-MEMSAN-ENV rc=3（缺 $SRC）"; exit 3; }
blog=/tmp/m420-cron-asan-build.$$.log
if ! PX_CC="$WRAP" "$RT/tools/px" build --no-quic "$SRC" > "$blog" 2>&1; then
  echo "  [4] 构建失败（见 $blog）"; tail -5 "$blog"
  echo "M420-MEMSAN rc=3 pass=$pass fail=$fail"; exit 3
fi
BIN="$RT/examples/build/m28_cron"
[ -x "$BIN" ] || { echo "M420-MEMSAN-ENV rc=3（缺产物）"; exit 3; }

# [5] 前提自证：产物真链了 ASan
#   ⚠ set -o pipefail 下 `strings | grep -q` 会因 SIGPIPE 把管道判非零（M412 pkg_pipe_grep 同族）
#   ⇒ 一律先落文件再 grep，不用管道形态。
strings -a "$BIN" > /tmp/m420-asan-syms.$$ 2>/dev/null
if grep -q '__asan_\|AddressSanitizer' /tmp/m420-asan-syms.$$; then
  ok "[5] 前提自证：产物含 ASan 符号（这次跑的确实是插桩版）"
else
  bad "[5] 产物不含 ASan 符号 —— 构建没走 sanitizer 通道"
fi

ASAN_OPTIONS=detect_leaks=0 timeout 90 "$BIN" > /tmp/m420-cron-asan.$$.out 2>/tmp/m420-cron-asan.$$.err
rc=$?
nasan=$(grep -c 'AddressSanitizer' /tmp/m420-cron-asan.$$.err || true)
echo "  [4] run rc=$rc asan_report=$nasan"
if [ "$rc" -eq 0 ] && [ "$nasan" -eq 0 ]; then
  ok "[4] ASan 运行零报告（cron 线程不再越界读 malloc 块）"
else
  bad "[4] ASan 运行有报告（缺陷 844 回归）"
  grep -A4 'AddressSanitizer' /tmp/m420-cron-asan.$$.err | head -12 | sed 's/^/      /'
fi

rm -f /tmp/m420-asan-syms.$$ /tmp/m420-asan-pos.$$ /tmp/m420-asan-pos.$$.c /tmp/m420-asan-pos.$$.err
echo "M420-MEMSAN-CRON pass=$pass fail=$fail"
[ "$fail" -eq 0 ] && exit 0
exit 1
