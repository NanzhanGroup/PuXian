# M83_PLAN · HTTP 服务面增强 + 签名族 + 归档批全量（qg-issue 16/17/18/19/20 · W1b 挂起除外）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 1d5cfe8，M82 之后 · v0.1.0-m82 已发布 · native 288）
> 来源：qg-issue **16-puxian-http-body-64k-nul**（GAP-SRV-2/GAP-STR-1）+ **17-puxian-ed25519**（GAP-ED25519-1）+ **18-puxian-rsa-sign-pem**（GAP-RSA-1）+ **19-puxian-http-sse-stream**（GAP-SRV-SSE）+ **20-puxian-archive-misc-gaps**（归档批 7 项）
> 用户指令（2026-09-06）：**「除了挂起的 W1b，全部纳入 M83，分批次实现。ed25519 你实现就行，我不管咋实现。写 M83 的文档吧。」**
> 目标：① http_serve/http_serve_unix 服务端请求体**动态缓冲**（去 64KB 截断、超限 413）+ 语言层 len()/contains() **NUL 一致化**（Issue 16）
>       ② **ed25519 签名/验签 native**（api-server /v1/family + ws-ddns 硬缺口，Issue 17）
>       ③ **RSA PKCS1v15-SHA256 DigestInfo 封装 + PEM 私钥解析**（ws-pay + agentmail DKIM 硬契约，Issue 18）
>       ④ **http_serve 同端口流式 SSE**（api-server OpenAI 兼容 /stream 无绕过硬缺口，Issue 19）
>       ⑤ **归档批 7 项**（Issue 20）：L0 AES-ECB / gzip_compress/uncompress / os_spawn setpgid + L1 stdlib HTML5 / cookie jar / multipart / smtp_send
> 性质：**L0 runtime（native 288→299，S2 实际 +6：AES-ECB hex+bytes 双形态）+ L1 stdlib（4 个 .px 库）+ 引入 tweetnacl（public domain）**，不改语言语法 → 需重链全部 bootstrap ELF + 自举证明 + 全量回归（M72 同款大回归链）
> 分批次：**S1→S6 六批**，每批独立开发 + examples verify + commit；S6 统一一次自举重建收口 + tag v0.1.0-m83

## 〇、现状侦查（2026-09-06 源码级实录，非记忆推断）

**Issue 16a（GAP-SRV-2）—— 服务端 body 固定 64KB，客户端早已动态**
- 服务端 `http_conn_worker`（runtime.c **L9413**）第 4 步读 body：`char body_buf[65536] = {0}`（**L9506**）→ `content_length > 65535` 静默截断、**不报 413**；handler 拿到残缺 body（req.body 由 `px_str_len(body_buf, body_len)` L9544 构造，长度正确但数据被腰斩）。
- **http_serve 与 http_serve_unix（M82）共享 http_conn_worker**（L9708/L9758 各自 px_spawn）→ 一处改双入口通。
- 不对称佐证：客户端 http_request 读响应已动态（L7829-7872 xmalloc/xrealloc 跟 content_length/chunked）——服务端落后。
- 头缓冲 `char buf[65536]`（L9423）只装请求头（一次 recv），不属本 issue 截断源；但超大头需循环补读（执行期核实 keep-alive 下头读取完整性）。

**Issue 16b（GAP-STR-1）—— len/contains 按 C strlen，NUL 截断**
- `px_unicode_len(const char* s)`（**L1596**）无长度边界，遍历到 `\0` 止；`px_len` 对 PX_STR 走 L2333 `px_unicode_len(v.data)` → 含 NUL 的 str 长度算错。
- `contains`：obj 方法分派 L2445 → `call_with_self("contains")` → 全局 `bi_contains`（L5275 注册）内部 strstr 语义。
- PX_STR 自带 `str.len`（px_str_len L1259-1271 保真；`+` 拼接 px_add L1842 保真；write(fd) L3500 保真）——**修 len()/contains() 尊重 str.len 与既有二进制保真路径一致**。

**Issue 17（GAP-ED25519-1）—— mbedtls 预编译库不含 ed25519（issue 描述「mbedtls 已就绪」不成立，修正）**
- `runtime/mbedtls/`：include + lib（x86_64/aarch64/windows 三档 .a），全仓 grep `ed25519`（*.h/*.c）= **0 命中**：无 ed25519.h 头、无符号 → mbedtls 未编 ed25519 模块且仓库**无 mbedtls 源码可重编**。
- ⇒ **实现来源决策（用户授权自定）：引入 tweetnacl**（public domain 单文件，自带 sha512/ed25519，无外部依赖）——以 `runtime/runtime_ed25519.c` 并入（self-contained），不动 mbedtls、不动现有链接面。RFC 8032 标准算法，与 Go `crypto/ed25519` 天然互通。
- 触发面：api-server /v1/family/* 节点互信（PEM 公钥 ed25519.Verify）；ws-ddns 更新签名+验签双向。

**Issue 18（GAP-RSA-1）—— rsa_sign 裸 type1 + 不收 PEM**
- `runtime/runtime_rsa.c`（277 行）：`rsa_sign(data, n_hex, d_hex)`（L229）→ `mbedtls_rsa_pkcs1_sign(..., MBEDTLS_MD_NONE, ...)` 裸 type1 无 DigestInfo；`rsa_verify`（L271）同。只收裸模数/指数 hex，**不收 PEM**。
- mbedtls `pk_parse_key`/`pk_parse_public_key` API 齐备（PEM/DER 自动识别 PKCS8/PKCS1/RSA PUBLIC KEY/PUBLIC KEY）→ PEM 解析走现成 API 即可，**零自研 DER**。
- DigestInfo 封装：mbedtls pk_sign/pk_verify 带 `MBEDTLS_MD_SHA256` 自动算 DigestInfo（标准 PKCS1v15-SHA256）→ 与 Go `rsa.SignPKCS1v15(crypto.SHA256)` 互通。

**Issue 19（GAP-SRV-SSE）—— 服务端两路径不相通**
- `http_serve` handler 一次性应答（返回 dict 整包写回）；`sse_serve(port, on_connect, on_msg)`（L5410 注册，实现区 L9883+）独立端口专用，不接受 http 路由、不与 http_serve 共享端口。
- `sse_send(conn, data)`（**L10082**）已线程安全：全局注册表 `g_sse_clients`（L9810）+ PxConn + px_conn_write，`sse_frame_c` 自动包 SSE data 帧；`sse_close`（L10117）同表注销。**通道机制现成**——缺的是「http 连接能进该注册表」。
- ⇒ **形态决策：B（http_serve 同实例注册流式路由）**——新增 native `http_stream(path, on_connect)` 注册流式路由表；http_conn_worker 命中流式 path → 该连接走 sse 注册流程（分配 conn id 进 g_sse_clients）→ px_spawn 调 on_connect(conn, req) → 语言层 sse_send 逐块推（每块即 flush）→ on_connect 返回后注销关闭。**http_serve 与 http_serve_unix 同端口同享**（共享 worker + 全局路由表）。普通 JSON 路由零回归。

**Issue 20（归档批 7 项）—— 逐项落点**
| 子项 | 源码落点 | 实现路径 |
|---|---|---|
| 1.1 AES-ECB | `runtime/runtime_aes.c`（CBC/GCM 现成；mbedtls AES 原生支持 ECB） | 新增 `aes_encrypt_ecb/aes_decrypt_ecb`（PKCS7 对齐 Go ref） |
| 1.2 gzip | runtime.c 内部 `px_gzip_compress`（**L9182**）/`px_gzip_decompress`（**L9219**）**已实现**（http gzip 用），zlib 已链接 | **只差暴露**：注册全局 native `gzip_compress/gzip_uncompress`（包 px_gzip_*，str\|bytes 入 → bytes 出） |
| 1.3 os_spawn setpgid | `bi_os_spawn`（**L5546**）固定 (cmd, args) 2 参、fork 后直接 execvp **无 setpgid**；先例：os_capture（L5891）/os_popen 已 setpgid(0,0)（M66） | 第 3 参可选 `group: bool`（默认 false 保持现状）→ fork 后子进程 setpgid(0,0)；supervisor 移植前置 |
| 1.4 HTML5 | L1 stdlib | `std.html` 简化容错解析（纯 .px） |
| 1.5 cookie jar | L1 stdlib | `std.cookiejar`（纯 .px，~100 行） |
| 1.6 multipart 生成 | L1 stdlib | `std.multipart`（纯 .px，~80 行） |
| 1.7 smtp_send | L1 stdlib | `std.smtp` tcp 手写 SMTP 客户端（agentmail 全移植不做——见边界） |

**重建链与同步（M72/M68 同款）**
- bootstrap/pxc 编 selfhost/*.px → C → tools/pxc build（gcc 静态链接 runtime/*.c + 三方 .a）→ ELF。
- 改 runtime/*.c → 重链受影响 bootstrap ELF 并提交新 ELF（git 跟踪）；改 selfhost/*.px → bootstrap_prove.sh 自举证明 + golden 对拍。
- **native 计数单一事实源**：docs/PUXIAN_CHEATSHEET.md（现 288）+ tools/gen_native_table.sh → docs/native_index.json；**CI 重跑 diff 防漂移**——新增 native 必同步 CHEATSHEET + native_index + ECOSYSTEM。
- 预编译 .o 缓存（M71-S1，.rtcache/）：runtime 源哈希变化自动失效重编，无手动干预。

## 一、范围与边界

**做（S1–S6，全部 qg-issue 待办除 W1b）**：
- **S1 · Issue 16**（runtime.c http_conn_worker + px_unicode_len/bi_contains）：服务端 body 动态缓冲（xmalloc 跟 content_length，上限默认 256MB、PX_HTTP_BODY_MAX 可配、超限 413）+ len() 尊重 str.len（px_unicode_len_n 字节边界）+ contains memmem 语义（NUL 保真）。native 288（不改数）。**✅ 已交付**（examples/m83_s1 verify 6/6 + m82 回归 8/8）。
- **S2 · Issue 20-L0 三件（✅ 已交付 2026-09-06）**：`aes_encrypt_ecb/aes_decrypt_ecb` + `aes_encrypt_ecb_bytes/aes_decrypt_ecb_bytes`（runtime_aes.c，PKCS7；hex 版供文本互通对拍、bytes 版供微信媒体二进制——微信网关 AES-128-ECB 加解密媒体文件为二进制，hex 版 aes_decrypt_ecb 的 utf8 校验会拒非 UTF-8，故按 aes_gcm/aes_cbc 家族惯例同时提供 bytes 版）；`gzip_compress/gzip_uncompress`（runtime.c 包 px_gzip_*）；`os_spawn` 可选 group 参数（setpgid）。native +6 → 294。
- **S3 · Issue 17 ed25519（✅ 已交付 2026-09-06）**：引入 tweetnacl-20140427（public domain，runtime/tweetnacl.c 逐字节上游 + 唯一扩展 crypto_sign_seed_keypair）→ `ed25519_sign(priv, msg)` / `ed25519_verify(pub, msg, sig)`（runtime_ed25519.c wrapper；hex 或 PEM 入参，PEM 走内部 PKCS8/PKIX 小 DER 解析）。与 Go crypto/ed25519 双向互通全绿（同 seed 同 msg 签名逐字节一致）。native +2 → 296。
- **S4 · Issue 18 RSA 标准签名**（runtime_rsa.c）：`rsa_sign_pkcs1v15_sha256(pem_priv, msg)` / `rsa_verify_pkcs1v15_sha256(pem_pub, msg, sig_hex)`（mbedtls pk_parse_key/pk_sign 带 MD_SHA256 自动 DigestInfo；支持 PKCS8/PKCS1/RSA PUBLIC KEY/PUBLIC KEY）；旧 rsa_sign/rsa_verify 裸 type1 **原样保留**。native +2 → 298。
- **S5 · Issue 20-L1 stdlib 四库**：std.html / std.cookiejar / std.multipart / std.smtp（纯 .px 入库 stdlib/ 随发布包分发）。
- **S6 · Issue 19 SSE 同端口 + 收口**：`http_stream(path, on_connect)` native（runtime.c，+1 → **299**）+ http_conn_worker 流式分支（复用 g_sse_clients/sse_send/sse_close）；随后全量重建链 + 回归总闸 + 文档 + qg-issue 16/17/18/19/20 归档 + tag v0.1.0-m83。
  - （SSE 放 S6 因其是 http 面最大改动，与前 5 批各自独立提交后再上，风险隔离；S6 = SSE + 收口，若 SSE 超预期可先发 m83a 再 m83b，执行期临机。）

**不做**：
- ❌ **W1b Windows runtime 平台分流**（用户 2026-09-06 延后挂起）——本里程碑**不碰 _WIN32/平台层**；tools/pxc 的 windows 分支不受影响。
- ❌ mbedtls 重编/升级去拿 ed25519（仓库无源码；tweetnacl public domain 更干净）。
- ❌ 语言语法/parser 改动；断点/单步/watch 调试器（AI 优先哲学不变）。
- ❌ agentmail SMTP 服务端/完整移植（协议栈本质，Issue 20 明示不建议）；SMTP 只做轻量客户端原语 smtp_send。
- ❌ 字节码 VM、spawn 子进程化、.px 层 try/catch（Result/Option 哲学不变）。
- ❌ 改现有 native 既有签名/语义：aes_*（CBC/GCM/hex/bytes 版）、rsa_sign/rsa_verify 裸模式、os_spawn 前两参、http_serve 普通 handler、sse_serve 独立端口——全部原样保留（M83 全部为**新增或可选参数增强**）。
- ❌ 不开 PR（维持既定边界，直接 commit + push origin main）。

## 二、分步计划

### S1 · Issue 16：服务端 body 动态缓冲 + len/contains NUL 一致化（runtime.c，native 288 不变）✅ 已交付
> 2026-09-06 完成：examples/m83_s1_http_body_nul verify 6/6 全绿 + m82 回归 8/8。runtime.c 六处补丁：
> ① body 固定 `body_buf[65536]` → 动态 xmalloc（默认上限 256MB、PX_HTTP_BODY_MAX 可配、超限 413，不再静默截断）；
> ② handler 返回后 xfree（防 keep-alive 累积）+ 无 body 空串（避 px_str_len(NULL,0) UB）；
> ③ 新增 `px_unicode_len_n(s,n)`（px_unicode_len 改包装、strlen 边界，旧调用零回归）；
> ④ px_len PX_STR 按 as.str.len；⑤ contains() 改 `px_memmem` 字节语义；
> ⑥ L12417 解释器内建服务端同形代码（有 body_tmp 落盘保护）不在范围不动。
> 2026-09-06 完成：examples/m83_s1_http_body_nul verify 6/6 全绿 + m82 回归 8/8。runtime.c 六处补丁：
> ① body 固定 `body_buf[65536]` → 动态 xmalloc（默认上限 256MB、PX_HTTP_BODY_MAX 可配、超限 413，不再静默截断）；
> ② handler 返回后 xfree（防 keep-alive 累积）+ 无 body 空串（避 px_str_len(NULL,0) UB）；
> ③ 新增 `px_unicode_len_n(s,n)`（px_unicode_len 改包装、strlen 边界，旧调用零回归）；
> ④ px_len PX_STR 按 as.str.len；⑤ contains() 改 `px_memmem` 字节语义；
> ⑥ L12417 解释器内建服务端同形代码（有 body_tmp 落盘保护）不在范围不动。
- **16a**：http_conn_worker L9506 区 body 读改动态——`content_length` 校验（>0 且 <= 上限，上限常量默认 256MB，超限回 413 并关闭该连接）；`body_buf = xmalloc(content_length+1)` + 循环 conn_recv 收满；chunked 分支同样动态（对齐客户端 L7837-7844 模式）。构造 req.body 用 px_str_len（长度保真，已如此）。http_serve + http_serve_unix 双入口同享。
- **16b**：`px_unicode_len` 增加带长度边界版本 `px_unicode_len_n(s, n)`（遍历 n 字节，UTF-8 连续字节跳过）；`px_len` 对 PX_STR 改走 `px_unicode_len_n(v.data, v.len)`（纯文本零变化——len 语义不变、仅不再被 NUL 提前截断）；`bi_contains` 改 memmem(data, len, needle, nlen) 长度感知（两参均按 str.len）。其余 C 字符串族（B2 全量）**不在本里程碑**（Issue 16 已拆 B1/B2，B2 留档）。
- 验证（examples/m83_s1）：1MB 随机二进制 POST → handler 回显 body 长度 + sha256 断言与源一致（http_serve 与 http_serve_unix 双入口）；>256MB 上限回 413（用小上限 env/参数测）；含 NUL body 的 `len(body)` 与 `contains(body, bin_needle)` 断言；现有小 body 用例（ws-approve/m82 自检形态）零回归；纯文本 len() golden 零漂移。

### S2 · Issue 20-L0：AES-ECB + gzip 暴露 + os_spawn setpgid（runtime_aes.c / runtime.c，native +6 → 294）✅ 已交付
- `aes_encrypt_ecb(data, key) → bytes|null` / `aes_decrypt_ecb(data, key) → bytes|null`：mbedtls aes setkey + crypt（16B 块），**PKCS7 自动填充/去填充**（对齐 Go 侧微信网关 ref），key 长决定 AES-128/192/256；data/key 收 str|bytes（vbytes 助手，M72-S4 模式）；失败 null。
- `gzip_compress(data) → bytes` / `gzip_uncompress(data) → bytes|null`：包 px_gzip_compress/decompress（已 L9182/L9219 就绪），str|bytes 入、bytes 出，失败（数据非法）null。
- `os_spawn(cmd, args, group?)`：第 3 参可选 bool 默认 false；true 时 fork 后子进程 `setpgid(0,0)`（对齐 os_capture L5891 先例）→ 语言层 `os_kill(pid, sig, true)` 可组杀孙进程。os_spawn_capture/os_capture 已有 setpgid 不动。
- 验证（examples/m83_s2）：ECB 与 Go `aes.NewCipher` + PKCS7 ref 双向互通（固定 key/任意二进制含 NUL）；gzip_compress→uncompress roundtrip 逐字节一致 + 与系统 gzip -d 互通（file magic 1f 8b）；os_spawn(group=true) 子进程再 spawn 孙 → os_kill(pid, SIGTERM, true) 后孙进程消失（ps 断言）、group 缺省行为不变（回归）。

### S3 · Issue 17：ed25519 签名/验签（新增 runtime/tweetnacl.c + runtime_ed25519.c，引入 tweetnacl，native +2 → 296）✅ 已交付
- **引入 tweetnacl（public domain）**：仓库无 mbedtls 源码不可重编 ed25519 → 取 **DJB tweetnacl-20140427**（https://tweetnacl.cr.yp.to/20140427/，ed25519 标准参考实现）。**落地形态（比计划初稿更稳）**：`runtime/tweetnacl.c` + `runtime/tweetnacl.h` 保持上游**逐字节不变**（可随时 sha256 对账），wrapper 独立为 `runtime_ed25519.c`；仅 tweetnacl.c 内 crypto_sign_keypair 后新增 1 处 `crypto_sign_seed_keypair`（RFC8032 seed→sk64，Go PKCS8 导出即 32B seed，文件内注释标明）。sha256 归档见 runtime_ed25519.c 文件头（c 02e65bc…/h 43f29ad…，均为 20140427 官方值）。
- native：
  - `ed25519_sign(priv, msg) → sig_hex | null`：priv 收 **hex（32B seed=64 hex 或 64B sk=128 hex，seed 自动 crypto_sign_seed_keypair 展开）或 PEM（PKCS8 PRIVATE KEY，内部小 DER 解析取 seed 32B）**；msg str|bytes（二进制安全）；输出 64B sig → 128 hex。RFC8032 确定性签名。失败 null。
  - `ed25519_verify(pub, msg, sig) → bool`：pub 收 hex（32B=64 hex）或 PEM（PUBLIC KEY / SPKI，解析取 32B）；sig hex 128；验签通过 true。
- **PEM 解析**：ed25519 的 PKCS8（OID 1.3.101.112）与 SPKI 结构固定 → 内部小解析（base64 解码 + 通用 DER TLV 走查，der_tlv/der_skip 短/长格式均支持）；RSA PEM 走 mbedtls pk_parse（S4），两族不共享复杂 DER 层（各自最简）。
- 验证（examples/m83_s3_ed25519，**与 Go 互通对拍全绿**）：Go 生成 keypair → export PKCS8 PEM + SPKI PEM + seed/sk64 hex + msg（文本+二进制含 NUL）→ px `ed25519_verify(pem_pub/hex_pub, msg, go_sig)` true（错 pub/篡改 msg/坏 sig false）；px 用 **PKCS8 PEM / seed / sk64 三路签 == Go 签逐字节一致**（RFC8032 确定性 + 格式等价双重证明）；px 签文本+二进制 → Go `ed25519.Verify` 均 true；公钥当私钥签 null / 私钥当公钥验 false 边界。

### S4 · Issue 18：RSA PKCS1v15-SHA256 + PEM（runtime_rsa.c 扩展，native +2 → 298）
- 新增：
  - `rsa_sign_pkcs1v15_sha256(pem_priv, msg) → sig_hex | null`：pem_priv 收 PEM 文本（`-----BEGIN (RSA )?PRIVATE KEY-----`，mbedtls `pk_parse_key` 自动 PKCS8/PKCS1）→ `pk_sign(MBEDTLS_MD_SHA256, ...)`（自动 sha256 + DigestInfo + PKCS1v15）→ hex。
  - `rsa_verify_pkcs1v15_sha256(pem_pub, msg, sig_hex) → bool`：pem_pub 收 PEM 公钥（`RSA PUBLIC KEY`/`PUBLIC KEY`，pk_parse_public_key）→ pk_verify MD_SHA256。
  - 私钥口令：pk_parse_key 支持口令参数——本里程碑**不支持加密 PEM**（口令=空），文档注明（[SECURITY] 私钥应走环境变量明文 PEM，加密 PEM 场景后续再评估）。
- 旧 `rsa_sign/rsa_verify`（裸 type1、hex 模数/指数）**零改动保留**。
- 验证（examples/m83_s4，Go 互通）：Go `rsa.GenerateKey` → 导出 PKCS8 私钥 PEM + PKIX 公钥 PEM → px 用私钥 PEM 签（任意 msg 含中文/二进制）→ Go `rsa.VerifyPKCS1v15(sha256)` true；反向 px 签 → Go 验 true；PKCS1 私钥 PEM 同样过；裸 rsa_sign 旧行为 verify 回归；错 key/错 sig false；超长 msg 自动 sha256 无长度限制。

### S5 · Issue 20-L1：stdlib 四库（stdlib/*.px 纯 .px，随发布包分发）
- **std.html**（简化 HTML5 容错解析，chat tool_browse/search_parse 场景够用）：`html_parse(text) → dom`（坏标签/自动闭合/实体最小容错）、`html_text(node)`（剥标签取正文）、`html_query(dom, selector)`（tag[.class][#id] 简单选择，够 888 行工具面）；命名与接口风格对齐 std.yaml/std.pxml 先例（先看 stdlib 现有命名再定模块名，如 html.px）。
- **std.cookiejar**：`new() → jar` / `update(jar, resp_headers_dict)`（解析 Set-Cookie 存 domain/path/expires 简化）/ `header(jar, url) → cookie_str`（自动带 Cookie header）。
- **std.multipart**：`encode(fields: dict, files: dict) → body`（自动 boundary + Content-Type，供 http_request 上传）；飞书 uploadImage/uploadFile。
- **std.smtp**：`send(host, port, from, to, msg, opts?) → bool`：tcp 手写 SMTP 客户端（EHLO/MAIL FROM/RCPT TO/DATA/QUIT + 可选 AUTH LOGIN base64）；TLS：先查 runtime 是否有裸 TLS 连接包装可用（px_conn/tls 面，执行期评估）——首版支持明文内网/587 与（若可得）STARTTLS；agentmail 全移植不做。wgfixer 告警场景 sendmail 子进程可先行绕过。
- 验证（examples/m83_s5）：html 真实坏网页（缺闭合/错嵌套）text 提取不含标签；cookie 两次请求带 session 断言；multipart body 与 curl -F 互通（本地 http_serve 接收断言字段+文件）；smtp 发到本地 127.0.0.1:25 收件箱（或 dev 中继）断言到达。stdlib 双模式 run+build 验证（M69 惯例）。

### S6 · Issue 19 SSE 同端口 + 全量收口（runtime.c，native +1 → 299）
- **Issue 19（B 形态）**：新增 native `http_stream(path, on_connect)`（注册流式路由表 path→fn，可多次注册）；http_conn_worker（L9413）解析 path 后**先查流式路由表**：命中 → 该连接进 sse 注册表（分配 conn id，复用 g_sse_clients/sse_find，L9810）→ 写 SSE 响应头（Content-Type: text/event-stream，对齐 sse_serve 现状）→ `px_spawn(on_connect, {conn_id, req})` → on_connect 内语言层 `sse_send(conn_id, data)` 逐块推（自动 data: 帧 + 每块写即 flush）→ on_connect 返回 → `sse_close(conn_id)` 注销 + 关闭连接。普通路由照旧（流式路由优先，.px 普通 handler 兜底 404）。**http_serve + http_serve_unix 同享**。
  - sse_send/sse_close 现成（L10082/L10117），只缺注册入口 → 改动面 = 流式路由表 + worker 分支 + 1 个注册 native。
- 验证（examples/m83_s6）：同一 http_serve 注册 `/json`（普通）与 `/stream`（SSE 3 chunk + data: [DONE]）；curl 同端口分别打两路由；断言 stream 头 `text/event-stream`、chunk 逐条实时（时间戳差）、客户端中途断开服务端不崩；http_serve_unix 同端口流式同样过；sse_serve 独立端口旧行为回归。
- **收口（S6b）**：全量重建链 + 回归总闸（见三）+ 文档同步 + qg-issue 归档 + tag。

## 三、新增 native 清单（288 → 299，+11）

| # | native | 文件 | 签名 | 里程碑 |
|---|---|---|---|---|
| 1 | `aes_encrypt_ecb` | runtime_aes.c | (data, key) → bytes\|null | S2 |
| 2 | `aes_decrypt_ecb` | runtime_aes.c | (data, key) → bytes\|null | S2 |
| 3 | `gzip_compress` | runtime.c | (data) → bytes | S2 |
| 4 | `gzip_uncompress` | runtime.c | (data) → bytes\|null | S2 |
| 5 | `ed25519_sign` | runtime_ed25519.c（新，tweetnacl） | (priv, msg) → sig_hex\|null | S3 |
| 6 | `ed25519_verify` | runtime_ed25519.c | (pub, msg, sig) → bool | S3 |
| 7 | `rsa_sign_pkcs1v15_sha256` | runtime_rsa.c | (pem_priv, msg) → sig_hex\|null | S4 |
| 8 | `rsa_verify_pkcs1v15_sha256` | runtime_rsa.c | (pem_pub, msg, sig) → bool | S4 |
| 9 | `http_stream` | runtime.c | (path, on_connect) → bool | S6 |

参数增强（不计 native）：os_spawn 第 3 参 group（S2）；len()/contains() NUL 语义修正（S1）。
同步义务：CHEATSHEET native 清单 288→299 + tools/gen_native_table.sh 重跑 → docs/native_index.json + ECOSYSTEM/CHEATSHEET §crypto/http 条目；CI 防漂移。

## 四、回归总闸（判据）
1. CI 六 job 全绿：regression（自举证明 + diffcheck 全量）+ examples + toolchain（fmt/lint/生态+原生索引防漂移/M65-M82 verify）+ multiarch 三架构交叉。
2. S1：1MB 随机二进制双入口回显 sha256 一致；超上限 413；含 NUL len/contains 断言；纯文本 golden 零漂移（len 语义不变）。
3. S2：ECB ↔ Go ref 互通；gzip roundtrip + 系统 gzip 互通；os_spawn group 组杀孙进程、缺省不回归。
4. S3：ed25519 ↔ Go 双向互通（PEM + seed）；错 key/坏 sig false。
5. S4：RSA PKCS1v15-SHA256 ↔ Go SignPKCS1v15/VerifyPKCS1v15 双向互通（PKCS8/PKCS1 两格式）；裸 rsa_sign 不回归。
6. S5：stdlib 四库双模式 run+build verify（M69 惯例）。
7. S6：同端口 /json + /stream 共存、SSE 逐块实时、断连不崩；sse_serve 旧行为回归；native_index/CHEATSHEET 299 防漂移 diff 通过。
8. fmt/lint 0 错；bash -n 通过；CHANGELOG/ROADMAP/README/ECOSYSTEM/CHEATSHEET/native_index/qg-issue 收口 commit。

## 五、验收方法（对齐各 ISSUE 五节 + 用户指令）
1. **Issue 16**：ws-backup receiver 场景——>64KB 二进制 PUT 到 http_serve（及 http_serve_unix）body 完整（sha256 一致）；语言层 len(body)/contains 对含 NUL body 结果正确（不再截断）；小 body 既有用例零回归。
2. **Issue 17**：api-server /v1/family 验签路径——Go 侧 ed25519 PEM 公钥/私钥与 px 互签互验全通过；ws-ddns 双向签名可用。
3. **Issue 18**：ws-pay 商户签名——Go 生成的商户 PKCS8 私钥 PEM 直接喂 px 签出标准 PKCS1v15-SHA256，微信/支付宝服务端验签通过（Go ref 对拍验证等价）；DKIM rsa-sha256 原语可用。
4. **Issue 19**：api-server OpenAI 兼容——同一 443/端口 `/v1/chat/completions`（stream=true）返回 `text/event-stream` 逐块 + [DONE]，普通 JSON 路由同端口共存；OpenAI SDK 直连不破坏。
5. **Issue 20**：AES-ECB 微信媒体链路 ↔ Go；gzip ↔ 系统 tar；supervisor stop 组杀；html/cookie/multipart/smtp 各自最小断言（§S5）。
6. **全量**：native 299、CHEATSHEET 同步、自举证明、diffcheck、CI 全绿、qg-issue 16-20 归档 done/。

## 六、风险与取舍
- **tweetnacl 引入**（S3）：第三方 public domain 源码进 runtime——需固定来源版本 + 文件头注明 + sha256 校验记录；仅取 ed25519/sha512 必要函数，避免整包。替代方案（重编 mbedtls 开 ed25519）仓库无源码，弃。
- **http_stream 与普通 handler 路由语义**（S6）：流式路由 C 层优先，.px 普通 handler 需兜底——文档化"path 命中 http_stream 即不再进普通 handler"；冲突由业务避免。连接生命周期：on_connect 返回即关（SSE 无 keep-alive 复用），与 sse_serve 一致。
- **16b 改 len() 语义**（S1）：纯文本字符串 NUL 不可能出现（文本语义 str 无 NUL）→ 实际零影响；含 NUL 二进制走 str 的用例才是收益面。B2（全 C 字符串族）明确不做、留档，避免回归面失控。
- **os_spawn group 参数**（S2）：新增可选第 3 参不破坏既有 2 参调用（nargs 判断）；fork 后 setpgid 竞态（子进程先 exec 后 setpgid 失败窗口）——同 os_capture 先例在 exec 前 setpgid，无新风险。
- **AES-ECB padding 契约**（S2）：微信 Go 侧 padding 细节以 ref-go 实测对拍定稿（PKCS7 假设先行验证），若 Go 侧无 padding（长度恒 16 倍数）则文档区分——执行期以互通测试为准。
- **S6 是 http 面最大改动**：放最后隔离风险；若 SSE 实现超预期，可切 tag m83a（S1-S5）先发，SSE 单独 m83b——执行期临机，不阻塞前 5 批交付。
- **回归链成本**：S1-S6 全部 runtime 改动集中在 S6 一次自举重建 + golden 重生成 + diffcheck 全量（M72 同款，摊薄多次重建）；每批开发期用 tools/pxc 现编 examples verify（不依赖 bootstrap 重链），commit 推进。

## 七、决策记录（2026-09-06）
1. **范围**：用户拍板「除挂起的 W1b 全部纳入 M83，分批次实现」→ qg-issue 16/17/18/19/20 全量（W1b Windows 平台分流挂起不动）。
2. **ed25519 实现来源**：用户「你实现就行，我不管咋实现」→ 东月定 **tweetnacl（public domain 单文件）**（mbedtls 预编译库无 ed25519 且仓库无源码，已源码级核实）。
3. **Issue 19 形态**：东月定 **B（http_stream 流式路由注册，复用 sse_send/sse_close 通道）**——贴合现有回调/px_spawn 模型，改动面最小；A（语言层通道）无 chan 类型不可行。
4. **Issue 18 形态**：东月定 **新增专用 rsa_sign_pkcs1v15_sha256/rsa_verify_pkcs1v15_sha256**（行为确定、旧裸模式零回归），PEM 解析走 mbedtls pk_parse 现成 API。
5. **分 6 批**：S1(16) → S2(20-L0) → S3(17) → S4(18) → S5(20-L1 stdlib) → S6(19+收口+tag v0.1.0-m83)；每批独立 commit/verify，S6 统一重建。
