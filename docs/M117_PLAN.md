# M117 —— 真实模块 `ws-install` PuXian 化暴露的三类缺陷

> 台账：`qg-issue 72` · 开工 2026-09-14 · 探针来源：`NanzhanGroup/ws-install`（Go，1580 行）整体 PuXian 化
> 主题：**继续拿真实模块当探针** —— 这一轮的三个缺陷都不是"写不出来"，而是
> **写出来了、跑起来了、还静默错/无限等**，正是 M116 之后仍藏在门后的那一类。

---

## 0. 探针与对拍

`ws-install` 是文殊装机器：下载（缓存/sha256/断点续传）→ 解包分发 → 生成 `.env`/密钥/agent_id →
AES 加密迁移 LLM 密钥 → 部署 OCR/Embedding/ws-ddns-server → systemd → SELinux → 启动校验。

移植方式：**PuXian 版落仓库根目录**（`main.px` + 6 个模块，共 ~1900 行，7 文件均 < 500 行），
原 Go 实现整体移入 `go/`。

对拍：`tests/parity_install.sh` —— 两侧在**同一套沙箱**里跑同一串参数：

- `unshare -m` + bind mount 顶掉 `/etc/environment`、`/etc/profile.d`、`/etc/systemd/system`、
  `/etc/ws-ddns-server`、`/usr/local/bin`（**宿主 /etc 与真实 `ws`/`ws-ddns-server` 二进制零影响**）
- `PATH` 前置 stub（`systemctl`/`ldconfig`/`dnf`/`yum`/`apt-get`/`apk`）→ 不真起服务、不真装包
- `-dir` 指向沙箱目录、`WS_INSTALL_RELEASE_URL` 指向本地 HTTP 发布目录
- 判据：退出码 + 归一化 stdout/stderr + 安装目录清单（路径/权限/大小/内容哈希）+ 语义文件
  + `/etc` 落盘 + systemd 单元 + `/usr/local/bin` 链接 + `systemctl` 调用序列

**结果：212 行归一化清单逐行相同**；另做 **AES-256-GCM 双向互通**证明
（PuXian 加密的 `api_key_encrypted` 用 Go `crypto/aes` 解出同一明文，反之亦然）。

---

## 1. 三类缺陷（本轮修复）

### F1 · `read_file` 读 `st_size==0` 的伪文件返回空串

`bi_read_file` 走 `fseek/ftell` 取大小 → `/proc`、`/sys` 的 `st_size` 恒为 0 ⇒ **一个字节都不读**。
现场：`read_file("/proc/sys/kernel/hostname")` = `""`（装机器读不到主机名 → `agent_id` 生成失败）。

- Go `os.ReadFile` 走 read-until-EOF，不受 `st_size` 影响 ⇒ 移植后行为**静默分叉**。
- 修：能 seek 且 size>0 走原快路径；否则**逐块读到 EOF**（`/dev/null` 仍是空串）。

### F2 · `http_request` 的 `opts.timeout_ms` 不约束 connect（无界等待）

`timeout_ms` 此前只写进 `SO_RCVTIMEO`/`SO_SNDTIMEO`，而这两个选项**不约束 `connect()`**。
现场：对不可达地址（丢 SYN）调用 `http_request(..., {timeout_ms: 800})`，**45 s 仍未返回**
（Go `http.Client{Timeout}` 覆盖连接阶段）。对装机器/守护进程是"永久卡死"级：
一个不可达镜像就能让整条安装流程挂住，且**没有任何超时报错**。

- 修：`px_tcp_connect_timeout()` —— 非阻塞 connect + `poll(POLLOUT, timeout_ms)` +
  `getsockopt(SO_ERROR)` 判定 + 恢复阻塞模式；`http_request` 用它（明文与 HTTPS 都走），
  无 `opts` 入口的 `http_get`/`http_post`/`px_http_request`/`s3_*` 统一受默认上限 **30s**。
- 零回归：`timeout_ms<=0` 时保持原阻塞语义；refuse 路径仍毫秒级返回 `Err`。

### F3 · 正则的 POSIX 字符类**静默不匹配**

字符类解析器不认识 `[[:space:]]`：把 `[` 当字面量、把内层 `]` 当**类结束** ⇒
`^[[:space:]]{2,}api_key:…` 这类正则**返回 null（不报错、不告警）**。
现场：装机器解析 `token-cache-llm.yaml` 的缩进行时整段失效。

- 修：识别 `[:name:]` 并展开为等价区间；覆盖
  `space/digit/alpha/alnum/upper/lower/xdigit/blank/punct/print/graph/cntrl`；
  **未知类名直接报错**（不允许静默）。
- 注意 `\s` 系列一直支持 —— 即"两套写法一个有一个没有"，正是最容易被 AI 抄错的组合。

## 2. 上游 Go 版的缺陷（移植实测发现，PuXian 版已修，属**有意差异**）

`.env` 追加空值键时写成 `KEY=   # 注释`，而 `ensureTokenCacheKey / ensureAdminToken /
ensureAgentIDEnv` 用 `TrimPrefix` 取值后只判 `val != ""` ⇒ **把注释文本当成了已有值**：

| 键 | 上游 Go 行为 | PuXian 版行为 |
|---|---|---|
| `WS_TOKEN_CACHE_TOKEN` | 不生成（值仍是 `"   # 词元缓存密钥（留空由安装程序生成）"`），日志却打"✅ 已生成" | 生成 64 hex |
| `WS_ADMIN_TOKEN` | 同上（正是 v0.3.0 声称修好的 api-server 401 根因） | 生成 64 hex |
| `WS_AGENT_ID` | 同上 | `<host>_<6hex>` |

**触发条件**：发行包 `.env` 模板缺这些键（对拍夹具刻意构造了这个条件）。
PuXian 版按"剥注释再判空"修（幂等语义不变）。对拍脚本把这条列为
**已知有意差异 K1**（定点归一，不掩盖其它差异）。

另记 `K3`：Go `os.WriteFile(path, data, mode)` 的 mode **只在创建时生效**（已存在文件保持原权限，
实测 `.env` 停在 `copyFile` 给的 0755），而 PuXian `write_file(path, s, mode)` 对已存在文件
**也 fchmod**（M116 引入）。两者都可辩护，PuXian 更可预期、且 644 比 755 更安全 ⇒ 保留 PuXian 语义。

## 3. 文档漂移（本轮一并修）

- `http_request` 签名：§2 曾写 `(method, url)`，**实际是 `(url, method)`**（§4 的例子才是对的）——
  写反时的报错是 `net: 不支持的协议: GET`，指不到病根。
- native 计数：全文写 311，`docs/native_index.json` 已是 **312**。
- 新增/补齐事实条目 20–26：`/` 是浮点除、`join(sep, list)` 顺序、`env` 缺失返回 `null`、
  `str(bytes)` 占位符、`read_file` 伪文件、HTTP 返回类型不统一、POSIX 字符类。

## 4. 门

- `examples/m117_realworld_defects/`（双轨 + 两条负控：未知 POSIX 类名必须报错、refuse 路径零回归）
- `selfhost/m117_gates.sh` = 既有全门 + M117 专项（**27 项全绿**）
- `selfhost/rebake_bin.sh --rebake-all`（14 件入库件重烘，全件指纹门 `--check-all` 绿）
- `ws-install/tests/parity_install.sh`（212 行清单 + 密文双向互通）

## 5. 未修（登记，附最小复现）

| # | 缺口 | 现状与绕法 | 影响 |
|---|---|---|---|
| G1 | 无 `chmod(path, mode)` native | 用系统 `chmod` 代偿（每文件一次 fork） | 改**已存在**文件的权限位无处可做（`write_file` 带 mode 可覆盖大部分场景） |
| G2 | 无 `hostname()` native | 读 `/proc/sys/kernel/hostname`（M117 后可行）或 `hostname` 命令 | 移植 Go `os.Hostname()` 时需自己找替代 |
| G3 | 无 `symlink(target, link)` native | 用 `ln -sf`（与 Go 版同款代偿） | — |
| G4 | 无 `mkdtemp` | `os_random_hex(6)` + `mkdir(0700)` 组合（语义等价） | — |
| G5 | 括号内**以二元运算符结尾**换行报 `E2001 意外的 token: 换行` | 长消息拆成逐行赋值 | 已知（速查包 §1.1 事实 3 提过），但错误信息不够指导 |
| G6 | `[[:space:]]` 之外的 POSIX 类此前静默 | 本轮已修 | — |

G1–G4 属**新增 native**（需 runtime 注册 + 解释器分发 + 名册 + 文档 + 门五处联动），
本轮不做：移植侧已有等价代偿、且**不影响正确性**，不阻塞任何交付。
