# M88-B-S4 验证归档（qg-issue 27 B 类：事件驱动量级压测 + 收口）

B-S4 验证 B 类目标（M88_PLAN §五 B 类验收）：**空闲 keep-alive / SSE 长连接不占 worker，
进程 OS 线程数 ≈ 池容量 + 常数，与连接数解耦**，连接量级上万。

## 验证实录（2026-09-07，B-S4 后）

```
[万级 http keep-alive]（PX_SERVE_WORKERS=8，s4_daemon :18099，s4_idle_press.py）
[1] 建立 10000 空闲 keep-alive（首请求+突发复用均 200）: 9720/10000
    （http keep-alive 空闲 15s 由事件循环 tick 关闭 = 设计语义；建立跨度 140s 内超窗连接被服务端正常关闭）
[2] 服务线程数: 10  （挂 9720 空闲连接；池=8 预期≈10，不随连接数涨）
[3] 2s 后线程数: 10（稳定）
[4] PASS —— 万级空闲连接挂载线程恒定（≈ 池容量 + 事件循环 + 主）

[短连接吞吐回归 ≥ A 类基线]（examples/m88_s3，http_serve_unix）
   并发 100×500 = 50000/50000 全 200、0 连接失败、0 err、进程不崩（69.6s，与 A 类基线 63s 同量级）
[SSE 长连接量级]（examples/m88b_bs3，bs3_sse_check.py）
   1000 SSE 长连接全建立收事件、3s 后 1000/1000 存活（SSE 空闲不超时）、线程恒 10、断开 500 重开 500 全成功
[回归总闸] m82 + m83_s1-s6 + m84_s1-s3 + m85_s1-s2 + m86_s0-s2 全部 rc=0（m83_s5/s6 收尾退出码已随
   af75807 修复归 0）；m23a SSE+WS PASS；自举证明 rc=0（B.c==golden 10595 行）；native 301 不变
```

## 文件
- `s4_daemon.px`     服务端（http_serve TCP :18099；PX_SERVE_WORKERS=8 跑）
- `s4_idle_press.py` 万级空闲 keep-alive 压测（并发建立 + 线程计数 + 窗口内突发复用）

## 用法
```
../../tools/pxc build --no-quic s4_daemon.px
PX_SERVE_WORKERS=8 ./build/s4_daemon &          # 后台
python3 s4_idle_press.py 10000 32               # 万级空闲连接（并发 32）
```

## 结论（B 类收口）
- B-S1 事件驱动内核（ConnCtx + 状态机 + epoll 事件循环）→ B-S2 http 空闲交还 → B-S3 SSE 长连接事件化
  + 注册表动态化 → **空闲连接不再占线程**：池 8 挂 1 万 idle keep-alive / 1000 SSE 线程恒 10。
- issue 27「并发即崩/exit」已根除（A）；连接量级 10³ → **1 万+**（B）；C 类（用户态协程 M:N）为 M88
  后续批次，按 M88_PLAN §四·A 顺序（VM 化 → 协程）另行立项。
