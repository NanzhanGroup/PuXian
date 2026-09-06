# M84_PLAN · bs-safeip 三缺口收官（qg-issue 21/22/23 全量 · qg-issue 待办清零）

> 创建：2026-09-06 · 处理人：东月
> 官方基准：github.com/NanzhanGroup/PuXian（HEAD main @ 43cdd3c = M83-S6 · v0.1.0-m83 已发布 · native **299** · stdlib 13 库）
> 来源：qg-issue **21-puxian-hmac-sha256**（GAP-HMAC-1：HMAC-SHA256 无 native + sha256 仅 ASCII）+ **22-puxian-dns-resolve**（GAP-DNS-1：域名解析无 native）+ **23-puxian-http-content-type-bug**（runtime 缺陷：http_request 双 Content-Type）
> 用户指令（2026-09-06）：**「查看本机 /data/qg-issue，制定 M84 规划」**
> 目标：① 修复 **http_request 双 Content-Type** runtime 缺陷（Issue 23，行为修正）
>       ② **HMAC-SHA256 native + sha256 bytes 通道**（Issue 21——腾讯云 TC3 4 级 HMAC 链 / SigV4 / webhook / JWT HS256 全系解锁）
>       ③ **DNS 域名解析 native**（Issue 22——getaddrinfo，守护域名→IP 不再 getent 代偿）
> 性质：**L0 runtime**（native 299 → **301**，全部为纯新增/行为修正，不改语言语法）→ 需重链 bootstrap ELF + 自举证明 + 全量回归（M72/M83 同款大回归链）
> 分批次：**S1→S4 四批**，每批独立开发 + examples verify + commit；S4 统一自举重建收口 + tag v0.1.0-m84
> 里程碑意义：M84 收官后 **qg-issue 1–23 全部清零**（仅剩 14-W1b ⏸️ 用户延后挂起 + 13-R2 业务自持），bs-safeip（腾讯云 TC3 + Lighthouse 守护）三缺口齐解、可原生移植，openssl 子进程 / getent 代偿退役。

## 〇、现状侦查（2026-09-06 源码级实录，非记忆推断）

**Issue 23（runtime 缺陷）—— http_request 双 Content-Type，根因已定位**
- `bi_http_request`（runtime.c **L8012**，body 发送前头构造）：`if (!strcasestr(extra_headers, "Content-Length"))` → 补 `Content-Type: application/x-www-form-urlencoded` + Content-Length。
- `bi_http_unix`（**L8107**）同构缺陷（unix socket 客户端）。
- **根因**：默认头补充的判定挂在 **Content-Length** 上而非 **Content-Type** 上 → 调用方显式带 Content-Type（无 Content-Length，绝大多数）时，默认 urlencoded CT 照加 → **双 Content-Type**，腾讯云等严服务端直接拒（UnsupportedProtocol）。
- 全仓另有 `Content-Type: application/x-www-form-urlencoded` 构造点 L8876（执行期逐一核对，确认所属函数后统一修复口径）。
- 现状行为矩阵（issue 文档实测）：带 CT 无 CL → 双头 ❌；带 CT + CL → 单头 ✅（碰巧可用，M83-S5 multipart 即靠此绕过）；无头 → 默认表单 ✅。

**Issue 21（GAP-HMAC-1）—— 无 hmac native，sha256 仅 ASCII**
- 全仓 `px_set_global(px_native...)` 无任何 hmac native；`bi_sha256`（runtime.c **L3925**）实现：`val_cstr(args[0])` + **`strlen(data)`** → ① PX_STR 含 NUL 截断 ② 无 PX_BYTES 通道（val_cstr 对 bytes 不保真）。
- mbedtls 已内置 `mbedtls_md_hmac`（md_info HMAC-SHA256）/ `mbedtls_sha256_ret`（ret 版接受显式长度）→ **纯暴露成本**。
- 触发：bs-safeip `tc3.px` 腾讯云 API 3.0 需 4 级 HMAC 链（kDate→kService→kSigning→signature）；同族：ws-ddns、AWS/阿里 SigV4、webhook 验签、JWT HS256。

**Issue 22（GAP-DNS-1）—— 无域名解析 native**
- 全仓 px native 无 getaddrinfo/A 记录查询；守护类（bs-safeip util.px `resolve_ips(domain)` 每轮解析）只能 `getent ahostsv4` 外部命令文本切分（脆弱、依赖 glibc 命令与输出格式）。
- 底层 glibc `getaddrinfo` 现成 → 纯暴露成本。

## 一、范围与边界

**做（S1–S4，qg-issue 剩余全部待办）**：
- **S1 · Issue 23**（runtime.c http_request 头构造）：CT 与 CL 判定**解耦**——调用方**已带 Content-Type 则不再补默认 CT**（与是否带 CL 无关）；默认 `application/x-www-form-urlencoded` CT 仅在**完全未指定 CT** 时补；Content-Length 仅在缺失时补。bi_http_request + bi_http_unix（及执行期核实到的 L8876 等全部同构点）统一修复。**行为修正，native 数不变（299）**。回归重点：无头请求仍得默认表单头（现状不变）、自定义 CT 请求只发单头、M83-S5 multipart 上传的 Content-Length hack 可卸（verify 断言卸除后仍过）。
- **S2 · Issue 21**（runtime.c）：新增 `hmac_sha256(key, msg) -> hex`（key 收 str\|hex 表达二进制、msg 收 str\|bytes **含 NUL 保真**，mbedtls_md_hmac）+ `sha256` **原地增强**支持 bytes/含 NUL 载荷（str.len / bytes.len 替代 strlen，推荐形态，见决策点②）。native **+1 → 300**。
- **S3 · Issue 22**（runtime.c）：新增 `dns_lookup(domain) -> list[str]`（getaddrinfo，A+AAAA 全部返回）；解析失败/NXDOMAIN/无记录返回可判定 Err（区分空结果与错误）。native **+1 → 301**。
- **S4 · 收口**：全量重建链 + 回归总闸 + 文档同步 + qg-issue 21/22/23 归档 `done/` + 00-README 更新 + tag **v0.1.0-m84** + CI 绿。
- **端到端解锁示范（S4 verify 内含）**：腾讯官方 TC3 文档固定向量复算（4 级 HMAC 链逐级对拍 Go `hmac.New(sha256)` 自算）——**无云密钥也能证明 tc3.px 的 openssl 子进程可整体替换**；dns_lookup 对公开域名返回与 `getent ahostsv4` 一致。

**不做（边界）**：
- ⏸️ **W1b（Issue 14 runtime `_WIN32` 平台分流）维持用户延后挂起**——M84 不碰 Windows/_WIN32（默认；若用户下令解冻可临时追加，见决策点①）。
- 不改 compiler/parser/语言语义/现有 native 签名（sha256 例外为原地增强输入域，输出语义不变）；不开 PR（直推 main）。
- Issue 22 的 TTL 返回、超时配置等可选增强不做（保持最小暴露，守护轮询节流后续按需再加）。
- 真实腾讯云 API 请求验证（issue 21 §4 提及）不做——需云密钥，verify 用官方固定向量对拍即可证伪 openssl 代偿；bs-safeip 真机验证由业务方自持。

## 二、分批次执行计划

| 批 | 内容 | native | verify（examples/） | 预计 commit |
|---|---|---|---|---|
| **S1** | Issue 23：http_request/http_unix 头构造 CT/CL 解耦修复（单头） | 299（不变） | m84_s1：本地 http_serve 回环抓头断言单 CT + 无头默认表单回归 + 自定义 CT（json/multipart）无 CL 单头 + M83-S5 multipart 上传卸 Content-Length hack 仍过 | M84-S1 |
| **S2** | Issue 21：`hmac_sha256` + sha256 bytes/含 NUL 增强 | **300**（+1） | m84_s2：RFC 4231 固定向量 + Go hmac 互通 + TC3 4 级链对拍 + sha256(bytes 含 NUL)==Go sha256.Sum256 | M84-S2 |
| **S3** | Issue 22：`dns_lookup`（getaddrinfo） | **301**（+1） | m84_s3：localhost/公开域名 vs getent ahostsv4 一致 + NXDOMAIN/无记录 Err 可判定 + IPv6 存在域返回 AAAA | M84-S3 |
| **S4** | 收口：重建链 + 自举证明 + 回归总闸 + 文档 + 归档 + tag | 301 | 全量：m82 + m83_s1–s6 + m84_s1–s3 每批独立 verify + TC3 固定向量端到端示范 | M84-S4 |

**重建链与同步（M72/M83 同款）**：
- 改 runtime/*.c → 重链受影响 bootstrap ELF 并提交新 ELF（git 跟踪）；selfhost/*.px 若动 → bootstrap_prove.sh 自举证明 + golden 对拍。
- **native 计数单一事实源**：tools/gen_native_table.sh → docs/native_index.json（count 299 现况，已核实）+ docs/PUXIAN_CHEATSHEET.md + docs/ECOSYSTEM.md；CI 重跑 diff 防漂移——新增 native 必同步三处。
- 预编译 .o 缓存（M71-S1，.rtcache/）：runtime 源哈希变化自动失效重编，无手动干预。
- 每批独立 commit + push（message 前缀 **M84-Sx**），CI 每批验证；S4 收口最后 push + tag v0.1.0-m84。

## 三、验收清单（S4 收口总闸）

- [ ] m84_s1–s3 每批 verify 独立 PASS（S1 含单 CT 抓包断言；S2 含 Go 互通；S3 含 getent 对拍）
- [ ] 回归：m82 + m83_s1–s6 全绿（重点 http 面 m83_s1/s6、crypto 面 m83_s3/s4 零回归）
- [ ] 自举证明 bootstrap_prove.sh rc=0（若 selfhost 层有变）；重链 bootstrap ELF 已提交
- [ ] native 表 299→**301**：native_index.json + CHEATSHEET + ECOSYSTEM 同步，CI diff 无漂移
- [ ] 双模式一致（pxi run == pxc build）抽查
- [ ] qg-issue 21/22/23 归档 `done/`；00-README 总览更新（标题：Issue 1–23 全部归档，仅 14-W1b ⏸️ 挂起 + 13-R2 业务自持）；M84_PLAN 全程记录
- [ ] tag **v0.1.0-m84** 推 GitHub；CI completed success（含三架构交叉矩阵）
- [ ] fmt/lint 0 错；worktree 干净

## 四、风险与预案

| 风险 | 预案 |
|---|---|
| Issue 23 修复误伤无头默认分支（最广调用面） | S1 单独一批先修 + 完整回归（m83_s1/s6 + m83_s5 multipart）；verify 显式断言「无头→默认表单头」与「带 CT→单头」两侧 |
| sha256 原地增强输入域：既有调用全为 ASCII str → 行为不变，风险低 | 输出语义不变仅拓宽输入；verify 用 Go 对含 NUL bytes 逐字节对拍 |
| dns_lookup 环境差异（容器无网 / 不同 glibc 排序） | verify 用 localhost + 解析结果**集合相等**断言（不依赖顺序）；断网环境跳过公网项并标注 |
| TC3 固定向量与腾讯文档版本漂移 | 以 RFC 4231 + Go 自算为主对拍（权威），TC3 向量仅作端到端叙事 |
| 多架构交叉 CI（aarch64/armv7/riscv64）新 native 隐式声明 | 遵循 M83-S4 教训：新增 native 全部显式声明（extern/static 前置），本地 zig cc 三架构预演后再 push |
| W1b 若被解冻插入 | 不在本计划默认范围；用户下令则拆为独立 M84b 单独批次，不混入 S1–S4 |
