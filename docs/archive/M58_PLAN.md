# M58 · 边缘设备层 dogfood：硬件健康守护 daemon「pxhwmond」

> 目标：M57 把使用域开到 Linux 边缘设备层（fd 原语 / mmap 活映射 / aarch64 交叉编译），
> M58 是**首个 dogfood 里程碑**：用 PuXian 写一个**真实边缘应用**——单静态二进制硬件
> 健康守护 daemon「pxhwmond」，把 M57 的能力（open/read 数据通道、mmap 共享、交叉
> 产物）揉进完整真实程序，x86 实测 + aarch64 交叉 qemu 验证，**暴露真实语言/工具链
> 缺口**（1→1.0n 的验证闭环，与项目「真实用户喂 bug」哲学一致）。
>
> 执行规范约束：MONITORING/P0（长期服务必须自带监控 + 自愈 + 通知机制，本应用即落地
> 载体）；代码每文件 <500 行（对大模型友好）→ 多文件拆分。
>
> 范围：PuXian 语言 / runtime / 工具链 + 本仓库 examples 内自研示例应用（不牵外部
> 生产应用 Mahesvara）。验证基调：容器无真板子/无 hwmon temp → 采集真实 /proc +
> /sys 条件探测降级；aarch64 产物 qemu-aarch64 跑自检模式验证。
>
> **状态：✅ 已完成（S1–S5 全部落地并回归，M58 里程碑闭环）**

## 一、现状（调研结论，均已实测）

| 维度 | 现状（事实） | 对 M58 的含义 |
|---|---|---|
| M57 能力移交 | open/close/ioctl/os_errno/read/write/mmap/munmap/mem_write（fd 数据通道 + MAP_SHARED 活映射） | daemon 采集/共享可直接用新通道，走真实内核路径 |
| 交叉工具链 | /opt/aarch64-linux-musl-cross（GCC 11.2.1）+ /usr/local/bin/qemu-aarch64-static v7.2.0（M57 S4 成果在位）；tools/cross_aarch64.sh + runtime/mbedtls/lib-aarch64/ + sqlite3-aarch64.o | M58 交叉验证零额外前置 |
| **源码模块 import** | `import "lib.px"`（相对路径源码模块）**解释 + 编译双模式实测通过**（/tmp/pxmodtest：lib def 跨文件调用，pxc run / pxc build 均 ok） | **daemon 可多文件拆分**，每文件 <500 行约束成立（重要新发现） |
| 环境真实资源 | x86_64 Rocky9 容器；/proc/stat、meminfo、loadavg、uptime、cpuinfo、net/dev **真实可读**；/sys/class/hwmon 目录存在但**无 temp\*_input**；/sys/class/thermal 仅 cooling_device（无 thermal_zone temp） | CPU/内存/负载真采；温度走**条件探测 + 降级 n/a**（框架验证，真值留真板子） |
| 语言解析 API | split / contains / str() / bytes_to_* / int_to_bytes 在位（examples 广泛使用） | /proc 文本解析可行 |
| HTTP 形态 | px_serve(port, docroot, timeout) = **docroot 脚本服务器**（M43 文件即路由）；另有 tcp_listen/accept/send/recv 原语 + http_get/http_post 客户端 | 见 D4：状态页用手写最小 HTTP，不依赖 docroot |
| 定时/并发 | cron("*/1…", fn) / sleep(ms) / spawn / clear_timer（m28_cron、p5） | 采样调度与线程可用 |
| 语言面欠账 | MINI_SUBSET §三/§八–§十二（自举编译器语言面锁定）；pxi 解释器 = Mini 子集（M57-S5 曾暴露 bytes() 等边界） | dogfood 主打**编译模式全能力**（同 M57 策略）；撞欠账 → 记录 + 小修评估，不中途重链 |

## 二、应用设计：pxhwmond（PuXian 硬件健康守护）

**定位**：Linux 边缘设备（树莓派/网关/盒子）硬件健康采集 + 状态页守护进程。
单静态 ELF（pxc build），可选 aarch64 交叉（--no-quic 裁剪，daemon 不用 H3）。

### 采集项（真实源 + 条件降级）
1. CPU 使用率：/proc/stat cpu 行两次采样差（user+nice+system / total）
2. 内存：/proc/meminfo（MemTotal / MemFree / MemAvailable / Buffers / Cached）
3. 负载与运行时长：/proc/loadavg（1/5/15）、/proc/uptime
4. 温度（条件）：hwmon temp\*_input / thermal_zone temp 探测，存在才 open 读；
   全缺 → "n/a" 降级（当前容器即此态，分支仍走真代码路径）
5. 网络流量（条件）：/proc/net/dev 非 lo 接口收/发字节

**采样通道 = M57 fd 原语主路径**：open(path,"r") + read(fd, 4096) + close；
解析用 split/str 工具。dogfood 目的即让新数据通道跑真实 /proc。

### 状态共享（mmap 活映射，体现真实用途）
最新采样快照序列化写入 **MAP_SHARED 文件**（默认 /tmp/pxhwmond.shm，env 可换）；
同机任何进程可 mmap 直读最新值（活映射，无需拷贝/轮询）。daemon 提供
`dump` 子命令（或独立 .px 读进程）验证**活映射双向**：写后立即可见、外部写读进程可见。

### 状态展示（HTTP 最小端点）
- `GET /healthz` → JSON：最新快照 + 心跳时间戳 + 自检状态 ok/deg + 运行时长
- `GET /` → HTML 状态页（表格 + 降级标记）
- **显式响应头**（Content-Type / Content-Length / Connection）——吸收 M57-S7
  「外部生产应用 vhost handler 自定义响应头丢失」教训，dogfood 自验响应头正确性。

### 监控 + 自愈 + 通知（MONITORING/P0 落地载体）
- **监控**：/healthz + 心跳 + 采样落盘 JSONL 日志（轮转保留 N 份，默认 7）
- **自愈**：run.sh wrapper 崩溃自动重启（边缘常规做法）+ daemon 内看门狗
  （连续采样失败 N 次 → 状态降级 deg → 自动恢复重试）
- **通知**：阈值告警（温度 / 内存可用 / 负载超阈值）→ 告警日志 + 可配置
  webhook URL（HTTP POST，env `PXHWMON_WEBHOOK`）；容器无真实接收端 →
  **dry-run 落盘**验证通知通道正确性。

### 运行模式（验证支柱：长期 daemon 必须能自检退出）
- 默认：常驻 daemon（interval 默认 5s）
- `--once`：采 1 次即输出退出（qemu/CI/回归用）
- `--n N`：采 N 次退出（有限时长冒烟）
- env 覆盖：PXHWMON_PORT / PXHWMON_INTERVAL / PXHWMON_SHM / PXHWMON_WEBHOOK /
  PXHWMON_ALERT_*（无配置文件、无密钥落盘，符合 SECURITY/P0）

### 目录结构（examples/m58_hwmond/，import 拆分已实测可行）
```
examples/m58_hwmond/
  main.px       采集循环 + 模式解析（--once/--n/常驻）+ 心跳 + 看门狗
  collect.px    /proc + /sys 采集（M57 fd 通道）+ 快照构造/解析
  serve.px      手写 HTTP 状态页（/healthz + /，显式响应头）
  notify.px     阈值判定 + 告警日志 + webhook dry-run
  run.sh        daemon 启动 + 崩溃自愈 wrapper（含 systemd unit 注释示例）
  verify_s1.sh … verify_s5.sh   每子步自检（examples 惯例）
  README.md     使用 / 部署 / 边界说明
（每 .px 文件 <500 行；build 产物 .gitignore）
```

## 三、架构决策

- **D1 · 形态 = 编译模式多文件程序**：`import "collect.px"` 等相对路径源码模块
  （已实测双模式可用）；pxc build 单静态 ELF；每文件 <500 行。主循环在 main.px，
  采集/服务/通知按模块拆分。dogfood 主打编译模式（同 M57 S1–S4 策略），解释模式
  非本里程碑重点（若 import 在 pxi 的 Mini 子集受限属已知欠账，记录不阻塞）。
- **D2 · 采集走 M57 fd 数据通道**：open(path,"r") + read(fd, 4096) + close 为主
  路径（read_file 路径式为对照备选）；CPU 使用率 = 两次采样差值 / 间隔。
- **D3 · 温度条件探测 + 降级**：hwmon/thermal 存在才读，全缺 → "n/a" 降级且
  状态页标记；不因缺设备崩溃（真实边缘部署的常态容错）。
- **D4 · 状态页 = 手写最小 HTTP/1.1**（tcp_listen/accept/recv/send + 显式响应头），
  不引 px_serve（docroot 脚本服务器形态不贴合嵌入式裸响应，且免 docroot 目录依赖、
  天然契合 --no-quic 静态小二进制）；仅覆盖 GET /healthz + GET / 两个端点 + 404，
  文档写明非完整服务器边界。
- **D5 · mmap 共享快照 = 真实 IPC 用途**：daemon 采样后 mem_write 进 MAP_SHARED
  文件；`dump` 子命令/读进程 mmap 活读验证双向可见（M57 B 组用例从示例升级为
  真实应用内建功能）。
- **D6 · 守护自愈 + 通知 dry-run**：run.sh wrapper（崩溃重启，`until` 循环 +
  退出码护栏）+ daemon 看门狗降级恢复；告警 webhook 在无接收端环境落盘验证。
- **D7 · 交叉 = 复用 M57 S4 工具链**：tools/cross_aarch64.sh + pxc build
  --no-quic --cc aarch64-linux-musl-gcc（daemon 无 H3 → 天然裁剪），
  qemu-aarch64-static 跑 `--once` 自检（长期 daemon 不 qemu 常驻）。
- **D8 · 语言面最小侵入**：预期零新 builtin（全部用 M57 + 既有 API）；若 dogfood
  撞到真实缺口（如解析/格式化不足）→ 记录进 MINI_SUBSET 欠账 + 评估小修
  （builtin 白名单 +10 流程参考 M57-S5），**重大缺口单独立项不进本里程碑**。

## 四、子步划分（每步可独立回归，逐步 commit）

| 子步 | 内容 | 验证（规划） |
|---|---|---|
| S1 | D1+D2 骨架 + 采集：多文件 import 工程搭起；collect.px 采 /proc（CPU 差值/内存/负载/uptime，M57 fd 通道）；main.px --once/--n 输出 | verify_s1.sh：pxc build + 跑 --once；CPU/内存/负载数值与 `cat /proc/stat`、`grep Mem /proc/meminfo`、`cat /proc/loadavg` 直接对拍（解析正确 + 非负 + 格式）；file 确认静态 ELF；每文件行数 <500 检查 |
| S2 | D3+D5 温度降级 + mmap 快照：hwmon/thermal 条件探测（缺 → n/a）；快照序列化 mem_write 进 MAP_SHARED；`dump` 子命令 mmap 活读 | verify_s2.sh：温度分支缺设备不崩 + n/a 标记；daemon --n 3 采样写共享 → dump 读到最新值（含 last_ts 递增证明活映射）；外部写共享文件 → daemon 下轮可见（双向） |
| S3 | D4+D6 状态页 + 告警通知：serve.px 手写 HTTP（/healthz JSON + / HTML + 404，显式响应头）；notify.px 阈值判定 + 告警日志 + webhook dry-run | verify_s3.sh：http_get /healthz → JSON 字段齐全 + Content-Type/Content-Length 断言（M57-S7 教训自验）；/ → HTML 含降级标记；构造超阈值快照 → 告警文件生成 + dry-run 落盘 webhook 报文格式正确 |
| S4 | D7 + 守护化回归：run.sh 崩溃自愈 wrapper；aarch64 交叉（m57_s4 工具链 --no-quic）→ qemu --once 验证；x86 全量小回归 | verify_s4.sh：交叉静态产物 file 确认 aarch64 + qemu 跑 --once 输出与 x86 一致（对拍）；wrapper kill -9 daemon → 自动拉起（存活断言）；x86 m57_s1/s3 复验 + capability 快速绿（语言面未动预期） |
| S5 | D8 文档收口：README 示例/里程碑表 M58 行 + CHANGELOG + ROADMAP 主线表 M58 行 + MINI_SUBSET 欠账更新（dogfood 撞到的如实记录）；M58 里程碑闭环 | 文档 diff 审阅；全仓回归快速绿；commit 收口推送 |

## 五、验证体系

- 每子步 verify_sN.sh（bash，examples/m5x 惯例），全部真实执行后才 commit。
- 数据真实性：/proc 采集与 `cat /proc/…` 直接对拍；注意容器 /proc 为宿主命名
  空间 → 对拍"解析正确/非负/格式一致"，不断言宿主语义绝对值。
- 双形态：x86 pxc build（dogfood 主打，静态 ELF）；aarch64 交叉 + qemu --once
  （复用 m57_s4_cross_verify.sh 工具链与库）。
- 语言面零改动（预期）→ 不触发全量自举；仅当新增 builtin 才走 capability 双模式
  + diffcheck + pxi 重建 + 自举证明（M57-S5 流程）。
- 常驻 daemon 不裸奔验证：一律 `--n N` / background + 日志 + 有限秒，
  规避执行超时(120s) 兜底。

## 六、风险与边界

- 容器无 hwmon/thermal temp → 温度"真读"留真板子/真机（宿主有 hwmon 时 open 即
  真内核路径，代码分支同）；本里程碑验证降级分支 + 条件探测逻辑。
- /proc 字段跨内核版本差异（字段名/顺序基本稳定）：解析按字段名匹配，
  parse 失败 → 该指标降级，不整体崩溃。
- mmap 共享文件在多写者无锁 → daemon 单写者 + 读进程只读快照区（写入原子性靠
  mem_write 单次 + 读方容忍瞬态），文档写明；不做复杂锁（边缘场景够用）。
- 手写 HTTP 覆盖最小端点（/healthz、/、404），非完整 Web 服务器；并发请求按
  单连接顺序处理 + spawn 可选，文档写明边界。
- 执行超时 120s：verify 内所有常驻/长跑均 --n + background；单步超预算任务拆开。
- dogfood 撞语言欠账（MINI_SUBSET）→ 如实记录 + 小修评估；不因 dogfood 扩语言
  面而偏离"锁定语言面"主线纪律。
- 多文件 import 已实测可用，但 pxi（解释器 Mini 子集）对相对路径 import 的支持
  未验证 → 本里程碑不承诺解释模式跑 daemon；如需在 S5 补验（非阻塞项）。

## 七、进度记录（每子步 commit 后回填，参照 M57_PLAN 惯例）

| 子步 | 状态 | commit / 验证摘要 |
|---|---|---|
| S1 | ✅ | commit 1694720：多文件 import 工程（collect.px + main.px，<500行/文件）；collect 采 /proc（CPU 两次采样差值/内存/负载/uptime，M57 fd 通道 open+read+close）；main --once/--n/--interval/常驻 + env PXHWMON_INTERVAL；verify_s1.sh 全 PASS（静态 ELF、mem_total 与 MemTotal 精确一致、load1/5/15 与 /proc/loadavg 对拍、cpu∈0..100、行数<500） |
| S2 | ✅ | commit c821005：collect.px 追加 temp_read（hwmon/thermal 条件探测，缺 → na 降级）+ net_dev（非 lo rx/tx 汇总）；shm.px（4096B MAP_SHARED 共享区：快照区+控制区，prepare/write_snap/read_ctl/write_ctl/dump）；main.px --dump/--ctl/--shm/--no-shm + SNAP 行含 temp/net/ctl；verify_s2.sh 全 PASS（temp 降级、net 非负、daemon→dump 活读、ts 递增、外部写 ctl → 下轮快照回显双向闭环、daemon 有限轮次退出 0）；verify_s1 回归 PASS |
| S3 | ✅ | commit b92be61：serve.px 手写最小 HTTP（tcp_listen/accept + 显式响应头 Content-Type/Content-Length/Connection/Server，/healthz JSON + / HTML 表格降级标记 + 404，数据源=mmap 快照活读）；notify.px 阈值 env（MEM_AVAIL_KB/LOAD1/TEMP_MC）→ 告警日志 + webhook dry-run（http_post 失败 panic 欠账记录）；main --port/--no-http + spawn serve + check_alerts；verify_s3 全 PASS（响应头断言 + 告警/webhook dryrun 各 3 行）；verify_s1/s2 回归 PASS（s1 修复 --no-http+SNAP 过滤） |
| S4 | ✅ | commit 25aa4bb：run.sh 崩溃自愈 wrapper（kill -9→137→attempt 2 自动拉起；0/130/143 不重启；systemd 示例）；verify_s4.sh aarch64 交叉多文件 import 工程（--no-quic + aarch64 mbedtls/sqlite）→ file ARM aarch64 static-pie + qemu-aarch64 --once 采集真实 /proc（mem_total 与 MemTotal 一致）+ m57_s1/s3 回归 PASS 全绿（交叉 ~45s 后台跑） |
| S5 | ✅ | commit 04eccf0：examples/m58_hwmond/README.md（定位/特性/构建运行/env/HTTP/监控自愈通知/欠账边界）；CHANGELOG M58 条目（顶部）；ROADMAP 主线表 M58 行；README/README.en M58 行 + m58_hwmond 示例行 + 原生开发段 M41–M58；MINI_SUBSET §十三（dogfood 语言欠账 8 项：http_post 失败 panic + spawn 不隔离最优先）；M58_PLAN S4 行回填；里程碑闭环（状态✅，全仓回归绿） |
