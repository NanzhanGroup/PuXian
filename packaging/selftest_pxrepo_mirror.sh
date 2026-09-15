#!/usr/bin/env bash
# ============================================================
# packaging/selftest_pxrepo_mirror.sh —— pxrepo_mirror.sh 判据离线回归
# ------------------------------------------------------------
# 只测「版本交叉校验」判据（不联网、不下载、不写站点）：
#   从 pxrepo_mirror.sh 的 `# >>> xcheck-rpm-tree >>>` ~ `# <<< xcheck-rpm-tree <<<`
#   之间**逐字抽取**真实代码段执行 ⇒ 无副本漂移。
# 回归对象（晨曦 2026-09-16 干跑发现）：原判据用 gh-pages **tip 提交信息**判定，
#   tip 为「站点文件同步」提交时必然误判 ⇒ 改为以 rpm 树内 `.mNNN` 为真值。
# 用法：bash packaging/selftest_pxrepo_mirror.sh   （退出码 0 = 全过）
# ============================================================
set -uo pipefail

SCRIPT="$(cd "$(dirname "$0")" && pwd)/pxrepo_mirror.sh"
[ -f "$SCRIPT" ] || { echo "❌ 找不到 $SCRIPT"; exit 2; }

W="$(mktemp -d /tmp/pxselftest.XXXXXX)"
trap 'rm -rf "$W"' EXIT

# ---- 抽取真实判据段 + 桩函数 ----
BLOCK="$W/judge.sh"
cat > "$BLOCK" <<'HDR'
log() { printf '   [log] %s\n' "$*"; }
die() { printf '   [die] %s\n' "$*"; exit 1; }
HDR
sed -n '/^# >>> xcheck-rpm-tree >>>/,/^# <<< xcheck-rpm-tree <<<$/p' "$SCRIPT" >> "$BLOCK"
grep -q 'RPM_MS=' "$BLOCK" || { echo "❌ 抽取失败：$SCRIPT 里的标记行缺失或被改"; exit 2; }

PASS=0; FAIL=0
mk_repo() { # $1=名 $2..=rpm文件名列表
  local d="$W/$1" f; shift
  mkdir -p "$d/rpm/9/x86_64"
  for f in "$@"; do : > "$d/rpm/9/x86_64/$f"; done
  git -C "$d" init -q
  git -C "$d" -c user.email=t@t -c user.name=t add -A
  git -C "$d" -c user.email=t@t -c user.name=t commit -qm "c"
  printf '%s\t\tbranch gh-pages of github\n' "$(git -C "$d" rev-parse HEAD)" > "$d/.git/FETCH_HEAD"
}
tip_msg() { # $1=名 $2=提交信息（模拟「站点同步提交为 tip」）
  local d="$W/$1"
  git -C "$d" -c user.email=t@t -c user.name=t commit -q --allow-empty -m "$2"
  printf '%s\t\tbranch gh-pages of github\n' "$(git -C "$d" rev-parse HEAD)" > "$d/.git/FETCH_HEAD"
}
check() { # $1=用例名 $2=期望rc $3=期望输出关键词 $4=clone $5=TAG
  local out rc
  out="$(CLONE="$4" TAG="$5" bash "$BLOCK" 2>&1)"; rc=$?
  if [ "$rc" = "$2" ] && printf '%s' "$out" | grep -q "$3"; then
    echo "✅ $1（rc=$rc，含「$3」）"; PASS=$((PASS+1))
  else
    echo "❌ $1（rc=$rc 期望 $2；输出: $(printf '%s' "$out" | tr '\n' ' ')）"; FAIL=$((FAIL+1))
  fi
}

echo "== 判据回归（抽取自 $SCRIPT）=="
mk_repo a "puxian-0.2.0-1.m122.el9.x86_64.rpm"
tip_msg a "site: 站点根文件改由仓库 packaging/ 同步"
check "A 站点同步提交为 tip（原判据误判处）" 0 "交叉校验：rpm 树含 m122" "$W/a" "v0.2.0-m122"

mk_repo b "puxian-0.2.0-1.m122.el9.x86_64.rpm"
tip_msg b "rpm: 发布 PuXian v0.2.0-m122（el7/el9 签名仓库）"
check "B rpm 发布提交为 tip" 0 "交叉校验：rpm 树含 m122" "$W/b" "v0.2.0-m122"

mk_repo c "puxian-0.2.0-1.m121.el9.x86_64.rpm"
tip_msg c "rpm: 发布 PuXian v0.2.0-m121（el7/el9 签名仓库）"
check "C 真不同步（树 m121 / tag m122）必须拦住" 1 "不含 m122" "$W/c" "v0.2.0-m122"

mk_repo d "puxian-0.2.0-1.m122.el9.x86_64.rpm" "puxian-0.2.0-1.m121.el9.x86_64.rpm"
tip_msg d "rpm: 发布 PuXian v0.2.0-m122"
check "D 树内多版本共存 → 放行 + 告警" 0 "多个里程碑版本" "$W/d" "v0.2.0-m122"

check "E tag 无 -mNNN 后缀 → 跳过" 0 "跳过 rpm 树版本交叉校验" "$W/a" "v0.2.0"

echo "== 结果：通过 $PASS / 失败 $FAIL =="
[ "$FAIL" = 0 ] || exit 1
