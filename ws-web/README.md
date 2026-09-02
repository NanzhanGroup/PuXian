# ws-web —— 用 PuXian 写的新时代 Web 服务器（v0.5.1）

> 自举闭环的最后一环：用 PuXian 写 PuXian 生态的生产 Web 服务器。
> 编译器已自举（M-B8），Rust 版已退役（M-B9a），全程使用 `tools/pxc` 工具链，**无需 Rust**。

## 功能总览

- **纯静态 vhost**：多站点 Host 路由 / deny（目录+扩展）/ SPA fallback / 反向代理 / TLS-SNI / HTTP→HTTPS 301 / 访问日志 / healthz
- **HTTP/3（M53-S4）**：config `"http3": true|{port,cert,key}` 启用 QUIC/UDP，随主服务同端口、Alt-Svc 自动通告
- **md 多语言站点模式（v0.5 新增）**：`src/*.md` 源 + 模板 + i18n 文案 + LLM 翻译 → 自动构建多语言静态 HTML
  - 默认语言裸路径（`/learn.html`），其它语言独立 URL（`/en/learn.html`）→ 可 CDN、可被 Google 收录（hreflang + sitemap）
  - 翻译发生在 **md 层**（纯文本，不碰 HTML/代码标签）；**磁盘缓存**（`sha256(源+provider+model)+lang`），源变只重翻该页
  - **预热 + 热更**：启动后台构建、源文件变更自动重建（60s 轮询）
  - 提示词三层可配：内置硬规则 + 站点 `prompt/prompt_file` + 页级 front-matter `prompt`
- 老纯静态站（无 `type`）**行为完全不变**，向后兼容
- **v0.5.1**：页面模板新增可选占位 `{{ROOT}}`（当前语言首页根）、`{{SITE_NAME}}`、`{{FOOTER_NOTE}}`，取值来自 i18n 的 `site_name` / `footer_note`（缺省 fallback），便于品牌名与页脚随语言切换

## 快速开始

```bash
# 1. 编译为静态二进制（生产 + 开发主路径）
./build.sh && ./build/ws-web --version

# 2. 本地跑 md 多语言演示站（provider=mock，离线联调）
./build/ws-web --config ws-web/test_site/config.example.json
#   访问 http://127.0.0.1:18082/（zh） /en/（en·mock） /ja/（ja·mock）

# 3. 接真实 LLM（OpenAI 兼容，key 走环境变量）
export WS_WEB_LLM_KEY=sk-xxxx
#   config 里 llm.provider 改为 openai-compat 并填 base_url/model 即可
#   （或 --llm-key <secret> 加密写入 key 文件，不落 config.json）
```

> ⚠️ 生产内置（`http_* / sqlite_* / tls_server / px_serve` 等）只在**编译模式**可用；
> 解释器 pxi 仅支持 Mini 子集白名单（MINI_SUBSET §2.5）。**请走 build.sh**。

## 配置

见 `config.json.example`。要点：

| 字段 | 说明 |
|---|---|
| `http3` | `true`（同 https 端口自签）或 `{port, cert, key}`；缺省关闭 |
| `sites.<host>.type = "md"` | 声明为 md 多语言站点（缺省=纯静态） |
| `sites.<host>.default_lang` | 默认语言 = 源语言（缺省 zh） |
| `sites.<host>.langs` | 启用语言列表（含默认语言） |
| `sites.<host>.public_url` | 对外 base（canonical/hreflang/sitemap 绝对地址） |
| `sites.<host>.nav` | 主导航：`{href, label?, i18n?}` |
| `llm.provider` | `mock`（默认，离线联调）或 `openai-compat` |
| `llm.api_key_env` | key 环境变量名（推荐） |
| `llm.key_file` | 加密 key 文件路径（`ws-web --llm-key <secret>` 生成） |

### md 站点目录约定（root 为 `sites.<host>.root`）

```
root/
├── src/                # 源 .md（front-matter: title/description/order/lang/prompt）
│   ├── index.md        #   → /index.html（默认语言） /en/index.html（其它语言）
│   └── learn/01.md     #   → /learn/01.html
├── templates/
│   └── page.html       # 页面骨架（占位 {{TITLE}}{{DESC}}{{LANG}}{{HREFLANG}}
│                       #         {{NAV}}{{LANGSWITCH}}{{CONTENT}}）
├── i18n/<lang>.json    # 界面文案（导航/按钮）
├── public/**           # 静态资源（css/js/img…），直读无需复制
├── cache/              # 翻译缓存（HTTP deny，不对外）
└── out/                # 构建产物（HTTP 服务目录）
```

## 端点

| 方法 | 路径 | 说明 |
|---|---|---|
| GET | `/healthz` | 健康检查 `{"ok": true, "uptime": ms}` |
| GET | `/*` | 站点内容（纯静态根 / md 产物 / deny / 反代） |

## 目录结构（v0.5 模块化）

```
ws-web/
├── main.px          # 入口：args/config/LLM key/启动/预热调度
├── cfg.px           # 配置加载与应用（含 md 站点/llm/http3 段）
├── http.px          # HTTP 层：vhost/deny/SPA/反代/TLS/HTTP3/md out 服务
├── md_render.px     # Markdown → HTML（front-matter/标题/列表/表格/围栏/行内）
├── template.px      # 页面模板注入 + i18n + 导航 + 语言切换 + hreflang/canonical
├── llm.px           # LLM 通道（OpenAI 兼容）+ key 加密存取（AES-256-GCM）
├── translate.px     # 翻译编排：三层提示词/缓存/切块/代码围栏保护/mock
├── md_site.px       # md 站点：扫描/排序/构建/sitemap/robots/预热/热更
├── test_site/       # 可运行的 md 多语言演示站（mock，离线）
├── build.sh / run.sh
└── tests/
```

## 设计要点（md 多语言）

1. **为什么翻 md 不翻 HTML**：md 是纯文本，代码围栏天然隔离、diff 可审、人能校对。
2. **为什么分语言静态 URL**：默认语言裸路径 + `/xx/` 独立静态页 → 同一 URL 内容恒定，
   CDN 随便缓存、蜘蛛无 cloaking 风险、hreflang/sitemap 收录各语言（英文用户可被搜索引擎找到）。
3. **成本控制**：缓存命中零 LLM 调用（重启即缓存命中）；源变只重翻该页；顺序翻译不并发烧钱。
4. **宁可不翻不可翻错**：代码围栏结构与语言标记不一致 → 重试 1 次 → 仍失败降级源语言；
   v0.5 以提示词约束代码不可变，注释翻译由模型遵守（后续版本将加"注释剥离逐字节校验"强化）。
5. **安全**：LLM key 不入 config.json（环境变量 / AES-GCM 加密 key 文件）；src/cache/templates/i18n 默认 deny。

## 写 PuXian 必须规避的坑（Mini 子集，自举编译器锁定）

> 完整规范见 `docs/MINI_SUBSET.md`。最易踩的：

| # | 别写 | 原因 | 替代 |
|---|---|---|---|
| 1 | 匿名 fn 捕获外部变量 | 编译模式仅无捕获闭包 | 顶层 def + 全局变量 |
| 2 | 函数内嵌套 `def` | 编译模式静默忽略 | 顶层 def |
| 3 | 三元 `a if b else c` | spec 明确禁止 | 普通 if 语句 |
| 4 | 字符串行继续符 `\`（反斜杠续行） | 词法非法字符 | 多语句拼接 |
| 5 | 变量名用保留字（如 `pub`） | 语法错误 | 换名（`pu`） |
| 6 | `try / throw` | spec 明确不做 | Result + `?` |
| 7 | 命名参数 / `*args` | parser 仅位置参数 | 位置参数 |
| 8 | 依赖解释器跑生产代码 | pxi 缺生产内置 | 编译模式 build |

## 反馈与贡献

遇到语言 bug 或特性缺失：写最小复现用例 → GitHub issue（label `ws-web` / `compiler`）→ 维护者修复 → 自举证明回归。

## 版本

- 工具链：`./tools/pxc --version` → `pxc 0.1.0 (普贤 PuXian · selfhosted M-B9a)`
- ws-web：`./build/ws-web --version` → `ws-web 0.5.1`
