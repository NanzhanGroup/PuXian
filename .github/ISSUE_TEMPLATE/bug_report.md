---
name: Bug 报告
about: 提交一个可复现的缺陷，帮助我们改进
title: "[Bug] 简要描述"
labels: bug
assignees: ''
---

## 描述

清晰简洁地描述这个 bug。

## 最小复现用例

请提供**单个 .px 文件**的最小复现（这是硬性要求，见 CONTRIBUTING.md）：

```px
# 你的复现代码
print("...")
```

## 期望 / 实际

- 期望输出：`...`
- 实际输出：`...`
- 命令：`./tools/pxc build|run 文件.px`

## 环境

- 工具链版本：`./tools/pxc --version` 输出
- 模式：编译（build） / 解释（run） / 双模式都有
- 系统：Linux（x86_64 / aarch64）等

## 其他

日志、报错全文、相关上下文（可选）。
