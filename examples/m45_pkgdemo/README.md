# M45 · registry 版本化示例（semver + lockfile）

演示 PuXian 包管理器（tools/pxpkg）的**可复现构建**：
`name@范围` → registry 选最高满足版本 → 锁定精确版本 + sha256 → `install --locked` 严格校验。

## 目录结构

```
examples/m45_pkgdemo/
├── registry/                本地 registry（PX_REGISTRY 指向）
│   ├── mylib/1.2.0/mylib.px  （低版本）
│   ├── mylib/1.2.5/mylib.px  （^1.2.0 满足的最高版本）
│   ├── mylib/2.0.0/mylib.px  （^1.2.0 不满足）
│   └── other/0.1.0/other.px
├── local_helper.px          本地路径依赖示例
├── app/                     演示应用（verify.sh 动态生成）
└── verify.sh                端到端验证脚本
```

## 用法

```bash
# 全链路验证（init → add → install → lock → --locked 校验/篡改检测）
examples/m45_pkgdemo/verify.sh

# 手动体验
cd examples/m45_pkgdemo/app
export PX_REGISTRY=../registry
pxpkg init --name demoapp
pxpkg add mylib@^1.2.0          # semver 范围依赖
pxpkg add other@0.1.0           # 精确版本
pxpkg add ../local_helper.px --name helper   # 本地路径
pxpkg install                   # 解析 + 安装 + 写 px.pkg.lock
pxpkg install --locked          # 可复现校验
pxpkg list
```

## 关键文件

| 文件 | 作用 |
|---|---|
| `stdlib/semver.px` | semver 2.0.0 子集：sv_parse / sv_cmp / sv_satisfies / sv_best |
| `tools/pxpkg.px` | 包管理器：init/add/install/list/remove + lockfile 读写 |
| `tools/pxpkg` | bash 包装入口（环境变量传参给 pxi） |
| `px.pkg.lock` | JSON 锁定：精确版本 + sha256 + source（可复现构建契约） |

## 验证要点

1. `mylib@^1.2.0` → 安装 **1.2.5**（1.2.0/1.2.5/2.0.0 中满足且最高）
2. lock 锁定 1.2.5 + sha256；与安装内容哈希一致
3. `--locked`：三个依赖校验通过（版本满足 + sha256 一致）
4. **篡改检测**：改 .px_modules 内容 → `--locked` 拒绝（sha256 不符）
5. **可复现**：registry 删除后 `--locked` 仍通过（不重新解析，严格按 lock）
