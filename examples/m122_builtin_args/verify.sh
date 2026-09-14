#!/usr/bin/env bash
# M122 · 内建函数实参校验一族（qg-issue 77）
# 缺陷：解释器（bootstrap/pxi）的「直调宿主 native」分支缺 arity 校验：
#   · 缺参（json_stringify()）→ 误报 R1003 列表索引越界（原因错+码错）
#   · 多参（json_stringify(1,2)）→ 静默只取 args[0]、rc=0（两轨不一致 + 吞错）
# 修复：ibuiltin.px 50 处直调分支补 len(args) 精确校验，文案逐字对齐 runtime.c（C 轨）。
# 轨：解释器（px run / bootstrap/pxi）· VM 轨（px build 默认）· C 轨（px build --c）
set -u
DIR="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$DIR/../.." && pwd)"
PX="$ROOT/tools/px"
WORK="$(mktemp -d /tmp/m122_gate.XXXXXX)"
PASS=0; FAIL=0
chk(){ if [ "$2" = "$3" ]; then echo "  ✅ $1"; PASS=$((PASS+1)); else echo "  ❌ $1 — 期望[$3] 实得[$2]"; FAIL=$((FAIL+1)); fi; }
chk_has(){ if grep -q -- "$3" "$2"; then echo "  ✅ $1（含 '$3'）"; PASS=$((PASS+1)); else echo "  ❌ $1 — 输出未见 '$3'"; head -2 "$2"; FAIL=$((FAIL+1)); fi; }

comp(){ "$PX" build "$2" "$1" >"$WORK/build.log" 2>&1 || { echo "  ❌ 编译失败 $1"; tail -8 "$WORK/build.log"; FAIL=$((FAIL+1)); return 1; }; }
# 用法: gate <name> <stmt> <期望rc前置标记>
#     断言：解释/VM 轨 rc 相等且非 0，且两轨输出都含 R1002 与文案关键词
gate_err(){  # $1=name $2=stmt $3=文案片段
  local name="$1" stmt="$2" kw="$3"
  cat > "$WORK/$name.px" <<PXI
def main():
    $stmt
PXI
  "$PX" run "$WORK/$name.px" > "$WORK/$name.i.txt" 2>&1; ri=$?
  comp --vm "$WORK/$name.px" && { "$WORK/build/$name" > "$WORK/$name.v.txt" 2>&1; rv=$?; } || rv=99
  comp --c  "$WORK/$name.px" && { "$WORK/build/$name" > "$WORK/$name.c.txt" 2>&1; rc=$?; } || rc=99
  ok="yes"; [ "$ri" -eq 0 ] && ok=no; [ "$rv" -eq 0 ] && ok=no; [ "$rc" -eq 0 ] && ok=no
  chk "$name·三轨 rc 全非 0" "$ok" "yes"
  chk_has "$name·解释轨 R1002" "$WORK/$name.i.txt" "R1002"
  if grep -q "R1003" "$WORK/$name.i.txt"; then echo "  ❌ $name·解释轨仍误报 R1003"; FAIL=$((FAIL+1)); else echo "  ✅ $name·解释轨无 R1003"; PASS=$((PASS+1)); fi
  chk_has "$name·解释轨文案" "$WORK/$name.i.txt" "$kw"
  chk_has "$name·VM 轨文案"   "$WORK/$name.v.txt" "$kw"
  chk_has "$name·C 轨文案"    "$WORK/$name.c.txt" "$kw"
}
# 合法调用护栏（回归：修复不得破坏正常语义）
gate_ok(){ # $1=name $2=stmt $3=期望输出
  local name="$1" stmt="$2" expect="$3"
  cat > "$WORK/$name.px" <<PXI
def main():
    $stmt
PXI
  "$PX" run "$WORK/$name.px" > "$WORK/$name.i.txt" 2>&1; ri=$?
  comp --vm "$WORK/$name.px" && { "$WORK/build/$name" > "$WORK/$name.v.txt" 2>&1; rv=$?; } || rv=99
  chk "$name·两轨 rc=0" "$ri$rv" "00"
  chk "$name·输出一致" "$(diff -q "$WORK/$name.i.txt" "$WORK/$name.v.txt" >/dev/null 2>&1 && echo same || echo diff)" "same"
  chk_has "$name·期望值" "$WORK/$name.i.txt" "$expect"
}

echo "══ A 组：缺参 → R1002（修复：不再误报 R1003） ══"
gate_err jsp0  'print(str(json_stringify()))'                       "json_stringify 需要一个参数"
gate_err jspa0 'print(str(json_parse()))'                            "json_parse 需要一个字符串参数"
gate_err sha0  'print(str(sha256()))'                                "sha256 需要一个参数"
gate_err env0  'print(str(env()))'                                   "env 需要一个变量名"
gate_err ex0   'print(str(exists()))'                                "exists 需要一个路径参数"
gate_err ld0   'print(str(list_dir()))'                              "list_dir 需要一个路径参数"
gate_err by0   'print(str(bytes()))'                                 "bytes 需要一个参数"
gate_err b64e0 'print(str(base64_encode()))'                         "base64_encode 需要一个参数"
gate_err hex0  'print(str(hex_to_int()))'                            "hex_to_int 需要一个参数"
gate_err ih0   'print(str(int_to_hex(255)))'                         "int_to_hex 需要 (n, width) 参数"
gate_err reg1  'print(str(regex_match("a")))'                        "regex_match 需要 2 个参数: (pattern, text)"
gate_err jp1   'print(str(json_path({"a":1})))'                      "json_path 需要 (json, path) 参数"
gate_err or1   'print(str(os_rename("a")))'                          "os_rename 需要 (old_path, new_path) 参数"
gate_err rb1   'print(str(read_bytes()))'                            "read_bytes 需要一个路径参数"
gate_err fs1   'print(str(file_size()))'                             "file_size 需要一个路径参数"
gate_err ow0   'print(str(os_wait()))'                               "os_wait 需要 (pid) 参数"
gate_err owj0  'print(str(os_remove_all()))'                         "os_remove_all 需要 (path) 参数"
gate_err uh0   'print(str(unix_connect()))'                          "unix_connect 需要 (socket_path) 参数"

echo "══ B 组：多参 → R1002（修复：不再静默吞参 rc=0） ══"
gate_err jsp2  'print(str(json_stringify(1, 2)))'                    "json_stringify 需要一个参数"
gate_err jspa2 'print(str(json_parse("{}", 1)))'                     "json_parse 需要一个字符串参数"
gate_err sha2  'print(str(sha256("ab", "cd")))'                      "sha256 需要一个参数"
gate_err regt  'print(str(regex_match("a", "b", "c")))'              "regex_match 需要 2 个参数: (pattern, text)"
gate_err bys   'print(str(bytes_set(bytes("ab"), 0, 1, 2)))'         "bytes_set 需要 (bytes, index, value) 参数"
gate_err exit2 'print(str(exit(0, 1)))'                              "exit 需要 0-1 个参数"

echo "══ C 组：合法调用护栏（回归） ══"
gate_ok ok_jsp 'print(str(json_stringify({})))'                      "null"
gate_ok ok_sha 'print(str(sha256("abc")))'                           "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
gate_ok ok_env 'print(str(env("PATH")))'                             "/"
gate_ok ok_int 'print(str(int("42")))'                               "42"
gate_ok ok_has 'd = {"a": 1}
    print(str(d.has("a")) + " " + str(d.contains("z")))'             "true false"
gate_ok ok_sw  'print(str(starts_with("abc", "ab")))'                "true"

echo ""
echo "══ 汇总：通过 $PASS · 失败 $FAIL ══"
exit $((FAIL > 0))