# 上游 registry-px 0.2.0 的缺陷登记（M282）

> 上游：`github.com/banshanhanfu/registry-px` @ **`01f6048c`**（2026-10-06T02:52Z）
> 本仓引入轮次：**M282**（引入 120 个 `0.2.0` 版本目录 + 110 个新用例 `*2_test.px`）
> 判据落点：`upstream-tests/EXPECTED.tsv` 的 **XFAIL** 等级 + `selfhost/run_upstream_tests.sh`

---

## 一 为什么需要这份文件

上游 0.2.0 一次加了 **113 个 `*2_test.px`**。跑在**我方官方 registry**（逐字节照搬的上游库）上，
**186 通过 / 40 失败**。逐条定性后：

| 类 | 条数（用例×轨） | 处置 |
|---|---|---|
| **我方跑法/夹具缺口** | 10 | ✅ **我方修**（R1–R4，见 §三）—— 不推给上游 |
| **上游用例与自家库矛盾** | 18 | ⚠️ XFAIL（§二 T1–T11） |
| **上游库自身缺陷** | 1 | ⚠️ XFAIL（§二 L1） |
| 环境不具备（需真实服务端） | 6 | SKIP（带理由） |
| 设计性（解释轨无并发） | 4 | SKIP（带理由） |
| 性能性 | 1 | SKIP（带理由） |

**关键纪律**：失败**不自动等于**上游的错。上表前 10 条就是**我方的**，已修。
只有「用例与它**自己的库**（同一次提交里的 `0.2.0/*.px`）互相矛盾」才判为上游缺陷 ——
判据是**源码对读**，不是「跑不过就赖上游」。

## 二 XFAIL 是什么（不是「把红记成绿」的出口）

`EXPECTED.tsv` 的取值有三档：`PASS` / `SKIP` / **`XFAIL`**。

- `XFAIL` = **期望失败**：这条用例**本来就跑不过**（上游自己矛盾），我方**不可修**
  （修它就要改上游用例 ⇒ 违反「用例逐字节照搬 + `MANIFEST.sha256` 对拍」）。
- ⭐ **带反向判据**：一旦某条 `XFAIL` **实测通过**，`run_upstream_tests.sh` **判红**并提示
  「上游可能已修，请复核并更新 EXPECTED.tsv」。
- ⇒ 它既**不隐藏**失败（跑一次就能看到 `XFAIL N` 的计数与理由），
  也**不会**在上游修好后悄悄漂绿。

---

## 三 ✅ 我方修掉的（R1–R4）—— 先说自己

| # | 现象 | 真因（我方） | 修法 |
|---|---|---|---|
| **R1** | `walk2` 两轨都 `stat 失败: registry/walk` | 0.2.0 用例改用**相对路径**访问 registry（`wk_walk("registry/walk")`），而运行器把产物的 cwd 设成 `$WORK/tests`（`$WORK/registry` 才是指向被测 registry 的软链） | 跑产物时 cwd 改 `$WORK`（构建仍留在 `$WORK/tests`） |
| **R2** | `ftp_2` → `SYST → 502 not implemented` | ftp mock 只实现了 0.1.0 用例用到的命令子集 | mock 补 **SYST/SIZE/MDTM/MKD/RMD/DELE/RNFR+RNTO/APPE/REST/CWD ..**（全是 RFC 959 标准命令，回复码由用例断言反推） |
| **R3** | `imap_2` → 连 `127.0.0.1:2143` 失败 | 0.1.0 用例用 **1143**、0.2.0 用 **2143**，mock 只听 1143 | imap mock 同时听 1143 + 2143；并补 **EXAMINE/SEARCH/NOOP/STORE/COPY/CREATE/DELETE/RENAME/CLOSE**（`CREATE` 后 `LIST` 必须看得见 ⇒ 邮箱表是进程内可变状态） |
| **R4** | `cli2` → `host from env: localhost`；`config2` → `recursive env: localhost` | 用例要求 shell 提供 `PX_TEST_HOST=env-host` / `APP_DB_HOST=env-db-host`（**用例明写的契约**） | 运行器导出这两个 fixture 变量，并 `unset PX_NONEXISTENT_ENV`（cli2 另有用例断言它不存在时回落 default） |

顺带修掉两处**运行器自身的**判据缺口（都属「判据静默变窄」族）：

- **成功判据过窄**：原为「`rc==0` 且输出含字面 `PASS`」，而 0.2.0 的 113 个用例里
  **`barcode2_test` 打印的是 `barcode 0.2.0 tests done`（无 PASS）** ⇒ 假红。
  改为 **「`rc==0` 且输出含完成标记（`PASS` **或** `done`）」**
  （0.1.0 的 125 个用例全部含 `PASS` ⇒ 旧判据对它们无影响）。
  ⚠️⚠️ **中途踩过一次坑（记在这里）**：第一版改成「`rc==0` 且输出无 `FAIL`」——
    那是**我猜的**契约。实测立刻被回归抓回：**`testkit_test` 的本职就是验证「断言失败时的报告」**，
    它会**故意**打印 `FAIL: testkit_test（7/24 失败）`（那几个 Err 是**被测对象**要产生的），
    再打印真正的结论 `testkit: PASS（成功/失败路径均符合预期）`，且 `rc==0`。
    ⇒ 铁律：**判据要贴着「实现」写，不是贴着「我猜的实现」写**；而这条错误正是被
      全量回归当场照出来的（说明该回归不是空跑）。
- **自我跳过的用例按 SKIP 登记**：`redis2_test` 在无 `6379` 时自己打印 `SKIP: redis 不可连接`
  后 `exit 0` ⇒ 按 **SKIP**（需真实 Redis）登记，**不**在运行器里放宽判据。
- **清单双向一致**：新增 ①b（`MANIFEST.sha256` 条目集 ⇄ `*_test.px` 集）
  与 ①c（`EXPECTED.tsv` ⇄ 用例目录 **双向一致 + 无重复**）。
  - ①b 的直接原因是实测事故：重建清单时写成 `sha256sum *.px fixtures/*.px`
    ⇒ **我们一改自己的 mock 就让「用例逐字节照搬」判据变红**（而 mock 是本仓文件，
    按 `fixtures/README.md` 本就**不该**进清单）—— 判据指不到真因。
  - ①c 抓到的实例：生成器用 `*2_test.px` 通配，把 **0.1.0 的 `oauth2_test.px`**
    （名字恰以 `2` 结尾！）也当成新用例重复登记；另有 2 个 0.2.0 用例在 M214 已登记。
    ⚠️ `exp_for` 的 awk 取**最后一个匹配** ⇒ 重复登记会让前一条被**静默忽略**。

---

## 四 ⚠️ 上游缺陷逐条（T1–T11 · L1）

> 判定口径：**同一次提交里**的 `tests/*2_test.px` 与 `registry/<n>/0.2.0/<n>.px`
> **互相矛盾**（或与同库的 0.1.0 用例互斥）。逐条给出**源码行号**，可原地复核。

### T1 `protobuf2_test` —— 用例传 list，库要 dict
- 用例：`protobuf2_test.px:9` `var schema = [`，`:14` `pb_encode(schema, obj)`
- 库：`registry/protobuf/0.2.0/protobuf.px:156` `if schema.has(fno):`
- 对照：`protobuf_test.px:8` `var schema = {`（0.1.0 传 **dict**）
- 且 `pb_encode` 在 **0.1.0/0.2.0 逐字节相同**（`0.2.0` 只在文件尾部追加了新函数）
⇒ 0.2.0 用例按一个**并不存在**的新 API 写的。

### T2 `targz2_test` —— 同型
- 用例：`targz2_test.px:9` `var files = [{"name": "a.txt", "data": [65,66,67]}]` → `tz_pack(files)`
- 库：`registry/targz/0.2.0/targz.px:76` `var ks = files.keys()`
- 对照：`targz_test.px:9` 传的是 **dict** `{"a.txt": …}`

### T3 `zlib2_test` —— 同型（参数类型）
- 用例：`zlib2_test.px:9` `var data = [72,101,108,108,111]` → `:10` `zl_compress(data)`
- 库：`registry/zlib/0.2.0/zlib.px:22` `zlib_compress(data, level)`（native 要 str/bytes）
- 对照：`zlib_test.px:8` `var msg = bytes("…")`

### T4 `tdtest2_test` —— 用例缺键
- 用例：`tdtest2_test.px:18` `{"name": "add", "fn": tc_add}`（**无 `args`**）
- 库：`registry/tdtest/0.2.0/tdtest.px:21` `var got = c["fn"](c["args"])`
- 对照：`tdtest_test.px:18` `cases.append({"name": "add1", "fn": add1, "args": 1, "want": 2})`

### T5 `shutil2_test` —— 漏 `.unwrap()`
- 用例：`shutil2_test.px:10` `var s = sh_read_text("/tmp/shtest.txt")` → `:11` `"read/write: " + s`
- 库：`registry/shutil/0.2.0/shutil.px:38-42` `sh_read_text` **返回 `Ok/Err`**
- 对照：`shutil_test.px:19` `sh_read_text(…).unwrap() == "hello"`

### T6 `sched2_test` —— 丢弃返回值
- 用例：`sched2_test.px:13` `sched_add(s, "task1", 100, my_cb)`（**无赋值**）⇒ 调度器里没有任务 ⇒ `g_count == 0`
- 库：`registry/sched/0.2.0/sched.px:22` 注释即写明「**返回新调度器 dict（不改入参）**」
- 对照：`sched_test.px:14` `s = sched_add(s, "fast", 40, tick_a)`
- 同文件 `:18` 还有一处：`sched_counts(s) == 0` 而 `sched_counts` 返回的是 **dict**（0.1.0 写法是 `len(…keys()) == 0`）

### T7 `readstat2_test` —— 期望值算错
- 用例：`readstat2_test.px:9` `s = "The cat sat on the mat. It was a good day."`，`:11` 断言 `len(rs_words(s)) == 12`
- 库：`registry/readstat/0.2.0/readstat.px:14-28` `rs_words` 按**空白**切分 ⇒ 该句是 **11** 个词
  （`The cat sat on the mat. It was a good day.`）
- `rs_words` 在 0.1.0/0.2.0 未变，且 0.1.0 用例（`rs_counts`）对另一句断言 9 词是通过的 ⇒ **是期望值错，不是实现错**

### T8 `pop3_2_test` —— 自相矛盾
- 用例：`pop3_2_test.px:22` 断言 `st["size"] == 114`，而同一条断言的**消息文本**写的是 `"STAT 2/100, got …"`
- 对照：`pop3_test.px:22` 断言 `st["size"] == 100`
- 夹具侧：`upstream-tests/fixtures/pop3_mock.px` 按 RFC 回 `+OK 2 100`（`:77`）
⇒ 一个 mock **无法同时**满足 100 与 114 ⇒ 0.2.0 与 0.1.0 用例对同一夹具互斥。

### T9 `ftp_2_test` —— 与 0.1.0 用例对同一夹具互斥
- 用例：`ftp_2_test.px:39` `expect(data.find("hello ftp") >= 0, "RETR hello.txt")`
- 对照：`ftp_test.px:46` `expect(got.unwrap() == "hello from ftp 普贤\n", …)`（**精确相等**）
- 而 `"hello from ftp 普贤\n".find("hello ftp") == -1`
⇒ 无任何单一 `hello.txt` 内容能同时满足两条用例。（我方**没有**为通过而改动 mock 的
`hello.txt` —— 那属于「为了绿而放宽夹具」。）

### T10 `workerpool2_test`（仅 build 轨）—— 读不存在的键
- 用例：`workerpool2_test.px:14-15` `wp_run(pool, …)` 之后 `len(pool["results"])`
- 库：`registry/workerpool/0.2.0/workerpool.px:52` `return Ok({"results": results})`（**不写回 pool**）
- 对照：`workerpool_test.px:24` `r.unwrap()["results"]`

### T11 `semaphore2_test`（仅 build 轨）—— 漏 `.unwrap()`
- 用例：`semaphore2_test.px:9-10` `var sem = sp_new(3)` → `sp_try_acquire(sem)`
- 库：`registry/semaphore/0.2.0/semaphore.px:15-18` `sp_new` 返回 `Err(…)` 或 `Ok({…})`
- 对照：`semaphore_test.px:23-25` `var sem = sp_new(2)` → `sem.is_ok()` / `sem.unwrap()`
- ⚠️ 附带一条**库侧**不一致（见 L1 同族）：0.2.0 新增的 `sp_stats`/`sp_is_full`/`sp_usage`
  读 `sem["count"]` / `sem["limit"]`，而 0.1.0 的数据模型是 `{"n":…, "used":…, "mu":…}`

### L1 `concurrent_map` 0.2.0 **库自身**缺陷（仅 build 轨）
- 用例：`concurrent_map2_test.px:16-17` `cm_clear(m)` 后断言 `cm_len(m) == 0`
- 库：`registry/concurrent_map/0.2.0/concurrent_map.px:63-65` 新增的 `cm_clear` 写的是 **`m["data"]`**
  （`:70`/`:71` 的 `cm_values`、`cm_to_string` 同）
- 而 0.1.0 的**全部**函数用 **`m["m"]`**（`cm_len` 见 `:50-54`）⇒ 清空后长度仍是 2
- ⇒ 这是**库的新增部分与自家数据模型不符**，不是用例写错。
  ⚠️ 我方**不改**（改了就不叫「逐字节照搬」，且 `THIRD_PARTY.md` 的 sha256 对拍会失效）。

---

## 五 SKIP 的 11 条（环境/设计/性能，均带理由）

| 用例 | 轨 | 理由 |
|---|---|---|
| `mongodb_2` `mysql_2` `mqtt_2` | 两轨 | 需真实服务端（27017 / 3306 / 1883），本机与 CI 均无；`--run-skipped` 可跑 |
| `actor2` `concurrent_map2` `semaphore2` `workerpool2` | interp | 解释轨不支持 通道/mutex（Mini 子集排除，设计如此） |
| `bcrypt2` | interp | `rc=124`（120s 超时）；同 `bcrypt_test` 先例（M214 实测 1801s 未完成） |

---

## 六 复现

```bash
# 全量（238 用例 × 2 轨；自动起 mock，EXIT 时清理）
selfhost/run_upstream_tests.sh --json /tmp/ut.json

# 只看登记与理由
selfhost/run_upstream_tests.sh --list

# 单条
selfhost/run_upstream_tests.sh --only protobuf2_test -v

# 跑「本应跳过」的（含上面 11 条）
selfhost/run_upstream_tests.sh --run-skipped --only mongodb_2_test
```

判据三条（脚本内在）：① `MANIFEST` 双向一致 ② 引用面完整 ③ `EXPECTED` 双向一致 + 无重复。

## 七 给上游的建议（可直接转发）

1. **`*2_test.px` 与 `0.2.0/*.px` 对不上** —— 建议上游把 `tests/` 也纳入 CI，
   并在改 API 时同步改用例。当前形态看起来是「用例先按目标 API 写好，库只做了追加」。
2. `cm_clear`/`cm_values`/`cm_to_string` 的键名（`data` vs `m`）、
   `sp_stats`/`sp_is_full`/`sp_usage` 的键名（`count`/`limit` vs `n`/`used`）
   建议与 0.1.0 的统一，否则 0.2.0 是**静默错值**而不是报错。
3. `pop3_2_test` 的 `size` 期望（114 vs 100）与 `ftp_2_test`/`ftp_test` 对同一夹具的互斥断言，
   建议二选一后固定下来，并**把 fixture 脚本入库**（否则下游只能自建 mock，且契约靠推断）。
4. 建议所有用例统一「成功标记」（现在有 `PASS: x`、`x 0.2.0: PASS`、`x 0.2.0 tests done` 三种），
   下游运行器才能用统一判据；或直接**约定「退出码即判据」**。
