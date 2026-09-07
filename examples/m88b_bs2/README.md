# M88-B-S2 验证归档（qg-issue 27 B 类：http 空闲连接事件驱动）

B-S2 把 `http_conn_worker` 从「阻塞处理连接整个生命周期」重构为「处理请求突发 + 空闲交还事件循环」：
- serve 连接 fd 一律非阻塞；请求读改用 `px_recv_wait`（poll+recv，语义 = 阻塞 recv + SO_RCVTIMEO 15s）；
- 响应写完且无下一请求数据在途 → `px_evc_idle_put` 交还 IDLE（B-S1 事件循环 epoll 照看），worker 释放去取新 job；
- 空闲 15s 超时 / 对端断开由事件循环 tick close（语义与原 SO_RCVTIMEO 对齐）；可读再派发回 fserve 池；
- 收尾统一 `px_evc_close`（清理连接上下文，防 fd 复用串扰）；非 Linux / fd 超 `PX_MAX_CONNS` 自动降级原阻塞路径，功能不降。

## 验证实录（2026-09-07，B-S2 后）
```
池容量 PX_SERVE_WORKERS=8 下：
[1] 首请求: 200/200 返回 200 (池容量=8)        ← A 类 8 worker 只能同时喂 8 连接，B-S2 空闲即释放
[2] 服务线程数: Threads: 10 (空闲连接 200/500 均 10)  ← 8 fserve + 1 事件循环 + 1 主；不随连接数涨
[3] 突发请求(空闲连接复用): 50/50 返回 200
[4] 15s 空闲超时语义: PASS（服务端 16s 后关闭空闲连接）
```
- 500 空闲连接挂载后线程数仍 10（连接数 500 > 池 8 → 空闲连接不占 worker 实证）。
- 回归：m82 http_serve_unix 8 项 PASS、m83_s6 同端口流式 SSE 5 项 PASS、短连接并发 100×300=30000 全 200 0 err。

## 文件
- `bs2_daemon.px`    服务端（http_serve TCP :18099；PX_SERVE_WORKERS=8 跑）
- `bs2_idle_check.py` 客户端验证（N 空闲长连接 + 突发 + 15s 超时语义）

## 用法
```
../../tools/pxc build --no-quic bs2_daemon.px
PX_SERVE_WORKERS=8 ./build/bs2_daemon &          # 后台
python3 bs2_idle_check.py 200 50
```
