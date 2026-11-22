# 发布自锁的口径：被测面 ⇄ 发布面（M433 · 缺陷 896）

## 1 一句话

链绿之后发布被拒，理由写着「工作树脏」—— 而门从来不在主工作树里跑。
把「工作副本的卫生」当成「证据的完整性」，是这次事故的全部。

## 2 病史（2026-11-22 16:40 实测 · M432）

```
16:40:03  链判绿 CHAIN-VERDICT=GREEN rc=0 green=306 listed=306 head=3b15806
16:40:03  交 publish_on_green → ABORT dirty-tree（rc=2）
          脏的 5 个文件 = 我同时在建的补丁（ci.yml / ci_diagnose.py / fixtures …）
16:40:22  pipeline 报 publish-no-tag p1，而正文写「扫描面 日志47行 · 红项 0 条」
          —— 真因（ABORT dirty-tree）就在日志里，判据只认 ❌/FAIL 行
16:42:50  人工 reset → 16:43:14 提交补2 → 16:43:18 回灌基线 → 手工 tag
          ⇒ 306 门全绿的结论本身没错，错的是「绿之后自动发不出去」
```

这不是偶发：链尾会回灌基线、我又常在链跑的时候写补丁 ⇒ **树脏是常态**，
自动发布被拒也就成了常态（M427/M428/M429 都靠人工补过一次 tag）。

## 3 为什么老口径是错杀（可核事实，不是信念）

```
① 分片：packaging/chain/shard_run.sh
     HEAD_SHA="$(git -C "$SRC" rev-parse HEAD)"
     git worktree add --detach "$WT" "$HEAD_SHA"        ← 检出的是**提交**
② 门清单：git show "HEAD:selfhost/gates.registry.sh"   ← 也取自提交
③ quiet 档：cd "$BASE/wt$k"（同一个 worktree）→ 慢门/抗并发门也跑在提交上
⇒ 被测面 = 那一个 commit。主树的未提交改动**进不了测试面**。
   反证：shard_run.sh 第 1 节注释记着 2026-10-12 的教训 ——
   主树多一个未提交的门，分片却检出 HEAD ⇒ 分片会去跑一个不存在的门。
```

门 `m433_publish_scope` 的 A1 把这条做成**动态判据**：
worktree@HEAD 读到的文件内容 = 提交版，主树读到 WIP（并且先自证主树真的脏）。

## 4 新口径（唯一实现：`packaging/chain/publish_scope_check.sh`）

```
SCOPE-OK      head 一致；WIP 计数 + 明细落盘（$AD/m<ms>-wip-at-publish.txt），不阻断
SCOPE-REJECT  reason=stamp-head-missing    stamp 里没 head=<sha> ⇒ 不知道测的是哪个提交
SCOPE-REJECT  reason=head-mismatch         被测提交 ≠ 当前 HEAD（门跑的是别的树）
SCOPE-REJECT  reason=wip-version-source    WIP 碰 tools/px
```

**为什么唯独 `tools/px` 还得拦**：`packaging/make_tag.sh` 的版本链闸读的是
`$REPO/tools/px` 的**工作树**（3.5 节 `_VERFILE`）。主树 WIP 若把版本串改成别的值，
版本闸看到的就是 WIP 的值 ⇒ tag 与源码版本号可能错配。这一类必须拒，而且要出声（p1）。

`publish_on_green.sh` 只调这一个实现；老的「head 一致」+「树必须干净」两段各自
为政的实现退场（stamp 里没 head 时它默认放行，那是个静默缺口）。

## 5 顺手修掉的两件

```
lib_pipeline_fail.sh（M433 起入仓：packaging/chain/lib_pipeline_fail.sh）
   红项 0 条时必须打印**真因候选**（ABORT / SCOPE-REJECT / VERDICT-NOT-GREEN / STOP / TIMEOUT）。
   负控：日志里没有任何线索 ⇒ 如实说「没有」，不许编造（自测 F14）。

m388_publish_chain 子自测（30 条）
   C4 从「树脏 ⇒ rc=2」改成「非版本源 WIP ⇒ rc=0 且留证」（缺陷 896 的回归判据）
   新增 C4d/C4e「WIP 碰 tools/px ⇒ rc=2 且 p1 一次」、C4f「stamp 无 head ⇒ rc=2」
```

## 6 边界（说清楚，不假装）

```
能说  主树 WIP 不再阻断发布；WIP 明细每次发布都留证；tools/px 被改仍会拦且 p1
能说  「测试面=commit」在代码里有锚点、在门里有动态自证、负控能把判据打红
不能说 preflight 也免疫 WIP —— release_preflight.sh --gates 在主仓（主树）里跑，
      它读到的是含 WIP 的树；preflight 是**前置自检**，证据面仍是链（worktree@commit）。
      要连这点也消掉，就得让 preflight 也跑在冻结副本上（列下一轮，未做）。
不说  这次修法能防「链跑起来之后我又改了被测的 commit」—— 那是 chain-freeze
      （.githooks/pre-commit 见标即拦提交）+ publish 的 head-mismatch 两道在守。
```
