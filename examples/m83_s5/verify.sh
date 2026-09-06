#!/bin/bash
# M83-S5 (Issue 20-L1) stdlib 四库专项验证：html / cookiejar / multipart / smtp
#   双模式（pxi run + pxc build）纯逻辑断言 + multipart 上传回环（px mp_encode ↔ px http_serve，
#   再 curl -F 对照 ↔ 标准 multipart）+ SMTP 真发信回环（本地假 SMTP server 收信断言）
# 依赖：tools/pxc、python3（假 SMTP）、curl（multipart 对照）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
PORT=18092
SMTP_PORT=2525
mkdir -p build out
SRV_PID=0
SMTP_PID=0
trap 'kill $SRV_PID $SMTP_PID 2>/dev/null; wait $SRV_PID $SMTP_PID 2>/dev/null; rm -f /tmp/m83s5_*.log x.png' EXIT

echo "== [1/6] 解释模式 libs_test（html/cookiejar/multipart 纯逻辑断言）=="
$PX run libs_test.px > /tmp/m83s5_libs_run.log 2>&1 || { echo "FAIL pxi run libs_test"; tail -20 /tmp/m83s5_libs_run.log; exit 1; }
cat /tmp/m83s5_libs_run.log
grep -q 'M83-S5-LIBS-OK' /tmp/m83s5_libs_run.log || { echo "FAIL libs_test 断言"; exit 1; }

echo "== [2/6] 编译模式 libs_test（双模式一致）=="
$PX build --no-quic libs_test.px > /tmp/m83s5_build.log 2>&1 || { echo "FAIL build libs_test"; tail -20 /tmp/m83s5_build.log; exit 1; }
./build/libs_test > /tmp/m83s5_libs_build.log 2>&1 || { echo "FAIL 编译产物运行"; tail -20 /tmp/m83s5_libs_build.log; exit 1; }
grep -q 'M83-S5-LIBS-OK' /tmp/m83s5_libs_build.log || { echo "FAIL libs_test 编译产物断言"; exit 1; }
# 双模式输出一致性（libs_test 断言无随机量）
diff <(grep -v 'M83-S5-LIBS-OK' /tmp/m83s5_libs_run.log) <(grep -v 'M83-S5-LIBS-OK' /tmp/m83s5_libs_build.log) > /dev/null || { echo "FAIL 双模式输出不一致"; exit 1; }
echo "双模式输出一致 OK"

echo "== [3/6] multipart 上传回环：px mp_encode ↔ px http_serve =="
$PX build --no-quic serve_daemon.px > /tmp/m83s5_build.log 2>&1 || { echo "FAIL build serve_daemon"; exit 1; }
./build/serve_daemon $PORT > /tmp/m83s5_srv.log 2>&1 &
SRV_PID=$!
sleep 0.6
curl -s http://127.0.0.1:$PORT/health | grep -q 'ok' || { echo "FAIL serve 未就绪"; tail -5 /tmp/m83s5_srv.log; exit 1; }
$PX build --no-quic m83_s5_client.px > /tmp/m83s5_build.log 2>&1 || { echo "FAIL build client"; exit 1; }
COUT=$(./build/m83_s5_client http://127.0.0.1:$PORT/upload 2>&1); RC=$?
echo "$COUT"
[ $RC -eq 0 ] || { echo "FAIL client 上传 rc=$RC"; exit 1; }
echo "$COUT" | grep -q 'M83-S5-MULTIPART-CLIENT-OK' || { echo "FAIL client 断言"; exit 1; }

echo "== [4/6] multipart 与标准 curl -F 互通（同服务端解析一致）=="
printf 'PNGDATA-curl' > x.png
CURL_OUT=$(curl -s -F token=t1 -F count=5 -F "avatar=@x.png;type=image/png;filename=me.png" http://127.0.0.1:$PORT/upload)
echo "$CURL_OUT"
echo "$CURL_OUT" | grep -q '"token":"t1"' || { echo "FAIL curl form 字段解析"; exit 1; }
echo "$CURL_OUT" | grep -q 'me.png' || { echo "FAIL curl 文件解析"; exit 1; }
echo "$CURL_OUT" | grep -q 'PNGDATA-curl' || { echo "FAIL curl 文件内容"; exit 1; }
kill $SRV_PID 2>/dev/null; wait $SRV_PID 2>/dev/null
SRV_PID=0

echo "== [5/6] SMTP 真发信回环（本地假 SMTP 收信）=="
python3 fake_smtp.py $SMTP_PORT ./out > /tmp/m83s5_smtpsrv.log 2>&1 &
SMTP_PID=$!
sleep 0.5
$PX build --no-quic smtp_test.px > /tmp/m83s5_build.log 2>&1 || { echo "FAIL build smtp_test"; exit 1; }
SOUT=$(./build/smtp_test 127.0.0.1 $SMTP_PORT 2>&1); RC=$?
echo "$SOUT"
[ $RC -eq 0 ] || { echo "FAIL smtp_test rc=$RC"; exit 1; }
echo "$SOUT" | grep -q 'M83-S5-SMTP-OK' || { echo "FAIL smtp 断言"; exit 1; }
sleep 0.3
kill $SMTP_PID 2>/dev/null; wait $SMTP_PID 2>/dev/null
SMTP_PID=0

echo "== [6/6] SMTP 收信内容断言（Subject/From/To/正文/点转义/AUTH）=="
ls out/mail_*.txt > /dev/null 2>&1 || { echo "FAIL 未收到邮件"; exit 1; }
echo "--- mail_1.txt ---"; cat out/mail_1.txt
echo "--- mail_2.txt ---"; cat out/mail_2.txt
MAIL1=$(cat out/mail_1.txt)
MAIL2=$(cat out/mail_2.txt)
echo "$MAIL1" | grep -q 'Subject: M83-S5 SMTP test' || { echo "FAIL Subject"; exit 1; }
echo "$MAIL1" | grep -q 'From: sender@px.test' || { echo "FAIL From"; exit 1; }
echo "$MAIL1" | grep -q 'To: rcpt1@px.test, rcpt2@px.test' || { echo "FAIL To"; exit 1; }
echo "$MAIL1" | grep -q 'hello from std.smtp' || { echo "FAIL 正文"; exit 1; }
echo "$MAIL1" | grep -q '^\.\.keep this line' || { echo "FAIL 行首点应转义为 .."; exit 1; }
echo "$MAIL2" | grep -q 'AUTHUSER=' || { echo "FAIL AUTH LOGIN 未走"; exit 1; }
echo "$MAIL2" | grep -q 'Subject: M83-S5 AUTH test' || { echo "FAIL AUTH 邮件 Subject"; exit 1; }
rm -rf out
rm -f x.png
echo "m83_s5 verify done"
exit 0
