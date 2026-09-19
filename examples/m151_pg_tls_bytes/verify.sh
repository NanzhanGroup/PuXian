#!/usr/bin/env bash
# ═══════════════════════════════════════════════════════════════════════
# M151 验证：TCP 二进制安全（缺陷 137）+ TLS 就地升级（tls_upgrade）+ 解释轨模块诊断
# ---------------------------------------------------------------
# 门做四件事：
#   ① `tcp_send_ex(fd, bytes)` / `tcp_send(fd, bytes)` 必须发**原始字节**
#      （修前：发的是占位符 `"<bytes N>"` 的 9 个字节）—— 受控回显服务端，逐字节比 hex。
#   ② `tls_upgrade(fd, opts)`：PG 式 SSLRequest 协商（发 8 字节 → 读 'S'）后**在同一个 fd
#      上升级 TLS**；与 **Go crypto/tls** 放同一个服务端上跑同一套动作 ⇒ 实现无关子集逐字节一致。
#   ③ 解释轨（`px run`）的「模块缺失」诊断**不再打挂进程**（缺陷 138：cg_pwarn 未定义）：
#      必须打印 `[警告] ...` 到 stderr、stdout 仍有 `ok`、退出码 0；`PX_STRICT_MODULE=1` 时报错退出。
#   ④ 负控 A/B/C：分别把三处关键语义退回去 ⇒ 门**必须变红**
#        A：tcp_send_ex 不认 PX_BYTES（退回占位符发送）
#        B：tls_upgrade 的 verify 缺省改成 true（自签证书必须握手失败）
#        C：篡改探针里的期望 hex（语料）
# 口径：
#   · 服务端在**本机回环**上起（复用 m150 的自签证书；证书用 openssl 现场校验）⇒ 无需外网。
#   · 端口从 19861 起找第一个空闲（可用 M151_PORT_OVERRIDE 固定）。
#   · 负控会改 runtime.c：**开头快照**，末尾必须逐字节还原。
# 用法：bash examples/m151_pg_tls_bytes/verify.sh
# 退出码：0 = M151-VERIFY-OK；1 = 有失败项
# ═══════════════════════════════════════════════════════════════════════
set -u
cd "$(dirname "$0")"
PX=../../tools/px
RT=../../runtime/runtime.c
LOG=/tmp/m151_build.log
FAIL=0
mkdir -p build
cp "$RT" /tmp/m151_rt_snapshot.c

# ── 缺陷 139（第 33 轮）：负控会改写 runtime/*.c；门**被打断**时篡改态会静默留在工作区 ──
#   （语法合法、语义反向 ⇒ 编译器不报错；而本轮 runtime.c 本来就带未提交改动 ⇒ `git diff` 判不出来。）
#   两道防线：① 开门先查「负控残留标记 NEGCTL」；② 信号兜底还原快照。
for _f in ../../runtime/runtime.c ../../runtime/vm.c; do
    if [ -f "$_f" ] && grep -q 'NEGCTL' "$_f" 2>/dev/null; then
        echo "FAIL 负控残留：$_f 仍含 NEGCTL 标记（上一轮门被中断？先还原再跑）"
        exit 1
    fi
done

cp px_probe.px /tmp/m151_probe_keep.px
restore_rt() { [ -f /tmp/m151_rt_snapshot.c ] && cp /tmp/m151_rt_snapshot.c "$RT" 2>/dev/null; [ -f /tmp/m151_probe_keep.px ] && cp /tmp/m151_probe_keep.px px_probe.px 2>/dev/null; }
trap restore_rt INT TERM HUP

say() { echo "$@"; }
chk() { # chk <描述> <实际> <期望>
  if [ "$2" = "$3" ]; then echo "  ✅ $1"; else echo "  ❌ $1 实际=[$2] 期望=[$3]"; FAIL=$((FAIL+1)); fi
}
chkc() { # chkc <描述> <条件命令…>
  shift
  if "$@" >/dev/null 2>&1; then echo "  ✅ $1"; else echo "  ❌ $1"; FAIL=$((FAIL+1)); fi
}

free_port() {
  python3 - "$1" <<'PY'
import socket, sys
p = int(sys.argv[1])
for q in range(p, p + 200):
    s = socket.socket()
    try:
        s.bind(("127.0.0.1", q)); s.close(); print(q); break
    except OSError:
        s.close()
PY
}
PORT_BASE=${M151_PORT_OVERRIDE:-19861}
P_ECHO=$(free_port $PORT_BASE)
P_SSL=$(free_port $((PORT_BASE + 50)))

rebuild() { bash "$PX" build px_probe.px >>"$LOG" 2>&1; }
run_px() { ./build/px_probe "$P_ECHO" "$P_SSL"; }

say "── 构建 ──"
: > "$LOG"
if ! bash "$PX" build px_probe.px >>"$LOG" 2>&1; then
  say "构建失败，见 $LOG"; tail -20 "$LOG"; exit 1
fi
say "  ✅ 构建成功"

# ══ ① 二进制安全 ══
say "── ① TCP 二进制安全（tcp_send_ex / tcp_send + bytes）──"
: > /tmp/m151_echo.log
python3 pg_tls_server.py "$P_ECHO" echo /tmp/m151_echo.log >/dev/null 2>&1 &
SRV1=$!
sleep 0.6
OUT1_RAW=$(run_px)
OUT1=$(echo "$OUT1_RAW" | grep '^[AB]|')
echo "$OUT1" | sed 's/^/    /'
chk "A 段 same=true" "$(echo "$OUT1" | sed -n 's/^A|.*|same=\(.*\)$/\1/p')" "true"
chk "B 段 same=true" "$(echo "$OUT1" | sed -n 's/^B|.*|same=\(.*\)$/\1/p')" "true"
chk "A 段 recv= 原文" "$(echo "$OUT1" | sed -n 's/^A|.*|recv=\([^|]*\)|.*$/\1/p')" "410042ffc3285a"
chk "B 段 recv= 原文" "$(echo "$OUT1" | sed -n 's/^B|.*|recv=\([^|]*\)|.*$/\1/p')" "410042ffc3285a"
# 服务端视角：收到的两块必须就是载荷本身（不是占位符）
if grep -q "reqs=410042ffc3285a,627965" /tmp/m151_echo.log; then
  say "  ✅ 服务端侧也收到原始载荷（含 NUL/0xFF/非法 UTF-8）"
else
  say "  ❌ 服务端侧收到的不是原始载荷：$(cat /tmp/m151_echo.log | head -2)"; FAIL=$((FAIL+1))
fi

# ══ ② tls_upgrade（PG 式协商） ══
say "── ② tls_upgrade（PG 式 SSLRequest 协商 → 同一 fd 升级 TLS）──"
: > /tmp/m151_ssl.log
python3 pg_tls_server.py "$P_SSL" pgssl /tmp/m151_ssl.log >/dev/null 2>&1 &
SRV2=$!
sleep 0.6
OUT2_RAW=$(run_px)
OUT2=$(echo "$OUT2_RAW" | grep '^C|')
# ⚠️ 服务端**必须活到 Go 探针跑完**（第 32 轮同型教训：先 kill 再跑 Go 探针 ⇒
#    Go 侧 conn|fail，看起来像「Go 实现失败」，实际是夹具被自己拆了）。
echo "$OUT2" | sed 's/^/    /'
GOOUT=$(go run go_probe.go "$P_ECHO" "$P_SSL" 2>/dev/null)
kill $SRV1 $SRV2 2>/dev/null; wait $SRV1 $SRV2 2>/dev/null
echo "$GOOUT" | grep '^G|C' | sed 's/^/    go: /'
chk "C 段协商包 = 0000000804d2162f" "$(echo "$OUT2" | sed -n 's/^C|neg=\([^|]*\)|.*$/\1/p')" "0000000804d2162f"
chk "C 段首字节 = S" "$(echo "$OUT2" | sed -n 's/^C|.*|first=\([^|]*\)|.*$/\1/p')" "S"
chk "C 段升级成功" "$(echo "$OUT2" | sed -n 's/^C|.*|up=\([^|]*\)|.*$/\1/p')" "true"
chk "C 段 verify=false（缺省不校验证书）" "$(echo "$OUT2" | sed -n 's/^C|.*|verify=\(.*\)$/\1/p')" "false"
PX_G=$(echo "$OUT2" | sed -n 's/^C|greet=\([^|]*\)|.*$/\1/p')
PX_E=$(echo "$OUT2" | sed -n 's/^C|.*|echo=\(.*\)$/\1/p')
GO_G=$(echo "$GOOUT" | sed -n 's/^G|C|greet=\([^|]*\)|.*$/\1/p')
GO_E=$(echo "$GOOUT" | sed -n 's/^G|C|.*|echo=\(.*\)$/\1/p')
chk "问候语与 Go 一致（READY\\n）" "$PX_G" "$GO_G"
chk "问候语 = 52454144590a" "$PX_G" "52454144590a"
chk "TLS 回显与 Go 逐字节一致" "$PX_E" "$GO_E"
chk "TLS 回显 = ECHO: + 载荷" "$PX_E" "4543484f3a410042ffc3285a"
# 版本号：两实现都报 0x0304/0x0303（**不跨实现比版本/套件**，见速查表事实 136）
PX_VN=$(echo "$OUT2" | sed -n 's/^C|.*|ver_num=\([0-9]*\)|.*$/\1/p')
if [ "$PX_VN" = "772" ] || [ "$PX_VN" = "771" ]; then say "  ✅ 版本号 ∈ {771,772}（$PX_VN）"; else say "  ❌ 版本号异常: $PX_VN"; FAIL=$((FAIL+1)); fi
# 服务端视角：IP 字面量 ⇒ **不发 SNI**（与 Go crypto/tls 的 hostnameInSNI 同口径）
if grep -q "phase=pgssl|sni=|ver=TLSv1" /tmp/m151_ssl.log; then
  say "  ✅ 服务端侧：握手成功且 SNI 为空（IP 字面量不发 SNI）"
else
  say "  ❌ 服务端日志异常：$(cat /tmp/m151_ssl.log | head -3)"; FAIL=$((FAIL+1))
fi
if grep -q "req=0000000804d2162f" /tmp/m151_ssl.log; then
  say "  ✅ 服务端侧确实先收到 8 字节 SSLRequest"
else
  say "  ❌ 服务端侧未收到 SSLRequest"; FAIL=$((FAIL+1))
fi

# ══ ③ 解释轨模块诊断 ══
say "── ③ 解释轨（px run）的模块缺失诊断（缺陷 138）──"
mkdir -p /tmp/m151_diag
cat > /tmp/m151_diag/miss.px <<'PXEOF'
import "./definitely_not_here.px"
print("ok")
PXEOF
DIAG_OUT=$(cd /tmp/m151_diag && bash "$OLDPWD/$PX" run miss.px 2>/tmp/m151_diag/err.txt)
DIAG_RC=$?
chk "解释轨退出码 = 0" "$DIAG_RC" "0"
chk "解释轨 stdout = ok" "$(echo "$DIAG_OUT")" "ok"
if grep -q "找不到模块" /tmp/m151_diag/err.txt; then
  say "  ✅ 解释轨打印了模块缺失警告（而不是未定义变量 cg_pwarn）"
else
  say "  ❌ 解释轨未打印预期的警告：$(cat /tmp/m151_diag/err.txt)"; FAIL=$((FAIL+1))
fi
if grep -q "cg_pwarn" /tmp/m151_diag/err.txt; then
  say "  ❌ 仍报未定义变量 cg_pwarn（缺陷 138 未修）"; FAIL=$((FAIL+1))
else
  say "  ✅ 不再出现 cg_pwarn 未定义"
fi
STRICT_RC=0
(cd /tmp/m151_diag && PX_STRICT_MODULE=1 bash "$OLDPWD/$PX" run miss.px >/dev/null 2>&1) || STRICT_RC=$?
if [ "$STRICT_RC" != "0" ]; then say "  ✅ PX_STRICT_MODULE=1 时拒绝（rc=$STRICT_RC）"; else say "  ❌ PX_STRICT_MODULE=1 未生效"; FAIL=$((FAIL+1)); fi

# ══ ④ 负控 ══
say "── ④ 负控（必须全红）──"
negctl() { # negctl <名> <sed 脚本文件>
  local name="$1"; shift
  cp "$RT" /tmp/m151_rt_before.c
  python3 - "$@" <<'PY'
import sys
path, old, new = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(path, encoding="utf-8").read()
assert s.count(old) == 1, ("锚点失配", s.count(old))
open(path, "w", encoding="utf-8").write(s.replace(old, new))
PY
  [ $? -ne 0 ] && { say "  ⚠️ $name 锚点失配"; cp /tmp/m151_rt_before.c "$RT"; return; }
  rebuild
  : > /tmp/m151_echo.log; : > /tmp/m151_ssl.log
  python3 pg_tls_server.py "$P_ECHO" echo /tmp/m151_echo.log >/dev/null 2>&1 & local s1=$!
  python3 pg_tls_server.py "$P_SSL" pgssl /tmp/m151_ssl.log >/dev/null 2>&1 & local s2=$!
  sleep 0.6
  local o; o=$(run_px)
  kill $s1 $s2 2>/dev/null; wait $s1 $s2 2>/dev/null
  cp /tmp/m151_rt_before.c "$RT"
  echo "$o" | sed "s/^/    [$name] /"
  echo "$o" > /tmp/m151_neg_$name.txt
}

# A：tcp_send_ex 不认 PX_BYTES
cp "$RT" /tmp/m151_rt_before.c
python3 - "$RT" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding="utf-8").read()
old = """    if (args[1].type == PX_STR || args[1].type == PX_BYTES) {
        data = args[1].as.obj->as.str.data;
        len = args[1].as.obj->as.str.len;
    } else { data = px_to_string(args[1]); len = (int)strlen(data); }"""
new = """    if (args[1].type == PX_STR) {   /* NEGCTL-151A */
        data = args[1].as.obj->as.str.data;
        len = args[1].as.obj->as.str.len;
    } else { data = px_to_string(args[1]); len = (int)strlen(data); }"""
assert s.count(old) == 2, s.count(old)
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PY
rebuild
: > /tmp/m151_echo.log
python3 pg_tls_server.py "$P_ECHO" echo /tmp/m151_echo.log >/dev/null 2>&1 & S1=$!
python3 pg_tls_server.py "$P_SSL" pgssl /tmp/m151_ssl.log >/dev/null 2>&1 & S2=$!
sleep 0.6
NEG_A=$(run_px)
kill $S1 $S2 2>/dev/null; wait $S1 $S2 2>/dev/null
cp /tmp/m151_rt_before.c "$RT"
echo "$NEG_A" | sed 's/^/    [A] /'
if echo "$NEG_A" | grep -q "|same=true"; then
  say "  ❌ 负控 A 未判红（去掉 PX_BYTES 后仍 same=true）"; FAIL=$((FAIL+1))
else
  say "  ✅ 负控 A 判红（A/B 段 same=false）"
fi

# B：tls_upgrade 的 verify 缺省改成 true
cp "$RT" /tmp/m151_rt_before.c
python3 - "$RT" <<'PY'
import sys
p = sys.argv[1]
s = open(p, encoding="utf-8").read()
old = """    int fd = (int)args[0].as.i;
    int verify = 0;
    int64_t read_timeout_ms = 0;
    char servername[256];
    servername[0] = 0;
    char host[256];"""
new = """    int fd = (int)args[0].as.i;
    int verify = 1;   /* NEGCTL-151B */
    int64_t read_timeout_ms = 0;
    char servername[256];
    servername[0] = 0;
    char host[256];"""
assert s.count(old) == 1, s.count(old)
open(p, "w", encoding="utf-8").write(s.replace(old, new))
PY
rebuild
: > /tmp/m151_ssl.log
python3 pg_tls_server.py "$P_SSL" pgssl /tmp/m151_ssl.log >/dev/null 2>&1 & S3=$!
sleep 0.6
NEG_B=$(run_px)
kill $S3 2>/dev/null; wait $S3 2>/dev/null
cp /tmp/m151_rt_before.c "$RT"
echo "$NEG_B" | grep '^C|' | sed 's/^/    [B] /'
if echo "$NEG_B" | grep -q "up=true"; then
  say "  ❌ 负控 B 未判红（verify=true 对自签证书仍握手成功）"; FAIL=$((FAIL+1))
else
  say "  ✅ 负控 B 判红（自签证书 + verify=true ⇒ 握手失败）"
fi

# C：篡改探针期望（语料）
cp px_probe.px /tmp/m151_probe_before.px
sed -i 's/^const EXPECT_HEX = .*/const EXPECT_HEX = "410042ffc3285b"/' px_probe.px
rebuild
: > /tmp/m151_echo.log
python3 pg_tls_server.py "$P_ECHO" echo /tmp/m151_echo.log >/dev/null 2>&1 & S4=$!
sleep 0.6
NEG_C=$(run_px)
kill $S4 2>/dev/null; wait $S4 2>/dev/null
cp /tmp/m151_probe_before.px px_probe.px
if echo "$NEG_C" | grep -q "|same=true"; then
  say "  ❌ 负控 C 未判红（篡改期望 hex 后仍 same=true）"; FAIL=$((FAIL+1))
else
  say "  ✅ 负控 C 判红（期望 hex 改一位 ⇒ same=false）"
fi

# ══ 还原验收 ══
say "── 还原验收 ──"
rebuild
if cmp -s "$RT" /tmp/m151_rt_snapshot.c; then say "  ✅ runtime.c 与开门前逐字节一致"; else say "  ❌ runtime.c 未还原"; FAIL=$((FAIL+1)); fi
if grep -q 'NEGCTL' "$RT"; then say "  ❌ runtime.c 仍有 NEGCTL 负控标记"; FAIL=$((FAIL+1)); fi
if cmp -s px_probe.px /tmp/m151_probe_before.px; then say "  ✅ px_probe.px 已还原"; else say "  ❌ px_probe.px 未还原"; FAIL=$((FAIL+1)); fi

if [ "$FAIL" = "0" ]; then
  say ""
  say "M151-VERIFY-OK"
  exit 0
fi
say ""
say "M151-VERIFY-FAIL（$FAIL 项）"
exit 1
