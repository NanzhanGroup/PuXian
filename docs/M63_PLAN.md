# M63 · 语言面剩余欠账 L8–L11（pxi API 深化 / 浮点全精度 / pxc --version）

里程碑：M63 · 分支：main · 目标：清完语言面欠账表 L8–L11 全部四项

## 背景与范围
用户欠账表 L8–L11 清账；其中 L8/L9 用户点名深化方向：
- L8 含「pxi 相对 import 深化」（实测文件级 import 已通，真缺口 = C builtin 注册面白名单差集）
- L9 含「%g 全精度打印」（实测双模式一致 6 位截断，宿主 C fmt_num 是共同根）
- L11 用户已拍板走「甲：bootstrap/pxc --version 自举重建」

## 侦查结论（事实复核，非记忆推断）
| 项 | 证据 |
|---|---|
| float→str 唯一根 = runtime.c `fmt_num`（:1594）| 编译模式 str/print 走 fmt_num（px_to_string:1771）；pxi 宿主 str() 同一 fmt_num；ival.px `i_fmt_float` 的 `str(f)` 最终仍落宿主 fmt_num → **双模式同根，单点修复即可** |
| L10 编译期字面量截断根 = codegen `cg_fmt_float` 用 `str(v)`（px 宿主 str=fmt_num %g 6 位）| codegen.px:295 `var s = str(v)`；pxc 重建后内嵌 roundtrip fmt_num → str() 自动全精度 → **codegen.px 源码零改动** |
| diffcheck 已为 L10 留豁免（v01_value）| diffcheck.sh codegen 分支「%g 截断 ≤4 行」+ value 分支「float** 1.41421 vs 1.4142135623730951」→ L10 修复后**豁免可移除、golden 更新** |
| pxi 白名单机制 | interp.px `i_register_builtins` names 表（env_define 让解释执行用户代码可见）；ibuiltin.px `i_call_builtin` 逐名转发；宿主 px_register_builtins 注册 277 个 C builtin（runtime.c:5103）供直调 |
| L8 差集（点名例） | http_post/http_request/http_get_stream/s3_* 等已在 C runtime（runtime.c:5239-5243）但不在解释器 names 白名单 → pxi 真实 HTTP/S3 应用不可用 |
| L11 | bootstrap/pxc = compiler.px 编译产物（main 在 selfhost/compiler.px:48）；当前 main 无条件 `read_file(a[last])` → `--version` 被当文件；照 interp.px main（M62 已加 pxi --version）模式补 |
| 自举/重建链 | `tools/pxc build selfhost/compiler.px` → selfhost/build/compiler（=bootstrap/pxc 产物）；copy_runtime 自动带**新** runtime/runtime.c → 重建即含 runtime 改动 |

## 修复方案
| # | 欠账 | 动作 | 源码改动 |
|---|---|---|---|
| L9 | float→str 最短 roundtrip | runtime.c `fmt_num` float 分支：`%g` 6 位 → 逐精度 `%.{1..17}g` + `strtod` 回读相等取最短（IEEE754 17 位必达、升序首个即最短）；M62-L1 `.0` 补丁语义保留 | runtime.c 一处 |
| L10 | 编译期浮点字面量截断 | 无源码改动：pxc 重建内嵌新 fmt_num → `cg_fmt_float` 的 `str(v)` 自动全精度；移除 diffcheck v01_value 两处 %g 豁免 + golden 更新 | diffcheck.sh + golden |
| L11 | bootstrap/pxc --version | compiler.px main 参数前置分支（len(a)==2 && a[1] in --version/-v → 输出版本退出 0）+ PXC_VER/PXC_MS 常量（照 interp.px PXI_VER 模式，值对齐 tools/pxc：0.1.0 / M-B9a）→ 重建 pxc | compiler.px |
| L8 | pxi 白名单差集 | 范围 = 网络真实应用缺口：http_post / http_request / http_get_stream / s3_get / s3_put / s3_list / s3_delete（其余 C builtin 属 Mini 排除面：quic/h3/udp/底层 serve/框架回调 __* / 配置变量等留档不扩，防爆炸面失控）。interp.px names 补 7 名；ibuiltin.px 加转发（Result 透传 helper：C 返回 Err 值透传不 Ok 包裹，保失败路径双模式一致）；重建 pxi | interp.px + ibuiltin.px |

## 子步
- S0 立项侦查 + PLAN（本文件）—— done
- S1 (L9): runtime.c fmt_num roundtrip → 重建 pxc/pxi → 浮点探针双模式一致 + roundtrip 断言
- S2 (L10): 高精度常量/运算编译用例（π/√2/0.1+0.2）值断言 + diffcheck 豁免移除准备
- S3 (L11): compiler.px main --version → 重建 pxc → bootstrap/pxc --version 与 tools/pxc 对齐
- S4 (L8): interp.px + ibuiltin.px 补齐 7 名 → 重建 pxi → 本地 mock HTTP/S3 双模式对拍
- S5 收口: golden 全量重生成（compiler.c + cases）+ diffcheck --all/--errors 全绿 + 自举证明 + capability 双模式 + 历史回归（m59_math/m61_gfx/m62_langfix）+ 文档回填（spec/MINI_SUBSET/CHANGELOG/欠账表勾除 L8–L11）+ 推送

## 已知边界（如实记录）
- 白名单仅补 HTTP/S3 7 名；quic/h3/udp/tcp-serve/ws-serve/session/bus/cron/sse 等编译模式高层 API 仍不在 pxi Mini 解释器（非 Mini 排除，文档留档）
- 失败路径一致性仅对新补 7 名用 Result 透传 helper 保证；既有白名单函数维持原 Ok() 包装（不扩大爆炸面）
- 双模式「逐字节一致」以成功路径 + 本次新补函数的失败路径为准
