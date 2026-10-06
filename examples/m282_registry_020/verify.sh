#!/usr/bin/env bash
# ============================================================
# M282 门（第 160 轮）：上游 registry-px **0.2.0 全量再引入** + **凭据泄漏守卫**
# ------------------------------------------------------------
# 本轮两件事：
#  ① **引入**：上游 `banshanhanfu/registry-px` @ `01f6048c` 的 `0.2.0` 全量入库 ——
#     **120 个新版本目录**（上游 122 包**全部**有 0.2.0；其中 fsnotify/xlsx 的 0.2.0
#     早在 M214 已引入）· `upstream-tests/` 新增 **110 个用例**（`*2_test.px`）·
#     版本目录 137 → 257。
#  ② **安全（缺陷 488）**：`tools/import_registry_px.sh:52` 把 `git config remote.origin.url`
#     **原样**写进 `registry/THIRD_PARTY.md`，而该文件**提交并发布到公开仓库** ⇒
#     用带 PAT 的 clone URL 时会把**密钥永久写进公开仓库**（沙箱实测命中）。
#     修法：剥离 userinfo（仅当含 `:`）+ 清洗后仍含 token 形状 ⇒ **拒写 rc=4**；
#     并新增守卫 `selfhost/check_no_secrets.sh`（全仓扫 8 类凭据形状 + 私钥实体）。
#
# 判据（离线可判的部分，逐层）：
#  [1] **表 ⇔ 磁盘**：`registry/THIRD_PARTY.md` 每一行重算 sha256/文件数与磁盘对拍
#  [2] **清单双向一致**：`upstream-tests/MANIFEST.sha256` ⇄ `*_test.px` 集
#  [3] **登记双向一致 + 无重复**：`EXPECTED.tsv` ⇄ 用例集（重复登记会让 awk 取末条 ⇒ 静默覆盖）
#  [4] **引用面完整**：每个 `../registry/...` 在 registry 里真实存在（M188 事故族）
#  [5] **无凭据**：全仓扫描（`check_no_secrets.sh`）+ 反向判据（注入假 token ⇒ 必红）
#      ⚠️ **扫描面 = 已跟踪 ∪ 未跟踪未忽略** —— M282 实测事故：原实现只扫 `git ls-files`
#      ⇒ 门**提交前绿、提交后红**（新写的门文件当时还没 `git add`，扫描面里根本不存在）。
#      [5e] 是这条的判据（临时 git 仓库 · 未 add 的 token 文件必须命中）；
#      NC-D 是它的对照档（`NO_SECRETS_TRACKED_ONLY=1` ⇒ [5e] 必红）。
#  [6] **规模下限**：版本目录 / 用例数 / XFAIL 条数（防判据静默变窄）
#  [7] **上游逐字节对拍**：有检出则逐文件比对；无检出 ⇒ **响亮记 SKIP**（不静默放过）
#  [8] **覆盖补丁**：上游 `ftp_2_test` 卡在 T9（与 0.1.0 用例对同一夹具互斥）⇒ 0.2.0 新增的
#      FTP API **上游从没跑到过**；本门用**本仓自写探针** `ftp020_probe.px` + 自建 mock 补覆盖。
# 负控（各自独立判红或反向判据）：
#  A 往**副本**注入假 `github_pat_…` ⇒ [5] 必红
#  B 往 `EXPECTED.tsv` **副本**加一条重复登记 ⇒ [3] 必红
#  C **判据自伤的反面**：干净副本（无凭据）⇒ [5] 必**不**红 ——
#    证明 A 的红来自**内容**而不是「判据恒红」（否则 A 是空的）。
#  D **对照档**（`NO_SECRETS_TRACKED_ONLY=1`，只扫已跟踪）⇒ [5e] 的未跟踪面必失效 ——
#    证明 [5e] 的红来自**扫描面**而不是「判据恒红」。
#
# 用法：bash examples/m282_registry_020/verify.sh [--neg-skip]
# 退出码：0 = 绿，1 = 红，2 = 门自身前置自查失败。
# ============================================================
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || { echo "❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）" >&2; exit 2; }
set -u
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
cd "$ROOT" || exit 2
export LC_ALL=C LANG=C

NEG=1
case "${NEG_SKIP:-0}" in 1) NEG=0 ;; esac   # 与其它门同口径：CI 用 NEG_SKIP=1
for a in "$@"; do case "$a" in --neg-skip) NEG=0 ;; esac; done

REG="$ROOT/registry"
PROV="$REG/THIRD_PARTY.md"
TD="$ROOT/upstream-tests"
DOC="$ROOT/docs/UPSTREAM_020_DEFECTS.md"
UPSTREAM="${M282_UPSTREAM:-/tmp/m283/up}"
W=/tmp/m282_gate
rm -rf "$W"; mkdir -p "$W"
pass=0; fail=0; skip=0
chk() { if ( eval "$2" ) >/dev/null 2>&1; then echo "  PASS $1"; pass=$((pass+1)); else echo "  FAIL $1"; fail=$((fail+1)); fi; }
skipped() { echo "  SKIP $1 —— $2"; skip=$((skip+1)); }

echo "══ M282 门：上游 0.2.0 全量再引入 + 凭据泄漏守卫 ══"

# ---------- [0] 前置自查（门自身的前提，失败即 rc=2，不算红）----------
[ -f "$PROV" ] || { echo "❌ 前置自查失败：缺 registry/THIRD_PARTY.md" >&2; exit 2; }
[ -f "$DOC" ]  || { echo "❌ 前置自查失败：缺 docs/UPSTREAM_020_DEFECTS.md（EXPECTED 的 XFAIL 指到它）" >&2; exit 2; }
[ -f "$ROOT/selfhost/check_no_secrets.sh" ] || { echo "❌ 前置自查失败：缺 selfhost/check_no_secrets.sh" >&2; exit 2; }
grep -q 'M282' "$ROOT/tools/import_registry_px.sh" || { echo "❌ 前置自查失败：引入器缺 M282 标记" >&2; exit 2; }
grep -q 'M282' "$ROOT/selfhost/run_upstream_tests.sh" || { echo "❌ 前置自查失败：运行器缺 M282 标记" >&2; exit 2; }
echo "  ✓ 前置自查通过（THIRD_PARTY.md · 缺陷文档 · 凭据守卫 · 两个工具都带 M282 标记）"

# ---------- [1] 表 ⇔ 磁盘 ----------
echo "── [1] 表 ⇔ 磁盘（THIRD_PARTY.md 逐行重算）──"
python3 - "$PROV" "$REG" > "$W/tbl.txt" 2>&1 <<'PY'
import sys, os, hashlib, io
prov, reg = sys.argv[1], sys.argv[2]
bad = []; n = 0
for line in io.open(prov, encoding="utf-8"):
    if not line.startswith("| ") or line.startswith("| 包 ") or line.startswith("|---"):
        continue
    c = [x.strip() for x in line.strip().strip("|").split("|")]
    if len(c) < 4:
        continue
    name, ver, nfiles, sha16 = c[0], c[1], c[2], c[3].strip("`")
    d = os.path.join(reg, name, ver)
    n += 1
    if not os.path.isdir(d):
        bad.append("%s/%s 目录不存在" % (name, ver)); continue
    px = sorted(f for f in os.listdir(d) if f.endswith(".px"))
    if str(len(px)) != nfiles:
        bad.append("%s/%s 文件数 表=%s 盘=%d" % (name, ver, nfiles, len(px)))
    entry = os.path.join(d, name + ".px")
    if not os.path.isfile(entry):
        bad.append("%s/%s 缺入口 %s.px" % (name, ver, name)); continue
    h = hashlib.sha256(io.open(entry, "rb").read()).hexdigest()[:16]
    if h != sha16:
        bad.append("%s/%s sha 表=%s 盘=%s" % (name, ver, sha16, h))
print("ROWS %d" % n)
if bad:
    print("BAD %d" % len(bad))
    for b in bad[:20]:
        print("   " + b)
else:
    print("OK")
PY
sed 's/^/    /' "$W/tbl.txt"
ROWS="$(awk '/^ROWS /{print $2}' "$W/tbl.txt" 2>/dev/null || echo 0)"
ROWS="${ROWS:-0}"
if [ "$ROWS" -ge 240 ] && grep -q '^OK$' "$W/tbl.txt"; then
    echo "  PASS [1] 表⇔磁盘 无漂移（$ROWS 行）"; pass=$((pass+1))
else
    echo "  FAIL [1] 表⇔磁盘 有漂移（行数 $ROWS，详见上）"; fail=$((fail+1))
fi
O2_ROWS="$(grep -c '| 0\.2\.0 |' "$PROV" 2>/dev/null || true)"
O2_ROWS="${O2_ROWS:-0}"
chk "[1b] 0.2.0 行已入表（≥120，实测 $O2_ROWS）" "[ '$O2_ROWS' -ge 120 ]"

# ---------- [2][3] 运行器的前置判据（--list 只跑判据不跑用例）----------
echo "── [2][3] 清单 / 登记 ──"
if bash selfhost/run_upstream_tests.sh --list > "$W/list.txt" 2>&1; then
    echo "  PASS [2][3] MANIFEST 双向一致 + EXPECTED 双向一致且无重复（--list rc=0）"; pass=$((pass+1))
else
    echo "  FAIL [2][3] --list 判红："; tail -8 "$W/list.txt" | sed 's/^/      /'; fail=$((fail+1))
fi

# ---------- [4] 引用面完整 ----------
echo "── [4] 引用面完整 ──"
REFMISS=0
for f in "$TD"/*_test.px; do
    while IFS= read -r rel; do
        [ -n "$rel" ] || continue
        [ -f "$REG/${rel#../registry/}" ] || { echo "      ✗ $(basename "$f") → 引用缺失 $rel"; REFMISS=$((REFMISS+1)); }
    done < <(grep -o '\.\./registry/[^"]*' "$f" | sort -u)
done
chk "[4] 引用面完整（每个 ../registry/... 都存在，缺 $REFMISS 处）" "[ '$REFMISS' = 0 ]"

# ---------- [5] 凭据泄漏守卫 ----------
echo "── [5] 凭据泄漏守卫 ──"
if bash selfhost/check_no_secrets.sh --self-test > "$W/sec_st.txt" 2>&1; then
    echo "  PASS [5a] 守卫自证（含判据自伤/反向）"; pass=$((pass+1))
else
    echo "  FAIL [5a] 守卫自证失败："; tail -5 "$W/sec_st.txt" | sed 's/^/      /'; fail=$((fail+1))
fi
if bash selfhost/check_no_secrets.sh --root "$ROOT" > "$W/sec.txt" 2>&1; then
    echo "  PASS [5b] 全仓无明文凭据"; pass=$((pass+1))
else
    echo "  FAIL [5b] 全仓扫描判红："; tail -10 "$W/sec.txt" | sed 's/^/      /'; fail=$((fail+1))
fi
chk "[5c] THIRD_PARTY.md 无凭据形状（PAT / x-access-token / URL userinfo）" \
    "! grep -qE 'github_pat_|x-access-token[[:punct:]]|://[^/]*:[^/@]*@' '$PROV'"
chk "[5d] 引入器已剥离 URL userinfo（源码在位）" \
    "grep -q 'userinfo' '$ROOT/tools/import_registry_px.sh'"

# ---------- [5e] 扫描面：未跟踪文件必须被扫到（M282 实测事故的判据）----------
# 事故：门**提交前**绿、**提交后**红 —— 原实现只扫 `git ls-files`（已跟踪），而新写的门文件
# 当时还没 `git add` ⇒ 扫描面里根本不存在 ⇒「干跑通过 ≠ 提交后通过」（与缺陷 487 同形）。
# 判据：临时 git 仓库里放 **已 add 的 205 个 filler + 1 个未 add 的 token 文件** ⇒ 必须判红**且指名它**。
echo "── [5e] 扫描面（未跟踪文件）──"
W5="$W/untracked"; rm -rf "$W5"; mkdir -p "$W5"
( cd "$W5" && git init -q . ) >/dev/null 2>&1
i=0; while [ "$i" -lt 205 ]; do printf 'filler %s\n' "$i" > "$W5/f_$i.txt"; i=$((i+1)); done
( cd "$W5" && git add f_*.txt ) >/dev/null 2>&1                    # 只 add filler ⇒ 造「已跟踪 ≥200」
# ⚠️ 假 token **运行时拼装**（源码里不出现连续凭据形状，否则本门自己的 [5b] 判红；
#    这不是为了绕开守卫，而是「夹具不该长得像真凭据」的卫生规则）。
printf 'url=https://x-access-token:%s@github.com/a/b.git\n' \
    "github_pat_11ABCDEFG0""abcdefghijklmnopqrstuv" > "$W5/newfile.txt"   # 刻意**不** add ⇒ 未跟踪
NO_SECRETS_ALLOW="$W/allow_empty.tsv" bash selfhost/check_no_secrets.sh --root "$W5" > "$W5.log" 2>&1; R5=$?
R5TOK=0; grep -q 'newfile.txt' "$W5.log" && R5TOK=1
chk "[5e] 未跟踪的 token 文件也被扫到（rc=$R5 · 指名=$R5TOK）" "[ '$R5' = 1 ] && [ '$R5TOK' = 1 ]"

# ---------- [6] 规模下限 ----------
echo "── [6] 规模下限 ──"
VDIRS="$(find "$REG" -mindepth 2 -maxdepth 2 -type d | wc -l)"
TESTS="$(ls -1 "$TD"/*_test.px | wc -l)"
XF="$(awk -F'\t' '/^[[:space:]]*#/ || NF==0 {next} ($2=="XFAIL" || $3=="XFAIL") {c++} END{print c+0}' "$TD/EXPECTED.tsv")"
echo "    版本目录 $VDIRS · 用例 $TESTS · XFAIL 登记 $XF"
chk "[6a] 版本目录 ≥ 250（实测 $VDIRS）" "[ '$VDIRS' -ge 250 ]"
chk "[6b] 用例数 ≥ 235（实测 $TESTS）" "[ '$TESTS' -ge 235 ]"
chk "[6c] XFAIL 登记 ≥ 12（上游缺陷须显式登记，不许静默绿；实测 $XF）" "[ '$XF' -ge 12 ]"
DOCMISS=0
for i in 1 2 3 4 5 6 7 8 9 10 11; do grep -q "^### T$i " "$DOC" || DOCMISS=$((DOCMISS+1)); done
grep -q '^### L1 ' "$DOC" || DOCMISS=$((DOCMISS+1))
chk "[6d] 缺陷文档覆盖 T1–T11 + L1（缺 $DOCMISS 节）" "[ '$DOCMISS' = 0 ]"

# ---------- [7] 上游逐字节对拍（需检出；无则响亮 SKIP）----------
echo "── [7] 上游逐字节对拍 ──"
if [ -d "$UPSTREAM/registry" ]; then
    SAME=0; DIFF=0; EXT=0
    for d in "$REG"/*/0.2.0; do
        n="$(basename "$(dirname "$d")")"
        u="$UPSTREAM/registry/$n/0.2.0"
        if [ -d "$u" ]; then
            if diff -rq "$d" "$u" >/dev/null 2>&1; then SAME=$((SAME+1)); else DIFF=$((DIFF+1)); echo "      ✗ $n 与上游不一致"; fi
        fi
    done
    for u in "$UPSTREAM"/registry/*/0.2.0; do
        n="$(basename "$(dirname "$u")")"
        [ -d "$REG/$n/0.2.0" ] || { EXT=$((EXT+1)); echo "      ✗ 上游有而我方缺：$n"; }
    done
    echo "    与上游一致 $SAME · 不一致 $DIFF · 上游有而我方缺 $EXT"
    chk "[7] 0.2.0 与上游逐字节一致（≥120 包 · 无差异 · 无遗漏）" \
        "[ '$SAME' -ge 120 ] && [ '$DIFF' = 0 ] && [ '$EXT' = 0 ]"
else
    skipped "[7] 上游逐字节对拍" "无上游检出（$UPSTREAM 不存在）—— 需网络；**不是**静默放过"
fi

# ---------- [8] ftp 0.2.0 补充覆盖（上游用例走不到，见 §T9）----------
echo "── [8] ftp 0.2.0 补充覆盖（本仓自写探针）──"
mkdir -p "$W/tests" "$W/fixtures"
ln -sfn "$REG" "$W/registry"
cp -f "$HERE/ftp020_probe.px" "$W/tests/"
cp -f "$TD"/fixtures/ftp_mock.px "$W/fixtures/"
PXB="$ROOT/tools/px"
PID_FTP=""
if ! ( cd "$W/fixtures" && "$PXB" build ftp_mock.px ) > "$W/mock.build.log" 2>&1; then
    echo "  FAIL [8] ftp_mock 编译失败："; tail -3 "$W/mock.build.log" | sed 's/^/      /'; fail=$((fail+1))
elif ! ( cd "$W/tests" && "$PXB" build ftp020_probe.px ) > "$W/probe.build.log" 2>&1; then
    echo "  FAIL [8] 探针编译失败："; tail -3 "$W/probe.build.log" | sed 's/^/      /'; fail=$((fail+1))
else
    ( cd "$W/fixtures" && exec ./build/ftp_mock ) > "$W/mock.log" 2>&1 &
    PID_FTP=$!
    # ⚠️ ftp mock 打印的是 `ftpmock: control … · data …`（不是 "listening"）⇒ 就绪判据要认它，
    #   否则白等 5s（实测无效等待，不是错，但慢）。
    for _ in $(seq 1 50); do grep -qE 'listening|ftpmock' "$W/mock.log" 2>/dev/null && break; sleep 0.1; done
    if ( cd "$W" && timeout 120 ./tests/build/ftp020_probe ) > "$W/probe.log" 2>&1 && grep -q 'M282-FTP020-PROBE-OK' "$W/probe.log"; then
        echo "  PASS [8] ftp 0.2.0 的 SYST/SIZE/MDTM/MKD/RMD/DELE/RNFR+RNTO/APPE/REST 全跑通"; pass=$((pass+1))
    else
        echo "  FAIL [8] 探针未通过："; tail -4 "$W/probe.log" | sed 's/^/      /'; fail=$((fail+1))
    fi
    [ -n "$PID_FTP" ] && kill "$PID_FTP" 2>/dev/null
    sleep 0.2; [ -n "$PID_FTP" ] && kill -9 "$PID_FTP" 2>/dev/null
fi

# ---------- [9] 负控 ----------
echo "── [9] 负控 ──"
# ⚠️⚠️ 负控的目录**必须 ≥200 个受管文件** —— 守卫自带「受管文件 ≥200」的**判据自证**
#   （防「空集 ⊇ 任意集」假绿），小目录会直接 rc=3 ⇒ 那样 NC-A 的"红"**不是**因为扫到了
#   token，而是因为自证失败 ⇒ 负控**空转**。M282 实测：首版用两三个文件的小目录，
#   NC-A「判红 ✓」其实什么都没证明；**是 NC-C（干净副本必须不红）把它抓出来的**
#   （干净副本也红 ⇒ 说明红与内容无关）。⇒ 造负控目录时**先喂够 filler 文件**。
# ⚠️ 还要给负控一份**空允许表**：守卫除「扫 token/私钥」外还有一条**允许表过期**判据
#   （允许表里某条不再命中 ⇒ rc=1），而合成目录里当然没有那些**真实仓库**里的豁免文件
#   ⇒ 干净副本也会因「过期 1 条」判红。⇒ 负控用 `NO_SECRETS_ALLOW=<空表>`，
#   只把**与内容无关**的那条判据摘掉（token 扫描本身一字不动）。
: > "$W/allow_empty.tsv"
mkneg() {  # $1=目录  $2=1 注入假 token / 0 干净
    rm -rf "$1"; mkdir -p "$1"
    local i=0
    while [ "$i" -lt 205 ]; do printf 'filler line %s\n' "$i" > "$1/filler_$i.txt"; i=$((i+1)); done
    if [ "$2" = 1 ]; then
        # ⚠️ 假 token **运行时拼装**（源码里不出现连续凭据形状 ⇒ 不然本门自己的 [5b] 会判红）
        printf '# fake\n> `https://x-access-token:%s@github.com/x/y.git`\n' \
            "github_pat_11AAAAAAA_""BBBBBBBBBBBBBBBBBBBBBBBBBB" > "$1/THIRD_PARTY.md"
    else
        printf '# clean\n> `https://github.com/banshanhanfu/registry-px.git`\n' > "$1/THIRD_PARTY.md"
    fi
}
# NC-A：假 token ⇒ 守卫必须红（**且不是靠自证 rc=3**）
mkneg "$W/negA" 1
# NC-C：干净副本 ⇒ 守卫必须**不**红（证明 NC-A 的红来自内容，不是判据恒红/自证失败）
mkneg "$W/negC" 0
if [ "$NEG" = 1 ]; then
    NO_SECRETS_ALLOW="$W/allow_empty.tsv" bash selfhost/check_no_secrets.sh --root "$W/negA" > "$W/negA.log" 2>&1; NA_RC=$?
    if [ "$NA_RC" = 1 ]; then
        echo "  PASS NC-A 注入假 token ⇒ rc=1 判红 ✓（=「有明文凭据」，**不是** rc=3 自证失败）"; pass=$((pass+1))
    elif [ "$NA_RC" = 3 ]; then
        echo "  FAIL NC-A 得到 rc=3（判据自身失效：受管文件不足）⇒ 负控**空转**，什么都没证明"; fail=$((fail+1))
    else
        echo "  FAIL NC-A 注入假 token 却未判红（rc=$NA_RC）"; fail=$((fail+1))
    fi
    if NO_SECRETS_ALLOW="$W/allow_empty.tsv" bash selfhost/check_no_secrets.sh --root "$W/negC" > "$W/negC.log" 2>&1; then
        echo "  PASS NC-C 干净副本不判红（⇒ NC-A 的红来自内容，判据不是恒红）✓"; pass=$((pass+1))
    else
        echo "  FAIL NC-C 干净副本被判红 ⇒ NC-A 是空的（判据恒红）"; fail=$((fail+1))
    fi
    # NC-B：EXPECTED 副本加重复登记 ⇒ --list 必红
    cp -f "$TD/EXPECTED.tsv" "$W/exp_dup.tsv"
    printf 'actor2_test\tPASS\tPASS\t重复登记（负控注入）\n' >> "$W/exp_dup.tsv"
    if bash selfhost/run_upstream_tests.sh --list --expected "$W/exp_dup.tsv" > "$W/negB.log" 2>&1; then
        echo "  FAIL NC-B 重复登记未判红"; fail=$((fail+1))
    else
        echo "  PASS NC-B EXPECTED 重复登记 ⇒ 判红 ✓"; pass=$((pass+1))
    fi
    # NC-D：对照档「只扫已跟踪」⇒ [5e] 的未跟踪面必须失效（证明 [5e] 的红来自**扫描面**，
    #       不是「判据恒红」—— 同 NC-C 的立论）
    NO_SECRETS_ALLOW="$W/allow_empty.tsv" NO_SECRETS_TRACKED_ONLY=1 \
        bash selfhost/check_no_secrets.sh --root "$W5" > "$W/NCD.log" 2>&1; R5D=$?
    if [ "$R5D" = 0 ]; then
        echo "  PASS NC-D 对照档（只扫已跟踪）漏掉未跟踪文件 ⇒ [5e] 有牙 ✓"; pass=$((pass+1))
    else
        echo "  FAIL NC-D 对照档下仍判红（rc=$R5D）⇒ [5e] 的红与扫描面无关"; fail=$((fail+1))
    fi
else
    echo "  ⊘ NC-A / NC-B / NC-C / NC-D（--neg-skip）"
fi

# ---------- 覆盖边界（如实登记，不假装覆盖）----------
cat <<'EOF'
── 覆盖边界（如实）──
  · [7] 上游逐字节对拍**需要网络检出**；CI 无网 ⇒ 记 SKIP（本机复核请先 clone 到 $M282_UPSTREAM）。
  · [1] 只对拍**入口文件**的 sha256（与 THIRD_PARTY.md 的口径一致）；多文件包的**辅助文件**
    由「文件数」判据覆盖，逐文件 sha 见 M187 门。
  · 本门**不判**上游用例的通过率（那是 selfhost/run_upstream_tests.sh 的职责，登记在 EXPECTED.tsv，
    逐条定性见 docs/UPSTREAM_020_DEFECTS.md）。
  · 凭据守卫扫的是**文本文件**（跳过二进制）；私钥实体按「PEM 头 + ≥200 字符 body」判。
  · [5e] 用**临时 git 仓库**验证扫描面 —— 真仓在 CI 检出后**未跟踪面为空**，只能这样验。
  · token 面**刻意不设允许表**（按文件豁免会让真凭据被静默放过）⇒ 假凭据一律运行时拼装。
EOF

echo "── 结果 ──"
echo "  通过 $pass · 失败 $fail · 跳过 $skip"
if [ "$fail" != 0 ]; then echo "══ 汇总：M282 门 判红 ══"; exit 1; fi
echo "══ 汇总：M282 门 通过（M282-VERIFY-OK）══"
exit 0
