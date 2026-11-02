#!/bin/bash
# ══════════════════════════════════════════════════════════════════════
# check_shard_wt_recovery.sh —— 门 `shard_wt_recovery`（M371）
#
# 守的是分片调度器 `packaging/chain/shard_run.sh` 建树那一段的两条**装置面**契约：
#
#   缺陷 649（已修 · 本门**能**证）：
#     建树失败时把 git 的 stderr 吞掉 ⇒ 只印「❌ 建树失败」，真因丢失。
#     2026-11-01 03:33 / 03:35 两轮 chain-m368 就是这样各空转一次（exit 3），
#     而且**至今无法定性**——因为原文没了。这是「先修说得清，再谈修得对」。
#
#   缺陷 647（**只能止住，不能算已修** · 如实登记）：
#     僵尸 worktree 注册 ⇒ 建树失败。本门**不主张** prune 是解药：
#     实测（本门 T2 的前提自证）本机 git 在「目录已删 / 目录残缺」两种形态下，
#     `rm -rf` 之后 `worktree add` 仍能成功 ⇒ **历史那次失败的状态复现不出来**，
#     也就造不出「删了 prune 判据必红」的负控。按 CRIT_3（负控红不了 ⇒ 判据不成立），
#     本门**不为 prune 立判据**，只立它能守的那条：
#       契约 C2：**注册表是脏的（remove 会失败）时，prep 仍必须把树建出来**。
#     prune + 失败后重试是这套防御动作，但"它是历史失败的解药"这句话没有证据，不写。
#
# 判据：
#   T1 正常仓库                    ⇒ rc=0 · wt1/wt2 建出且 .rtcache 副本在位
#   T2 前提自证：脏注册（目录在、.git 缺失）⇒ `worktree remove --force` 必须失败
#   T3 契约 C2：同一脏状态下 prep ⇒ rc=0 · 注册表里没有指向 wt1 的过期条目
#   T4 契约 C1 正控（649）：commit 对象缺失 ⇒ 失败输出必须含 git 原文（`git#1:`）
#   N1 契约 C1 负控（649）：把副本改回吞 stderr ⇒ 同场景 rc 仍是 3，但**无** git 原文
#   T5 判据通道自证：`git worktree list --porcelain` 可读（通道断了要红，不能当绿）
#
# ⚠️ 为什么用一个 `--prep-only` 接缝，而不是「grep 一下脚本」：
#   注释里的代码也算代码（CRIT_8），而这段逻辑的价值全在**运行时行为**上。
#   `--prep-only` 让本门跑的是**同一份代码、同一个函数**，不是复制品。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail

HERE="$(cd "$(dirname "$0")" && pwd)"
SRC="$(cd "$HERE/.." && pwd)"
SHARD="$SRC/packaging/chain/shard_run.sh"
FAIL=0

ok(){  printf '  ✅ %s\n' "$1"; }
bad(){ printf '  ❌ %s\n' "$1"; FAIL=$((FAIL+1)); }

[ -f "$SHARD" ] || { echo "❌ 找不到 $SHARD"; exit 1; }
command -v git >/dev/null 2>&1 || { echo "⚠️ 无 git ⇒ SKIP"; exit 0; }
git --version >/dev/null 2>&1 || { echo "⚠️ git 不可用 ⇒ SKIP"; exit 0; }

WORK="$(mktemp -d /tmp/shardwt.XXXXXX)" || { echo "❌ mktemp 失败"; exit 1; }
trap 'rm -rf "$WORK" 2>/dev/null' EXIT

make_repo(){                 # $1=repo dir
  local r="$1"
  mkdir -p "$r/.rtcache" || return 1
  printf 'hello\n' > "$r/README.md"
  printf 'cache\n' > "$r/.rtcache/marker"
  git -C "$r" init -q >/dev/null 2>&1 || return 1
  git -C "$r" -c user.email=t@t -c user.name=t add -A >/dev/null 2>&1 || return 1
  git -C "$r" -c user.email=t@t -c user.name=t commit -qm init >/dev/null 2>&1 || return 1
  return 0
}

# 脏注册：注册在、目录在，但目录里的 .git 文件没了 ⇒ `worktree remove --force` 拒绝
make_dirty(){                # $1=repo $2=path
  local r="$1" p="$2"
  git -C "$r" worktree add --detach "$p" HEAD >/dev/null 2>&1 || return 1
  rm -f "$p/.git"
  printf 'junk\n' > "$p/junk"
  git -C "$r" worktree list --porcelain 2>/dev/null | grep -qF "$p"
}

run_prep(){                  # $1=repo $2=base $3=script $4=outfile -> rc
  PX_SRC="$1" PX_GATEVM="$2" bash "$3" 2 --prep-only > "$4" 2>&1
  echo $?
}

regs_for(){                  # $1=repo $2=path -> 注册表里指向该路径的条目数
  git -C "$1" worktree list --porcelain 2>/dev/null | grep -cF "worktree $2"
}

echo "═══ 门 shard_wt_recovery：分片建树契约（缺陷 649 可证 · 647 只止住）═══"

# ── T1 正常仓库 ────────────────────────────────────────────────────────
R1="$WORK/r1"; B1="$WORK/b1"; mkdir -p "$B1"
if ! make_repo "$R1"; then
  echo "⚠️ 造不出测试仓库（git 环境异常）⇒ SKIP"; exit 0
fi
rc=$(run_prep "$R1" "$B1" "$SHARD" "$WORK/t1.log")
if [ "$rc" = 0 ] && [ -f "$B1/wt1/.rtcache/marker" ] && [ -f "$B1/wt2/.rtcache/marker" ]; then
  ok "T1 正常仓库：rc=0 · wt1/wt2 建出且 .rtcache 副本在位"
else
  bad "T1 正常仓库：rc=$rc（期望 0）· wt1=$([ -d "$B1/wt1" ] && echo 有 || echo 无)"
  sed 's/^/      /' "$WORK/t1.log" | tail -8
fi

# ── T2/T3 脏注册：前提自证 + 契约 C2 ───────────────────────────────────
R2="$WORK/r2"; B2="$WORK/b2"; mkdir -p "$B2"
make_repo "$R2"
if ! make_dirty "$R2" "$B2/wt1"; then
  bad "T2 前提不成立：造不出脏注册（本格标红 ⇒ 本门无法执行，绝不静默跳过）"
else
  git -C "$R2" worktree remove --force "$B2/wt1" >/dev/null 2>&1
  rc_rm=$?
  if [ "$rc_rm" != 0 ]; then
    ok "T2 前提自证：脏注册下 worktree remove 真的失败（rc=$rc_rm）⇒ 状态确实脏"
    rc=$(run_prep "$R2" "$B2" "$SHARD" "$WORK/t3.log")
    stale=$(git -C "$R2" worktree list --porcelain 2>/dev/null | grep -c 'prunable' || true)
    n_wt1=$(regs_for "$R2" "$B2/wt1")
    if [ "$rc" = 0 ] && [ -f "$B2/wt1/README.md" ] && [ "$n_wt1" = 1 ] && [ "${stale:-0}" = 0 ]; then
      ok "T3 契约 C2：脏注册下仍建成（rc=0）· wt1 条目 $n_wt1 个 · prunable 残留 ${stale:-0}"
    else
      bad "T3 契约 C2 破了：rc=$rc（期望 0）· wt1 条目 $n_wt1（期望 1）· prunable ${stale:-0}（期望 0）"
      sed 's/^/      /' "$WORK/t3.log" | tail -10
    fi
  else
    bad "T2 前提不成立：脏注册下 worktree remove 竟然成功（rc=0）⇒ 造的状态不是脏的"
  fi
fi

# ── T4/N1 契约 C1（649）：失败必须说得出真因，且负控必须红 ─────────────────
R3="$WORK/r3"; B3="$WORK/b3"; mkdir -p "$B3"
make_repo "$R3"
_sha="$(git -C "$R3" rev-parse HEAD 2>/dev/null)"
rm -f "$R3/.git/objects/${_sha:0:2}/${_sha:2}"      # 对象缺失 ⇒ add 必失败（与 647 无关）
_still="$(git -C "$R3" rev-parse HEAD 2>/dev/null)"
if [ "$_still" != "$_sha" ]; then
  bad "T4 前提不成立：引用解析已失效（$_still），失败原因不是要测的那一个"
else
  rc=$(run_prep "$R3" "$B3" "$SHARD" "$WORK/t4.log")
  if [ "$rc" != 0 ] && grep -q 'git#1: ' "$WORK/t4.log"; then
    ok "T4 正控：建树失败时打出了 git 原文（rc=$rc）"
  else
    bad "T4 正控失效：rc=$rc（期望非 0）· git 原文命中=$(grep -c 'git#1: ' "$WORK/t4.log" 2>/dev/null || true)（期望 ≥1）"
    sed 's/^/      /' "$WORK/t4.log" | tail -8
  fi

  # 负控：把 stderr 捕获改回旧写法（吞掉）⇒ 同一场景必须"说不出真因"
  MUT="$WORK/shard_run_quieterr.sh"
  sed 's#worktree add --detach "\$WT" "\$HEAD_SHA" 2>&1#worktree add --detach "$WT" "$HEAD_SHA" >/dev/null 2>\&1#g' \
    "$SHARD" > "$MUT"
  left=$(grep -cE '^[[:space:]]*if ! err=.*worktree add --detach "\$WT" "\$HEAD_SHA" >/dev/null' "$MUT" 2>/dev/null || true)
  if [ "${left:-0}" -lt 1 ]; then
    bad "N1 前提不成立：副本里没出现「吞 stderr」的改法（sed 未命中，扰动是空操作）"
  else
    ok "N1 前提自证：副本已改回吞 stderr 的旧写法（$left 处）"
    B4="$WORK/b4"; mkdir -p "$B4"
    rc2=$(run_prep "$R3" "$B4" "$MUT" "$WORK/n1.log")
    if [ "$rc2" != 0 ] && ! grep -q 'git#1: ' "$WORK/n1.log"; then
      ok "N1 负控成立：吞 stderr 时 rc=$rc2 但无 git 原文 ⇒ 649 的修复不是空操作"
    else
      bad "N1 负控失效：rc=$rc2（期望非 0）· git 原文命中=$(grep -c 'git#1: ' "$WORK/n1.log" 2>/dev/null || true)（期望 0）"
      sed 's/^/      /' "$WORK/n1.log" | tail -8
    fi
  fi
fi

# ── T5 判据通道自证（通道没了必须红，不能当绿）──────────────────────────
ghost=$(git -C "$R1" worktree list --porcelain 2>/dev/null | grep -c '^worktree ' || true)
if [ "${ghost:-0}" -ge 1 ]; then
  ok "T5 判据通道：worktree list --porcelain 可读（条目 $ghost）"
else
  bad "T5 判据通道断了：worktree list --porcelain 读不到条目"
fi

echo
if [ "$FAIL" = 0 ]; then
  echo "✅ shard_wt_recovery PASS（T1/T2/T3/T4/N1/T5 全绿 · 647 仅止住、未定性，见头注）"
  exit 0
fi
echo "❌ shard_wt_recovery FAIL：$FAIL 格红"
exit 1
