# pxhwmond —— PuXian 硬件健康守护 daemon（M58 dogfood）

> **M58 里程碑落地载体**：用 PuXian 写的**真实边缘应用**——单静态二进制硬件健康
> 守护 daemon。把 M57 的能力（fd 原语数据通道、mmap 活映射、aarch64 交叉编译）
> 揉进完整真实程序，x86 实测 + aarch64 交叉 qemu 验证，暴露真实语言/工具链缺口
> （1→1.0n 的验证闭环，与项目「真实用户喂 bug」哲学一致）。
> 定位：Linux 边缘设备（树莓派 / 网关 / 盒子）硬件健康采集 + 状态页守护进程。

## 特性

| 能力 | 实现 |
|---|---|
| 采集（真实 /proc） | CPU 使用率（两次采样差值）、内存（MemTotal/Free/Available…）、负载（1/5/15）、运行时长、网络流量（非 lo 汇总）——全部走 **M57 fd 数据通道** `open`+`read`+`close` |
| 温度（条件降级） | hwmon `temp*_input` / thermal_zone `temp` 条件探测，存在才读；全缺 → `temp=na`（当前容器即此态，真板子 open 即真内核路径） |
| mmap 活映射共享 | 每轮快照 `mem_write` 进 **MAP_SHARED** 文件（`/tmp/pxhwmond.shm`，env 可换）；外部进程 `--dump` mmap 活读；控制区命令通道验证**双向活映射**（外部写 → daemon 下轮快照回显 `ctl=`） |
| HTTP 状态页 | 手写最小 HTTP/1.1（`tcp_listen/accept/read/write`）：`GET /healthz`（JSON）+ `GET /`（HTML 表格）+ 404；**显式响应头** Content-Type/Content-Length/Connection/Server（M57-S7 教训自验） |
| 监控 + 自愈 + 通知 | `run.sh` wrapper 崩溃自动重启（kill -9 → attempt 递增）+ 阈值告警（内存/负载/温度 env 阈值）→ 告警日志 + webhook dry-run 报文落盘 |
| 交叉产物 | `pxc build --no-quic` aarch64 静态二进制（daemon 无 H3 → 天然裁剪），qemu-aarch64 跑 `--once` 采集真实 /proc（跨架构同源实证） |

## 目录结构

```
examples/m58_hwmond/
  main.px        入口：模式解析（--once/--n/--interval/--port/--dump/--ctl）+ 采集循环
  collect.px     /proc + /sys 采集（M57 fd 通道）+ SNAP 行解析 parse_snap
  shm.px         mmap MAP_SHARED 快照共享区（快照区 + 控制区，prepare/write/read/dump）
  serve.px       手写最小 HTTP 状态页（/healthz JSON + / HTML + 404，显式响应头）
  notify.px      阈值判定 + 告警日志 + webhook dry-run
  run.sh         daemon 启动 wrapper（崩溃自动重启 + systemd 示例）
  verify_s1..s4.sh  各子步自检（每步独立回归）
  verify_s3_client.px  HTTP 断言客户端（verify_s3.sh 使用）
  README.md      本文档
```

## 构建与运行

```bash
# x86_64 静态 ELF（多文件 import 工程，每文件 <500 行）
cd <repo-root>
tools/pxc build examples/m58_hwmond/main.px
./examples/m58_hwmond/build/main --once          # 单轮自检即退出
./examples/m58_hwmond/build/main --n 3 --interval 1   # 3 轮冒烟
./examples/m58_hwmond/build/main                 # 常驻 daemon（每 5s 一轮）
./examples/m58_hwmond/run.sh --port 19858        # 崩溃自动重启方式运行

# aarch64 交叉（复用 M57-S4 工具链：tools/cross_aarch64.sh 产物）
export PATH=/opt/aarch64-linux-musl-cross/bin:$PATH
tools/pxc build --no-quic --cc aarch64-linux-musl-gcc \
  --mbedtls-lib runtime/mbedtls/lib-aarch64 \
  --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
  examples/m58_hwmond/main.px   # 产物 examples/m58_hwmond/build/main（aarch64）
qemu-aarch64-static examples/m58_hwmond/build/main --once --no-shm
```

## 运行模式 / 参数

| 参数 | 说明 |
|---|---|
| `--once` | 采 1 轮即退出（CI / qemu / 回归自检） |
| `--n N` | 采 N 轮退出（有限时长冒烟） |
| `--interval N` | 采样间隔秒（默认 5） |
| `--port N` | HTTP 状态页端口（默认 19858；`--no-http` 关闭） |
| `--dump` | 读 mmap 共享区首行快照后退出（外部活读） |
| `--ctl MSG` | 写 mmap 共享区控制区（外部命令通道模拟，验证双向活映射） |
| `--shm PATH` | 共享文件路径（默认 `/tmp/pxhwmond.shm`） |
| `--no-shm` | daemon 不写共享（纯输出） |

## 环境变量（SECURITY/P0：无配置文件、无密钥落盘）

| env | 作用 |
|---|---|
| `PXHWMON_INTERVAL` | 间隔秒（CLI `--interval` 优先） |
| `PXHWMON_SHM` | 共享文件路径 |
| `PXHWMON_PORT` | HTTP 端口 |
| `PXHWMON_ALERT_MEM_AVAIL_KB` | mem_avail < 阈值 → 内存不足告警 |
| `PXHWMON_ALERT_LOAD1` | load1 > 阈值 → 负载告警（float） |
| `PXHWMON_ALERT_TEMP_MC` | temp(毫°C) > 阈值 → 高温告警（temp=na 跳过） |
| `PXHWMON_ALERT_LOG` | 告警日志路径（默认 `/tmp/pxhwmond_alerts.jsonl`） |
| `PXHWMON_WEBHOOK` | webhook URL（dry-run 报文落盘，见边界） |
| `PXHWMON_WEBHOOK_DRYRUN_LOG` | dry-run 报文路径（默认 `/tmp/pxhwmond_webhook_dryrun.jsonl`） |

每轮输出：`SNAP ts=… cpu=… mem_total=… mem_avail=… load1=… load5=… load15=… up=… temp=… net_rx=… net_tx=… [ctl=…]`

## HTTP 状态页

```bash
./examples/m58_hwmond/build/main --port 19858 &   # 常驻
curl -s http://127.0.0.1:19858/healthz            # JSON：最新快照 + 心跳 + 状态
curl -s http://127.0.0.1:19858/                    # HTML 表格（temp=na 标降级）
```

数据源 = mmap 共享区**最新快照活读**（shm_dump）——状态页与 daemon 经共享内存 IPC，
体现 mmap 真实用途。响应头显式写 `Content-Type`/`Content-Length`/`Connection`/`Server`。

## 监控 · 自愈 · 通知（MONITORING/P0 落地）

- **监控**：`/healthz`（心跳 + 快照）+ mmap 共享区实时快照 + 告警日志
- **自愈**：`run.sh` wrapper（daemon 崩溃/kill -9 → 137 → 1s 自动拉起；0/130/143 优雅
  退出不重启；`PXHWMON_RESTART_MAX` 上限）——systemd 部署见 run.sh 头注释示例
- **通知**：阈值告警 → 告警日志 append + webhook **dry-run** 报文落盘（url/alert/ts JSON）

## 验证（examples 惯例，全部真实执行）

```bash
bash examples/m58_hwmond/verify_s1.sh   # 骨架 + /proc 采集（mem_total 与 MemTotal 精确对拍）
bash examples/m58_hwmond/verify_s2.sh   # 温度降级 + mmap 双向活映射（dump 活读/ctl 回显）
bash examples/m58_hwmond/verify_s3.sh   # HTTP 状态页（响应头断言）+ 阈值告警/webhook dryrun
bash examples/m58_hwmond/verify_s4.sh   # run.sh 崩溃自愈 + aarch64 交叉 qemu（~90s，建议后台）
```

## 已知语言欠账 / 边界（dogfood 撞到，记录不阻塞）

- **`http_post` 失败即 panic**（无错误返回），且 **spawn 协程不隔离 panic**（实测整个
  进程被杀）→ 长期 daemon 内**不能安全发起网络通知请求**：webhook 以 dry-run 报文落盘
  验证通知通道，真实网络发送待语言补 HTTP 客户端错误返回后启用。
- **`int()` 前缀截断**：`int("0.45")=0`、`int("123abc")=123`、`int("na")=0` → 数值
  转换须先确认纯整数串（本应用以字段白名单 + 字符串保留策略规避）。
- **`{}` 空 dict 字面量不可靠**（import 合并场景实测为 null）→ 动态 dict 用
  `json_parse("{}")` 创建。
- **import 只合并函数定义，不执行模块顶层** → 跨文件共享常量须函数内局部化（本应用
  模块无全局可变状态，函数式传递）。
- **mmap 固定 PROT_READ|PROT_WRITE** → 只读 fd 映射失败，共享文件须 O_RDWR 打开。
- 解释模式（pxi，Mini 子集）对相对路径 import / open+read / mmap 的完整支持**未承诺**
  （M58 主打编译模式全能力；如需补验属非阻塞项）。
- 容器无 hwmon/thermal temp → 温度"真读"留真板子/真机（降级分支 + 条件探测已验）；
  `/proc` 字段跨内核版本差异按字段名匹配解析，parse 失败该指标降级不崩。
- 手写 HTTP 覆盖最小端点（/healthz、/、404），非完整 Web 服务器（无 keep-alive/
  chunked）；正文全 ASCII 保证 Content-Length = len() 字节数。
- mmap 共享区单写者无锁（daemon 唯一写快照区；控制区消费式读清），写入原子性靠
  mem_write 单次 + 读方容忍瞬态，边缘场景够用。
