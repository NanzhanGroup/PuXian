#!/bin/bash
# M66-S1 m66_proc 专项验证（编译模式）：os 五件套 + unix_connect + os_capture/os_popen/os_kill group
#   + write_file mode + zip 密码（zipcrypto + AES-256）
# 用法：bash verify.sh
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
echo "== [1/6] os_fs_test（qingge 交付 16 断言）=="
$PX build --no-quic os_fs_test.px >/dev/null 2>&1 && ./build/os_fs_test || { echo "FAIL os_fs_test"; exit 1; }
echo "== [2/6] exec_demo（os_exec 进程替换，PID 不变 / echo 透传）=="
$PX build --no-quic exec_demo.px >/dev/null 2>&1 && ./build/exec_demo || { echo "FAIL exec_demo"; exit 1; }
echo "== [3/6] m66_proc_test（capture/popen/kill group/mode/unix 失败路径）=="
$PX build --no-quic proc_test.px >/dev/null 2>&1 && ./build/proc_test || { echo "FAIL proc_test"; exit 1; }
echo "== [4/6] unix_connect 行协议（python 模拟 unix sock daemon）=="
$PX build --no-quic unix_test.px >/dev/null 2>&1 || { echo "FAIL build unix_test"; exit 1; }
python3 - <<'PYEOF' &
import os, socket, time
sock = "/tmp/m66_test.sock"
try: os.unlink(sock)
except FileNotFoundError: pass
s = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
s.bind(sock); s.listen(1); s.settimeout(20)
c, _ = s.accept()
data = c.recv(4096)          # 行协议：读一行
c.sendall(b"pong:" + data)   # 回 pong:<原文>
c.close(); s.close()
os.unlink(sock)
PYEOF
PY_PID=$!
sleep 1
timeout 10 ./build/unix_test; RC=$?
kill $PY_PID 2>/dev/null
[ $RC -eq 0 ] || { echo "FAIL unix_test rc=$RC"; exit 1; }
echo "== [5/6] zipcrypto 密码解包 =="
$PX build --no-quic zip_test.px >/dev/null 2>&1 || { echo "FAIL build zip_test"; exit 1; }
rm -rf /tmp/m66_zip_src && mkdir -p /tmp/m66_zip_src/sub
printf 'hello zipcrypto 中文内容\n' > /tmp/m66_zip_src/a.txt
printf 'secret-data\n' > /tmp/m66_zip_src/sub/b.txt
rm -f /tmp/m66_zc1.zip && (cd /tmp/m66_zip_src && zip -q -P m66pass1 /tmp/m66_zc1.zip a.txt sub/b.txt)
rm -rf /tmp/m66_zc1_out && mkdir -p /tmp/m66_zc1_out
./build/zip_test /tmp/m66_zc1.zip /tmp/m66_zc1_out m66pass1; RC=$?
[ $RC -eq 0 ] || { echo "FAIL zipcrypto rc=$RC"; exit 1; }
grep -q "hello zipcrypto 中文内容" /tmp/m66_zc1_out/a.txt || { echo "FAIL zipcrypto 内容不符"; exit 1; }
grep -q "secret-data" /tmp/m66_zc1_out/sub/b.txt || { echo "FAIL zipcrypto 子目录内容不符"; exit 1; }
echo "== [6/6] WinZip AES-256 密码解包 =="
python3 - <<'PYEOF'
import pyzipper
with pyzipper.AESZipFile("/tmp/m66_aes.zip", "w", compression=pyzipper.ZIP_DEFLATED, encryption=pyzipper.WZ_AES) as zf:
    zf.setpassword(b"m66aes-pass")
    zf.writestr("a.txt", "hello aes-256 中文\n")
    zf.writestr("sub/c.txt", "aes-secret\n")
print("aes zip written")
PYEOF
rm -rf /tmp/m66_aes_out && mkdir -p /tmp/m66_aes_out
./build/zip_test /tmp/m66_aes.zip /tmp/m66_aes_out m66aes-pass; RC=$?
[ $RC -eq 0 ] || { echo "FAIL aes rc=$RC"; exit 1; }
grep -q "hello aes-256 中文" /tmp/m66_aes_out/a.txt || { echo "FAIL aes 内容不符"; exit 1; }
echo "ALL PASS"
