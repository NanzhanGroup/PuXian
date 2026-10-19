#!/usr/bin/env bash
# ============================================================
# examples/m313_mysql_driver/verify.sh
# ------------------------------------------------------------
# M313（第 184 轮）：**mysql 驱动的三处握手缺陷 + 读无界**
#
# 来历（半山 2026-10-19 总线报障 m18dfe2e3b5203b56f130014f，附补丁 mysql-0.2.0-fix.patch）：
#   `mysql_connect()` 连 MySQL 5.7 时**永久阻塞**（不返回/不报错/不超时），
#   服务器错误日志同时出现 "Bad handshake"。
#   ① `soff = ver[1] + 1` —— `my_cstr_at()` 返回的是**终止符之后**的偏移，再 +1 整段错位
#      （nonce 只读到 8/20 字节、capability 读成 0x15C12DFF 而非 0xC203FFFF）
#   ② CONNECT_WITH_DB 用错常量（0x00000800 是 CLIENT_IGNORE_SPACE，应为 0x00000008）
#      ⇒ payload 末尾塞了库名、标志位却没置 ⇒ 服务器判「多余字节」⇒ Bad handshake
#   ③ `my_recv_n()` → `tcp_recv_ex` **无超时** ⇒ 协议不匹配/网络抖动时 worker 永久挂死
#
# ⭐ 为什么这道门重要：registry/mysql 此前**没有任何动态覆盖** ——
#   上游 mysql_test.px 需要真实 MariaDB（CI 上恒 SKIP）。三处缺陷**只在真实握手时显形**，
#   而「三轨对拍」看不见它们（同一份 .px 代码三轨跑的是同一份语义）。
#   ⇒ 本门用**普贤自己写的 mock MySQL 服务端**（mock_mysql.px）把握手真正跑起来，
#     并**逐字段**校验能力位 / 用户名 / 库名 / 插件名 / 20 字节 auth token / 有无多余字节。
#
# 判据层次：
#   [1] 静态：两版本（0.1.0 / 0.2.0）三处修复齐备 + 读超时已设 + 规模锚点
#   [2] 构建（mock + 0.2.0 探针 + 0.1.0 探针）
#   [3] 动态主判据（0.2.0）：握手 + 认证 token 对拍 + USE + SELECT ⇒ PROBE-OK（affected=7）
#   [4] 动态（0.1.0）：同一份代码、同样三处修复 ⇒ 同样 PROBE-OK
#   [5] 读超时有界：对「只接受连接、不发字节」的服务端 ⇒ 有界返回 Err（修前永久阻塞）
#   [6] 负控 A：忠实撤回 `soff = ver[1]` ⇒ 必须 Err（原因指到 auth token 错位）
#   [7] 负控 B：忠实撤回 CONNECT_WITH_DB 位 ⇒ 必须 Err（原因指到 IGNORE_SPACE/多余字节）
#   [8] 负控 C：去掉 tcp_opt 读超时 ⇒ 超时档必须**挂住**（timeout 5 判 rc=124）
#   [9] 覆盖边界（如实）
#
# ⚠️ 本门**不改仓库源码**（负控改的是 W 里的驱动副本）⇒ 不需要 restore 仓库；
#   但仍 source `selfhost/gate_lock.sh`（M276：与全量门互斥，避免互相看见中间态）。
#
# 用法：verify.sh [--neg-skip]
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
W="${M313_GATE_W:-$(mktemp -d /tmp/m313_gate.XXXXXX)}"
NEG_SKIP=0
for a in "$@"; do [ "$a" = "--neg-skip" ] && NEG_SKIP=1; done

PASS=0; FAIL=0
ok()  { PASS=$((PASS + 1)); echo "  ✅ $1"; }
bad() { FAIL=$((FAIL + 1)); echo "  ❌ $1"; }
chk() { if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1：期望 $3，实际 $2"; fi; }
num() { case "${1:-}" in ''|*[!0-9]*) echo 0 ;; *) echo "$1" ;; esac; }

D020="$ROOT/registry/mysql/0.2.0/mysql.px"
D010="$ROOT/registry/mysql/0.1.0/mysql.px"
TO=$(command -v timeout || true)
MOCK_PID=""
PORTS_USED=""

cleanup() {
  if [ -n "$MOCK_PID" ] && kill -0 "$MOCK_PID" 2>/dev/null; then kill "$MOCK_PID" 2>/dev/null; fi
  [ "${KEEP_W:-0}" = "1" ] || rm -rf "$W"
}
trap cleanup EXIT

port_new() { local p=$((20000 + RANDOM % 500)); PORTS_USED="$PORTS_USED $p"; echo "$p"; }

# 去注释（判据只看代码行 —— M251 老坑：注释里的同名文本会造成假红/假绿）
strip_comments() { sed -e 's/[[:space:]]*#.*$//' "$1"; }

echo "══ M313 · mysql 驱动：握手偏移 / 能力位 / 读无界（缺陷 536–538）══"

# ── [1] 静态 ─────────────────────────────────────────────────
echo "── [1] 静态：两版本三处修复齐备（去注释后判）"
for f in "$D020" "$D010"; do
  tag="$(basename "$(dirname "$f")")"
  code="$W/code_$tag.txt"; strip_comments "$f" > "$code"
  chk "[1.1] $tag soff 用 ver[1]（不再 +1）" "$(grep -c 'soff = ver\[1\]$' "$code")" "1"
  chk "[1.2] $tag 无 soff = ver[1] + 1" "$(grep -c 'soff = ver\[1\] *+ *1' "$code")" "0"
  chk "[1.3] $tag CONNECT_WITH_DB 用 0x00000008（2 处：置位 + 判据）" "$(grep -c 'cap41 [|&] 0x00000008' "$code")" "2"
  chk "[1.4] $tag 无 0x00000800（IGNORE_SPACE）" "$(grep -c '0x00000800' "$code")" "0"
  chk "[1.5] $tag 只在给了库名时置位（want_db）" "$(grep -c 'want_db' "$code")" "2"
  chk "[1.6] $tag 建连后设读超时（tcp_opt + read_timeout_ms）" "$(grep -c 'tcp_opt(fd, {"read_timeout_ms"' "$code")" "1"
  chk "[1.7] $tag 形参与文档一致（dbname/opts 有默认值）" "$(grep -c 'def mysql_connect(host, port, user, password, dbname = "", opts = null)' "$code")" "1"
done
L020="$(wc -l < "$D020")"; L010="$(wc -l < "$D010")"
if [ "$L020" -ge 400 ] && [ "$L010" -ge 360 ]; then ok "[1.8] 规模锚点（0.2.0=$L020 行 ≥400 · 0.1.0=$L010 行 ≥360）"; else bad "[1.8] 规模锚点：0.2.0=$L020 / 0.1.0=$L010"; fi
# 两版本三处必须**同源**（半山原话：「0.1.0 是同一份代码，行号相同」）
A020="$W/a020"; A010="$W/a010"
grep -E 'soff = ver\[1\]|cap41 [|&] 0x00000008|tcp_opt\(fd, \{"read_timeout_ms"' "$W/code_0.2.0.txt" | sed 's/[0-9][0-9]*//g' > "$A020"
grep -E 'soff = ver\[1\]|cap41 [|&] 0x00000008|tcp_opt\(fd, \{"read_timeout_ms"' "$W/code_0.1.0.txt" | sed 's/[0-9][0-9]*//g' > "$A010"
if cmp -s "$A020" "$A010"; then ok "[1.9] 两版本三处修复形态一致（同一份代码）"; else bad "[1.9] 两版本三处修复形态不一致"; fi

# ── [2] 构建 ─────────────────────────────────────────────────
echo "── [2] 构建（mock + 两版本探针；驱动复制进 W，仓库零写入）"
mk_driver_copy() { # $1=src dir, $2=dst dir
  mkdir -p "$2" && cp -f "$1"/*.px "$2"/ 2>/dev/null
}
mk_driver_copy "$ROOT/registry/mysql/0.2.0" "$W/drv020"
mk_driver_copy "$ROOT/registry/mysql/0.1.0" "$W/drv010"
mk_probe() { # $1=drv dir name, $2=out name
  sed "s|\.\./\.\./registry/mysql/0\.2\.0/mysql\.px|$1/mysql.px|" "$HERE/probe.px" > "$W/$2.px"
}
mk_probe drv020 probe020
mk_probe drv010 probe010
BUILD_OK=1
for f in "$HERE/mock_mysql.px" "$W/probe020.px" "$W/probe010.px"; do
  if ! out=$("$ROOT/tools/px" build "$f" 2>&1); then echo "$out" | tail -3; BUILD_OK=0; fi
done
chk "[2.1] 三个产物构建成功" "$BUILD_OK" "1"
MOCK="$HERE/build/mock_mysql"; P020="$W/build/probe020"; P010="$W/build/probe010"
for b in "$MOCK" "$P020" "$P010"; do [ -x "$b" ] || { bad "[2.2] 产物缺失：$b"; BUILD_OK=0; }; done
[ "$BUILD_OK" = "1" ] && ok "[2.2] 产物齐备（mock_mysql / probe020 / probe010）"

# ── 运行装置 ─────────────────────────────────────────────────
run_mock() { # $1=port $2=mode $3=log ; 设置 MOCK_PID
  : > "$3"
  "$MOCK" "$1" "$2" > "$3" 2>&1 &
  MOCK_PID=$!
  local i
  for i in $(seq 1 200); do grep -q 'MOCK-READY' "$3" 2>/dev/null && break; sleep 0.05; done
  # 强就绪（M283 P2）：不只看 ready 字样，还要端口**真在监听**
  for i in $(seq 1 100); do ss -ltn 2>/dev/null | grep -q ":$1 " && return 0; sleep 0.05; done
  return 1
}
stop_mock() { [ -n "$MOCK_PID" ] && kill "$MOCK_PID" 2>/dev/null; wait "$MOCK_PID" 2>/dev/null; MOCK_PID=""; }

# ── [3] 动态主判据（0.2.0）────────────────────────────────────
echo "── [3] 动态：0.2.0 握手 + 认证 token 对拍 + USE + SELECT"
P=$(port_new); LOG="$W/mock020.log"
if run_mock "$P" ok "$LOG"; then
  ok "[3.0] mock 就绪且端口 $P 在监听"
  OUT="$("$P020" "$P" ok 2>&1)"; RC=$?
  chk "[3.1] 探针退出码" "$RC" "0"
  echo "$OUT" | sed 's/^/     /'
  echo "$OUT" | grep -q '^PROBE-OK' && ok "[3.2] PROBE-OK（连接 + 查询全通）" || bad "[3.2] 未拿到 PROBE-OK"
  echo "$OUT" | grep -q 'affected=7' && ok "[3.3] affected=7（响应包解析正确）" || bad "[3.3] affected 不是 7"
  grep -q 'MOCK-HANDSHAKE-OK' "$LOG" && ok "[3.4] 服务端逐字段校验通过（能力位/用户/库名/插件/token/无多余字节）" || bad "[3.4] 服务端拒绝了握手：$(grep -m1 'MOCK-REJECT' "$LOG")"
  grep -q 'sql=USE pxtest' "$LOG" && ok "[3.5] 库名走 USE 命令（与 CONNECT_WITH_DB 位一致）" || bad "[3.5] 未见 USE pxtest"
else
  bad "[3.0] mock 未就绪（端口 $P）"
fi
stop_mock

# ── [4] 动态（0.1.0）─────────────────────────────────────────
echo "── [4] 动态：0.1.0 同形态（同一份代码 ⇒ 必须一起修）"
P=$(port_new); LOG="$W/mock010.log"
if run_mock "$P" ok "$LOG"; then
  OUT="$("$P010" "$P" ok 2>&1)"; RC=$?
  chk "[4.1] 探针退出码" "$RC" "0"
  echo "$OUT" | grep -q '^PROBE-OK' && ok "[4.2] 0.1.0 PROBE-OK" || bad "[4.2] 0.1.0 未通过：$(echo "$OUT" | tail -1)"
  grep -q 'MOCK-HANDSHAKE-OK' "$LOG" && ok "[4.3] 0.1.0 服务端校验通过" || bad "[4.3] 0.1.0 服务端拒绝：$(grep -m1 'MOCK-REJECT' "$LOG")"
else
  bad "[4.0] mock 未就绪（端口 $P）"
fi
stop_mock

# ── [5] 读超时有界 ───────────────────────────────────────────
echo "── [5] 读超时有界：服务端只 accept、不发字节 ⇒ 必须**有界**返回"
P=$(port_new); LOG="$W/mock_silent.log"
if run_mock "$P" silent "$LOG"; then
  T0=$(date +%s%N)
  OUT="$("$P020" "$P" timeout 2>&1)"; RC=$?
  T1=$(date +%s%N); EL=$(( (T1 - T0) / 1000000 ))
  echo "     $OUT（wall=${EL}ms）"
  chk "[5.1] 退出码（有界返回 Err ⇒ 0）" "$RC" "0"
  echo "$OUT" | grep -q 'PROBE-TIMEOUT-ERR' && ok "[5.2] 有界返回 Err" || bad "[5.2] 没有有界返回：$OUT"
  echo "$OUT" | grep -q 'i/o timeout' && ok "[5.3] 原因是读超时（不是 EOF / 不是连接失败）" || bad "[5.3] 原因不是 i/o timeout"
  if [ "$EL" -lt 5000 ]; then ok "[5.4] 墙钟 ${EL}ms < 5000ms（timeout_ms=500 生效）"; else bad "[5.4] 墙钟 ${EL}ms 过长"; fi
else
  bad "[5.0] silent mock 未就绪（端口 $P）"
fi
stop_mock

# ── [6][7][8] 负控 ───────────────────────────────────────────
neg_drv() { # $1=name, $2=python patch 表达式文件
  local d="$W/drv_$1"; mk_driver_copy "$ROOT/registry/mysql/0.2.0" "$d"
  python3 - "$d/mysql.px" "$1" <<'PY'
import sys
p, name = sys.argv[1], sys.argv[2]
s = open(p, encoding='utf-8').read()
if name == 'soff':
    n = s.count('\n    soff = ver[1]\n')
    s = s.replace('\n    soff = ver[1]\n', '\n    soff = ver[1] + 1\n')
elif name == 'cap':
    n = s.count('cap41 | 0x00000008') + s.count('cap41 & 0x00000008')
    s = s.replace('cap41 | 0x00000008', 'cap41 | 0x00000800').replace('cap41 & 0x00000008', 'cap41 & 0x00000800')
elif name == 'notmo':
    n = s.count('tcp_opt(fd, {"read_timeout_ms": rtmo, "write_timeout_ms": wtmo})')
    s = s.replace('    var tmo_res = tcp_opt(fd, {"read_timeout_ms": rtmo, "write_timeout_ms": wtmo})\n',
                  '    var tmo_res = {"ok": true, "err": ""}\n')
else:
    raise SystemExit('unknown patch ' + name)
open(p, 'w', encoding='utf-8').write(s)
print('patched', name, 'hits', n)
PY
  [ $? -eq 0 ] || return 1
  mk_probe "drv_$1" "probe_$1"
  "$ROOT/tools/px" build "$W/probe_$1.px" >/dev/null 2>&1 || return 1
  return 0
}

if [ "$NEG_SKIP" = "1" ]; then
  echo "── [6][7][8] 负控：**已按 --neg-skip 跳过**（CI 档；重建两次驱动不划算）"
else
  echo "── [6] 负控 A：忠实撤回 soff = ver[1] ⇒ 必须 Err"
  if neg_drv soff; then
    P=$(port_new); LOG="$W/mock_negA.log"
    if run_mock "$P" ok "$LOG"; then
      OUT="$("$W/build/probe_soff" "$P" ok 2>&1)"; RC=$?
      echo "     $OUT"
      chk "[6.1] 退回偏移 +1 ⇒ 退出码非 0" "$([ "$RC" -ne 0 ] && echo yes || echo no)" "yes"
      echo "$OUT" | grep -q 'auth token 不符' && ok "[6.2] 原因指到 nonce 错位（auth token 不符）" || bad "[6.2] 原因不明确：$OUT"
      grep -q 'MOCK-REJECT' "$LOG" && ok "[6.3] 服务端拒绝（不是客户端自己崩）" || bad "[6.3] 服务端没有拒绝记录"
    else bad "[6.0] mock 未就绪"; fi
    stop_mock
  else bad "[6.0] 负控 A 打桩失败"; fi

  echo "── [7] 负控 B：忠实撤回 CONNECT_WITH_DB 位 ⇒ 必须 Err"
  if neg_drv cap; then
    P=$(port_new); LOG="$W/mock_negB.log"
    if run_mock "$P" ok "$LOG"; then
      OUT="$("$W/build/probe_cap" "$P" ok 2>&1)"; RC=$?
      echo "     $OUT"
      chk "[7.1] 退回 0x800 ⇒ 退出码非 0" "$([ "$RC" -ne 0 ] && echo yes || echo no)" "yes"
      echo "$OUT" | grep -q 'IGNORE_SPACE\|多余' && ok "[7.2] 原因指到能力位/多余字节" || bad "[7.2] 原因不明确：$OUT"
    else bad "[7.0] mock 未就绪"; fi
    stop_mock
  else bad "[7.0] 负控 B 打桩失败"; fi

  echo "── [8] 负控 C：去掉读超时 ⇒ 超时档必须**挂住**（证明 [5] 不是白给的）"
  if neg_drv notmo; then
    P=$(port_new); LOG="$W/mock_negC.log"
    if run_mock "$P" silent "$LOG"; then
      if [ -n "$TO" ]; then
        "$TO" -k 2 5 "$W/build/probe_notmo" "$P" timeout > "$W/negC.out" 2>&1; RC=$?
      else
        "$W/build/probe_notmo" "$P" timeout > "$W/negC.out" 2>&1 & local np=$!; sleep 5; kill "$np" 2>/dev/null; wait "$np" 2>/dev/null; RC=124
      fi
      echo "     rc=$RC · $(tail -1 "$W/negC.out" 2>/dev/null)"
      chk "[8.1] 无读超时 ⇒ 挂住（rc=124/137）" "$([ "$RC" = "124" ] || [ "$RC" = "137" ] && echo hung || echo returned)" "hung"
    else bad "[8.0] mock 未就绪"; fi
    stop_mock
  else bad "[8.0] 负控 C 打桩失败"; fi
fi

# ── [9] 覆盖边界 ─────────────────────────────────────────────
echo "── [9] 覆盖边界（如实）"
cat <<'EOF'
  ℹ️ ① 本门覆盖「握手 / 认证 / 文本协议 OK 响应」；**结果集（列定义 + 行）** 仍只由上游
       mysql_test.px 覆盖（需真实 MariaDB，CI 上 SKIP）⇒ 未在本门面内。
  ℹ️ ② 认证只覆盖 mysql_native_password；caching_sha2_password 只支持 **fast auth**
       （full auth 需 TLS/RSA，源码里直接返回 Err）⇒ MySQL 8 + 非 TLS 账号仍会踩，已在文档登记。
  ℹ️ ③ 参数化语句（COM_STMT_*）未在本门面内（上游 mysql_params_test.px 需要真实服务端）。
  ℹ️ ④ 「永久阻塞」的现场是 5.7.44；本门用 mock 复刻协议形态，不依赖具体版本。
EOF

echo
if [ "$FAIL" -eq 0 ]; then
  echo "══ M313-VERIFY-OK · $PASS 通过 / 0 失败 ══"
  exit 0
else
  echo "══ M313-VERIFY-FAIL · $PASS 通过 / $FAIL 失败 ══"
  exit 1
fi
