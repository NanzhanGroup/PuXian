# PuXian 生态总览（Ecosystem Overview）

> 面向人（开发者/使用者）与 AI（agent/大模型）的 PuXian 生态入口：**有什么库、能干什么、怎么拿来用、怎么写对**。
> 建立：M69-S1（2026-09-05）· 维护：stdlib 变更须同步本文件与机器索引（见 §5 防漂移）。
> 配套：`docs/PUXIAN_CHEATSHEET.md`（AI 速查包，M69-S2）· `docs/ECOSYSTEM_GAPS.md`（写库规范与语言缺口评估，M69-S4）。

---

## 1. 标准库一览（stdlib/ · 9 库）

全部为纯语言 `.px`（零 C），随发布包分发；`import std.<name>` 或 `from std.<name> import <fn>`。
除注明外均为纯函数库（无 IO、无状态），**编译（pxc build）与解释（pxi run）双模式一致**（M68 起 native 可达性根治，本表示例均双模式实测）。

| 库 | import | 定位 | 顶层导出（公开 API） | 适用场景 |
|---|---|---|---|---|
| **collections** | `std.collections` | 集合高阶操作（M5，最早 stdlib） | `each` / `unique` / `flatten` / `zip_lists` / `chunk` / `group_by` / `sort_by` | 列表去重/分组/分块/展平/拉链，纯函数组合 |
| **semver** | `std.semver` | 语义化版本 2.0.0（子集，M45） | `sv_parse` / `sv_cmp` / `sv_range_parse` / `sv_satisfies` / `sv_best` | 版本解析/比较/范围匹配（^ ~ * x），pxpkg registry 依赖解析 |
| **webroute** | `std.webroute` | 文件即路由命名规则（M43） | `wr_methods` / `wr_is_special` / `wr_seg_to_pattern` / `wr_parse_file` | `get_healthz.px → {method:GET, pattern:/healthz}`，routegen 路由生成 |
| **yaml** | `std.yaml` | YAML 子集解析器（M66 收编） | `yaml_parse` | 配置文件（注释/嵌套 map/序列/标量），返回 `{ok,value}` 或 `{ok,error}` |
| **pxml** | `std.pxml` | PXML 配置语言解析器（M66 收编，规范 `docs/PXML.md`） | `pxml_parse`（+ 内部 tokenize/parse 系列） | 类 ini 增强配置（类型化标量/单位 720h/数组/嵌套块），dogfood 闭环 |
| **lunar** | `std.lunar` | 农历 1900-2100 公历互转（M66 收编，天文推算 0 误差） | `lr_solar_to_lunar` / `lr_lunar_to_solar` / `lr_info` / `lr_leap_month` / `lr_month_days` / `lr_md_in_year` 等 | 农历生日/节气/节日、公农历互转（含闰月） |
| **gfx** | `std.gfx` | 纯语言 2D 画布与图形（M61） | `canvas_create` / `canvas_w` / `canvas_h` / `canvas_pixels` / `set_px` / `get_px` / `line` / `rect` / `fill_rect` / `circle` / `fill_circle` / `blit` / `text` / `text_size` | 无屏 2D 合成（Mandelbrot/贪吃蛇），画布 `list[int]` 0xRRGGBB |
| **png** | `std.png` | 纯语言 PNG 编码器（stored，M61） | `png_encode(w, h, pixels) -> bytes` | 把 gfx 画布编码为 PNG 文件字节（CRC32+ADLER+zlib stored 自实现） |
| **edge** | `std.edge` | 边缘设备：GPIO V2 / I2C / 串口 / PWM（M60） | `gpio_request/input/output/read/write/wait/event` · `i2c_open/read_reg/write_reg` · `serial_open` · `pwm_setup/enable/set_duty` | Linux 边缘设备（树莓派/网关/盒子）直控；依赖 fd 原语，需真设备或 PTY 验证 |

> **edge 说明**：失败语义与 M57 fd 原语一致——设备失败返回 int `-1`/`false`，`os_errno()` 查询，不杀进程；x86 无真板时示例走 SKIP 通道，真板段用 `examples/m60_*.px` 与 PTY 内核回环验证。

## 2. 快速用库（实测输出）

```px
import std.collections
import std.semver
import std.yaml

print(unique([3, 1, 3, 2, 1]))                          # [3, 1, 2]
print(json_stringify(group_by([1,2,3,4,5,6], fn (x): x % 2)))
# {"1":[1,3,5],"0":[2,4,6]}

print(json_stringify(sv_parse("1.2.3-alpha.1+b5")))
# {"major":1,"minor":2,"patch":3,"pre":"alpha.1","build":"b5","raw":"1.2.3-alpha.1+b5"}
print(sv_satisfies("1.2.5", "^1.2.0"))                  # true
print(sv_best(["1.2.0", "1.2.5", "2.0.0"], "^1.2.0"))   # 1.2.5

var ya = yaml_parse("name: PuXian\nver: 1\nlist:\n  - a\n  - b\n")
print(ya["ok"])                                         # true（ya["value"] 为解析节点）
```

其余库一行式：`wr_parse_file("get_healthz.px")` → 路由规则 dict；`lr_solar_to_lunar(2026, 9, 5)` → 农历（实测 `{"ok":true,"year":2026,"month":7,"day":24,"leap":false}`）；`pxml_parse("a = 1\nb = \"hi\"\n")["ok"]` → true；`set_px(canvas_create(4,4), 1, 1, 0xFF0000)` 后 `get_px` → 16711680；`png_encode(2, 2, [...])` → 82 字节 PNG。完整 API 文档：`tools/pxc doc stdlib/<name>.px`。

## 3. Dogfood 资产图（examples/ · 119 个 .px）

PuXian 每个里程碑都用普贤自己写示例/工具/应用（dogfooding 自证）。examples/ 是可复用资产库与能力导航：

### 3.1 能力域导航（找「某某怎么做」的参考）

| 能力域 | 代表 examples（.px） | 说明 |
|---|---|---|
| 语言内核 / GC / 并发 | `hello` `fib` `struct` `match` `concurrent` `gc_demo` `std_demo` `toolchain_demo` `m22_tracing_gc` `m25_closure_gc` `m30_comp` `m39_gc/result/err_main` `m40_str_interp` | 语法/闭包/GC/Result/插值入门 |
| HTTP 客户端 | `https_demo` `http_neterr_result` `m23c_http_adv` `m24_http_adv` `m37_s3`(S3 对象存储) `m38_chunked`(流式) | 客户端全特性（TLS/gzip/chunked/池化/Unix socket） |
| HTTP 服务端 / 应用平台 | `m27a_webprod` `m28_route` `m29_webprod` `m31_sandbox/vhost` `m32_hot_reload` `m33_access_log/route_rate_limit/sni` `m34_pool_cfg` `m35_gzip_rl` `m36_log_ctx/pool_grace` `p4_http_server` `p5_px_serve` `m82_http_serve_unix` | px_serve 生产形态：路由/中间件/限流/日志/优雅关闭/Session/基础认证；http_serve_unix（M82）= Unix socket HTTP 服务端（ws-approve serve 场景） |
| WebSocket / SSE | `m22_websocket` `m23a_sse_ws` `m26_ws_heartbeat/sse_https` `m32_ws_url/sse_reconnect` `m34_bus_ws` `m36_ws_hb` `m38_ws_reconnect` | 双端 + 心跳/重连/广播 |
| TLS / 加密 / 压缩 / 编码 | `m23d_rsa` `m25_tls_resume` `m27b_tls_serve` `m30_tls_pool` `m33_sni` `m35_gzip_rl` `m37_h2_tls` `p2_crypto_hash` `p7_aes_xml_zip` `p8_slice_base64` `m23b_bytes` `m26_ushr` `m30_int_bytes` | RSA/AES/证书/压缩/字节视图 |
| SQLite / 时间 / cron / 会话 | `m28_time_sqlite` `m28_cron` `m36_log_ctx` | sqlite_exec/query、6 字段 cron、会话 |
| UDP | `m33_udp_alt_svc` `m38_udp_serve` | udp_open/send/recv/serve |
| QUIC / HTTP/3 专项 | `m46_*` `m47_h3_*` `m48/m49/m50/m51/m52_*` `m53_s1~s5_*` `m54_s1~s5_*`（30+） | QUIC→H3 全链路：QPACK/多路复用/0-RTT/迁移/三栈合一，aioquic 互操作对拍 |
| 边缘设备 / 系统 | `m57_s1_ioctl/s2_mmap/s3_devctl/s3_gpio/s3_i2c/s5_pxi_smoke/s7_vhost_headers` `m60_gpio/i2c/pwm/serial_pty` `m23d_proc_signal` | fd 原语/mmap/GPIO/I2C/串口/PWM（真板或 PTY） |
| 生产 demo 十连（p0-p9） | `p0_random_io` `p1_mutex_rwlock` `p2_crypto_hash` `p3_regex` `p4_http_server` `p5_px_serve` `p6_timer` `p7_aes_xml_zip` `p8_slice_base64` `p9_http_adv` | 每里程碑能力合辑，适合通读 |
| 问题复现 / 研究 | `repro_h2_vhost` `repro_tls_no_sni_handshake` `s3_neterr_fail/result` | 历史 bug 复现与错误语义研究 |

### 3.2 里程碑主线导航（m22 → m67，每段主题）

| 段 | 主题 | 代表文件 |
|---|---|---|
| m22–m26 | 位运算/GC/WS/SSE/字节/TLS/闭包 | `m22_bitwise_data` `m22_websocket` `m23a_sse_ws` `m24_slice_xml` `m25_tls_resume` `m26_ws_heartbeat` |
| m27–m30 | Web 生产化/路由/cron/SQLite | `m27a_webprod` `m28_route` `m28_cron` `m28_time_sqlite` `m29_jsonpath_web` `m30_tls_pool` |
| m31–m40 | 沙箱/vhost/热更/限流/H2/优雅关闭/Result | `m31_vhost` `m32_hot_reload` `m33_access_log` `m35_h2` `m36_pool_grace` `m38_chunked` `m39_result` `m40_str_interp` |
| m45–m54 | registry/pxpkg/QUIC→H3 全栈（见上） | `m46_quic_*` … `m54_s5_pxi_quic_smoke` |
| m57–m60 | 边缘设备/std.edge | `m57_s2_mmap` `m60_serial_pty`（x86 PTY 可实跑） |
| m58–m61 | 首个真实 daemon/数学随机/2D+PNG | `m58_hwmond/`（子目录工程）`m59_math*` `m61_*` |
| m64–m67 | 工具链自举验证/多架构 | `m64_fmt*` `m65_lsp/mcp`（tools）`m67_aarch64*`（qemu） |

### 3.3 子目录工程（多文件 dogfood，可复用骨架）

- `examples/m58_hwmond/`：硬件健康守护 daemon（多文件 import + mmap IPC + HTTP 状态页 + run.sh 自愈）—— **生产工程骨架**。
- `examples/m66_yaml/ m66_pxml/ m66_lunar/`：stdlib 收编对拍验证工程（真实用例集）。
- `examples/m61_*/`：2D 游戏内圈（Mandelbrot / 贪吃蛇 / PNG 落盘）。
- `examples/m67_aarch64/`：跨架构三用例（hello/http/sqlite）。

## 4. 消费路径（把库/示例用起来）

1. **import std.***：`import std.collections` → 直接调函数（双模式皆可；M68 起 native 零 extern def）。
2. **pxpkg + 官方 registry**（M45 + M69-S3 打通 fetch→import 闭环）：`PX_REGISTRY=<仓库>/registry` → `pxpkg init` + `pxpkg add semver@^0.1.0` + `pxpkg install` → 安装到 `.px_modules/<name>/<name>.px`，**`import <name>`（裸名）编译/解释双模式可用** + `px.pkg.lock` 可复现（`--locked` 防篡改/registry 离线仍复现）。官方 9 库镜像随库入库（`registry/<name>/0.1.0/<name>.px`，见 `registry/README.md`）；端到端验证 `examples/m69_registry/verify.sh`（11 断言）。远程：registry 目录随 git clone 分发，或单包 `http(s) URL#sha256`。
3. **拷源码改**：stdlib 全纯语言，直接读源码/拷进项目改（每文件 <500 行，符合大模型友好约束）。
4. **文档链**：本文件（总览）→ `docs/PUXIAN_CHEATSHEET.md`（速查）→ `docs/spec.md`（规范）→ `docs/MINI_SUBSET.md`（子集边界）→ `docs/PXML.md`（PXML 规范）→ `docs/ROADMAP.md`（里程碑史）。
5. **代码内文档**：`tools/pxc doc stdlib/<name>.px` 从 `##` 注释生成该库 Markdown API 文档。

## 5. 生态索引与防漂移（机器可读）

- **生成器**：`tools/gen_ecosystem.px` —— 扫描 `stdlib/*.px` 顶层 `def` 与头部注释块，生成 **`docs/ecosystem_index.json`**（库名/行数/def 清单/头部描述）。
- **运行**：仓库根目录 `tools/pxc run tools/gen_ecosystem.px`（幂等：同一 stdlib 快照重跑字节一致）。
- **CI 校验**：重跑生成器写回索引后 `git diff --exit-code docs/ecosystem_index.json` —— stdlib 变更（新增/改名/删 def）未同步索引与本文档即 CI 失败，防漂移自举。
- **人工职责**：机器只管可验证事实（def 清单/行数）；库的「定位一句话 + 适用场景」由人维护（本文档 §1 表）。

## 6. 生态健康速查（写库/用库必读）

- **`{}` 字面量 = `null`**（普贤语言事实）——空 dict 须 `json_parse("{}")` 构造（stdlib 内 `yl_nd`/`px_nd` 即此惯用法）。M69-S1 修复 `collections.group_by` 历史 bug（旧用 `{}` 致返回 null + 不支持 `d[k]=v`）。
- **dict 操作 API**：写 `.set(k, v)`、查 `.has(k)`、读 `d[k]`；**不存在 `d[k] = v` 赋值**。
- 纯函数库保持无 IO/无状态，天然双模式一致；依赖 native（加密/IO）的库注意 pxi 侧能力（M68 后零 extern def 即可达，但如 `aes_*` 等语义以编译模式为生产主）。
- 新库/改库后：`pxc fmt` + `pxc lint` + `pxc doc` + 双模式跑 `pxc run` 与 `pxc build`，并重跑 `gen_ecosystem.px` 同步索引。
- 语言缺口（模块 var/数组跨行/let 不可变等）与详细写库规范见 `docs/ECOSYSTEM_GAPS.md`。
