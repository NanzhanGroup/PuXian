#!/usr/bin/env bash
# ============================================================
# packaging/check_registry_external.sh —— registry **外链可达性自证门**（M363 · 缺陷 631）
# ------------------------------------------------------------
# 为什么有它（M363 第 235 轮实测，证据 /data/autodrive/logs/M363-PROBE-files-sha256.md）：
#   `version.json` 的 `usage.verify` 与首页「校验」栏都印着
#       cd ~/.px && sha256sum -c files.sha256
#   而 `files.sha256` 在**两个站点**（源站 soft.wsai.chat / 边缘 soft.xiusoft.cn）
#   都是 **403 forbidden（9 字节）**。≡ 照官方指引走**必然失败**。
#   146 个文件的逐文件哈希**确实存在**，第三方取不到 ⇒ 「校验失败 = 包不可信」的误判。
#   ⇒ 本门只看**对外可见面**：三个 URL 的 HTTP 状态是否与钉一致。
#
# 判定模型（★ 钉住「此刻真实状态」，不是「我们希望的状态」）：
#   期望表 = packaging/check_registry_external.expect（键=值）
#   线上状态与钉**不一致**（无论变好变坏）⇒ 判红，逼人同步 version.json / 文档 / 删除钉。
#   理由：外链站不是本仓发布物（归属未定），用「必须全 200」当判据 ⇒ 全量门长期常红
#         ⇒ 门变噪音（本仓反复登记的「把红当绿」）；完全不判 ⇒ 「官方指引永远跑不通」无人看守。
#   `known_defect_<号>` 行 = 已登记的非 ok 状态：命中时 **绿但响亮**（⚠️ 大字打印 URL + 危害）。
#
# 判据：
#   E1 每个 base × 每个逻辑项：实际状态分类 == 钉（ok/blocked/missing/other:<码>）
#   E2 known_defect 行仍然成立（命中的项确实非 ok）；若已变 ok ⇒ 红（钉过期，逼删行 + 改指引）
#   E3 指引自洽：files_manifest 非 ok 时，version.json 正文里**不许**仍印 guidance_marker；
#      仍印 ⇒ ⚠️ 已知缺陷（响亮）；已不印 ⇒ 红（钉过期，缺陷该销号）
#   E4 规模锚点：探测项 ≥4 · base ≥2（防「提取器/配置坏了 ⇒ 空跑恒绿」）
#   E5 全体不可达（000）⇒ rc=3「无法判定」，**不算绿**
#   E6 传输层失败（000）单次不判死：重试 RETRIES=3 次（退避 1s）；拿到任何 HTTP 码即停。
#      连打 3 次仍 000 ⇒ 仍判 unreachable（判据强度不变，只滤抖动 —— 缺陷 653）
#
# 用法：
#   packaging/check_registry_external.sh              # 实地探测（全量门用）
#   packaging/check_registry_external.sh --self-test  # 离线自证（9 条，含 5 种负控；CI 用）
#   packaging/check_registry_external.sh --probe-once <url>   # 单探自证钩子（S8/S9 用）
#   EXPECT_FILE=<路径> packaging/check_registry_external.sh
# 退出码：0 绿（含已登记缺陷的响亮警告）/ 1 钉不符或有违例 / 2 用法错 / 3 无法判定（外链不可达）
# ============================================================
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
EXPECT="${EXPECT_FILE:-$HERE/check_registry_external.expect}"
TIMEOUT="${REGISTRY_PROBE_TIMEOUT:-5}"

# 内部自证钩子：只探一个 URL 并按 class/attempts 打印（S8/S9 用；不属于对外用法）
if [ "${1:-}" = --probe-once ]; then
    PROBE_ONCE_URL="${2:-}"
    [ -n "$PROBE_ONCE_URL" ] || { echo "用法错：--probe-once 需要 URL" >&2; exit 2; }
    set --
fi

for a in "$@"; do
    case "$a" in
        --self-test) SELFTEST=1 ;;
        -h|--help) sed -n '2,36p' "$0" | sed 's/^# \{0,1\}//'; exit 0 ;;
        *) echo "用法错：未知参数 $a" >&2; exit 2 ;;
    esac
done

# ── 离线自证：用同一份判定代码跑 7 个桩场景（4 个是负控）──────
selftest() {
    local W; W="$(mktemp -d /tmp/m363reg.XXXXXX)" || exit 3
    local p=0 f=0

    mk_pin() {  # $1=文件名 $2=files_manifest 期望 $3=指引正文
        cat > "$W/$1" <<EOF
bases=https://a.example/reg,https://b.example/reg
path.version_json=version.json
path.sha256sums=sha256sums.txt
path.files_manifest=files.sha256
path.home=
version_json=ok
sha256sums=ok
files_manifest=$2
home=ok
guidance_marker=files.sha256
known_defect_631=files_manifest
EOF
    }
    run() {  # $1=pin文件 $2=actuals $3=guidance
        EXPECT_FILE="$W/$1" SELFTEST_ACTUALS="$2" SELFTEST_GUIDANCE="$3" \
            bash "$0" > "$W/out.log" 2>&1
        echo $?
    }

    local G='{"usage":{"verify":"cd ~/.px && sha256sum -c files.sha256"}}'
    local A_OK='version_json=200,sha256sums=200,files_manifest=403,home=200'

    mk_pin good.expect blocked "$G"
    if [ "$(run good.expect "$A_OK" "$G")" = 0 ] && grep -q '已知缺陷 631' "$W/out.log"; then
        echo "  ✅ S1 钉一致 + 已登记缺陷 ⇒ 绿且响亮"; p=$((p+1))
    else echo "  ❌ S1 应绿且打印已知缺陷 631：$(grep -E '❌' "$W/out.log" | head -2)"; f=$((f+1)); fi

    if [ "$(run good.expect 'version_json=200,sha256sums=200,files_manifest=200,home=200' "$G")" = 1 ] \
       && grep -q '已好转' "$W/out.log"; then
        echo "  ✅ S2 负控：manifest 变 200（钉 blocked）⇒ 红并提示销号"; p=$((p+1))
    else echo "  ❌ S2 好转未判红"; f=$((f+1)); fi

    if [ "$(run good.expect 'version_json=200,sha256sums=200,files_manifest=404,home=200' "$G")" = 1 ] \
       && grep -q '钉不符' "$W/out.log"; then
        echo "  ✅ S3 负控：manifest 变 404 ⇒ 红（状态变化不许无人知）"; p=$((p+1))
    else echo "  ❌ S3 状态变化未判红"; f=$((f+1)); fi

    mk_pin stale.expect ok "$G"
    if [ "$(run stale.expect 'version_json=200,sha256sums=200,files_manifest=200,home=200' "$G")" = 1 ] \
       && grep -q '已过期' "$W/out.log"; then
        echo "  ✅ S4 负控：已好转但 known_defect 行仍留 ⇒ 红（钉过期）"; p=$((p+1))
    else echo "  ❌ S4 过期钉未判红"; f=$((f+1)); fi

    if [ "$(run good.expect 'version_json=000,sha256sums=000,files_manifest=000,home=000' "$G")" = 3 ]; then
        echo "  ✅ S5 负控：全不可达 ⇒ rc=3 无法判定（不当绿）"; p=$((p+1))
    else echo "  ❌ S5 全不可达未判 rc=3"; f=$((f+1)); fi

    if [ "$(run good.expect "$A_OK" '{"usage":{"verify":"sha256sum -c sha256sums.txt"}}')" = 1 ] \
       && grep -q '钉过期' "$W/out.log"; then
        echo "  ✅ S6 负控：指引已不印 marker 而件仍 403 ⇒ 红（缺陷该销号）"; p=$((p+1))
    else echo "  ❌ S6 指引自洽未判红"; f=$((f+1)); fi

    printf 'bases=https://a.example/reg\npath.version_json=version.json\nversion_json=ok\n' > "$W/tiny.expect"
    if EXPECT_FILE="$W/tiny.expect" SELFTEST_ACTUALS='version_json=200' bash "$0" > "$W/s7.log" 2>&1; then
        echo "  ❌ S7 规模锚点失效（探项 1 个也判绿 ⇒ 空跑恒绿）"; f=$((f+1))
    else
        grep -q '规模锚点' "$W/s7.log" && { echo "  ✅ S7 负控：探项过少 ⇒ 大规模锚点判红"; p=$((p+1)); } \
            || { echo "  ❌ S7 红但没指名规模锚点"; f=$((f+1)); }
    fi

    # ── S8/S9：传输层抖动重试（缺陷 653）──────────────────────
    cat > "$W/stub.sh" <<'EOS'
#!/usr/bin/env bash
code="$(head -1 "$STUB_SEQ" 2>/dev/null)"
if [ -n "$code" ]; then tail -n +2 "$STUB_SEQ" > "$STUB_SEQ.tmp" 2>/dev/null; mv "$STUB_SEQ.tmp" "$STUB_SEQ"; fi
printf '%s\n' "${code:-000}"
EOS
    chmod +x "$W/stub.sh"

    printf '000\n200\n' > "$W/seq1"
    local o8
    o8="$(STUB_SEQ="$W/seq1" REGISTRY_PROBE_STUB="$W/stub.sh" REGISTRY_PROBE_BACKOFF=0 SELFTEST_RETRY_NO_SLEEP=1 \
          bash "$0" --probe-once https://x.example/reg/sha256sums.txt 2>&1)"
    if grep -q 'class=ok attempts=2' <<< "$o8"; then
        echo "  ✅ S8 正控：首探 000 → 重试得 200 ⇒ ok（一次抖动不判红）"; p=$((p+1))
    else echo "  ❌ S8 重试未生效：$o8"; f=$((f+1)); fi

    printf '000\n000\n000\n' > "$W/seq2"
    local o9
    o9="$(STUB_SEQ="$W/seq2" REGISTRY_PROBE_STUB="$W/stub.sh" REGISTRY_PROBE_BACKOFF=0 SELFTEST_RETRY_NO_SLEEP=1 \
          bash "$0" --probe-once https://x.example/reg/sha256sums.txt 2>&1)"
    if grep -q 'class=unreachable attempts=3' <<< "$o9"; then
        echo "  ✅ S9 负控：连打 3 次 000 ⇒ 仍判 unreachable（重试不削弱判据）"; p=$((p+1))
    else echo "  ❌ S9 重试把真不可达洗绿了：$o9"; f=$((f+1)); fi

    echo "══ 自证：$p 通过 / $f 失败 ══"
    rm -rf "$W"
    [ "$f" = 0 ] || exit 1
    exit 0
}

[ -n "${SELFTEST:-}" ] && selftest

# ── 状态分类（唯一实现；现场探测与自证共用）─────────────────
# 001 之外的 3 位码原样归入 other:<码>；curl 的 000 = 传输层失败
classify() {
    case "$1" in
        200) echo ok ;;
        403) echo blocked ;;
        404) echo missing ;;
        000) echo unreachable ;;
        *)   echo "other:$1" ;;
    esac
}

# ── 缺陷 653（M370）：外链偶发抖动曾「一次判死」⇒ 全量门假红 ─────────────
# 证据：chain-m370 首轮 shard1 registry_external ❌（soft.xiusoft.cn/…/sha256sums.txt = unreachable，
#       同站其余 3 项 200）；随后同一 URL 连打 20 次全 200（均值 ≈60ms），files.sha256 5/5 200。
# ⇒ 传输层失败（000 = 连 HTTP 回答都没拿到）单次不构成判据：给 RETRIES 次机会，间隔 BACKOFF 秒退避；
#   **拿到任何 HTTP 码（含 403/500）即停** —— 那是真回答，不重试。
# ★ 判据强度不变：连打 RETRIES 次仍 000 ⇒ 仍判 unreachable（红；全灭时 rc=3），
#   只是不再把「一次网络抖动」当成「发布物坏了」。
RETRIES="${REGISTRY_PROBE_RETRIES:-3}"
BACKOFF="${REGISTRY_PROBE_BACKOFF:-1}"
PROBE_ATTEMPTS=0

fetch_code() {   # $1=url $2=body(可空) —— 单次尝试，echo HTTP 码
    local url="$1" body="${2:-}" code=""
    if [ -n "${REGISTRY_PROBE_STUB:-}" ]; then
        # 自证桩：外层可用 STUB_SEQ 编排「先 000 后 200」的序列
        code="$("$REGISTRY_PROBE_STUB" "$url" "$body" 2>/dev/null)"
        code="${code%%$'\n'*}"      # 只取第一行（不用 | head —— 口径 19：避免 SIGPIPE）
    elif [ -n "$body" ]; then
        code="$(curl -s -o "$body" -w '%{http_code}' --max-time "$TIMEOUT" "$url" 2>/dev/null)"
    else
        code="$(curl -s -o /dev/null -w '%{http_code}' --max-time "$TIMEOUT" "$url" 2>/dev/null)"
    fi
    [ -n "$code" ] || code=000
    printf '%s\n' "$code"
}

# 探测一个 URL：echo 状态分类；正文落到 $BODY（若给）；尝试次数留在 $PROBE_ATTEMPTS
probe() {
    local name="$1" url="$2" body="${3:-}" code try=1
    PROBE_ATTEMPTS=1
    if [ -n "${SELFTEST_ACTUALS:-}" ]; then
        code="$(echo "$SELFTEST_ACTUALS" | tr ',' '\n' | awk -F= -v k="$name" '$1==k{print $2; exit}')"
        [ -n "$code" ] || code=000
        [ -n "$body" ] && printf '%s\n' "${SELFTEST_GUIDANCE:-}" > "$body"
        classify "$code"
        return
    fi
    while : ; do
        code="$(fetch_code "$url" "$body")"
        [ "$code" != 000 ] && break
        [ "$try" -ge "$RETRIES" ] && break
        echo "     ⏳ 第 $try/$RETRIES 次探测传输层失败（000）· ${BACKOFF}s 后重试  $url"
        [ "${SELFTEST_RETRY_NO_SLEEP:-0}" = 1 ] || sleep "$BACKOFF"
        try=$((try+1))
        PROBE_ATTEMPTS=$try
    done
    if [ "$try" -gt 1 ] && [ "$code" != 000 ]; then
        echo "     ✅ 重试后可达：第 $try 次 = $code（前 $((try-1)) 次 000，抖动不取）  $url"
    fi
    classify "$code"
}

# ── 内部钩子执行点（须在 probe 定义之后）─────────────────────────
if [ -n "${PROBE_ONCE_URL:-}" ]; then
    # 必须在本 shell 里调 probe（不能放 $( ) 子壳，否则 $PROBE_ATTEMPTS 回传不出来）
    _pf="$(mktemp)"
    probe probe_once "$PROBE_ONCE_URL" > "$_pf"
    cat "$_pf"
    pc="$(tail -1 "$_pf")"
    rm -f "$_pf"
    echo "class=$pc attempts=$PROBE_ATTEMPTS"
    [ "$pc" = ok ] && exit 0 || exit 1
fi

# ── 读取钉表 ────────────────────────────────────────────────
declare -A E=()
[ -r "$EXPECT" ] || { echo "前提不成立：钉表不存在 $EXPECT" >&2; exit 3; }
while IFS='=' read -r k v; do
    case "$k" in ''|\#*) continue ;; esac
    E["$k"]="$v"
done < "$EXPECT"

BASES="${E[bases]:-}"; GUIDANCE_MARK="${E[guidance_marker]:-}"
[ -n "$BASES" ] || { echo "前提不成立：钉表缺 bases=" >&2; exit 3; }

# 逻辑项清单 = path.* 键（顺序稳定：按键名排序）
ITEMS=()
while read -r k; do [ -n "$k" ] && ITEMS+=("${k#path.}"); done \
    < <(printf '%s\n' "${!E[@]}" | grep '^path\.' | sort)
IFS=',' read -r -a BASE_ARR <<< "$BASES"

echo "=== registry 外链自证（M363 · 缺陷 631）==="
echo "  钉表 $EXPECT"
echo "  探项 ${#ITEMS[@]}（${ITEMS[*]}）· 站点 ${#BASE_ARR[@]}"

P=0; F=0; KNOWN=0; UNREACH=0; TOTAL=0
ok()   { echo "  ✅ $*"; P=$((P+1)); }
bad()  { echo "  ❌ $*"; F=$((F+1)); }
warn() { echo "  ⚠️  $*"; }

# E4 规模锚点
[ "${#ITEMS[@]}" -ge 4 ] && ok "规模锚点：探项 ${#ITEMS[@]} ≥4" || bad "规模锚点失效：探项只 ${#ITEMS[@]} 项 —— 钉表/解析坏了？（防空跑恒绿）"
[ "${#BASE_ARR[@]}" -ge 2 ] && ok "规模锚点：站点 ${#BASE_ARR[@]} ≥2（源站 + 边缘）" || bad "规模锚点失效：站点只 ${#BASE_ARR[@]} 个 —— 双活变单点无人知"

# 已登记缺陷（可能多行：known_defect_<号>=<项>）
declare -A KNOWN_DEF=()
while read -r k; do
    [ -n "$k" ] || continue
    num="${k#known_defect_}"
    IFS=',' read -r -a _ns <<< "${E[$k]}"
    for n in "${_ns[@]}"; do KNOWN_DEF["$n"]="$num"; done
done < <(printf '%s\n' "${!E[@]}" | grep '^known_defect_' | sort)

TMP="$(mktemp -d)"; trap 'rm -rf "$TMP"' EXIT
MANIFEST_NONOK=(); MANIFEST_NONOK_URLS=()

for base in "${BASE_ARR[@]}"; do
    base="${base% }"; base="${base# }"      # 容忍钉表里的空格
    echo "  ── 站点 $base"
    body=""
    for name in "${ITEMS[@]}"; do
        rel="${E[path.$name]:-}"
        want="${E[$name]:-}"
        url="$base/$rel"
        [ "$name" = home ] && url="$base/"
        TOTAL=$((TOTAL+1))

        if [ "$name" = version_json ]; then body="$TMP/version.json"; else body=""; fi
        got="$(probe "$name" "$url" "$body")"
        [ "$got" = unreachable ] && UNREACH=$((UNREACH+1))

        if [ -z "$want" ]; then bad "钉表缺期望值：$name"; continue; fi

        if [ "$got" = "$want" ]; then
            if [ "$got" = ok ]; then
                ok "$name = ok  ($url)"
            else
                # 与钉一致但不是 ok ⇒ 必须已被 known_defect 登记，否则钉在悄悄包庇
                if [ -n "${KNOWN_DEF[$name]:-}" ]; then
                    KNOWN=$((KNOWN+1))
                    warn "已知缺陷 ${KNOWN_DEF[$name]}：$name = $got（钉住）  $url"
                else
                    bad "$name = $got 与钉一致，但**没有任何已知缺陷登记** —— 要么登记（known_defect_<号>），要么修"
                fi
            fi
        else
            bad "钉不符：$name 期望 $want / 实际 $got  ($url)"
            if [ "$want" != ok ] && [ "$got" = ok ]; then
                echo "     ⇒ 线上**已好转**。请删掉钉里的 $name=$want 改为 ok，"
                echo "       并同步 version.json 的 usage.verify / 文档 / 首页校验栏（缺陷号该销号）。"
            fi
        fi

        # E2 known_defect 行过期判据（独立于 E1，防止「恰好都是 ok 就蒙过去」）
        if [ -n "${KNOWN_DEF[$name]:-}" ] && [ "$got" = ok ]; then
            bad "缺陷 ${KNOWN_DEF[$name]} 登记项 $name 已变 ok ⇒ 钉/缺陷已过期，删 known_defect_${KNOWN_DEF[$name]} 行 + 改指引"
        fi

        # E3 指引自洽：**延后**到每站探完再判（ITEMS 按名排序，manifest 先于 version_json 探 ⇒ 此刻正文还没有）
        if [ "$name" = files_manifest ] && [ "$got" != ok ]; then
            MANIFEST_NONOK+=("$base|$got")
            MANIFEST_NONOK_URLS+=("$url")
        fi
    done
done

# E3（延后判定）指引自洽：manifest 取不到时，官方指引不许仍教用户跑它
for entry in "${MANIFEST_NONOK[@]:-}"; do
    [ -n "$entry" ] || continue
    base="${entry%%|*}"; got="${entry##*|}"
    [ -n "$GUIDANCE_MARK" ] || continue
    probe version_json "$base/version.json" "$TMP/vj.json" > /dev/null
    if [ -s "$TMP/vj.json" ] && grep -q "$GUIDANCE_MARK" "$TMP/vj.json"; then
        warn "官方指引仍印 \`$GUIDANCE_MARK\`，而它此刻是 $got ⇒ 照指引走必然失败（缺陷 631 现症）  $base"
    else
        bad "指引已不再印 \`$GUIDANCE_MARK\`，而该件仍 $got ⇒ 钉过期（缺陷 631 应销号：删 known_defect 行并收敛钉）  $base"
    fi
    rm -f "$TMP/vj.json"
done

# E5 全不可达 ⇒ 无法判定
if [ "$TOTAL" -gt 0 ] && [ "$UNREACH" -eq "$TOTAL" ]; then
    echo "  ❌ 全部 $TOTAL 次探测传输层失败（000）—— 外链不可达，**无法判定**（不等于绿）"
    echo "     ⚠️ 先判是「外链真挂了」还是「装置把人关黑屋」：本门要出网，若在分片里跑，"
    echo "        必须在 \$PX_GATEVM/net_needed.txt（默认 /data/gatevm/net_needed.txt）登记 registry_external，"
    echo "        否则会被塞进 netns 隔离分片 ⇒ 8 次探测全 000（M363 第 235 轮实测踩过）。"
    echo "══ 结论：UNKNOWN（外链不可达）══"
    exit 3
fi

echo "  ── 汇总：通过 $P / 违例 $F / 已登记缺陷 $KNOWN / 不可达 $UNREACH"
if [ "$KNOWN" -gt 0 ]; then
    cat <<EOF

  ╔══════════════════════════════════════════════════════════════════════════╗
  ║ ⚠️  缺陷 631 仍在：registry 的 files.sha256 对外取不到（403），            ║
  ║     而 version.json.usage.verify 与首页校验栏仍在教用户跑它。             ║
  ║     受影响 URL：                                                        ║
$(printf '  ║   · %s\n' "${MANIFEST_NONOK_URLS[@]}" | cut -c1-74 | sed 's/$/ ║/')
  ║     判据：本门只在「状态变化」时红；本缺陷的修复动作 = 发布侧放行 .sha256 ║
  ║           或改指引（二者都要同步钉表）。归属未定（源站/边缘同病）。        ║
  ╚══════════════════════════════════════════════════════════════════════════╝
EOF
fi

if [ "$F" -eq 0 ]; then
    echo "══ 结论：GREEN（钉一致$( [ "$KNOWN" -gt 0 ] && echo '，含已登记缺陷 631 的响亮警告' )）══"
    exit 0
fi
echo "══ 结论：RED（$F 条违例）══"
exit 1
