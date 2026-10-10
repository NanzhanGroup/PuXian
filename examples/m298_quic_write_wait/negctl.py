#!/usr/bin/env python3
# M298 负控打桩器（门用）——只改 runtime/runtime_quic.c 的**写流数据**那一段。
# 设计纪律（M161/M164/M226/M227/M213 反复踩过）：
#   · 锚点**必须唯一**（命中数 != 1 即拒绝打桩，避免改到别处）
#   · 撤回要**忠实**（A 用 M297 的原文本整段换回，不是"随便改坏"）
#   · 打桩器自带 --selftest，门的 [1] 层先跑它
import sys

NEW_LOOP_START = "    // M298（缺陷 514）：**等到真的写进去**，或者**响亮失败**。"
WOFF_ANCHOR = "    size_t woff = 0;\n    int64_t total = 0;\n    int spins = 0;\n"
TAIL_ANCHOR = "    return total;\n}\n\n// 从指定流读取"

OLD_LOOP = """    size_t woff = 0;
    int64_t total = 0;
    int spins = 0;
    while (woff < len && spins < 16) {
        uint8_t out[QUIC_PKT_BUF];
        ngtcp2_vec v = { (uint8_t*)data + woff, len - woff };
        ngtcp2_ssize ndone = 0;
        ngtcp2_path_storage ps;                 // M297（缺陷 504）：发送目标由 ngtcp2 给出
        ngtcp2_path_storage_zero(&ps);
        ngtcp2_ssize n = ngtcp2_conn_writev_stream(qc->conn, &ps.path, NULL,
                                                   out, sizeof(out), &ndone,
                                                   NGTCP2_WRITE_STREAM_FLAG_NONE,
                                                   sid, &v, 1, quic_now());
        if (n > 0) {
            quic_send_to_path(qc, out, (size_t)n, &ps.path);
        } else if (n < 0 && n != NGTCP2_ERR_WRITE_MORE) {
            return -1;
        }
        if (ndone > 0) { woff += (size_t)ndone; total += (int64_t)ndone; }
        if (ndone == 0 && n == 0) {
            // 流控阻塞：泵一次等 ACK 释放窗口
            int pr = quic_pump(qc, 1000, 0, -1);
            if (pr != 0) break;
        }
        spins++;
    }
    return total;
}

// 从指定流读取"""

PUMP_CALL = """            int64_t t0 = quic_now();
            int pr = quic_pump(qc, QUIC_WRITE_PUMP_MS, 1, sid);
            if (pr == -2) break;                // 真错误（对端关闭/协议错误）
            // quic_pump 命中「本流已有数据/已 FIN」时会立即返回 ⇒ 防忙等
            if (quic_now() - t0 < (uint64_t)NGTCP2_MILLISECONDS) usleep(1000);"""
PUMP_NONE = """            int64_t t0 = quic_now();
            (void)t0;                           // NC-B：只等不推进（证明「推进」那半有牙）"""


def read(p):
    with open(p, encoding="utf-8") as f:
        return f.read()


def write(p, s):
    with open(p, "w", encoding="utf-8") as f:
        f.write(s)


def apply_A(path):
    """忠实换回 M297 原文（修前形态）：整段替换，其余一字不动。"""
    s = read(path)
    if s.count(NEW_LOOP_START) != 1 or s.count(WOFF_ANCHOR) != 1 or s.count(TAIL_ANCHOR) != 1:
        print("锚点不唯一/缺失（A）", file=sys.stderr)
        sys.exit(1)
    i = s.index(WOFF_ANCHOR)
    j = s.index(TAIL_ANCHOR)
    if j <= i:
        print("锚点顺序异常（A）", file=sys.stderr)
        sys.exit(1)
    write(path, s[:i] + OLD_LOOP + s[j + len("    return total;\n}\n\n"):])


def apply_B(path):
    """保留 deadline（会等），但去掉推进（不 pump）。"""
    s = read(path)
    if s.count(PUMP_CALL) != 1:
        print("锚点不唯一（PUMP_CALL）", file=sys.stderr)
        sys.exit(1)
    write(path, s.replace(PUMP_CALL, PUMP_NONE, 1))


def selftest(path):
    s = read(path)
    bad = 0
    if s.count(NEW_LOOP_START) != 1:
        print("SELFTEST-FAIL: NEW_LOOP_START 不唯一"); bad = 1
    if s.count(WOFF_ANCHOR) != 1:
        print("SELFTEST-FAIL: WOFF_ANCHOR 不唯一"); bad = 1
    if s.count(TAIL_ANCHOR) != 1:
        print("SELFTEST-FAIL: TAIL_ANCHOR 不唯一"); bad = 1
    if s.count(PUMP_CALL) != 1:
        print("SELFTEST-FAIL: PUMP_CALL 不唯一"); bad = 1
    if "spins < 16" in s:
        print("SELFTEST-FAIL: 修前形态（spins < 16）仍在源码里"); bad = 1
    if "if (total == 0)" not in s:
        print("SELFTEST-FAIL: 缺「一字节未写入 ⇒ 响亮」分支"); bad = 1
    if "QUIC_WRITE_STALL_MS" not in s:
        print("SELFTEST-FAIL: 缺 deadline 常量"); bad = 1
    if "ngtcp2_conn_update_pkt_tx_time" in s:
        print("SELFTEST-FAIL: 混进了被否证的 E1 方案"); bad = 1
    if not bad:
        print("SELFTEST-OK: 4 个锚点各命中 1 次 · 修前形态已消失 · 响亮分支与 deadline 在位 · 无 E1 残留")
    return bad


def main():
    if len(sys.argv) < 3:
        print("用法: negctl.py --selftest <file> | apply A|B <file>", file=sys.stderr)
        return 2
    if sys.argv[1] == "--selftest":
        return selftest(sys.argv[2])
    if sys.argv[1] == "apply":
        which, path = sys.argv[2], sys.argv[3] if len(sys.argv) > 3 else ""
        if not path:
            print("缺文件参数", file=sys.stderr); return 2
        if which == "A":
            apply_A(path); print("NC-A 已打桩（忠实换回 M297 原文）")
        elif which == "B":
            apply_B(path); print("NC-B 已打桩（只等不推进）")
        else:
            print("未知负控", file=sys.stderr); return 2
        return 0
    print("用法: negctl.py --selftest <file> | apply A|B <file>", file=sys.stderr)
    return 2


if __name__ == "__main__":
    sys.exit(main())
