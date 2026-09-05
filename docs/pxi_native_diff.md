# pxi_native_diff · pxi 解释器与编译模式 native 可达性差异表

> M68-A1 产出 · 2026-09-05 · 侦查基线 main @ 126f1f0（M67 闭环）
> 口径：**编译模式默认可达** = runtime `px_set_global(name, px_native(...))` 启动注册的全局 native（codegen 对裸名调用生成 `px_get_global(name)` 查表）；**pxi 默认可达** = `interp.px i_register_builtins()` names 数组（129 名，env_define 注册进 g_globals）。

## 结论摘要

| 集合 | 数量 | 说明 |
|---|---|---|
| 编译产物全局 native（px_set_global px_native） | 281 | runtime.c px_register_builtins + quic/h3/qpack/ws 注册函数（完整编译） |
| pxi 默认可达（i_register_builtins names） | 129 | interp.px 硬编码 names 数组 |
| **差集（编译可达、pxi 不可达 → R1001）** | **155** | 本表逐名归因 |
| ├ quic/h3 族（条件编译 PX_NO_QUIC） | 64 | B 类：完整宿主经全局表兜底自动可达；裁剪宿主与编译模式一致不可达 |
| └ 其余真 native | 91 | A 类：修复后 pxi 零 extern def 裸脚本可达 |

## 根因链（复述）

1. 编译模式：用户裸名调用 → codegen `px_call(px_get_global("name"))` → runtime 全局表命中（px_register_builtins 已注册）→ 无声明直接可调。
2. pxi 解释模式：用户裸名调用 → `i_eval_call` Var 分支 → ① i_ffi（用户 extern def 声明）→ FFI 桥；② chan/mutex/rwlock 特殊；③ 普通调用 `i_eval_expr(callee)` → `env_get`（用户作用域 + i_register_builtins 的 129 names）→ 未命中 → **R1001 未定义变量**。
3. → 差集 155 个 runtime native 在 pxi 默认不可达；`capability.px` 253 PASS 系对 sqlite/quic/h3 等**显式 extern def**，掩盖了裸脚本不对称。

## A 类 · 真 native 应可达（91 个）

> 修复：C 侧 `bi_ffi_call` 双表查找（ffi 表 → 全局 PX_NATIVE 表），未命中返回可辨 Err；pxi `i_eval_call` Var 分支 env 未命中 → 自动回退 `ffi_call(cname, args)`。下表全部修复后 pxi 裸脚本可达，与编译产物一致。

| native | 说明/所属 |
|---|---|
| `aes_decrypt` | A 类 |
| `aes_encrypt` | A 类 |
| `aes_gcm_decrypt` | A 类 |
| `aes_gcm_encrypt` | A 类 |
| `args` | A 类 |
| `basic_auth` | A 类 |
| `bit_count` | A 类 |
| `bit_length` | A 类 |
| `bus_publish` | A 类 |
| `bus_subscribe` | A 类 |
| `bus_unsubscribe` | A 类 |
| `clear_timer` | A 类 |
| `cron` | A 类 |
| `ctx_clear` | A 类 |
| `ctx_get` | A 类 |
| `ctx_set` | A 类 |
| `event_bus` | A 类 |
| `fsync_file` | A 类 |
| `gc` | A 类 |
| `http_get_stream` | A 类 |
| `http_serve` | A 类 |
| `input` | A 类 |
| `middleware` | A 类 |
| `now_ms` | A 类 |
| `os_pid` | A 类 |
| `panic` | A 类 |
| `px_exec` | A 类 |
| `px_serve` | A 类 |
| `rate_limit` | A 类 |
| `read_at` | A 类 |
| `route` | A 类 |
| `rsa_decrypt` | A 类 |
| `rsa_encrypt` | A 类 |
| `rsa_gen_key` | A 类 |
| `rsa_sign` | A 类 |
| `rsa_verify` | A 类 |
| `sandbox_enter` | A 类 |
| `session_del` | A 类 |
| `session_destroy` | A 类 |
| `session_get` | A 类 |
| `session_id` | A 类 |
| `session_open` | A 类 |
| `session_set` | A 类 |
| `set_interval` | A 类 |
| `set_timeout` | A 类 |
| `signal` | A 类 |
| `sqlite_close` | A 类 |
| `sqlite_escape` | A 类 |
| `sqlite_exec` | A 类 |
| `sqlite_last_insert_rowid` | A 类 |
| `sqlite_open` | A 类 |
| `sqlite_query` | A 类 |
| `sse_close` | A 类 |
| `sse_connect` | A 类 |
| `sse_read` | A 类 |
| `sse_send` | A 类 |
| `sse_serve` | A 类 |
| `tcp_accept` | A 类 |
| `tcp_close` | A 类 |
| `tcp_connect` | A 类 |
| `tcp_listen` | A 类 |
| `tcp_recv` | A 类 |
| `tcp_send` | A 类 |
| `time_format` | A 类 |
| `time_parse` | A 类 |
| `tls_server` | A 类 |
| `truncate_file` | A 类 |
| `tz_offset` | A 类 |
| `udp_close` | A 类 |
| `udp_open` | A 类 |
| `udp_recv` | A 类 |
| `udp_send` | A 类 |
| `udp_serve` | A 类 |
| `vhost` | A 类 |
| `write_at` | A 类 |
| `ws_broadcast` | A 类 |
| `ws_close` | A 类 |
| `ws_connect` | A 类 |
| `ws_connect_auto` | A 类 |
| `ws_heartbeat` | A 类 |
| `ws_ping` | A 类 |
| `ws_recv` | A 类 |
| `ws_send` | A 类 |
| `ws_serve` | A 类 |
| `xml_build` | A 类 |
| `xml_escape` | A 类 |
| `xml_parse` | A 类 |
| `xml_unescape` | A 类 |
| `xxhash` | A 类 |
| `zip_pack` | A 类 |
| `zip_unpack` | A 类 |

## B 类 · quic/h3 族（64 个，条件编译）

> `#ifndef PX_NO_QUIC` 注册（runtime_quic.c/runtime_h3.c/runtime_h3_qpack_dyn.c）。完整宿主（pxi 默认）经全局表兜底自动可达；`--no-quic` 裁剪宿主与裁剪编译产物一致不可达（无此全局 → ffi_call 未命中 → R1001，行为对齐）。另注：quic/h3 同时已 `px_ffi_register`（extern def 路径本就可达）。

| native | 说明 |
|---|---|
| `h3_client_connect` | B 类（条件编译）|
| `h3_client_open_stream` | B 类（条件编译）|
| `h3_client_read_response` | B 类（条件编译）|
| `h3_client_read_response_stream` | B 类（条件编译）|
| `h3_client_send_request` | B 类（条件编译）|
| `h3_client_send_request_stream` | B 类（条件编译）|
| `h3_conn_close` | B 类（条件编译）|
| `h3_conn_peer` | B 类（条件编译）|
| `h3_conn_setup` | B 类（条件编译）|
| `h3_conn_stats` | B 类（条件编译）|
| `h3_frame` | B 类（条件编译）|
| `h3_huff` | B 类（条件编译）|
| `h3_qdec` | B 类（条件编译）|
| `h3_qenc` | B 类（条件编译）|
| `h3_qs_ack_inc` | B 类（条件编译）|
| `h3_qs_ack_sec` | B 类（条件编译）|
| `h3_qs_close` | B 类（条件编译）|
| `h3_qs_de_len` | B 类（条件编译）|
| `h3_qs_dec` | B 类（条件编译）|
| `h3_qs_dec_ingest` | B 类（条件编译）|
| `h3_qs_dec_ric` | B 类（条件编译）|
| `h3_qs_en_len` | B 类（条件编译）|
| `h3_qs_enc` | B 类（条件编译）|
| `h3_qs_enc_ric` | B 类（条件编译）|
| `h3_qs_ins` | B 类（条件编译）|
| `h3_qs_krc` | B 类（条件编译）|
| `h3_qs_open` | B 类（条件编译）|
| `h3_qs_take_enc` | B 类（条件编译）|
| `h3_serve_poll_stream` | B 类（条件编译）|
| `h3_serve_read_request` | B 类（条件编译）|
| `h3_serve_read_request_stream` | B 类（条件编译）|
| `h3_serve_send_response` | B 类（条件编译）|
| `h3_serve_send_response_stream` | B 类（条件编译）|
| `h3_server_listen` | B 类（条件编译）|
| `h3_server_listen_stateless` | B 类（条件编译）|
| `h3_settings_dec` | B 类（条件编译）|
| `h3_settings_enc` | B 类（条件编译）|
| `h3_unhuff` | B 类（条件编译）|
| `quic_0rtt_rejected` | B 类（条件编译）|
| `quic_0rtt_save` | B 类（条件编译）|
| `quic_accept` | B 类（条件编译）|
| `quic_close` | B 类（条件编译）|
| `quic_close_listener` | B 类（条件编译）|
| `quic_conn_handshake_done` | B 类（条件编译）|
| `quic_conn_local` | B 类（条件编译）|
| `quic_conn_path` | B 类（条件编译）|
| `quic_conn_resumed` | B 类（条件编译）|
| `quic_connect` | B 类（条件编译）|
| `quic_connect_0rtt` | B 类（条件编译）|
| `quic_connect_resume` | B 类（条件编译）|
| `quic_extend_max_streams` | B 类（条件编译）|
| `quic_h3_listen` | B 类（条件编译）|
| `quic_listen` | B 类（条件编译）|
| `quic_migrate` | B 类（条件编译）|
| `quic_open_stream` | B 类（条件编译）|
| `quic_open_uni_stream` | B 类（条件编译）|
| `quic_poll` | B 类（条件编译）|
| `quic_recv` | B 类（条件编译）|
| `quic_recv_stream` | B 类（条件编译）|
| `quic_send` | B 类（条件编译）|
| `quic_send_stream` | B 类（条件编译）|
| `quic_session_save` | B 类（条件编译）|
| `quic_set_max_client_streams` | B 类（条件编译）|
| `quic_streams_left` | B 类（条件编译）|

## 归因排除项（不在差集，记录依据）

| 项 | 排除依据 |
|---|---|
| `__px_docroot`/`__px_port`/`__px_timeout`/`__http_handler`/`__sse_handler`/`__ws_handler` | 伪全局：运行期动态 px_set_global（非 px_native 启动注册），runtime 内部位，非用户 native API |
| `pi`/`e` | 全局常量（px_float），非 px_native；pxi 已单独 env_define 直读宿主全精度 |
| `args`/`input`/`panic`/`gc` | A 类应可达；语义注意：pxi 场景宿主进程即 pxi 自身 → args() 返回 pxi 的 argv（间接层固有差异，与编译产物 = 用户程序 argv 不同）；input/panic/gc 语义与编译一致 |
| chan/send/recv/spawn/select/mutex/rwlock | 语言构造/关键字，非 px_set_global native；pxi 已显式 Err「Mini 子集排除」 |
| dict/flatten/unique（pxi 有、runtime native 无） | pxi 解释器内部实现（语言层），无需 native 注册 |

## 修复后状态（A3 实现后回填 — ✅ 2026-09-05 已全绿）

- [x] pxi 零 extern def 裸脚本跑通 A 类代表性 native（sqlite_open/json/aes/os_pid/now_ms/tz_offset/xxhash 等），结果与编译产物一致 —— **t_native.px 19/19，pxi == 编译产物（commit 3b47dbe）**
- [x] quic/h3 完整宿主 pxi 可达（经全局表兜底）；`--no-quic` 裁剪一致不可达 —— 机制天然覆盖（px_global_native 查全局 PX_NATIVE），capability extern def quic/h3 段 253 PASS 不回归
- [x] typo 语义：真拼错名（如 sqliite_open）pxi 仍 R1001 且可辨，不误调 —— t_typo.px 实测 R1001 未定义变量 'sqliite_open'
- [x] capability.px 双模式 253 PASS 不回归 —— pxi 253 = bin 253 逐字节一致
- [x] diffcheck --all 全量对拍通过；m66 stdlib verify yaml35/lunar36/pxml68/proc14 双模式 PASS

> 修复机制（commit 3b47dbe）：C 侧 `bi_ffi_call` 双表（① ffi 注册表 → ② `px_global_native()`
> 全局 PX_NATIVE），未命中返回可辨 Err；pxi `i_eval_call` Var 分支 env 未命中自动回退
> `ffi_call(cname, args)`。全局表即单源（px_set_global 注册者 = 编译产物裸名调用同源），
> 无需逐行注册宏 / ffi 表扩容 / 双源漂移。
