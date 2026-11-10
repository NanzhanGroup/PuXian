#!/usr/bin/env bash
# ============================================================================
# publish_selftest.sh —— 发布轨唯一实现的自测（M388）
#
# 判据 C1..C10，负控 N1..N3，前提自证 P1..P2。
# 全部离线跑：假 git 仓库 + 假裸远端 + 假 dy-notify，不碰 GitHub、不发通知。
#
# 用法：publish_selftest.sh [-v]
# 输出：每行 "OK/FAIL <ID> <说明>"；结尾 "PUBLISH-SELFTEST-OK n/n" 或 "PUBLISH-SELFTEST-FAIL"
# ============================================================================
set -uo pipefail
HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." 2>/dev/null && pwd || echo "$HERE")"
# 被测件：仓库内 packaging/ 那份是唯一实现；独立运行时也允许用 /data/autodrive 的部署副本
AD="${M388_AD:-$ROOT/packaging}"
[ -f "$AD/publish_on_green.sh" ] || AD=/data/autodrive
PUB="${M388_PUB:-$AD/publish_on_green.sh}"
BODY="${M388_BODY:-$AD/publish_notify_body.sh}"
W="$(mktemp -d /tmp/p388st.XXXXXX)"
# 锁与输出全部改到临时目录：门在 CI/临时环境里跑，不能碰系统 /var/lock 与 /data
export DY_PUBLISH_LOCK_DIR="$W/locks"; mkdir -p "$DY_PUBLISH_LOCK_DIR"
export DY_PUBLISH_AD="$W/ad"; mkdir -p "$DY_PUBLISH_AD"
cp -f "$BODY" "$DY_PUBLISH_AD/" 2>/dev/null || true
export DY_PUBLISH_OUT="$W/out"; mkdir -p "$DY_PUBLISH_OUT"
trap 'fuser -k "$W/locks"/*.lock 2>/dev/null; pkill -f "publish_on_green.sh --ms 39[0-9]" 2>/dev/null; rm -rf "$W"' EXIT
VERB=0; [ "${1:-}" = "-v" ] && VERB=1

PASS=0; FAIL=0; FAILED=""
ok()  { PASS=$((PASS+1)); printf 'OK   %-4s %s\n' "$1" "$2"; }
no()  { FAIL=$((FAIL+1)); FAILED="$FAILED $1"; printf 'FAIL %-4s %s\n' "$1" "$2"; }
chk() { [ "$2" = "$3" ] && ok "$1" "$4" || no "$1" "$4（期望 $3 实得 $2）"; }
# 缺陷 696 同形陷阱：grep -c 无匹配时输出 0 且 rc=1 ⇒ 写成 $(grep -c x f || echo 0)
# 会得到两行 "0\n0"，字符串比较永远不等。这里统一走 cnt()。
cnt() { grep -c "$1" "${2:-/dev/null}" 2>/dev/null | head -1; }

# ── 假 dy-notify：记录被调用次数与等级 ─────────────────────────────────────
FAKEN="$W/dy-notify-fake.sh"
cat > "$FAKEN" <<'EOS'
#!/usr/bin/env bash
printf '%s\t%s\n' "${1:-?}" "${2:-?}" >> "$FAKE_NOTIFY_OUT"
exit 0
EOS
chmod +x "$FAKEN"
export DY_NOTIFY_BIN="$FAKEN"

# ── 假仓库 + 假裸远端 ─────────────────────────────────────────────────────
setup_repo() {
    local d="$1"; rm -rf "$d"; mkdir -p "$d"
    git init -q --bare "$d/remote.git"
    git init -q -b main "$d/repo"
    ( cd "$d/repo" && echo a > a.txt && git add -A \
      && git -c user.email=t@t -c user.name=t commit -qm init \
      && git remote add origin "$d/remote.git" && git push -q origin main ) >/dev/null 2>&1
}
R1="$W/r1"; setup_repo "$R1"; REPO1="$R1/repo"

# P1 前提自证：假仓库真的能用（否则后面所有"前置不满足"的用例都会因同一原因红/绿，判据失效）
H1="$(cd "$REPO1" && git rev-parse --short=7 HEAD 2>/dev/null)"
[ -n "$H1" ] && ok P1 "假仓库可用（HEAD=$H1，origin 可 ls-remote）" || no P1 "假仓库不可用，后续判据无意义"
L1="$(cd "$REPO1" && timeout 10 git ls-remote origin refs/heads/main 2>/dev/null | cut -f1)"
[ -n "$L1" ] && ok P1b "自带裸远端的 ls-remote 通" || no P1b "ls-remote 空 ⇒ 落后判据无法被测"

stamp_green() { printf 'CHAIN-VERDICT=GREEN rc=0 green=264 listed=264 expect=264 timeout=0 head=%s log=/dev/null\n' "${2:-$H1}" > "$1"; }

# ── C9 参数校验 ───────────────────────────────────────────────────────────
bash "$PUB" --ms abc >/dev/null 2>&1; chk C9 $? 2 "--ms 非数字必须 rc=2"
bash "$PUB" >/dev/null 2>&1;               chk C9b $? 2 "缺 --ms 必须 rc=2"

# ── C1 孤儿锁 ⇒ exit 3 且 p1 一次（缺陷 703） ──────────────────────────────
export FAKE_NOTIFY_OUT="$W/n1.txt"; : > "$FAKE_NOTIFY_OUT"
MS1=390
LOCK1="$DY_PUBLISH_LOCK_DIR/m${MS1}-publish.lock"; fuser -k "$LOCK1" 2>/dev/null; rm -f "$LOCK1"
setsid bash -c "exec 9>'$LOCK1'; flock 9; sleep 90" >/dev/null 2>&1 < /dev/null & ORPHAN_PID=$!
sleep 1.5
bash "$PUB" --ms $MS1 --repo "$REPO1" >/dev/null 2>&1; RC1=$?
N1C=$(cnt p1 "$FAKE_NOTIFY_OUT")
chk C1  "$RC1" 3 "孤儿锁 ⇒ rc=3（不是静默 exit 0）"
chk C1b "$N1C" 1 "孤儿锁 ⇒ p1 恰好 1 次"

# ── C2 同脚本持锁 ⇒ exit 0 且不 p1 ────────────────────────────────────────
export FAKE_NOTIFY_OUT="$W/n2.txt"; : > "$FAKE_NOTIFY_OUT"
MS2=391
STAMP2="$W/stamp2-does-not-exist"
setsid bash "$PUB" --ms $MS2 --repo "$REPO1" --stamp "$STAMP2" >/dev/null 2>&1 < /dev/null & HOLD_PID=$!
sleep 1.5
bash "$PUB" --ms $MS2 --repo "$REPO1" --stamp "$STAMP2" >/dev/null 2>&1; RC2=$?
N2C=$(cnt p1 "$FAKE_NOTIFY_OUT")
kill "$HOLD_PID" 2>/dev/null; pkill -f "publish_on_green.sh --ms $MS2" 2>/dev/null
chk C2  "$RC2" 0 "同脚本持锁 ⇒ rc=0（正常跳过）"
chk C2b "$N2C" 0 "同脚本持锁 ⇒ 不 p1（不误报）"

# ── C3 判红 ⇒ exit 1 ──────────────────────────────────────────────────────
S3="$W/stamp3"; printf 'CHAIN-VERDICT=RED reason=红 2 门 head=%s\n' "$H1" > "$S3"
export FAKE_NOTIFY_OUT="$W/n3.txt"; : > "$FAKE_NOTIFY_OUT"
bash "$PUB" --ms 392 --repo "$REPO1" --stamp "$S3" >/dev/null 2>&1
chk C3 $? 1 "判红 ⇒ rc=1 不发布"

# ── C4 树脏 ⇒ exit 2 ──────────────────────────────────────────────────────
S4="$W/stamp4"; stamp_green "$S4"
echo dirtyoops >> "$REPO1/a.txt"
export FAKE_NOTIFY_OUT="$W/n4.txt"; : > "$FAKE_NOTIFY_OUT"
bash "$PUB" --ms 393 --repo "$REPO1" --stamp "$S4" >/dev/null 2>&1
chk C4 $? 2 "树脏 ⇒ rc=2"
chk C4b "$(cnt p1 "$FAKE_NOTIFY_OUT")" 1 "树脏 ⇒ p1 一次（出声）"
( cd "$REPO1" && git checkout -q -- a.txt )

# ── C5 head 不一致 ⇒ exit 2 ───────────────────────────────────────────────
S5="$W/stamp5"; stamp_green "$S5" "deadbee"
export FAKE_NOTIFY_OUT="$W/n5.txt"; : > "$FAKE_NOTIFY_OUT"
bash "$PUB" --ms 394 --repo "$REPO1" --stamp "$S5" >/dev/null 2>&1
chk C5 $? 2 "stamp head≠HEAD ⇒ rc=2（不拿旧树的绿去发版）"

# ── C6 落后远端 ⇒ exit 2 ──────────────────────────────────────────────────
( cd "$R1" && git clone -q -b main remote.git other && cd other \
  && echo b > b.txt && git add -A && git -c user.email=t@t -c user.name=t commit -qm a \
  && git push -q origin main ) >/dev/null 2>&1
S6="$W/stamp6"; stamp_green "$S6"
export FAKE_NOTIFY_OUT="$W/n6.txt"; : > "$FAKE_NOTIFY_OUT"
bash "$PUB" --ms 395 --repo "$REPO1" --stamp "$S6" >/dev/null 2>&1
chk C6 $? 2 "落后 origin/main ⇒ rc=2（缺陷 690 方向：remote 是 HEAD 祖先才算不落后）"
( cd "$REPO1" && git fetch -q origin && git merge -q --ff-only origin/main )

# ── C7 通知正文：sha 必须展开（缺陷 698-②） ───────────────────────────────
OUT7="$(bash "$BODY" v0.2.400 5e1a1d3 "正文样例" "https://soft.xiusoft.net" 2>/dev/null)"
case "$OUT7" in *"tag v0.2.400 → 5e1a1d3"*) ok C7 "通知尾部 sha 已展开" ;; *) no C7 "通知尾部没有展开后的 sha（实得：$(printf '%s' "$OUT7" | tail -2 | tr '\n' ' '))" ;; esac
case "$OUT7" in *'$TAGSHA'*) no C7b "仍是字面量 \$TAGSHA" ;; *) ok C7b "不含字面量 \$TAGSHA" ;; esac
case "$OUT7" in *"镜像 https://soft.xiusoft.net"*) ok C7c "镜像地址透传" ;; *) no C7c "镜像地址丢了" ;; esac
bash "$BODY" v0.2.400 "" "正文" >/dev/null 2>&1; chk C7d $? 2 "空 sha 必须拒绝（空 sha = 读成「没落地」）"
printf '文件正文行\n' > "$W/b7.txt"
case "$(bash "$BODY" v0.2.400 abc1234 "$W/b7.txt" 2>/dev/null)" in *文件正文行*) ok C7e "正文文件被读取" ;; *) no C7e "正文文件未读取" ;; esac

# ── C8 双发禁用：hook 置空必须在 make_tag 之前 ────────────────────────────
LN_HOOK=$(grep -n 'export PX_RELEASE_NOTIFY_HOOK=/bin/true' "$PUB" | head -1 | cut -d: -f1)
LN_TAG=$(grep -n 'packaging/make_tag.sh' "$PUB" | head -1 | cut -d: -f1)
if [ -n "$LN_HOOK" ] && [ -n "$LN_TAG" ] && [ "$LN_HOOK" -lt "$LN_TAG" ]; then
    ok C8 "make_tag 的 hook 在调用前被置为 /bin/true（单出口，防双发）"
else
    no C8 "hook 置空缺失或顺序不对（hook=${LN_HOOK:-无} tag=${LN_TAG:-无}）"
fi

# ── C10 dry-run 不打 tag ─────────────────────────────────────────────────
S10="$W/stamp10"; stamp_green "$S10" "$(cd "$REPO1" && git rev-parse --short=7 HEAD)"
BEFORE=$( cd "$REPO1" && git tag | wc -l )
export FAKE_NOTIFY_OUT="$W/n10.txt"; : > "$FAKE_NOTIFY_OUT"
bash "$PUB" --ms 396 --repo "$REPO1" --stamp "$S10" --dry-run >/dev/null 2>&1; RC10=$?
AFTER=$( cd "$REPO1" && git tag | wc -l )
chk C10  "$RC10" 0 "dry-run ⇒ rc=0"
chk C10b "$BEFORE" "$AFTER" "dry-run 不打 tag"
chk C10c "$(cnt USER-NOTIFY "$FAKE_NOTIFY_OUT")" 0 "dry-run 不发用户通知"

# ── 负控：把修法回退，判据必须红 ─────────────────────────────────────────
NEG="$W/neg.sh"
cp "$PUB" "$NEG"
# N1 回退缺陷 703 的修法（孤儿锁也 exit 0）
sed -i 's/^        exit 3$/        exit 0/' "$NEG"
# 只查**非注释行** —— 注释里写着「并 exit 3」的说明文字不算实现（否则假红）
if grep -q 'exit 3' <(grep -v '^[[:space:]]*#' "$NEG"); then
    no N3-pre "负控未生效：回退后非注释行仍存在 exit 3（sed 未命中）"
else
    ok N3-pre "负控 N1 扰动已生效（实现的 exit 3 被改成 exit 0）"
fi
rm -f "$DY_PUBLISH_LOCK_DIR/m397-publish.lock"
setsid bash -c "exec 9>'$DY_PUBLISH_LOCK_DIR/m397-publish.lock'; flock 9; sleep 30" >/dev/null 2>&1 < /dev/null & O2=$!
sleep 0.5
bash "$NEG" --ms 397 --repo "$REPO1" >/dev/null 2>&1
RC_N1=$?
kill "$O2" 2>/dev/null
[ "$RC_N1" = "0" ] && ok N1 "负控成立：回退后孤儿锁静默 exit 0 ⇒ C1 判据有牙" \
                   || no N1 "负控不成立：回退后仍 rc=$RC_N1（C1 可能靠别的东西在绿）"

# N2 回退通知 sha 展开
cp "$BODY" "$W/body_neg.sh"
sed -i 's|printf .\\n%s\\n. "tag \$TAG → \$SHA"|printf "\\ntag \\$TAG → \\$TAGSHA\\n"|' "$W/body_neg.sh"
OUT_N2="$(bash "$W/body_neg.sh" v0.2.401 5e1a1d3 "x" 2>/dev/null)"
case "$OUT_N2" in *'$TAGSHA'*) ok N2 "负控成立：回退后正文出现字面 \$TAGSHA ⇒ C7 判据有牙" ;; *) no N2 "负控不成立：回退后仍未出现字面量（该 sed 未命中，需人工核）" ;; esac

# ── P2 前提自证：假 dy-notify 真被调用过 ──────────────────────────────────
TOT=$(cat "$W"/n*.txt 2>/dev/null | awk 'END{print NR+0}')
[ "$TOT" -gt 0 ] && ok P2 "假 dy-notify 累计被调用 $TOT 次（p1 计数不是查空文件得出的假绿）" \
                || no P2 "假 dy-notify 从未被调用 ⇒ 所有「没 p1」的结论都是假绿"

echo
if [ "$FAIL" -eq 0 ]; then
    echo "PUBLISH-SELFTEST-OK $PASS/$PASS"
else
    echo "PUBLISH-SELFTEST-FAIL 失败：$FAILED （通过 $PASS，失败 $FAIL）"
    exit 1
fi
