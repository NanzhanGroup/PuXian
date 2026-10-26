#!/bin/bash
# ============================================================
# M103-S2a (Issue 29 GAP-DNS-TXT-1) 专项验证：dns_txt 手写 DNS UDP TXT 查询
# ------------------------------------------------------------
# 缺陷 585 修复（M334）：原版「断网跳过」写在第 [4/4] 步，而第 [3/4] 步的 px 断言
#   里就含 dns_txt("qq.com") ⇒ 断网时先在 [3/4] 判红，跳过分支是**死代码**。
#   同时外网依赖让本门在离线环境**零观测**。
# 现结构（判据档位分明，每段都标「判红 / 观察 / 跳过」）：
#   [1/5] 编译：Go 参照 + 3 个 px 客户端 + DNS 桩（cc）
#   [2/5] 离线段断言 —— **零 DNS 依赖**，判红（非法入参 ⇒ Err）
#   [3/5] 桩档 —— **判红**：门内自建 DNS 桩造语义（trunc/plain/nx/empty），
#         px dns_txt 与桩真值逐字节比 + 与 Go net.LookupTXT 集合对拍（都在命名空间内）
#         前提：root 或 sudo -n + unshare -mn（隔离，只碰本命名空间 lo 与 /etc/resolv.conf）
#         NC1 负控：桩 TCP 面故意破损（tcp-broken）⇒ 判据**必须红**
#   [4/5] 外网档 —— **观察项**：先做 UDP 53 + TCP 53 **双通道探针**（探针在断言之前），
#         任一道不通 ⇒ SKIP 并**响亮计数**，不判红；通了才拿 qq.com 真域名对拍
#         NC2 负控：黑洞 192.0.2.1 双通道必须判 down（证明探针有牙、跳过不是「永远跳过」）
#   [5/5] 结论：F=0 才绿
# 退出码：0 绿 · 1 红
# 用法：bash verify.sh
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
cd "$(dirname "$0")"
PX=../../tools/pxc
STUB_SRC=../m116_builtin_semantics/dns_stub.c
GOBIN=./build/go_txt
mkdir -p build artifacts
TMP=$(mktemp -d); trap 'rm -rf "$TMP"' EXIT
STUB_BIN="$TMP/dns_stub"

P=0; F=0; S=0
ok()    { P=$((P+1)); echo "  ✅ $1"; }
bad()   { F=$((F+1)); echo "  ❌ $1"; }
skips() { S=$((S+1)); echo "  ⏭  $1"; }

# ============ [1/5] 编译 ============
echo "== [1/5] 编译（Go 参照 + 3 个 px 客户端 + DNS 桩）=="
go build -o "$GOBIN" go_txt.go || { echo "FAIL go build"; exit 1; }
for f in dns_txt_test dns_txt_offline dns_txt_stub; do
    $PX build --no-quic "$f.px" > "$TMP/b_$f.log" 2>&1 \
        || { echo "FAIL build $f"; tail -20 "$TMP/b_$f.log"; exit 1; }
done
${CC:-cc} -O2 -o "$STUB_BIN" "$STUB_SRC" > "$TMP/b_stub.log" 2>&1 \
    || { echo "FAIL 编译 DNS 桩（$STUB_SRC）"; tail -20 "$TMP/b_stub.log"; exit 1; }
ok "编译全过（go_txt + 3 个 px + dns_stub）"

# ============ [2/5] 离线段（判红） ============
echo "== [2/5] 离线段断言（零 DNS 依赖，判红）=="
OUT=$("./build/dns_txt_offline" 2>&1); RC=$?
echo "$OUT" | sed 's/^/     | /'
if [ "$RC" = 0 ] && echo "$OUT" | grep -qE 'DNS_TXT_OFFLINE: [0-9]+P/0F'; then
    ok "离线段：非法入参全部 Err 可判定（$(echo "$OUT" | sed -n 's/^DNS_TXT_OFFLINE: //p')）"
else
    bad "离线段断言红（rc=$RC，与网络无关 ⇒ 产品问题）"
fi

# ============ [3/5] 桩档（判红） ============
echo "== [3/5] 桩档（判红；需 root 或 sudo -n + unshare -mn）=="
SUDO=""
if [ "$(id -u)" -ne 0 ] && command -v sudo >/dev/null 2>&1 && sudo -n true 2>/dev/null; then
    SUDO="sudo -n"
fi
IP_BIN=$(command -v ip || echo /usr/sbin/ip)
STUB_MODE="skip"
if [ "$(id -u)" -eq 0 ] || [ -n "$SUDO" ]; then
    if command -v unshare >/dev/null 2>&1 && $SUDO unshare -mn true 2>/dev/null; then
        if [ -x "$IP_BIN" ]; then
            STUB_MODE="run"
        else
            skips "[585-SKIP-LOUD] 桩档不可用（找不到 ip 命令）—— 本段零观测"
        fi
    else
        skips "[585-SKIP-LOUD] 桩档不可用（unshare -mn 不可用）—— 本段零观测"
    fi
else
    skips "[585-SKIP-LOUD] 桩档不可用（非 root 且无 sudo -n）—— 本段零观测"
fi

if [ "$STUB_MODE" = "run" ]; then
    cat > "$TMP/ns_run.sh" <<'EOS'
#!/bin/bash
# 在 unshare -mn 内跑：只碰本命名空间的 lo 与 /etc/resolv.conf（不碰宿主）
set -u
mode="$1"; out="$2"; gout="$3"; hits="$4"; truth="$5"
"$IP_BIN" link set lo up 2>/dev/null || { echo "NS_FAIL=lo" > "$out"; exit 90; }
mount --make-rprivate / 2>/dev/null || true
printf 'nameserver 127.0.0.1\n' > "$RESOLV_TMP" 2>/dev/null || { echo "NS_FAIL=resolv-write" > "$out"; exit 91; }
mount --bind "$RESOLV_TMP" /etc/resolv.conf 2>/dev/null || { echo "NS_FAIL=resolv-bind" > "$out"; exit 92; }
grep -q 'nameserver 127.0.0.1' /etc/resolv.conf 2>/dev/null || { echo "NS_FAIL=resolv-visible" > "$out"; exit 93; }
: > "$hits"
"$STUB_BIN" serve 53 "$hits" "$mode" 2> "$STUB_ERR" &
sp=$!
for _i in $(seq 1 50); do grep -q STUB_READY "$STUB_ERR" 2>/dev/null && break; sleep 0.1; done
"$CLIENT" > "$out" 2>&1; rc=$?
"$STUB_BIN" expected | sed 's/^R[0-9]*=//' > "$truth" 2>/dev/null
GODEBUG=netdns=go "$GOBIN" dump trunc.dnsstub.test > "$gout" 2> "$gout.err"
grc=$?
kill "$sp" 2>/dev/null; wait "$sp" 2>/dev/null
[ "$rc" = 0 ] || exit "$rc"
exit "$grc"
EOS
    ns_run() {   # $1=mode $2=out $3=gout $4=hits $5=truth
        $SUDO env STUB_BIN="$STUB_BIN" CLIENT="$PWD/build/dns_txt_stub" \
            GOBIN="$PWD/build/go_txt" IP_BIN="$IP_BIN" \
            RESOLV_TMP="$TMP/resolv.conf" STUB_ERR="$TMP/stub.err" \
            unshare -mn bash "$TMP/ns_run.sh" "$1" "$2" "$3" "$4" "$5"
    }

    # ---- 正常桩：判红 ----
    ns_run normal "$TMP/n.out" "$TMP/n.go" "$TMP/n.hits" "$TMP/n.truth"; NRC=$?
    case $NRC in
        90|91|92|93)
            bad "桩档命名空间构造失败（$(head -1 "$TMP/n.out" 2>/dev/null)）—— 桩档是必红判据，不因构造失败判绿" ;;
        *)
            echo "$(sed 's/^/     | /' "$TMP/n.out" 2>/dev/null)" | grep -E 'FAIL|DNS_TXT_STUB' | head -8
            sed -n '/----TXT-DUMP----/,$p' "$TMP/n.out" 2>/dev/null | tail -n +2 | grep -v '^DNS_TXT_STUB' > artifacts/px_stub.txt
            if [ "$NRC" != 0 ]; then
                bad "桩档：dns_txt 断言或 Go 对拍红（rc=$NRC）"
            else
                ok "桩档：px 断言全过（$(sed -n 's/^DNS_TXT_STUB: //p' "$TMP/n.out")）"
            fi
            # 逐字节：px 记录 vs 桩自己吐的 TCP 面真值（避免两处手抄 190 字节 × 3）
            if diff -q <(sort artifacts/px_stub.txt) <(sort "$TMP/n.truth") >/dev/null 2>&1; then
                ok "桩档：3 条长 TXT 逐字节等于桩真值（多段 character-string 已合并，$(wc -l < "$TMP/n.truth") 条）"
            else
                bad "桩档：px 记录与桩真值不一致（逐字节）"; diff <(sort "$TMP/n.truth") <(sort artifacts/px_stub.txt) | head -6 | sed 's/^/     | /'
            fi
            # 与 Go net.LookupTXT 集合对拍（同一命名空间内，参照物也指向桩）
            if [ -s "$TMP/n.go" ]; then
                RV=$("$GOBIN" cmp "$TMP/n.go" artifacts/px_stub.txt 2>/dev/null)
                [ "$RV" = "CMP-EQUAL" ] && ok "桩档：与 Go net.LookupTXT 集合相等（参照物同指桩 ⇒ 对拍也有意义）" \
                                        || bad "桩档：Go 对拍 $RV"
            else
                bad "桩档：Go 参照侧无输出（$(head -1 "$TMP/n.go.err" 2>/dev/null)）"
            fi ;;
    esac

    # ---- NC1：判据有牙（破损桩必须判红）----
    ns_run tcp-broken "$TMP/b.out" "$TMP/b.go" "$TMP/b.hits" "$TMP/b.truth" >/dev/null 2>&1
    BRC=$?
    if [ "$BRC" != 0 ]; then
        ok "NC1 判据有牙：桩 TCP 面故意破损 ⇒ 客户端判红（rc=$BRC，$(grep -m1 -oE 'dns: tcp [^)]*' "$TMP/b.out" 2>/dev/null || echo '见输出')）"
    else
        bad "NC1 判据无牙：桩 TCP 面破损仍然全绿 ⇒ 本段不是判据"
    fi
fi

# ============ [4/5] 外网档（观察项；探针前置） ============
echo "== [4/5] 外网档（观察项）：UDP 53 + TCP 53 双通道探针**前置** =="
NSIP=$(awk '/^[[:space:]]*nameserver/{print $2; exit}' /etc/resolv.conf 2>/dev/null || true)
NET_OK=0
if [ -z "${NSIP:-}" ]; then
    skips "[585-SKIP-LOUD] 外网档跳过：/etc/resolv.conf 无 nameserver ⇒ 本段零观测"
elif "$STUB_BIN" probe "$NSIP" 53 > "$TMP/probe.out" 2>&1; then
    NET_OK=1
    ok "双通道探针通（$NSIP：UDP 53 + TCP 53）⇒ 外网档可判红"
else
    skips "[585-SKIP-LOUD] 外网档跳过：$(tr '\n' ' ' < "$TMP/probe.out")⇒ 真域名断言不执行（不是先红后跳）"
fi
# NC2：探针本身有牙（黑洞地址必须判 down）
if "$STUB_BIN" probe 192.0.2.1 53 > "$TMP/probe_black.out" 2>&1; then
    bad "NC2 探针无牙：黑洞 192.0.2.1 也判通 ⇒ 跳过口径形同虚设"
else
    ok "NC2 探针有牙：黑洞 192.0.2.1 判 down（$(tr '\n' ' ' < "$TMP/probe_black.out")）"
fi

if [ "$NET_OK" = 1 ]; then
    OUT=$(./build/dns_txt_test 2>&1); RC=$?
    echo "$OUT" | tail -3 | sed 's/^/     | /'
    echo "$OUT" | sed -n '/----TXT-DUMP----/,$p' | tail -n +2 | grep -v '^DNS_TXT_TEST' > artifacts/px.txt
    if [ "$RC" = 0 ] && echo "$OUT" | grep -qE 'DNS_TXT_TEST: [0-9]+P/0F'; then
        ok "外网档：px 断言全过（$(echo "$OUT" | sed -n 's/^DNS_TXT_TEST: //p')）"
        if "$GOBIN" dump qq.com > artifacts/go.txt 2>/dev/null; then
            RV=$("$GOBIN" cmp artifacts/go.txt artifacts/px.txt 2>/dev/null)
            [ "$RV" = "CMP-EQUAL" ] && ok "外网档：与 Go net.LookupTXT 集合相等" || bad "外网档：Go 对拍 $RV"
        else
            skips "[585-SKIP-LOUD] 外网档 Go 参照侧失败（探针过但查询失败）⇒ 对拍跳过"
        fi
    else
        bad "外网档：px 断言红（rc=$RC；探针已过 ⇒ 这不是环境问题）"
    fi
fi

# ============ [5/5] 结论 ============
echo
echo "m103_s2a_dns_txt: ${P}P/${F}F/${S}S"
echo "M103_S2A_VERIFY: ${P}P/${F}F/${S}S"
if [ "$F" -eq 0 ]; then
    echo "m103_s2a_dns_txt verify done"
    exit 0
fi
echo "m103_s2a_dns_txt verify FAILED"
exit 1
