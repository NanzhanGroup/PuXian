#!/usr/bin/env bash
# patch_file.py 自证 v2（M304 候选原型 · 只跑 /tmp，不碰仓库）
# 判据：每一条都必须能独立判红
# ⚠️ 本文件自身的教训（v1 踩过）：**期望串一律用单引号**
#    —— 含 $ 或 \ 的期望放进双引号会被 bash 展开（`set -u` 下直接 unbound variable）
set -u
PF="${PF:-$(cd "$(dirname "$0")/../.." && pwd)/selfhost/patch_file.py}"
W="${W:-/tmp/m304_gate/selftest}"
rm -rf "$W"; mkdir -p "$W"; cd "$W" || exit 1

PASS=0; FAIL=0
ok(){ PASS=$((PASS+1)); printf '  OK   %s\n' "$1"; }
no(){ FAIL=$((FAIL+1)); printf '  FAIL %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1"; else no "$1（期望 [$3] 实际 [$2]）"; fi; }

echo "── T1 元字符零转义（替换文本含 & \\ \$ \\u | ，模式含 . * [ ]）──"
printf 'x = 1;\n' > m1.txt
python3 "$PF" --file m1.txt \
  --old 'x = 1;' \
  --new 'y = "a&b\c$d\u|e"; /* .*[] */' >/dev/null 2>&1
chk "T1a rc=0" "$?" "0"
actual=$(cat m1.txt)
chk "T1b 替换结果逐字节一致" "$actual" 'y = "a&b\c$d\u|e"; /* .*[] */'
if [ "$(printf '%s' "$actual" | od -An -c | wc -l)" -ge 1 ] && printf '%s' "$actual" | grep -q 'a&b\\c\$d\\u'; then
  ok "T1c & / \\ / \$ / \\u 全部原样"
else
  no "T1c 有字符被改写"
fi

echo "── T2 找不到 ⇒ rc=1 且文件一字未动 ──"
printf 'hello\n' > m2.txt; cp m2.txt m2.bak
python3 "$PF" --file m2.txt --old 'NOT_THERE' --new 'zzz' >/dev/null 2>&1; rc=$?
chk "T2a rc=1" "$rc" "1"
if cmp -s m2.bak m2.txt; then ok "T2b 文件未变"; else no "T2b 文件被改动"; fi
if [ -z "$(ls -A . | grep '^\.patch_file\.' || true)" ]; then ok "T2c 无临时文件残留"; else no "T2c 有 .tmp 残留"; fi

echo "── T3 命中数与 --count 不符 ⇒ rc=2 且文件未动 ──"
printf 'A\nA\n' > m3.txt; cp m3.txt m3.bak
python3 "$PF" --file m3.txt --old 'A' --new 'B' --count 1 >/dev/null 2>&1; rc=$?
chk "T3a rc=2（实际 2 次，要求 1 次）" "$rc" "2"
if cmp -s m3.bak m3.txt; then ok "T3b 文件未变"; else no "T3b 文件被改动"; fi
python3 "$PF" --file m3.txt --old 'A' --new 'B' --count 2 >/dev/null 2>&1; rc=$?
chk "T3c --count 2 正确通过" "$rc" "0"
chk "T3d 两处都换成 B" "$(tr -d '\n' < m3.txt)" "BB"

echo "── T4 多行 --spec（调用点零转义）──"
printf 'def f():\n    return 1\n' > m4.py
cat > s4.json <<'JSON'
{"file":"m4.py","patches":[{"old":"def f():\n    return 1\n","new":"def f():\n    return 42  # A&B\\C$D\\u\n","count":1}]}
JSON
python3 "$PF" --spec s4.json >/dev/null 2>&1; rc=$?
chk "T4a rc=0" "$rc" "0"
chk "T4b 多行替换正确（含 \\ 与 \$）" "$(sed -n '2p' m4.py)" '    return 42  # A&B\C$D\u'

echo "── T5 --dry-run 不改文件 ──"
printf 'q\n' > m5.txt; cp m5.txt m5.bak
out=$(python3 "$PF" --file m5.txt --old 'q' --new 'Q' --dry-run 2>&1); rc=$?
chk "T5a rc=0" "$rc" "0"
case "$out" in *"dry_run=1"*) ok "T5b 摘要标明 dry_run=1" ;; *) no "T5b 摘要缺 dry_run" ;; esac
if cmp -s m5.bak m5.txt; then ok "T5c 文件未变"; else no "T5c 文件被改动"; fi
out2=$(python3 "$PF" --file m5.txt --old 'NO' --new 'Q' --dry-run 2>&1); rc=$?
chk "T5d dry-run 也会判 not-found" "$rc" "1"

echo "── T6 文件模式保持 ──"
printf 'p\n' > m6.txt; chmod 640 m6.txt
python3 "$PF" --file m6.txt --old 'p' --new 'P' >/dev/null 2>&1
chk "T6a 模式仍为 640" "$(stat -c '%a' m6.txt)" "640"

echo "── T7 非 UTF-8 / 用法错误 → 明确 rc ──"
printf '\xff\xfe\x00bad' > m7.bin
python3 "$PF" --file m7.bin --old 'x' --new 'y' >/dev/null 2>&1; rc=$?
chk "T7a 非 UTF-8 ⇒ rc=4" "$rc" "4"
python3 "$PF" --file m6.txt >/dev/null 2>&1; rc=$?
chk "T7b 缺 --old/--new ⇒ rc=3" "$rc" "3"
python3 "$PF" --file /nonexistent/x --old 'a' --new 'b' >/dev/null 2>&1; rc=$?
chk "T7c 文件不存在 ⇒ rc=3" "$rc" "3"

echo "── T8 与 sed 对照（对照物 = 缺陷 523 的形状）──"
printf 'keep = 1;\n/* MARK */\n' > m8.txt
sed -i 's|keep = 1;|keep = 2;|' m8.txt; rc=$?
chk "T8a sed 正常替换 rc=0" "$rc" "0"
printf 'keep = 1;\n/* MARK */\n' > m8.txt
sed -i 's|\uNO|X|' m8.txt >/dev/null 2>&1; rc=$?
chk "T8b sed 匹配不到时 rc 仍为 0（静默失效）" "$rc" "0"
printf 'keep = 1;\n/* MARK */\n' > m8.txt
sed -i 's|keep = 1;|r = "\u";|' m8.txt 2>/dev/null
case "$(cat m8.txt)" in
  *'r = "";'*) ok "T8c sed 把 \\u 吃成 u（替换文本被改写）" ;;
  *) no "T8c sed 未改写（对照失效）" ;;
esac
printf 'keep = 1;\n/* MARK */\n' > m8.txt
python3 "$PF" --file m8.txt --old 'keep = 1;' --new 'r = "\u";' >/dev/null 2>&1; rc=$?
chk "T8d patch_file rc=0" "$rc" "0"
case "$(cat m8.txt)" in
  *'r = "\u";'*) ok "T8e patch_file 原样保留 \\u" ;;
  *) no "T8e patch_file 改写了 \\u" ;;
esac
python3 "$PF" --file m8.txt --old '\uNO' --new 'X' >/dev/null 2>&1; rc=$?
chk "T8f patch_file 匹配不到 ⇒ rc=1（响亮）" "$rc" "1"

echo "── T9 幂等失败不改文件（多 patch 中途失败 ⇒ 全部回退）──"
printf 'one\ntwo\n' > m9.txt; cp m9.txt m9.bak
cat > s9.json <<'JSON'
{"file":"m9.txt","patches":[{"old":"one","new":"ONE"},{"old":"NOSUCH","new":"X"}]}
JSON
python3 "$PF" --spec s9.json >/dev/null 2>&1; rc=$?
chk "T9a 第 2 处失败 ⇒ rc=1" "$rc" "1"
if cmp -s m9.bak m9.txt; then ok "T9b 第 1 处的改动未落盘（全有或全无）"; else no "T9b 文件处于半途状态"; fi

echo
printf 'SELFTEST %s · 通过 %d / 失败 %d\n' "$([ "$FAIL" -eq 0 ] && echo OK || echo FAIL)" "$PASS" "$FAIL"
[ "$FAIL" -eq 0 ]
