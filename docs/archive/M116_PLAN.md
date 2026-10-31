# M116_PLAN · 真实模块 PuXian 化暴露的「静默错 / 无信号错」修复

> 来源：继续用 `github.com/NanzhanGroup/ws-center` 与 `github.com/NanzhanGroup/ws-ddns`
> 的 **PuXian 化**当探针（qg-issue 71 台账）。M115 补的是"**缺**原语"（写不出），
> M116 治的是"**有**原语但**错得没信号**"（写得出来、跑得过去、结果是错的）。
> 五条缺陷的共同点：**既有门全绿** —— 因为两条轨各错各的、或错在同一方向（对拍/自举/示例都看不见）。

## 一、缺陷与修法

| # | 现象（实测） | 根因 | 修法 | 探测方式（门） |
|---|---|---|---|---|
| **D2** | `d["缺的键"]`：编译轨**静默 `null`**、解释轨 `R1008` 杀进程 | `px_index`/`px_field` 的 dict 分支直接 `px_dict_get`（缺键返回 null），而解释器 `i_index`/`i_field` 判 `has` 后报 R1008 | 两处补 `px_dict_has` → 统一 `R1008: 字典没有键 'x'`（非 str 键文案对齐 `R1002`） | `dict_missing_key.px` **两轨都必须非零退出 + 含 R1008 + 出错点之后的语句不得执行** |
| **D4** | 函数体里写多行 list/dict/调用 → `语法错误 E2001 意外的 token: 去缩进`，**行号指向下一行行首** | 词法器把续行缩进**照常压栈**（spec 只说"parser 忽略这些 token"，但栈被压入一个永不配对的层级）→ 行尾一次弹两级 → 多出一枚「去缩进」把外层块**提前结束** | `pxlexer.px` 增 `g_bracket_depth`（在 `emit_at` 唯一入口记账，含插值产出的 `(` `)`）；深度 >0 时行首缩进不参与缩进栈、不发缩进/去缩进 | 双轨语义用例（更深/顶格/嵌套/尾逗号）+ "续行之后的语句仍属本函数"反例自检 |
| **D5** | `dns_txt("google.com")` / `"cloudflare.com"` 直接 Err（自称"TCP 回退二期"），Go 侧成功 | UDP 响应 TC=1 时直接报错 | 按 RFC 1035 §4.2.2 实现 TC → TCP 重发（2 字节长度前缀 + 短读循环） | 联网软检（离线记 SKIP）；实测 google 17 条 / cloudflare 28 条 |
| **D6** | `mkdir(dir, 0o700)` 报"需要一个路径参数"；权限恒 0755 | `bi_mkdir` 单参、硬编码 0755 | `mkdir(path[, mode])`：mode 作用于**所有新建层级**，最终仍受 umask 约束（同 Go）；`selfhost/ibuiltin.px` **同步透传**（qg-issue 69 同族：手写转发层漏列尾参 = 解释器轨静默丢弃） | 绝对权限位断言（按 umask 折算）+ 缺省 0o755 零回归 |
| **D7** | handler 出错：VM 轨客户端收 **`204 No Content`**；C 轨**整台服务器 exit(1)** | ① `coro.c` 用 `if (px_spawn_isolate_begin())` 判"协程是否出错"，而该函数跨编译单元的 **returns_twice 语义不可靠**（runtime.h 早有告警，同 M96-S2 教训）→ 出错协程被当"正常跑完"，done_cb 收到 `vm.ret_val` 初值 `PX_NULL`；② 同步轨 `px_call(handler)` 外无错误边界 | ① 出错标记改由 `px_spawn_isolate_begin()` 在 **longjmp 落点**置 TLS、调用方事后查 `px_isolate_errored()`；② 新增 `px_serve_error_resp(kind)`（handler/middleware 通用 500 值）；③ 协程轨 done_cb 与同步轨（`runtime_route.c` + vhost 直调，改用既有 `px_native_call_capture`）**行为一致** | 端到端：起服务 → 打出错路由 → 断言 **500** + **服务存活**（VM 轨与 C 轨各跑一遍） |
| **D1** | `def main()` + 顶层 `main()` → 整个程序**跑两遍**（副作用翻倍） | 设计上 `main` 是自动入口，显式调用是第二次 | 不改语义，`px lint` 新增 **L009 告警** | 现有 lint 门（warn 不失败；`--strict` 才红） |
| **D7 续** | `http_serve` 轨（非 `px_serve`）同样出错 → **200 + 空 body** | 同一个洞，但漏在 `http_handler_done`（另一条 done 回调） | 五处（两个 px_serve done + http_handler_done + runtime_route 同步轨 + vhost 直调）统一 | ws-center 对拍（main.px 用 `http_serve`）当场照出 |
| **D13** | multipart 上传 18 字节文本 → 服务端存成 **17 字节**（吃掉数据自身的结尾换行） | `px_parse_multipart` 在 `ce` 扫描（已停在段体后 CRLF）之后**又**削一次 `\n`/`\r` | 删掉那次多余的削除 | ws-center `/v1/files/upload` 的 `size` 与 Go 逐字节对拍 |

## 二、边界（写明没做什么）

- D2 的修法选**编译轨补报错**（以 R1008 为唯一口径），**不是**放宽解释轨 —— 依据
  `docs/M65_PLAN.md` 记载的口径「R1008 + `.has()` 守卫」。
- D5 的 **NXDOMAIN 语义保持不变**（空 list 非报错）：`examples/m103_s2a` 的既有断言即规格，
  且 ws-ddns 调用方已按"空 = 无记录"处理。与 Go `net.LookupTXT` 的差异**在速查包显式注明**。
- D7 未覆盖 SSE / UDP handler（`sse_serve` / `udp_serve`）：两者的错误面表达不是 HTTP 状态码，
  保持原行为并登记。
- D1 只加 lint **告警**，不改语言语义（避免破坏既有代码）。
- **D12（未做，已登记）**：无文件 stat/mtime 原语 ⇒ ws-center 的 `mod_time` 经
  `/bin/stat` 外部命令取。新增 native 需五处登记 + 全件重烘，与本批分开评估（本轮不夹带）。
- **D11（未做，已登记）**：括号内 dict **冒号后换行**（`{"k":\n 1}`）仍报 E2001 ——
  属"括号内换行"能力的**未覆盖子集**，非静默错；报错位置清晰，按新功能评估而非缺陷。

## 三、门与验收

- `examples/m116_builtin_semantics/verify.sh`（并入 CI 工具自测步）：
  ① 双轨语义断言（D4/D6/D2 守卫正向）② D2 两轨负控 ③ D5 联网软检
  ④ D7 端到端 500/存活（VM + C）⑤ 门自检负控（改错值必须红）⑥ D4 反例自检（续行后函数体完整）。
- 回归：`rebake_bin.sh --check/--check-vm/--check-all`、`diffcheck.sh`（lexer/parser/errors/codegen/value/interp）、
  `engine_parity.sh`、`interp_builtin_parity.sh`、`zombie_reap_check.sh`、双自举证明。

## 四、执行约定

- 多步骤里程碑用 task_checkpoint 记录子步进度（网关重启后可续）。
- 完成一个里程碑：更新本文档状态 + CHANGELOG.md + README.md（示例/能力表）+ spec.md（如需）+ 一次 commit 推送。
