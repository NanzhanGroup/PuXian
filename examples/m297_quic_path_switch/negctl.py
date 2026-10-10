#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M297 门 · 负控打桩器（精确替换，每步 assert 锚点唯一）。

用法：
    negctl.py apply A <runtime_quic.c>   # 收包时不传「真实来源」⇒ 服务端看不到新源 ⇒ 验证不触发
    negctl.py apply B <runtime_quic.c>   # local 退回 qc->local_sa ⇒ 客户端不回应 ⇒ 验证不完成

还原：verify.sh 用打桩前的快照 cp 回去（逐字节）。
"""
import sys

APPLY = {
    "A": (
        # 把 quic_pump 收包路径的 remote 从「真实来源」退回「已确认的对端地址」
        """            ngtcp2_path_storage_init(&sps, lsa_p, lsa_len, (const ngtcp2_sockaddr*)&from,
                                     (ngtcp2_socklen)flen, NULL);""",
        """            ngtcp2_path_storage_init(&sps, lsa_p, lsa_len,
                                     (const ngtcp2_sockaddr*)&qc->remote_sa,
                                     quic_sa_len(&qc->remote_sa), NULL);""",
        "A · 收包不传真实来源",
    ),
    "B": (
        # 把 local 从「ngtcp2 当前路径的本地地址」退回「qc->local_sa（可能已变）」
        """        const ngtcp2_sockaddr* lsa_p = (cur && cur->local.addr)
                                       ? (const ngtcp2_sockaddr*)cur->local.addr
                                       : (const ngtcp2_sockaddr*)&qc->local_sa;
        ngtcp2_socklen lsa_len = (cur && cur->local.addr)
                                 ? cur->local.addrlen : quic_sa_len(&qc->local_sa);""",
        """        const ngtcp2_sockaddr* lsa_p = (const ngtcp2_sockaddr*)&qc->local_sa;
        ngtcp2_socklen lsa_len = quic_sa_len(&qc->local_sa);
        (void)cur;""",
        "B · local 退回 qc->local_sa",
    ),
}


def main() -> int:
    if len(sys.argv) != 4 or sys.argv[1] != "apply":
        print(__doc__)
        return 2
    key, path = sys.argv[2], sys.argv[3]
    if key not in APPLY:
        print("未知负控键：%s（可选 %s）" % (key, ",".join(sorted(APPLY))))
        return 2
    old, new, label = APPLY[key]
    try:
        s = open(path, encoding="utf-8").read()
    except OSError as e:
        print("读取失败：%s" % e)
        return 2
    n = s.count(old)
    if n != 1:
        print("锚点不唯一 [%s]：count=%d（源码可能已变 ⇒ 打桩必须失败，不许静默）" % (label, n))
        return 3
    s = s.replace(old, new, 1)
    with open(path, "w", encoding="utf-8") as f:
        f.write(s)
    print("打桩完成：%s" % label)
    return 0


if __name__ == "__main__":
    sys.exit(main())
