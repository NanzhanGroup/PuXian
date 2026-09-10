# M103_PLAN · 清歌 Issue 29 + 30 语言层缺口全量收官（dns TXT / ed25519_keygen / YAML 写 / 图片 JPEG）

> 状态：✅ **已完成（S1–S2d + S3 收口）**——四缺口全量落地，qg-issue 29/30 归档 done。
> 基线 v0.2.0-m102（8377953，.px 子进程池 px_exec offload 收口后）。native **301 → 306**。
> 来源：/data/qg-issue **29-puxian-ddns-gaps** + **30-puxian-api-server-gaps**（清歌提报，
> 2026-09-07，官方基准 v0.1.0-m88b——现已推进至 v0.2.0-m102 成熟期，缺口仍在）。
> 用户指令（2026-09-10）：**「开工 M103」**（远景路线坐标检查后，按三连计划推进 M103 =
> 清歌 Issue 29/30 能力交付批）。
> 性质：**L0 runtime（native 301 → 306，+5）+ L1 stdlib**——需重链 bootstrap ELF + 自举
> 证明 + 全量回归 + qg-issue 归档。

## 〇-1、实施结果（2026-09-10，S3 前汇总）

- **S2a · dns_txt(domain) → list[str]**（Issue 29-1）✅：手写 DNS UDP TXT 查询（native 302）。
  examples/m103_s2a 9P/0F + Go net.LookupTXT 对拍 CMP-EQUAL + m84_s3_dns 回归全绿。
- **S2b · ed25519_keygen() → dict{pk_hex,sk_hex,pk_pem,sk_pem}**（Issue 29-2）✅：
  RFC 8410 PKCS8/SPKI 与 Go x509 逐字节互通（native 303）。m103_s2b 14P/0F + Go 字节级
  互通三断言 + m83_s3 零回归。
- **S2c · std.yaml yaml_stringify**（Issue 30-2）✅：stdlib/yaml.px 纯 .px 序列化。
  m103_s2c 46P/0F + Go yaml.v3 双向互认 + m66_yaml 35P 零回归。
- **S2d · img_decode/img_scale/img_encode_jpeg**（Issue 30-1）✅：stb_image v2.30 +
  stb_image_write v1.16 public domain 入库；img 模块 --no-img 裁剪（native 306）。
  m103_s2d 21P/0F + Go image.Decode 验 px JPEG 512×384 合法 + 体积 13209 vs 13177B 同量级。
- **S3 收口**：native_index 306 + CHEATSHEET 306；vm_ab 38P/0GAP/0F + diffcheck --all rc=0
  + 双自举证明（C/BC 轨）+ pxi/pxi_vm 重链 + ROADMAP M83–M103（20 连）+ qg-issue 29/30
  归档 done + tag **v0.2.0-m103**。

## 〇、范围（Issue 29/30 四缺口，全量做）

| # | 缺口 | 来源 | 现状（D0 实据） | M103 交付 |
|---|---|---|---|---|
| 1 | **dns TXT 查询** | Issue 29 §2 | `dns_lookup`（runtime.c L4414）getaddrinfo **只 A/AAAA**，结构上不可能返回 TXT；无 TXT native | `dns_txt(domain) → list[str]`（手写 DNS UDP，跨 glibc/musl/mingw 零依赖） |
| 2 | **ed25519_keygen** | Issue 29 §3 | runtime_ed25519.c 只有 sign/verify（M83-S3）；`crypto_sign_seed_keypair` extern + randombytes + e_hex **已就绪**，缺暴露与 PEM/DER 构造 | `ed25519_keygen() → {pk_hex, sk_hex, pk_pem(SPKI), sk_pem(PKCS8)}` |
| 3 | **YAML 写（stringify）** | Issue 30 §3 | stdlib/yaml.px（M66-S2，403 行）只有 `yaml_parse` 只读；api-server models/config 写回 YAML 需序列化 | stdlib/yaml.px 补 `yaml_stringify(value, indent) → str`（纯 .px，parse 对称） |
| 4 | **图片 decode→resize→JPEG 编码** | Issue 30 §2 | std.png 仅纯 .px stored **编码**（不能吃输入图）；全仓无 decode/resize/JPEG | runtime native `img_decode` + `img_encode_jpeg` + `img_scale`（stb_image v2.30 + stb_image_write v1.16，public domain） |

## 一、D0 侦察结论（2026-09-10，dongyue，基线 8377953）

### 缺口 1 · dns_txt —— ✅ 成立，方案：手写 DNS UDP（否决 res_query）

- **getaddrinfo 无 TXT 面**（D0 实据：bi_dns_lookup L4423 `hints.ai_family=AF_UNSPEC` →
  仅地址记录）。TXT 属 DNS wire 记录（TYPE=16），libc 高层 API 不暴露。
- **res_query 否决**：glibc 需 -lresolv（2.34 前）/ 静态链体积；**musl（交叉 aarch64/
  armv7/riscv64 工具链 aarch64-linux-musl-gcc 等）res_query 为兼容桩仅 A/AAAA，TXT 不可靠**；
  mingw（Issue 14 Windows）无 res_query。跨平台零依赖 ⇒ **手写 DNS 报文**（构造 query →
  UDP 发系统 resolv.conf nameserver → 解析响应，含压缩指针/多段 TXT 合并），与既有
  udp_send/hconnect 的 getaddrinfo 前置无关，纯 socket 独立。
- **接口语义**（Issue 29 §2 验证标准）：`dns_txt(domain)` 返回 TXT 记录字符串 list；
  **无记录域返回空 list（非报错）**——与 dns_lookup（无记录报 Err）不同，TXT 属可选
  存在（授权 TXT 可能未配置，空 list 更合用）；查询失败（无 nameserver/超时/格式错/
  TC 截断）返回可判定 Err。每条记录 = 该 RR 整串（多段 character-string 合并；DNS
  单段 ≤255B，DNSPod 授权 TXT 单条场景，多段合并与 Go net.LookupTXT 语义对齐）。
- **超时**：socket 非阻塞 + poll（runtime 已有 poll 用先例，px_pool_recv_result），
  默认 3s；TC 位置位（UDP 响应截断）→ 返回 Err（TXT 大记录场景记录二期 TCP 回退）。
- **验证**：服务器无 dig（D0 实据）→ 与 **Go net.LookupTXT 对拍**（M84-S3 用 Go
  net.LookupIP 先例；Go toolchain 可用）；公开稳定 TXT 域（qq.com 等，nameserver
  8.8.8.8/114.114.114.114 可达）；空 TXT 域（构造一个不存在的 TXT 域名如
  `_nonexist_txt_probe.invalid`）→ 空 list；dns_lookup A/AAAA 零回归。

### 缺口 2 · ed25519_keygen —— ✅ 成立，低风险（tweetnacl 现成）

- runtime_ed25519.c（290 行）已备：`crypto_sign_seed_keypair`（tweetnacl.c 本地扩展，
  头文件 extern 已声明 L26）、`randombytes`（/dev/urandom）、`e_hex`、DER 解析族
  （der_tlv/der_skip/der_oid_is_ed25519/e_pem_der/e_parse_priv/e_parse_pub）。缺：
  **keypair 调用暴露 + DER 构造（PKCS8/SPKI 导出）**。
- **DER 模板**（RFC 8410）：PKCS8 = `302e020100300506032b657004220420` + seed32；
  SPKI = `302a300506032b6570032100` + pk32（与 Go ed25519 PKCS8/SPKI 逐字节一致）。
- **接口**（与 M83-S3 sign/verify 的 hex/PEM 同族）：`ed25519_keygen() → dict`
  {`pk_hex`(64), `sk_hex`(128=seed\|pub), `pk_pem`(SPKI), `sk_pem`(PKCS8)}。
  base64 编码用 runtime 既有 base64（bi_base64_encode L4198 同族，C 侧直用）。
- **归属**：进 runtime_ed25519.c + native_mod_map `ed25519_keygen=ed25519`（--no-ed25519
  裁剪组，mod_srcs 现成）→ runtime.c px_set_global 注册（PX_NO_ED25519 内）。
- **验证**：px keygen → px sign → Go verify（PKCS8 seed 导入）+ Go keygen → px
  verify + px 自 roundtrip + 现有 m83_s3 sign/verify 零回归（examples/m103_s2 含
  Go 对拍程序，复制 m83_s3_ed25519/go_s3.go 骨架）。

### 缺口 3 · YAML stringify —— ✅ 成立，纯 .px（stdlib/yaml.px 扩展）

- **语言语义 D0 实测**（pxi /tmp/probe_m103.px）：type() 返回 `dict/bool/null/int/
  float/string/list`；`for k in dict` 遍历 **保持插入序**（与 Go yaml 无关，键序不要求
  保留）；dict 构造 `json_parse("{}")` + `.set(k,v)`；list `.append`；`str(true)="true"`
  `str(null)="null"`。
- **stringify 设计**：`yaml_stringify(value, indent)`（indent 默认 2，对标 Go yaml 缩进）
  → 字符串。标量：string 判定是否需要引号（空/首尾空白/含 `:#-?[]{},&*!|>'"%@\`` 或
  数字/布尔/null 歧义 → 双引号 + \n\t\r\\\"\u 转义；否则裸输出）；int `str(i)`；float
  保小数（验证 `str(3.0)` 若丢 `.0` 需补）；bool/null 直出。容器：dict `k: v`、list
  `- v`、嵌套按 indent 递归。空 dict `{}` 空 list `[]` 输出 `{}`/`[]`（Go yaml 读回
  亦空容器，roundtrip 语义等价）。
- **验证**（examples/m103_yaml）：`yaml_parse(yaml_stringify(v))` 值语义等价（type +
  嵌套深比较）；**与 Go yaml 互认**——px stringify 的 YAML → Go yaml.Unmarshal 结构
  对拍；Go yaml.Marshal（api-server models 形状：llm_providers 列表/嵌套 map）→ px
  yaml_parse 读回一致；特殊字符转义 roundtrip；现有 yaml_test.px（M66-S2）零回归。

### 缺口 4 · 图片 decode/resize/JPEG —— ✅ 成立，方案：stb native（模块 img）

- **现状**：stdlib/png.px（99 行）纯语言 stored **PNG 编码**（画布→PNG，不吃输入图）；
  gfx.px 画布原语。JPEG DCT/哈夫曼 + PNG/JPEG 解码**纯 .px 不可能**（Issue 30 §2 论证）。
- **选型**：stb_image.h v2.30（283,010 B）+ stb_image_write.h v1.16（71,221 B）已下载
  （raw.githubusercontent.com，**public domain**，无许可约束）→ runtime/third_party/stb/
  （sha256 归档 + 来源注释，循 tweetnacl 引入记录规范）。
- **native 3 个**（runtime_image.c，模块 `img`，仿 ed25519 裁剪）：
  - `img_decode(data:bytes|str) → dict {w,h,channels,pixels:bytes(RGBA)} | Err`
    （stbi_load_from_memory，req_comp=4 强制 RGBA；解码失败 Err 可判定）。
  - `img_scale(pixels:bytes, w, h, nw, nh) → bytes(RGBA)`（双线性插值，≤512 等比缩放；
    纯 C ~60 行；stb 无 resize，自写）。
  - `img_encode_jpeg(w, h, pixels:bytes(RGBA), q:int) → bytes`（stbi_write_jpg_to_func
    → 内存缓冲，rgb 抽 4→3 通道；q 默认 70 由调用方传）。
  - 输入 bytes 面：PX_BYTES/PX_STR 均取带长视图（val_cstr 截断 NUL 不可用，须
    val 类型分支拿 len）。
- **裁剪**：img 模块 = runtime_image.c + third_party/stb/*.h；tools/px mod_srcs +
  rt_src_files + native_mod_map（img_decode/img_encode_jpeg/img_scale=img）+
  runtime.c PX_NO_IMG 注册隔离。默认全量链入（stb 281KB C 编译进产物，pxi/pxi_vm
  体积预计 +~80-150KB；--no-img 可裁，与 --no-ed25519 正交）。
- **验证**（examples/m103_img）：PNG 输入（m61_gfx scene.png/mandelbrot.png 现成落盘）
  → decode 尺寸像素对拍（std.png 已知 W/H）→ encode_jpeg q70 → 再 decode back 尺寸
  一致 + JPEG 魔数 FF D8 + 体积量级（512² q70 ≈ 20-60KB）；**Go 参照**（avatar.go
  语义：image.Decode → resize ≤512 → jpeg q70 同输入图对拍输出尺寸/体积量级；
  允许 DCT 微差）；畸形输入（空/随机字节）Err 不崩；png+jpeg 输入均可。jpeg 输入
  fixture：verify 内用 Go 生成（image/jpeg encode）落盘再喂 px——不 checkin 二进制。

## 二、分批次执行计划

| 批 | 内容 | native | verify（examples/） | commit |
|---|---|---|---|---|
| **S1** | 立项 + D0 侦察定稿（本文档） | 301 | — | M103-S1 |
| **S2a** | Issue 29-1：`dns_txt`（手写 DNS UDP，resolv.conf nameserver） | **302** | m103_s2a：qq.com TXT vs Go net.LookupTXT 集合一致 + 无 TXT 域空 list + .invalid Err + dns_lookup A/AAAA 零回归 | M103-S2a |
| **S2b** | Issue 29-2：`ed25519_keygen`（runtime_ed25519.c + PKCS8/SPKI DER + map + 注册） | **303** | m103_s2b：px keygen sign → Go verify + Go keygen → px verify + 自 roundtrip + m83_s3 零回归 | M103-S2b |
| **S2c** | Issue 30-2：stdlib/yaml.px 补 `yaml_stringify`（纯 .px） | 303 | m103_s2c：roundtrip 语义等价 + Go yaml 双向互认 + 特殊字符 + M66 现有 yaml_test 零回归 | M103-S2c |
| **S2d** | Issue 30-1：stb 入库 + runtime_image.c（img_decode/img_scale/img_encode_jpeg）+ img 裁剪模块 | **306** | m103_s2d：PNG/JPEG 输入 decode→scale→encode_jpeg→decode back + Go avatar 语义对拍 + 畸形输入 Err + m61_gfx std.png 零回归 | M103-S2d |
| **S3** | 收口：native 计数同步（gen_native_table → index/cheatsheet/ecosystem 306）+ diffcheck --all + vm_ab 38P + 双自举证明（C/BC 轨）+ pxi/pxi_vm 重链 + 全量回归 suites + CHANGELOG/ROADMAP + qg-issue 29/30 归档 done + tag **v0.2.0-m103** | 306 | 全量：m82/m83_s3/s6/m89_s3d/m93-m102 各 suite + m103_s2a-s2d + m43_webapp 等 | M103-S3 |

**同步链（M84/M102 同款）**：native 计数单一事实源 tools/gen_native_table.sh →
docs/native_index.json + PUXIAN_CHEATSHEET.md + ECOSYSTEM.md；新增 native 必同步。
改 runtime/*.c → 重链受影响 bootstrap ELF（pxi/pxi_vm）+ 提交新 ELF（git 跟踪）。
stdlib/*.px（yaml.px）改动 → 影响 import std.yaml 的全部编译面，回归 std 引用 suites
（m66_yaml / m43_webapp / 自举 compiler 若 import std.* 则触发 bootstrap_prove）。

## 三、风险与预案

| 风险 | 预案 |
|---|---|
| 手写 DNS UDP 细节（压缩指针/多段/超时）回归 | 参照 Go net 语义对拍（LookupTXT 集合相等）；TC/超时/格式错 Err 可判定；UDP 512B 截断场景记录二期 TCP 回退（先 Err 不静默） |
| stb 281KB 编译体积/rtcache 重建 | 独立 img 模块可 --no-img 裁剪；rtcache key 含源哈希自动失效重编；编译时间一次性 ~几秒 |
| stb 与 mbedtls/zlib 符号冲突 | stb 全静态函数（STB_IMAGE_IMPLEMENTATION 单 TU 实例化于 runtime_image.c），符号局部化不导出；diffcheck --all 把关 |
| yaml stringify 与 Go yaml 产出互认偏差 | 验证标准为「Go yaml.Unmarshal 可读 + roundtrip 语义等价」（键序不要求）；特殊字符转义矩阵专测 |
| float str(3.0) 丢小数破坏 roundtrip | stringify 内 float 强制保留小数（探测 str(3.0)；若丢 .0 则拼接）；专测 float 边界 |
| 断网/无 TXT 域环境 | verify 用 Go 对拍（本机可达）；公开域不可达时 skip 标注（m84 断网容错先例） |

## 四、验收（对应 Issue 29/30 §6 验证标准）

- dns_txt 与 Go net.LookupTXT 结果一致（含空域空 list、多 TXT、多段合并语义）；dns_lookup A/AAAA 零回归。
- ed25519_keygen 与 Go ed25519.GenerateKey 双向互操作（seed 级互通）+ sign/verify 自洽；m83_s3 sign/verify 零回归。
- yaml_stringify(parse(f)) roundtrip 语义等价；标量引号/特殊字符转义正确；list/map 缩进与 Go yaml 产出互认（px 写→Go 读、Go 写→px 读）。
- img_decode→img_scale(≤512)→img_encode_jpeg(q70) 对同一输入图与 Go avatar.go 产出尺寸/体积量级一致（允许 DCT 微差）；png/jpeg 输入均可；畸形输入 Err 不崩。
- ws-ddns .px 移植（Issue 29 阻塞面）与 api-server 移植（Issue 30 阻塞面）缺口解锁记录。
