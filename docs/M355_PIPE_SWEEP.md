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
