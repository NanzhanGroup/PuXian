#!/usr/bin/env python3
# -*- coding: utf-8 -*-
# ============================================================
# M53 S4：aioquic（Python 第三方独立 HTTP/3 实现）互操作客户端
# ------------------------------------------------------------
# 用途：验证 PuXian px_serve 托管的 HTTP/3 服务可与**第三方独立实现**
# 正常互操作（HTTP/3 外部互操作验证零突破）。非 PuXian 自 client ↔
# 自 server 回环，而是 aioquic(libp2p/pylsqpack/QPACK) ↔ PuXian(ngtcp2)
# 全栈独立实现间完成握手、QPACK、请求/响应。
# 用法：
#   python3 h3_aioquic_client.py HOST PORT URL...
#   例：python3 h3_aioquic_client.py 127.0.0.1 18021 /api/hi /hello.txt
# 输出（供 verify.sh 断言）：
#   AIOQUIC <path> <status> <body>
# ============================================================
import asyncio
import ssl
import sys

from aioquic.asyncio import QuicConnectionProtocol, connect
from aioquic.h3.connection import H3Connection
from aioquic.h3.events import DataReceived, HeadersReceived, H3Event
from aioquic.quic.configuration import QuicConfiguration
from aioquic.quic.events import ConnectionTerminated, QuicEvent


class H3Client(QuicConnectionProtocol):
    """最小 HTTP/3 客户端：每请求开一流，按流聚合头/体直到流结束。"""

    def __init__(self, *args, **kwargs):
        super().__init__(*args, **kwargs)
        self._http = H3Connection(self._quic)
        self._stream_events = {}   # stream_id -> [H3Event]
        self._waiters = {}         # stream_id -> asyncio.Future

    def quic_event_received(self, event: QuicEvent) -> None:
        if isinstance(event, ConnectionTerminated):
            for w in self._waiters.values():
                if not w.done():
                    w.set_exception(RuntimeError("connection terminated"))
            self._waiters.clear()
            return
        try:
            for hev in self._http.handle_event(event):
                sid = hev.stream_id
                evs = self._stream_events.setdefault(sid, [])
                evs.append(hev)
                w = self._waiters.get(sid)
                if w and not w.done() and self._stream_finished(evs):
                    w.set_result(evs)
        except Exception as exc:  # H3 协议错误 → 让等待方失败
            for w in self._waiters.values():
                if not w.done():
                    w.set_exception(exc)
            self._waiters.clear()

    @staticmethod
    def _stream_finished(evs):
        for e in evs:
            if isinstance(e, DataReceived) and e.stream_ended:
                return True
            if isinstance(e, HeadersReceived) and e.stream_ended:
                return True
        return False


async def request(client, authority, path, timeout=10):
    sid = client._quic.get_next_available_stream_id()
    loop = asyncio.get_event_loop()
    fut = loop.create_future()
    client._waiters[sid] = fut
    client._http.send_headers(
        sid,
        [
            (b":method", b"GET"),
            (b":scheme", b"https"),
            (b":authority", authority.encode()),
            (b":path", path.encode()),
        ],
        end_stream=True,
    )
    client.transmit()
    evs = await asyncio.wait_for(fut, timeout)
    status, body = "", b""
    for e in evs:
        if isinstance(e, HeadersReceived):
            for k, v in e.headers:
                if k == b":status":
                    status = v.decode()
        elif isinstance(e, DataReceived):
            body += e.data
    return status, body


async def amain(host, port, paths):
    authority = "%s:%d" % (host, port)
    config = QuicConfiguration(
        alpn_protocols=["h3"], is_client=True, verify_mode=ssl.CERT_NONE
    )
    async with connect(host, port, configuration=config, create_protocol=H3Client) as client:
        for p in paths:
            st, body = await request(client, authority, p)
            print("AIOQUIC %s %s %s" % (p, st, body.decode("utf-8", "replace")))


if __name__ == "__main__":
    if len(sys.argv) < 4:
        print("usage: h3_aioquic_client.py HOST PORT URL...", file=sys.stderr)
        sys.exit(2)
    host, port = sys.argv[1], int(sys.argv[2])
    paths = sys.argv[3:]
    asyncio.run(amain(host, port, paths))
