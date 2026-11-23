#!/usr/bin/env python3
"""M437 worker 协议测试装置（门 selfhost/check_m437_worker.sh 的判据实现）。

为什么单独成文件而不是 heredoc（实测教训）：
  门里用 `python3 - <<'PY'` 时 heredoc 占着 python 的 stdin，与「给 worker 写帧」的
  subprocess.PIPE 混在一起排查成本高；同一段逻辑独立成文件后行为可复现、可单独跑。
用法：python3 -u selfhost/check_m437_worker_probe.py <pxi路径> <工作目录>
输出：每行 "OK <判据名> ..." 或 "NG <判据名> ..."；全部通过退出 0，否则 1。
"""
import fcntl
import os
import signal
import struct
import subprocess
import sys

signal.alarm(180)   # 装置 bug 不许把门挂死（超时自杀 ⇒ 判据红）


def frame(path, env="", dump="0", tmo=5000):
    p = (path + "\0" + env + "\0" + dump + "\0" + str(tmo)).encode()
    return struct.pack(">I", len(p)) + p


def read_frame(f):
    h = f.read(4)
    if len(h) < 4:
        return None
    n = struct.unpack(">I", h)[0]
    b = b""
    while len(b) < n:
        c = f.read(n - len(b))
        if not c:
            return None
        b += c
    code, _, out = b.partition(b"\0")
    try:
        code = int(code)
    except ValueError:
        return None
    return code, out


def worker_pipe():
    """把结果通道写端放到 fd 3。

    ⚠️ 不能直接 os.dup2(w, 3)：当 w 恰好等于 3 时那是空操作，随后的 os.close(w)
    会把唯一副本关掉（实测症状：worker 写 EBADF 后退出、父端 read 永久阻塞）。
    """
    r, w = os.pipe()
    # ⚠️ 读端也必须先挪到高位：pipe() 若恰好返回 r=3，下面的 os.dup2(hi, 3) 会把**读端**
    #   覆盖成写端 ⇒ 父端拿到的 fd 3 成了只写端，`rf.read(4)` 报 EBADF
    #   （11-23 M437 全量门实测：probe P1/P2/P4/P5/N1/N2 全红，根因就在这里）。
    r_hi = fcntl.fcntl(r, fcntl.F_DUPFD, 20)
    os.close(r)
    hi = fcntl.fcntl(w, fcntl.F_DUPFD, 10)
    os.close(w)
    os.dup2(hi, 3)
    os.close(hi)
    return r_hi


def main():
    pxi = sys.argv[1]
    work = sys.argv[2]
    ok_px = os.path.join(work, "t_ok.px")
    err_px = os.path.join(work, "t_err.px")
    miss_px = os.path.join(work, "no_such_file.px")
    open(ok_px, "w").write('print("hello-" + str(6 * 7))\n')
    open(err_px, "w").write('print("before-exit")\nexit(7)\n')

    r3 = worker_pipe()
    p = subprocess.Popen([pxi, "--worker"], stdin=subprocess.PIPE,
                         stdout=subprocess.DEVNULL, stderr=subprocess.PIPE,
                         pass_fds=(3,))
    rf = os.fdopen(r3, "rb")

    def run(path):
        p.stdin.write(frame(path))
        p.stdin.flush()
        return read_frame(rf)

    fails = 0

    def rep(name, good, detail):
        nonlocal fails
        print(("OK " if good else "NG ") + name + " " + detail, flush=True)
        if not good:
            fails += 1

    # P1 单帧
    a = run(ok_px)
    rep("P1", a == (0, b"hello-42\n"), "单帧 → %r" % (a,))
    # P2 同一 worker 连续 3 帧（脚本级状态重置）
    seq = [run(ok_px) for _ in range(3)]
    rep("P2", all(x == (0, b"hello-42\n") for x in seq), "同 worker 连 3 帧 → %r" % (seq,))
    # P4 运行时错误传播（**不用 exit()**：exit 会终止 worker 自身 —— 见下方 P5 的说明）
    bad_px = os.path.join(work, "t_bad.px")
    open(bad_px, "w").write('print("before-err")\nread_file("/nonexistent-m437-file")\n')
    e = run(bad_px)
    rep("P4", e is not None and e[0] != 0, "运行时错误 → %r" % (e,))
    # P5 错误后 worker 仍能服务下一帧（状态恢复 + 进程存活）
    #   ⚠️ 已知边界：脚本调用 exit(N) 会**终止 worker 进程**（exit 是进程级原语），
    #   父进程侧表现为「worker 崩溃」⇒ respawn + 退化 fork+exec，最终结果仍正确（只是慢一次）。
    #   要支持 exit 语义需在解释器里做「仅中止当前脚本」的控制流，列为后续项。
    a2 = run(ok_px)
    rep("P5", a2 == (0, b"hello-42\n"), "错误后下一帧 → %r" % (a2,))
    # N1 缺失路径不得静默成功
    m = run(miss_px)
    rep("N1", m is not None and m[0] != 0, "缺失路径 → %r" % (m,))
    p.stdin.close()
    try:
        p.wait(timeout=15)
    except subprocess.TimeoutExpired:
        p.kill()
    err = p.stderr.read().decode("utf-8", "replace")
    if err.strip():
        print("worker stderr: " + err.strip()[:300], flush=True)

    # N2 越界帧必须响亮退出（rc=2 + 指名），不许 hang
    p2 = subprocess.Popen([pxi, "--worker"], stdin=subprocess.PIPE,
                          stdout=subprocess.DEVNULL, stderr=subprocess.PIPE)
    p2.stdin.write(struct.pack(">I", 65537))
    p2.stdin.flush()
    p2.stdin.close()
    try:
        rc2 = p2.wait(timeout=15)
    except subprocess.TimeoutExpired:
        p2.kill()
        rc2 = "TIMEOUT"
    err2 = p2.stderr.read().decode("utf-8", "replace")
    rep("N2", rc2 == 2 and "非法任务帧长度" in err2,
        "越界帧 rc=%r stderr=%r" % (rc2, err2.strip()[:80]))

    return 0 if fails == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
