#!/usr/bin/env bash
# ============================================================
# M245 门（第 122 轮）：**证书热加载（重注册）与并发 TLS 握手的竞态 ⇒ UAF**（缺陷 414）
# ------------------------------------------------------------
# 来历：晨曦报障（Mahesvara WS 统一入口 P0 崩溃，m240 编译件）。
#   一句话根因：`tls_server()` 注册/重注册会 `free` + 重新 `parse`
#   `g_sni_certs[slot]` 与 `g_srv_cert`/`g_srv_key`，而**并发握手正在用同一对象**
#   （TLS1.3 的 `ssl_tls13_write_certificate_verify_body` 用该私钥做 ECDSA 签名，
#    配置阶段还用它 clone 私钥）。EC 私钥走「共享只读」（M101 只对 RSA 做 per-连接 clone），
#   其前提正是「不会被 free」—— 而注册把前提打破了。
#   修前：注册只持 `g_srv_tls_mu`，握手每一步持 `g_srv_hs_mu` ⇒ **两把锁互不排斥**
#     ⇒ 「free → parse」窗口必然可与握手步重叠 ⇒ 悬垂窗口被读 ⇒ UAF。
#
# 判据设计（为什么用「客户端成功率」而不是「崩不崩」）：
#   竞态的直接后果是**握手读到被 free / 清零的 cert/key ⇒ 握手失败**（坏证书/空私钥），
#   而「崩溃」只是其中一种可能（取决于 mbedtls 内部是否把结构体清零）。
#   ⇒ 判据取**可观测的行为**：风暴期间客户端成功率必须 100% 且服务端存活。
#   ⚠️ 靠时序凑巧发作的竞态，「修好了」与「这次没复现」无法区分 ⇒ 用**测试钩子**
#     `PX_TLS_RELOAD_GAP_MS` 把「free → parse」窗口放大成**必然**（与 PX_GC_STRESS 同族）。
#
# 层级：
#   [1] 静态：注册持 hs_mu 且**先于** tls_mu · 4 个出口**形状配对** · 锁序口径在位 ·
#            EC 前提修正到位 · 测试钩子「默认无感」
#   [2] 动态：修后 + 放大窗口 + 8 并发客户端 ⇒ **成功率 100% 且服务端存活**
#   [3] 负控 A：**忠实摘锁**（只摘 g_srv_hs_mu，钩子保留）⇒ 同载荷 ⇒ **必须判红**
#   [4] 负控 B：判据自伤 ⇒ A 的红**必须消失**
#   [5] 覆盖边界登记
#
# 用法：bash examples/m245_tls_cert_reload/verify.sh [--neg-skip]
# 退出码：0=绿 1=红 2=门自身前置自查失败
# ============================================================
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
[ "${1:-}" = "--neg-skip" ] && NEG=0
W=/tmp/m245_gate
rm -rf "$W"; mkdir -p "$W"

RT=runtime/runtime.c
REPRO="$HERE/repro.px"
BIN="$HERE/build/repro"
PORT=18445
GAP=40          # 窗口放大（ms）：注册持有 hs_mu 期间的长度
SLEEP=10        # 两次注册之间的间隔（ms）
NREL=240        # 注册次数 ⇒ 约 240×50ms ≈ 12s
CPAR=8          # 并发客户端数
CURLTO=10       # 客户端单请求超时（s）—— 修后握手要排队，给足
CERT="$W/ec_cert.pem"
KEY="$W/ec_key.pem"

pass=0; fail=0
chk() { if ( eval "$2" ); then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }

# ── 源快照/还原（负控打桩必须回滚）──
snapshot() { cp -f "$RT" "$W/rt.bak"; }
restore_all() { [ -f "$W/rt.bak" ] && cp -f "$W/rt.bak" "$RT"; }
trap 'restore_all' EXIT
snapshot

# ── 构建（失败把构建日志尾打进 stdout —— M201 补的教训：门红了要读得到原因）──
build_repro() {
    local tag="$1"
    rm -f "$BIN"
    if ! ./tools/px build --no-quic "$REPRO" > "$W/build.$tag.log" 2>&1; then
        echo "  ❌ 构建失败（$tag）—— 日志尾："
        tail -12 "$W/build.$tag.log" | sed 's/^/     /'
        return 1
    fi
    [ -x "$BIN" ] || { echo "  ❌ 构建产物不存在：$BIN"; tail -12 "$W/build.$tag.log" | sed 's/^/     /'; return 1; }
    return 0
}

# ── 一次场景：起服务 → 并发客户端 → 等注册窗口结束 → 收尾判据 ──
#    结果写 $W/<tag>.res：`n200 nbad alive rc done ready`
run_case() {
    local tag="$1" i c srv pids p
    : > "$W/$tag.codes"
    PX_TLS_RELOAD_GAP_MS="$GAP" "$BIN" "$PORT" "$CERT" "$KEY" "$SLEEP" "$NREL" \
        > "$W/$tag.srv.log" 2>&1 &
    srv=$!
    local ready=0
    for i in $(seq 1 150); do
        c=$(curl -sk -o /dev/null -w '%{http_code}' -m 5 "https://127.0.0.1:$PORT/index.txt" 2>/dev/null)
        if [ "$c" = "200" ]; then ready=1; break; fi
        sleep 0.2
    done

    pids=""
    for i in $(seq 1 "$CPAR"); do
        (
            n=0
            while [ "$n" -lt 400 ]; do
                if [ $(( i % 2 )) -eq 1 ]; then
                    curl -sk -o /dev/null -m "$CURLTO" --http1.1 \
                        "https://127.0.0.1:$PORT/index.txt" >/dev/null 2>&1
                    r=$?
                else
                    curl -sk -o /dev/null -m "$CURLTO" --http1.1 \
                        --resolve "localhost:$PORT:127.0.0.1" \
                        "https://localhost:$PORT/index.txt" >/dev/null 2>&1
                    r=$?
                fi
                echo "$r" >> "$W/$tag.codes"
                n=$((n+1))
            done
        ) &
        pids="$pids $!"
    done

    local done=0
    for i in $(seq 1 300); do
        if grep -q 'M245-RELOAD-DONE' "$W/$tag.srv.log" 2>/dev/null; then done=1; break; fi
        kill -0 "$srv" 2>/dev/null || break
        sleep 0.2
    done

    for p in $pids; do kill -TERM "$p" 2>/dev/null; done
    wait $pids 2>/dev/null

    local alive=0 rc=0 ntot n200 nbad
    kill -0 "$srv" 2>/dev/null && alive=1
    kill -TERM "$srv" 2>/dev/null
    wait "$srv" 2>/dev/null; rc=$?

    ntot=$(wc -l < "$W/$tag.codes" 2>/dev/null); ntot=${ntot:-0}
    n200=$(grep -c '^0$' "$W/$tag.codes" 2>/dev/null); n200=${n200:-0}
    nbad=$(( ntot - n200 ))
    echo "$n200 $nbad $alive $rc $done $ready" > "$W/$tag.res"
    return 0
}

# ════════════════════════════════════════════════════════════
echo "══ [0] 前置：EC(P-256) 自签证书 + 构建 ══"
# 必须用 **EC**：M101 对 RSA 私钥做 per-连接 clone ⇒ 连接用的是独立副本，
#   注册释放的是全局对象 —— RSA 场景下 UAF 面被 clone 掩盖。EC 走「共享只读」才是病灶路径。
openssl ecparam -name prime256v1 -genkey -noout -out "$KEY" 2>/dev/null
openssl req -new -x509 -key "$KEY" -out "$CERT" -days 1 -subj "/CN=localhost" 2>/dev/null
v_ec=0
[ -s "$CERT" ] && [ -s "$KEY" ] && openssl pkey -in "$KEY" -text -noout 2>/dev/null | grep -q 'prime256v1' && v_ec=1
chk "EC(P-256) 自签证书生成（病灶路径：EC 私钥是共享只读）" "[ '$v_ec' = 1 ]"
v_b=0; build_repro fixed && v_b=1
chk "构建 repro（修后源码）" "[ '$v_b' = 1 ]"

# ════════════════════════════════════════════════════════════
echo "══ [1] 静态判据（源码结构）══"
#   ⚠️ 抽取必须匹配**定义**（行尾 `{`）而不是**前置声明**（行尾 `;`，见 L121）——
#     否则从声明处开始、到别处第一个 `^}` 就停 ⇒ 抽到**别的函数**，判据静默失效（本轮实测踩过）。
awk '/^static LXValue bi_tls_server\(.*\) \{$/{f=1} f{print} f&&/^\}/{exit}' "$RT" > "$W/fn.txt"
fn_lines=$(wc -l < "$W/fn.txt")
chk "函数体抽取成功（规模下限 ≥ 60 行）" "[ '$fn_lines' -ge 60 ]"

lh=$(grep -n 'pthread_mutex_lock(&g_srv_hs_mu);' "$W/fn.txt" | head -1 | cut -d: -f1)
lt=$(grep -n 'pthread_mutex_lock(&g_srv_tls_mu);' "$W/fn.txt" | head -1 | cut -d: -f1)
chk "注册持 g_srv_hs_mu" "[ -n '$lh' ]"
chk "锁序：hs_mu **先于** tls_mu（不引入反向序）" "[ -n '$lh' ] && [ -n '$lt' ] && [ '$lh' -lt '$lt' ]"

n_ul_tls=$(grep -c 'pthread_mutex_unlock(&g_srv_tls_mu);' "$W/fn.txt"); n_ul_tls=${n_ul_tls:-0}
n_ul_hs=$(grep -c 'pthread_mutex_unlock(&g_srv_hs_mu);' "$W/fn.txt"); n_ul_hs=${n_ul_hs:-0}
chk "出口配对：tls_mu unlock = 4（1 正常 + 3 报错）" "[ '$n_ul_tls' = 4 ]"
chk "出口配对：hs_mu  unlock = 4（与 tls_mu 一一对应）" "[ '$n_ul_hs' = 4 ]"
# 形状判据（不依赖变量名 —— M242 缺陷 407 的教训）：每个 tls unlock 的下一行就是 hs unlock
paired=$(awk '{ if (prev == 1 && $0 ~ /pthread_mutex_unlock\(&g_srv_hs_mu\);/) n++; prev = ($0 ~ /pthread_mutex_unlock\(&g_srv_tls_mu\);/) ? 1 : 0 } END { print n + 0 }' "$W/fn.txt")
chk "形状：每个 tls_mu unlock 之后**紧邻** hs_mu unlock（实测 $paired/4）" "[ '$paired' = 4 ]"
# px_error 在 spawn 隔离点 / json 捕获点会 longjmp ⇒ 绝不可持锁调用。
#   ⚠️ 只对**取锁之后**的 px_error 判（函数开头的 3 处实参校验本来就在锁外 ——
#     首版没排除它们 ⇒ 3 条假红；「判据比被测面更宽」与更窄一样是错）。
bad_perr=$(awk -v start="$lh" '{ L[NR] = $0 } /px_error\(/ { if (NR <= start) next; ok = 0; for (k = NR - 1; k >= NR - 3 && k > 0; k--) if (L[k] ~ /pthread_mutex_unlock\(&g_srv_hs_mu\)/) ok = 1; if (!ok) bad++ } END { print bad + 0 }' "$W/fn.txt")
chk "每个 px_error 前 3 行内都有 hs_mu unlock（longjmp 不泄漏锁）" "[ '$bad_perr' = 0 ]"

chk "钩子在位（PX_TLS_RELOAD_GAP_MS）" "grep -q 'PX_TLS_RELOAD_GAP_MS' $RT"
awk '/static void px_tls_reload_gap/{f=1} f{print} f&&/^\}/{exit}' "$RT" > "$W/hook.txt"
v_hook=0; grep -qE 'if \(!e \|\| !\*e\) return;' "$W/hook.txt" && v_hook=1
chk "钩子「默认无感」：未设环境变量即 return（不改变生产行为）" "[ '$v_hook' = 1 ]"
n_hookcall=$(grep -c 'px_tls_reload_gap();' "$RT"); n_hookcall=${n_hookcall:-0}
chk "钩子两处调用（默认证书支 + SNI 支）" "[ '$n_hookcall' = 2 ]"

n_seq=$(grep -c '锁序：hs_mu → tls_mu' "$RT"); n_seq=${n_seq:-0}
chk "锁序口径注释在位（≥2 处：定位 + 包装函数）" "[ '$n_seq' -ge 2 ]"
n_prem=$(grep -c 'M245 前提修正' "$RT"); n_prem=${n_prem:-0}
chk "M101「EC 共享只读」前提已修正（≥2 处）" "[ '$n_prem' -ge 2 ]"
v_stale=0; grep -q 'tls_server 注册仅持 tls_mu' "$RT" && v_stale=1
chk "总注释不再宣称「tls_server 注册仅持 tls_mu」（口径不落后于代码）" "[ '$v_stale' = 0 ]"

# ════════════════════════════════════════════════════════════
echo "══ [2] 动态正判据：修后 + 放大窗口 + $CPAR 并发客户端 ══"
run_case fixed
read -r f_n200 f_nbad f_alive f_rc f_done f_ready < "$W/fixed.res"
echo "     请求 200=$f_n200 失败=$f_nbad 存活=$f_alive rc=$f_rc 注册跑完=$f_done 就绪=$f_ready"
chk "服务端就绪（重试探测）" "[ '$f_ready' = 1 ]"
chk "注册风暴真的跑完（判据的窗口终点）" "[ '$f_done' = 1 ]"
chk "样本量下限（≥200 次真实握手尝试）" "[ $(( f_n200 + f_nbad )) -ge 200 ]"
chk "★ 修后：**失败数 = 0**（握手只排队，不读到悬垂对象）" "[ '$f_nbad' = 0 ]"
chk "★ 修后：服务端存活" "[ '$f_alive' = 1 ]"
chk "★ 修后：退出码正常（非信号杀死）" "[ '$f_rc' != 139 ] && [ '$f_rc' != 134 ] && [ '$f_rc' != 136 ]"

# ════════════════════════════════════════════════════════════
echo "══ [3] 负控 A：忠实摘锁（钩子保留）⇒ 必须判红 ══"
negA="skip"
if [ "$NEG" = 1 ]; then
    restore_all
    if python3 "$HERE/negctl.py" "$RT" > "$W/negctl.log" 2>&1; then
        if build_repro negA; then
            run_case negA
            read -r a_n200 a_nbad a_alive a_rc a_done a_ready < "$W/negA.res"
            echo "     请求 200=$a_n200 失败=$a_nbad 存活=$a_alive rc=$a_rc 注册跑完=$a_done 就绪=$a_ready"
            # 分类（⚠️ 首版把 `done=0` 一律当「载荷没跑起来」⇒ 把**最强的红**（服务端 SIGSEGV
            #   提前死掉、注册循环自然跑不完）误判成 `unreached` ⇒ 判据反而判红。实测：
            #   `200=19 失败=68 存活=0 rc=139 注册跑完=0` = 崩溃，正是要复现的现象。）
            if [ "$a_ready" != 1 ]; then
                negA="unreached"                                   # 连就绪都做不到 ⇒ 载荷本身没跑起来
            elif [ "$a_nbad" != 0 ] || [ "$a_alive" != 1 ]; then
                negA="red"                                         # 失败 >0 或进程死 ⇒ 正是要的红
            elif [ "$a_done" != 1 ]; then
                negA="partial"                                     # 零失败但窗口没走完 ⇒ 不算绿
            else
                negA="green"
            fi
        else
            negA="buildfail"
        fi
    else
        negA="anchor"
    fi
    restore_all
    chk "负控 A 打桩成功（5 处锚点唯一）" "[ '$negA' != 'anchor' ] && [ '$negA' != 'buildfail' ]"
    chk "★ 负控 A：摘锁后**必须判红**（失败>0 或进程死）—— 实测 $negA" "[ '$negA' = 'red' ]"
else
    echo "     （--neg-skip：跳过负控 A）"
fi

# ════════════════════════════════════════════════════════════
echo "══ [4] 负控 B：判据自伤 ⇒ A 的红必须消失（同一条数据重判）══"
if [ "$NEG" = 1 ] && [ "$negA" = "red" ]; then
    read -r b_n200 b_nbad b_alive b_rc b_done b_ready < "$W/negA.res"
    # 判据自伤：把两条判据换成**永不触发**的极性（`< 0` / `> 1`）⇒ 同一份 negA 数据
    #   必须**不再**判红。⚠️ 首版写成「两次用同一个条件算两个变量」= 同义反复，
    #   什么都没关掉（B 自己判红）—— 自伤必须真的**改变判据**，不是重述它。
    self_red=0
    if [ "$b_nbad" -lt 0 ] || [ "$b_alive" -gt 1 ]; then self_red=1; fi
    raw=0
    if [ "$b_nbad" != 0 ] || [ "$b_alive" != 1 ]; then raw=1; fi
    chk "★ 负控 B：判据关掉后同一份数据不再判红（证明红来自判据本身）" "[ '$self_red' = 0 ]"
    chk "★ 负控 B：关掉前**确实**判红（不是数据没产生）" "[ '$raw' = 1 ]"
else
    echo "     （A 未判红或 --neg-skip：跳过 —— **如实登记**，不假装通过）"
fi

# ════════════════════════════════════════════════════════════
echo "══ [5] 覆盖边界（如实登记）══"
cat <<'EOF'
      · 本门动态面只覆盖 **EC(P-256) 私钥**这条路：RSA 私钥有 per-连接 clone（M101），
        注册释放的是全局对象而连接用独立副本 ⇒ RSA 场景的 UAF 面被 clone 掩盖。
        ⇒「RSA 是否也有窗口」**未在本门判定**（未覆盖面）。
      · 证书来源是**文件路径** ⇒ 覆盖 `x509_crt_parse_file` / `pk_parse_keyfile` 那条；
        **PEM 内容直传**（`strstr(cert,"-----BEGIN")` 分支）走同一段窗口代码，未单独跑动态档。
      · 判据是「客户端成功率 + 服务端存活」，**不是** ASan/valgrind：若某次竞态恰好没造成
        握手失败，本门看不见 —— 所以[3]必须靠钩子把窗口放大成必然，负控才判得出来。
      · 不覆盖「真续期」（acme 换证）的业务语义：那里证书**内容会变**，修后行为是
        「在途握手排队后拿到新证书」（不崩；该次握手可能需重来）—— 属**有意**语义。
      · 与 M108-S2a 的交互：注册现在会持 hs_mu ⇒ **长时间注册会阻塞握手**（有意，见 CHANGELOG）。
        「注册不阻塞握手」需要**换代 + 引用计数**（旧代延迟释放），属未做（候选）。
      · 仓内写 `g_sni_certs` / `g_srv_cert` 的地方**只有** `bi_tls_server` 一处（已 grep 核验），
        故其它"改证书"路径（如 px_exec 热重载）不在面内。
EOF

echo "══ [6] 收尾：源逐字节还原 ══"
restore_all
v_back=0; cmp -s "$RT" "$W/rt.bak" && v_back=1
chk "runtime.c 与判据前**逐字节一致**（负控打桩已回滚）" "[ '$v_back' = 1 ]"
v_neg=0; grep -q 'px_tls_reload_gap();' "$RT" && v_neg=1
chk "还原后钩子仍在位（不是把修复一起还原掉了）" "[ '$v_neg' = 1 ]"
v_stale2=0; grep -q 'g_srv_hs_mu' "$RT" || v_stale2=1
chk "还原后 g_srv_hs_mu 仍在（不是把它删掉了）" "[ '$v_stale2' = 0 ]"

echo "────────────────────────────────────────────"
if [ "$fail" = 0 ]; then echo "M245-VERIFY-OK（$pass 通过）"; exit 0
else echo "M245-VERIFY-FAIL（$pass 通过 / $fail 失败）"; exit 1; fi
