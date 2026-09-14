#!/usr/bin/env bash
# M122 · 内建函数实参校验一族 —— 复现清单探针（qg-issue 77）
# 对一组"错误实参"探针，分别跑 解释轨(px run) / VM 轨(px build --vm) / C 轨(px build --c)
# 输出：每探针 × 每轨 的 退出码 + 首行输出（识别 段错误/静默 null/两轨不一致）
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="$(mktemp -d /tmp/m122_poke.XXXXXX)"
trap 'rm -rf "$WORK"' EXIT
mkdir -p "$WORK/build"

# probe <name> <body: def main() 之后的语句>
probe() {
  local name="$1" body="$2"
  cat > "$WORK/$name.px" <<PXI
def main():
$body
PXI
}

run_all() {
  local name="$1"
  local txt rc
  # 解释轨
  "$PX" run "$WORK/$name.px" >"$WORK/$name.interp.txt" 2>&1; ri=$?
  # VM 轨
  "$PX" build --vm "$WORK/$name.px" >"$WORK/$name.vm.log" 2>&1 && "$WORK/build/$name" >"$WORK/$name.vm.txt" 2>&1; rv=$?
  # C 轨
  "$PX" build --c "$WORK/$name.px" >"$WORK/$name.c.log" 2>&1 && "$WORK/build/$name" >"$WORK/$name.c.txt" 2>&1; rc=$?
  echo "── $name  (interp=$ri vm=$rv c=$rc)"
  echo "   interp: $(head -1 "$WORK/$name.interp.txt" | cut -c1-100)"
  echo "   vm    : $(head -1 "$WORK/$name.vm.txt"     | cut -c1-100)"
  echo "   c     : $(head -1 "$WORK/$name.c.txt"      | cut -c1-100)"
}


# 先跑一组（打表），确认产物路径后放开
probe p_len_arity '    print(str(len(1, 2)))'
probe p_len_null  '    print(str(len(null)))'
probe p_sw_num    '    print(str(starts_with(1, "a")))'
probe p_sw_arity  '    print(str(starts_with("abc")))'
probe p_range_s   '    print(str(range("a")))'
probe p_sorted_i  '    print(str(sorted(123)))'
probe p_sleep_s   '    print(str(sleep("x")))'
probe p_rd_arity  '    print(str(read_file()))'
probe p_json_a    '    print(str(json_stringify(1, 2)))'
probe p_int_ok    '    print(str(int("42")))'
probe p_has_base  '    d = {"a": 1}'$'\n    print(str(d.has(1)))'
for n in p_len_arity p_len_null p_sw_num p_sw_arity p_range_s p_sorted_i p_sleep_s p_rd_arity p_json_a p_int_ok p_has_base; do
  run_all "$n"
done