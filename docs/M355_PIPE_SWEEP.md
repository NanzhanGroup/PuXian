# M355 · 「生产者 | grep -q」同族扫尾（缺陷 621/622/624）

> 口径 19（编排层退出码与「红」同向）之外的**第二根支柱**：判据内部的取数方式。
> 本档是这族的**量化台账 + 收口进度 + 棘轮基线口径**。修复本身见各门 `verify.sh`。

## 1 机理（一句话）

`set -o pipefail` 下写 `producer | grep -q PAT`：

- `grep -q` 一命中就退出 ⇒ 读端关闭；
- 生产者后续 `write()` 拿 `EPIPE`/`SIGPIPE` ⇒ 退出码 141；
- `pipefail` 取管线最右非零 ⇒ 管线 rc=141 ⇒ **「命中」被读成「不含」**。

实测（M354 · 缺陷 624 · strace）：bash 的 `printf '%s'` 对 672 字节输出做了 **19 次 write**（逐行刷）。
⇒ **「小输出不会撞 SIGPIPE」是错的**；任何进程写端 + 提前退出的读端都会中招。

否定分支（`! ... | grep -q`、`|| bad`）相反方向更危险：**假绿**。

## 2 正确形态

| 场景 | 写法 |
|---|---|
| 变量内容 | `grep -q PAT <<< "$var"` |
| 生产者输出 | `grep -q PAT <<< "$(producer …)"`（先整段取回，再喂 grep） |
| 文件内容 | `grep -q PAT <file>`（本来就没有写端，不受影响） |
| 只判「有没有输出」 | `[ -n "$(producer …)" ]` |

## 3 量化（M355 起，棘轮）

扫描面 = 仓内**含 `pipefail`** 的 `.sh`（`examples/` `packaging/` `selfhost/` `tools/`），
判据条数 = 匹配 `\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q` 的行数。

| 时点 | 命中文件 | 残留条数 |
|---|---|---|
| M355 开工（改前） | — | **141** |
| M355 本轮收口 6 门 21 条后 | 53 | **120** |

> 注：M355 首轮用 Go 扫描器初测报 114，**欠计** —— `bufio.Scanner` 单行上限 64KB，超长行会让该文件**静默截断**。
> 守卫（`grep -c`）实测 141 才是真值。这条本身记一笔：**扫描器也会骗人**（口径 17 的同族）。

守卫：`selfhost/check_pipe_grep.sh`（门 `pkg_pipe_grep`）

- `--self-test` 四档：生产者接 `grep -q` 必须检出 / here-string 不检出 / 文件输入不检出 / 无 pipefail 不入面；
- 默认：按 `selfhost/pipe_grep_baseline.txt` 做**棘轮**（每文件允许残留条数，只许减）⇒ 新建/增加即判红；
- `--gate` = 自证 + 棘轮（门入口）；`--emit-baseline` 重生成基线；`--list` 逐条列残留。

## 4 本轮收口的 6 门（各 `bash examples/<门>/verify.sh` 单跑自证）

| 门 | 处数 | 形态 | 单跑 |
|---|---|---|---|
| m284_orchestrator | 4 | `printf '%s' "$ST/$SC/$out" \| grep -q` | ✅ |
| m261_lock_alloc | 3 | `echo "$OUT" \| grep -q`（含 1 处否定） | ✅ |
| m262_lock_alloc2 | 3 | 同上 | ✅ |
| m269_gcroot_coverage | 3 | `printf '%s' "$outA/B/C" \| grep -q` | ✅ |
| m222_http_body | 4 | `echo "$line/$r" \| grep -q` + `grep -A1 … \| grep -q` | ✅ |
| m116_builtin_semantics | 4 | `tail -1 file \| grep -q` ×2 + `ss … \| grep -q` ×2 | ✅ |

## 4.1 M356：口径修正 —— 注释不计入（120 → 97）与风险分层

**发现（M356 开工第一件事）**：门 `pkg_pipe_grep` 的计数正则 `\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q`
**不排除整行注释**。后果 twofold：

1. 基线虚高：M355 报的 120 条里，**23 条是注释**（历史说明如「缺陷 487 `tar -tzf … | grep -q`」被当成债务）；
2. **棘轮可被规避**：删一行注释就能让指标下降，「门更绿」而判据没修 —— 这是口径 19 的反面（假绿）。

修法：`scan_one` / `scan_one_file` / `--list` **三处同口径**改为
`grep -vE '^[[:space:]]*#' | grep -cE …`（行号保留用 `grep -n | grep -vE '^[0-9]+:#'`），
并加**夹具 E 负控**：整行注释里的同一形态 ⇒ 必须不检出（`--self-test` 4/4 → **5/5**）。
基线重生成：**120 → 97 条 / 50 → 42 文件**（= 120 − 23 注释 + 1，新增的夹具 E 文本行本身被计 1 条）。
⇒ **这 23 条是口径修正，不是判据修复**，不得计入「已收口」业绩。

仍未覆盖（待判，不猜）：单引号字符串里的**夹具文本**（`printf '… | grep -q …'` 生成测试文件用）也被计入，
`selfhost/check_pipe_grep.sh` 自身即有此类；要排除需引入引号感知解析，风险高于收益，暂记为已知偏差。

### 97 条的风险分层（M356 首测）

| 层 | 条数 | 说明 | 处置 |
|---|---|---|---|
| 大产出/易阻塞（`tail` 2 · `head` 2 · `readelf` 2 · `sed` 1 · `grep` 1 · `rpm` 1 · `dnf` 1） | **10** | 产出可能 > 64KB 或写多次，grep 命中即退出 ⇒ 生产者必 141 | **优先**，逐条改 |
| 小产出 `printf`/`echo`（变量回显） | **44** | M355 实测：`printf '%s'` 672 字节做了 19 次 write ⇒ 仍可能撞 SIGPIPE | 次批，按门归属批量改 |
| shell 函数/装置文本/其他（`iv_ok` `iv_bad` `chk` `body_of` `has()` …） | **43** | 多为自证装置里的字符串常量 | 最后，逐条判断真假 |

大产出 10 条明细：`m115_proc_env.sh:23,31`(tail) · `m217_shift_count/ub_proof.sh:97`(sed) ·
`m271_run_fast/verify.sh:66`(grep) · `build_rpm_el7.sh:51`(rpm) · `latest_guard.sh:59`(head) ·
`release_reconcile.sh:97`(head) · `verify_repo_openeuler.sh:60`(dnf) ·
`check_bin_portability.sh:90`(readelf) · `native_bootstrap.sh:295`(readelf)。

## 5 待办（按价值）

1. **大产出 10 条打头**（上表），一门一改一验 + 该门单跑；（否定分支优先，因为它造的是假绿）
2. 小产出 44 条按门归属批改，重点文件：`packaging/selftest_pxrepo_mirror.sh`(原报 21，现 19)、
   `examples/m314_mysql_err_conn`、`examples/m83_s5`、`examples/m64_doc`、`examples/m57_s2_mmap_verify.sh`；
3. `selfhost/check_gate_premise.sh`(5)、`packaging/make_tag.sh`(4)、`packaging/selftest_make_tag.sh`(7) 属**工具链**，
   改动要连带跑对应 selftest；
4. 每收口一批就 `--emit-baseline` 降基线，让棘轮咬住。

## 6 纪律（本族专用）

- 改动前后**同一条判据**的语义必须一致：`printf|grep -q` 与 `grep -q <<<` 在「命中」时都得 0；
- 否定条件不许顺手「简化」成肯定（那是另一个缺陷的入口）；
- 同一文件的多处替换**不要并发** —— M355 踩过：并行 edit 同一文件互相覆盖，只剩最后一处生效，
  门直接红（rc=2），看着像「改坏了判据」。**逐条串行改 + 改完 grep 复查 0 残留**。

## 7 M357：大产出 10 条收口（97 → 87）

改法统一为「先把产出**落到变量/文件**，再对落盘物 grep」——
`<<< "$(生产者)"`（here-string，无管道）或 `> 日志文件` 后 `grep 文件`。命中语义不变（都 rc=0），否定分支保持否定。

| 文件:行 | 原形态 | 改法 |
|---|---|---|
| `examples/m115_proc_env.sh:23,31` | `tail -1 log \| grep -q` | `grep -q X <<< "$(tail -1 log)"` |
| `examples/m217_shift_count/ub_proof.sh:97` | `sed -n 范围 dis \| grep -qE` | `grep -qE P <<< "$(sed …)"` |
| `examples/m271_run_fast/verify.sh:66` | `grep -A12 … \| grep -q` | `grep -q P <<< "$(grep -A12 …)"` |
| `packaging/build_rpm_el7.sh:51` | `rpm -E dist \| grep -q` | `grep -q 'el7' <<< "$(rpm -E …)"` |
| `packaging/latest_guard.sh:59` | `head -c1 f \| grep -q '\['` | `grep -q '\[' <<< "$(head -c1 f)"` |
| `packaging/release_reconcile.sh:97` | `head -c1 f \| grep -q '\['` | 同上 |
| `packaging/verify_repo_openeuler.sh:60` | `dnf list \| tee log \| grep -q` | `dnf list > log 2>&1 \|\| ! grep -q '^puxian' log`（tee 在中间，grep 提前退出会让 tee 吃 141 ⇒ 假红） |
| `selfhost/check_bin_portability.sh:90` | `readelf -d f \| grep -q NEEDED` | `grep -q NEEDED <<< "$(readelf -d f)"` |
| `selfhost/native_bootstrap.sh:295` | 同上 | 同上 |

结果：`PIPE-GREP-VERDICT=OK remaining=87 files=36`（97−10，命中文件 42→36：6 个文件清零被移出基线）。

**未动、属次层的 4 条**（不在大产出层）：`m271:202`（`printf '%s' "$out"`）、`latest_guard:148`（`curl -w http_code` 三字节）、
`check_bin_portability:66`（`is_elf()` 的 `head -c4 | od | tr | grep -qi`）、`m278_release_reconcile:89/93/100`（装置文本与自证）。

### 7.1 顺带发现的真红（不是本轮引入，登记待判）

`bash examples/m115_proc_env.sh` 单跑 **3/3 全红**，根因不在管道：
`examples/m115_proc_env.px:28` 等处的 `remove(WORKDIR + "/env.txt")` 对**不存在的文件**报
`运行时错误 … fs: 删除失败`。`remove()` 是否该容 ENOENT 是**运行时语义问题**，需单独判定（本轮不碰 runtime，避免重烘）。
另：该门脚本**不在 `gates.registry.sh`**（registry 里无 m115）⇒ 全量门从未跑到它，这是「门清单外的手工脚本」盲区，
与 M355 漏注册 ci.yml 同族形态。⇒ 待判：① `remove` 容 ENOENT；② 仓内 `examples/*.sh` 是否有其它未注册脚本。

### 7.2 全量门验收（M357）

`chain-m357-20261029-195742-3547422`：**绿 236 · 红 0 · 门清单 236 · 跳过 0**，`shard_run rc=0` / `release_guard rc=0`；
分片墙钟 4787s、链路总 5070s、loadavg max 80.34（16 核 ⇒ 5.02/核）、memory_peak 7902.8 MiB；
tag **v0.2.357**（对象 `376fae3`）→ commit `8f0c792`，latest 自证 OK。
⇒ 管道改法（here-string / 落盘后 grep）在 236 门规模上**未产生假红**；本层 10 条视为收口。

下一批（44 条小产出）开工前先做一件事：把本节的「改法」固化成**改法清单**（`<<<` 吞尾换行、
多行输出引号、`2>/dev/null` 位置三个坑），批量改时逐条对表。
