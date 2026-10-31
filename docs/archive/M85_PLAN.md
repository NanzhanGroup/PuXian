# M85_PLAN · pxc build 编译产物按需裁剪（qg-issue 24 · 细粒度模块开关）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD 8868849 = M84-S4 · tag v0.1.0-m84 · native **301** · stdlib 13 库）
> 来源：qg-issue **24-puxian-build-size**（用户提问「编译出来都在 9M 以上，能去掉未使用模块吗」→ 真机实测 + 源码级解剖）
> 用户指令（2026-09-06）：**「立项 M85 吧」**
> 目标：`pxc build` 支持**细粒度模块开关**，裁剪未用 runtime 模块，产物 9.0M → 视裁剪面 **2~4M**；默认全能力 build 行为不变
> 性质：**L0 工具链/build 面**（tools/pxc bash 装配器 + runtime.c 条件编译包裹；不改语言语法、不改现有 native 语义、native 总数不变 **301**）
> 模板：**PX_NO_QUIC（M57-S4）单开关 → 泛化为模块开关集**（runtime.c 已有 6 处 `#ifndef PX_NO_QUIC` 先例 + tools/pxc --no-quic + 缓存 key + 裁剪态 R1001 语义文档化）
> 分批次：**S1→S3 三批**，每批独立开发 + examples verify + commit；S3 统一自举重链收口 + 全量回归（M84 同款大回归链）

## 〇、现状侦查（2026-09-06 源码级实录，非记忆推断）

**装配链**（tools/pxc = bash，M-B9a）：
1. `bootstrap/pxc build <file>` → 用户 `base.c`（单文件 C，stdout）；
2. `rt_ensure` → `.rtcache/<key>/` 预编译全部 runtime `runtime*.c` 成 .o（key = runtime 源×cc×no_quic，M71-S1）；
3. 链接 = base.o + **缓存全部 .o** + `sqlite3.o`（无条件）+ mbedtls `libmbedtls/x509/crypto.a`（无条件）+ `libz.a`（无条件）+ quic 模式再 + ngtcp2×2 + openssl ssl/crypto×2 → 产物与程序实际所用模块无关。

**native 注册**（为何无法自动丢弃未用 .o）：`runtime.c px_register_builtins()` L5313 集中注册 —— sqlite 族内联注册 L5557-5562（`px_ffi_register("sqlite_open", bi_sqlite_open)`…）、各模块注册函数无条件调用：`px_register_zlib()` L5567 / `px_register_quic()` L5570（#ifndef PX_NO_QUIC 内）/ `px_register_h3()` L5572 / `px_register_h3_qpack_dyn()` L5574 …。各 `runtime_<mod>.c` 顶部的 `px_register_<mod>()` 均被 runtime.c 引用 → ld 全链。

**体积大头**（仓库实测）：openssl ssl+crypto .a ≈ 10.4M（quic 链入抽用后仍大头）、sqlite3.o 1.3M、mbedtls 三 .a ≈ 1.5M、ngtcp2 ≈ 816K、zlib 152K、miniz ≈ 100K+。实测 `hello.px`：默认 **9,010,184 B** / `--no-quic` **3,929,808 B**（−56%）。

**全仓无其他 `PX_NO_*` 宏**（仅 PX_NO_QUIC，已核实）→ 模块开关为全新扩展面。

## 一、范围与边界

**做（S1–S3，qg-issue 24 方案 B 全量）**：
- **模块开关集**（--no-quic 之后的新 flag，全部与 --no-quic 正交可组合）：
  `--no-sqlite`（去 sqlite3.o + 注册）/ `--no-ws`（去 runtime_ws.o）/ `--no-zip`（去 runtime_zip.o + miniz 若仅 zip 用）/ `--no-xml`（去 runtime_xml.o）/ `--no-aes`（去 runtime_aes.o）/ `--no-rsa`（去 runtime_rsa.o）/ `--no-ed25519`（去 runtime_ed25519.o + tweetnacl）/ `--no-tls`（去 mbedtls 三 .a —— 连带 https/wss/h2s 面；依赖矩阵 S1 首步核清 mbedtls 引用面后收口宏范围）/ `--no-route`（去 runtime_route.o，vhost/middleware/session 等面）。zlib/ffi/h2 视依赖矩阵归并（zlib 若仅 gzip 面且 miniz 复用则慎裁——首步侦查定）。
- **runtime.c 宏包裹**：`px_register_builtins()` 各模块注册行/调用行 `#ifndef PX_NO_<MOD>` 包裹（sqlite 内联段整段包；各 `px_register_<mod>()` 调用行包）；模块 .c 不参与编译即可（相互独立、仅被 runtime.c 注册引用 → 已核实模式）。
- **tools/pxc**：flags 解析 + `rt_src_files`/`rt_cache_compile` srcs 与 `-DPX_NO_<MOD>` 按开关集过滤 + 链接命令按开关去 sqlite3.o / mbedtls .a / 模块 .o / libz.a + **缓存 key 纳入模块开关集**（rt_key 参扩）。
- **语义对齐**：裁剪态缺 native 调用 → R1001（与 --no-quic / pxi_native_diff 既有叙事一致）；capability/文档补"裁剪态 native 数动态"说明。
- **验证**：examples/m85_sN verify —— 各开关裁剪态 build 成功 + 产物体积断言 + 缺 native R1001 可判定 + 裁剪态功能面自测（如 no-ws 态 http 仍过）+ **全能力默认零回归**（capability 双模式 + 既有 examples 全量）。
- **S3 收口**：runtime.c 文本变更 → 全能力重链 bootstrap/pxi（语义不变但按 M84-S4 惯例重链证明）+ 自举证明 + 回归总闸 + 文档同步 + CHANGELOG + qg-issue 24 归档 done/ + 00-README 更新（**tag v0.1.0-m85 待用户令**，规划默认推）。

**不做（边界）**：
- ⏸️ 方案 C 自动 GC（--gc-sections 自动裁）→ 二期评估，不承诺。
- 不改 compiler/parser/语言语义/现有 native 签名与默认 build 产物行为（不传 flag = 现状 9.0M）。
- 不降级 bootstrap/pxi/pxc/pxl 等发布物（始终全能力构建）。
- 不裁 runtime.c 主文件内置面（cron/signal/time/tcp/http_serve 主 HTTP1.1 等核心依赖，不设开关）。

## 二、分批次执行计划

| 批 | 内容 | verify（examples/） | 预计 commit |
|---|---|---|---|
| **S1** | 依赖矩阵首步侦查（mbedtls/miniz/zlib/h2/route 引用面核清）→ 开关集定稿 → runtime.c 宏包裹 → tools/pxc flags+缓存 key+链接裁剪 | m85_s1：hello 各开关组合 build 成功 + 体积断言（--no-quic --no-sqlite --no-ws --no-tls 等 ≤ 阈值）+ 缺 native R1001 + 默认全能力产物尺寸/行为零漂移 | M85-S1 |
| **S2** | profile 预设（--min 聚合 = 去 sqlite/ws/tls/zip/xml 等全裁，服务端纯 HTTP 态）+ 交叉 target 组合验证（--target aarch64 + 裁剪，交叉库完整性检查适配）+ 文档同步（CHEATSHEET/spec/pxi_native_diff 裁剪态叙事） | m85_s2：--min hello ≈ 最小体积断言 + aarch64 裁剪产物 file/qemu 冒烟 + capability 全能力段不受裁剪 flag 影响（不传=全绿） | M85-S2 |
| **S3** | 收口：全能力重链 bootstrap/pxi + 自举证明 + 回归总闸（m82 + m83_s1–s6 + m84_s1–s3 + m85_s1–s2）+ 文档 + qg-issue 24 归档 done/ + CHANGELOG + tag v0.1.0-m85 | 全量回归 PASS + 双模式抽查 + fmt/lint 0 + worktree 干净 + CI 绿 | M85-S3 |

## 三、验收清单（S3 收口总闸）

- [ ] 开关集全部落地且相互正交可组合；默认不传 flag 产物行为/尺寸与 M84 零漂移
- [ ] 各裁剪态 verify PASS（体积断言 + 缺 native R1001 + 功能面自测）
- [ ] 回归：m82 + m83_s1–s6 + m84_s1–s3 全绿（默认全能力路径）
- [ ] 全能力重链 bootstrap/pxi + 自举证明 rc=0
- [ ] 文档同步（CHEATSHEET/spec/pxi_native_diff/README）；native 总数 301 不变（裁剪态动态计数有说明）
- [ ] qg-issue 24 归档 `done/`；00-README 总览更新；M85_PLAN 全程记录
- [ ] tag v0.1.0-m85 推 GitHub（待用户令）；CI completed success
- [ ] fmt/lint 0 错；worktree 干净

## 四、风险与预案

| 风险 | 预案 |
|---|---|
| 依赖矩阵误判（如 mbedtls 被 http 明文路径引用 / miniz 被 zip+gzip 双用 / h2 依赖 tls）导致裁剪态链接失败或运行崩溃 | S1 首步全仓 grep 引用面核清后定开关集；每开关独立 verify 编译+冒烟；裁剪态功能面自测兜底 |
| runtime.c 宏包裹改动影响全能力路径（宏默认不激活，理论上零行为漂移） | S3 全能力重链 + capability 双模式 + 全量回归总闸；M84 同款流程 |
| 缓存 key 未纳入模块开关 → 裁剪/全能力互串旧 .o | rt_key 签名扩为全开关集；verify 断言切换 flag 后产物尺寸变化（缓存未命中重建） |
| 交叉 target（aarch64 等）与裁剪 flag 组合时库完整性检查误报 | target_defaults 输出保留全库路径；裁剪只去链接不去检查，或按开关跳过对应检查 |
| flag 爆炸（10+ 开关难记） | --min/--server 等 profile 预设（S2），细粒度 flag 保底；help 文档同步 |
| 裁剪态 native 数漂移冲击 gen_native_table / CI diff | native_index 以全能力为准（301）；裁剪态在 pxi_native_diff/README 叙事说明，不进 CI diff |

---

## 附：执行状态记录（2026-09-06）

| 批 | 状态 | 说明 |
|---|---|---|
| S1 | ✅ done（M85-S1 commit） | runtime.c 14 处宏包裹（sqlite/ws/zip/xml/aes/rsa/ed25519/route/zlib/h2，10 模块）+ tools/pxc 10 个 --no-* flag + cuts 缓存隔离 + 链接裁剪；verify PASS=9 FAIL=0：默认 9010184 零漂移 / --no-quic 3929808 / --no-quic --no-sqlite 2884072 / 全裁 2713472（9.0M→2.7M −70%）；缺 native → 未定义变量（R1001 叙事）+ 核心 http native 保留 |
| S2 | ✅ done（M85-S2 commit） | --min profile 预设（聚合全裁 2713472）+ --min==全裁等价 + --target x86_64 折叠+裁剪叠加（7968768<9.0M）+ 文档同步（README/CHEATSHEET/pxi_native_diff/spec）；verify PASS=8 FAIL=0；aarch64 真机交叉组合由 CI 覆盖（本机无 musl 交叉链，预案 4） |
| S3 | ✅ done（M85-S3 收口 commit，tag 待用户令） | 全能力重链 bootstrap/pxi（产物 9457456 与 m84 ELF 同字节仅 build-id 异 → 宏包裹零语义影响实证）+ 自举证明 rc=0 + 回归总闸 12 批内容全 PASS（m82/m83_s1-6/m84_s1-3/m85_s1-2；m83_s5/s6 收尾 trap 边界同 M84-S4 记录不修）+ 文档同步 + qg-issue 24 归档 done/ + CHANGELOG；tag v0.1.0-m85 待用户令 |

### S1 侦查修正（2026-09-06 源码级实录，覆盖规划假设）

- **`--no-tls` 从开关集剔除**：mbedtls 非仅 TLS 面——runtime.c 主文件 **104 处直接调用**（http/https/wss/tls_server 核心 native，不设开关）+ aes/rsa/zip/ws(sha1) 亦以 mbedtls 为实现底座 → mbedtls **恒链**；真正的"去 mbedtls"需先把 HTTPS/TLS 逻辑从 runtime.c 拆独立模块（超 M85 边界，候选 M85b/M86）。
- miniz 被 runtime.c（gzip 内联核心 bi_gzip_* L9401）与 runtime_zip.c 共用 → **恒链**，不随 --no-zip 裁。
- tls_server/session/basic_auth（M27 webserver 生产化）定义在 **runtime.c 内联**（非 ws 模块）→ 不随 --no-ws 裁。
- sqlite 裁剪最净：runtime.c 对 sqlite3 API **零直接调用**，注册两段（px_set_global L5547-5553 + ffi_register L5556-5562）宏包 + 去 sqlite3.o 直链 1.3M。
- runtime_h2.c 无 native 注册（http_serve 内部 px_h2_handle 两调用段宏包）→ h2 裁剪语义 = http_serve 退化为 HTTP/1.1（非 native 缺失叙事）。
