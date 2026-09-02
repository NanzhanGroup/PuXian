# ws-web md 多语言演示站（test_site）

离线可跑的 md 多语言站点示例（`llm.provider = "mock"`，不消耗真实 LLM）：

```
src/index.md        首页（zh 源）    → /index.html
src/learn/01.md     教程页           → /learn/01.html
i18n/zh|en|ja.json  界面文案
templates/page.html 页面骨架（{{TITLE}}{{NAV}}{{LANGSWITCH}}…）
public/css/style.css 样式（直读，无需复制）
```

## 运行

```bash
cd <repo-root>
./ws-web/build/ws-web --config ws-web/test_site/config.example.json
# 然后访问：
#   http://127.0.0.1:18082/            中文（默认语言，裸路径）
#   http://127.0.0.1:18082/en/         英文（mock 标注 [en·mock]）
#   http://127.0.0.1:18082/ja/         日文（mock 标注 [ja·mock]）
#   http://127.0.0.1:18082/sitemap.xml 全语言 URL 收录
```

> mock 模式只给 front-matter 的 title 加 `[lang·mock]` 标注、正文保持原文，
> 用于离线验证多语言 URL/切换/hreflang/缓存管线。接真实 LLM：
> 改 `config.example.json` 中 `llm.provider = "openai-compat"` + `base_url/model`，
> 并 `export WS_WEB_LLM_KEY=sk-xxx`（key 不落配置文件）。

## 结构语义

| 路径 | 说明 |
|---|---|
| `src/<rel>.md` | 源 markdown，URL = `<rel>` 去 `.md` 加 `.html`；`index.md` → `/index.html` |
| front-matter | `title` / `description`（翻译随页面）`order`（排序）`prompt`（页级提示词） |
| 站内链接 | 写相对/绝对 `.md`（如 `/learn/01.md`），构建自动改到当前语言 `.html` |
| 代码围栏 | 构建/翻译均保留结构与语言标记（代码防错保护） |
