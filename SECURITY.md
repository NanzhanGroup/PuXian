# Security Policy

## 报告安全问题（Reporting a Vulnerability）

PuXian 是一个编程语言与运行时项目。如果你发现**安全漏洞**（如沙箱逃逸、
C 运行时内存安全问题、TLS/加密实现缺陷、可被利用的崩溃等），
**请不要公开提交 Issue**——以免在修复前被恶意利用。

请通过以下渠道私下报告：

- 在 GitHub 上创建 **Private security advisory**（推荐）：
  `仓库主页 → Security → Report a vulnerability`
- 或发送邮件至项目维护者（见 [AUTHORS.md](AUTHORS.md) / [README.md](README.md) 联系方式）

请在报告中包含：

1. 受影响版本（`./tools/pxc --version`）
2. 漏洞类型与危害评估
3. 最小复现用例（单个 `.px` / C 用例 + 触发步骤）
4. 期望行为与实际行为

## 处理流程

- 维护者确认后会在私有渠道跟进，评估影响范围并准备修复
- 修复发布后将公开披露（CVE 编号 + 致谢），披露前不会公开细节
- 一般漏洞目标修复周期：P0（主动利用/远程）数日内、P1/P2 随下次发布

## 受支持版本

| 版本 | 支持状态 |
|---|---|
| main（最新开发版） | ✅ 积极开发 |
| 0.1.0 | ✅ 支持 |
