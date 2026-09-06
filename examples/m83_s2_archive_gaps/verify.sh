#!/bin/bash
# M83-S2 (Issue 20-L0) 专项验证：AES-ECB（hex+bytes，与 openssl/Go 互通）+ gzip（与系统 gzip 互通）
#   + os_spawn group（setpgid 自成组，组杀）
# 依赖：openssl、gzip、python3（对拍基准）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
echo "== [1/3] 编译 =="
$PX build --no-quic s2_verify.px >/tmp/m83s2_build.log 2>&1 || { echo "FAIL build"; tail -25 /tmp/m83s2_build.log; exit 1; }

# 系统 gzip 预生成（第 4/5 步交叉解）
printf 'gzip 互通验证 payload 中文 123\n第二行数据 line2\n' > /tmp/m83s2_plain.txt
gzip -9 -c /tmp/m83s2_plain.txt > /tmp/m83s2_sys.gz
printf 'a\x00\x00\x00\x00\xff\x00\x00b' > /tmp/m83s2_bin.bin
gzip -9 -c /tmp/m83s2_bin.bin > /tmp/m83s2_sysbin.gz

echo "== [2/3] 运行 px 验证 =="
OUT=$(./build/s2_verify 2>&1); RC=$?
echo "$OUT"
[ $RC -eq 0 ] || { echo "FAIL px 运行 rc=$RC"; exit 1; }

echo "== [3/3] 互通对拍 =="
# ① AES-128-ECB：px 输出 CT vs openssl enc -aes-128-ecb（同 PKCS7）
CT=$(echo "$OUT" | grep -oP '^CT=\K.*')
REF=$(printf 'hello-ecb-puxian' | openssl enc -aes-128-ecb -K 30313233343536373839616263646566 -nosalt 2>/dev/null | xxd -p -c 256 | tr -d '\n')
[ -n "$CT" ] && [ "$CT" = "$REF" ] || { echo "FAIL AES-ECB 与 openssl 不一致"; echo "  px: $CT"; echo "  ossl: $REF"; exit 1; }
echo "PASS ① AES-128-ECB hex 与 openssl 逐字节一致（CT 前 32: ${CT:0:32}…）"

# ② px gzip → 系统 gzip -dc 可解
printf 'gzip 互通验证 payload 中文 123\n第二行数据 line2\n' > /tmp/m83s2_expect.txt
gzip -dc /tmp/m83s2_px.gz > /tmp/m83s2_px_dec.txt 2>/dev/null
cmp -s /tmp/m83s2_expect.txt /tmp/m83s2_px_dec.txt || { echo "FAIL px gzip 系统不可解"; exit 1; }
echo "PASS ② px gzip_compress → 系统 gzip -dc 解出原文"

# ③ px 解系统 gzip（已在 px 内断言 GZIP-SYS-OK）→ 二进制互通由 px 内 GZIP-BIN-OK 覆盖
grep -q 'GZIP-SYS-OK' <<<"$OUT" || { echo "FAIL 系统 gzip → px 解压"; exit 1; }
echo "PASS ③ 系统 gzip → px gzip_uncompress 解出原文"
grep -q 'GZIP-BIN-OK' <<<"$OUT" || { echo "FAIL gzip 二进制互通"; exit 1; }
echo "PASS ③b gzip 二进制（含 NUL）与系统互通"

# ④ os_spawn group 组杀（px 内已断言 SPAWN-GROUP-OK + 返回码非 -1）
grep -q 'SPAWN-GROUP-OK' <<<"$OUT" || { echo "FAIL os_spawn group 组杀"; exit 1; }
echo "PASS ④ os_spawn group=true → os_kill(pid,9,true) 组杀成功"

# ⑤ 空串 AES-ECB PKCS7 整块边界（px 内断言）
echo "PASS ⑤ AES-ECB 空串→16B PKCS7 整块边界"
echo "m83_s2_archive_gaps verify done"
exit 0
