#!/usr/bin/env python3
# ============================================================
# pair_runner.py —— 回环配对（server/client 示例）的端口分配 + 运行器
# ------------------------------------------------------------
# 为什么有它（缺陷 872 同族 · 顶层示例真跑的配对面）：
#   examples 里有 12 对 *_client.px / *_server.px（H3/QUIC/QPACK/pipe），
#   每对都把端口写死（17997/18010/18011/18020/18021…）。⇒ ① 并发跑必撞端口；
#   ② 上一对漏下的监听会让下一对假绿；③ 顶层示例门只能整对豁免 = 不跑。
#
# 做法：不改示例语义 —— 把源码里出现的固定端口字面量（17000..19999）
#   按「出现顺序」各自映射到一个现分配的空闲端口（保持相对关系，例如
#   TCP 18020 / UDP 18021 仍映射到两个不同端口），编译后 server/client 同映射。
#
# 判据（供门断言）：
#   P1 端口替换后 server 必须真的在**某个新端口**上监听（ss -lntu 可见，TCP/UDP 都算）
#      —— 这一条挡的是「替换没生效、实际连的还是旧端口」这类假绿
#   P2 server 与 client 退出码都必须为 0（示例自身的 assert 就是判据）
#   P3 收尾后新端口必须已释放（M430：门跑完不留监听）
#   N1 负控 --break-server：server 端口映射整体 +1000（client 不变）⇒ 该对必须失败，
#      且必须出现 P2-RC-FAIL（证明扰动真发生了，CRITERIA ④）
#
# 用法：
#   pair_runner.py --repo /data/code/puxian
#   pair_runner.py --repo DIR --pair m47_h3,m46_quic [--break-server] [--verbose]
#   pair_runner.py --repo DIR --list
# 退出码：0 符合预期（正控全绿 / 负控全失败）/ 1 不符合预期 / 3 环境不成立
# ============================================================
import argparse, os, re, socket, subprocess, sys, tempfile, time

PORT_RE = re.compile(r'\b1[7-9]\d{3}\b')
SERVER_WAIT = 35          # 最长的 server（m53_s3 pipe 等 25s）留足余量
CLIENT_TIMEOUT = 60


def free_port():
    s = socket.socket()
    s.bind(('127.0.0.1', 0))
    p = s.getsockname()[1]
    s.close()
    return p


def listening_ports():
    """ss 里所有 LISTEN 的本地端口（TCP+UDP）"""
    try:
        out = subprocess.run(['ss', '-lntuH'], capture_output=True, text=True, timeout=10).stdout
    except Exception:
        return set()
    ps = set()
    for line in out.splitlines():
        f = line.split()
        if len(f) < 5:
            continue
        lp = f[4].rsplit(':', 1)[-1]
        if lp.isdigit():
            ps.add(int(lp))
    return ps


def discover(repo):
    ex = os.path.join(repo, 'examples')
    pairs = []
    for f in sorted(os.listdir(ex)):
        if not f.endswith('_client.px'):
            continue
        prefix = f[:-len('_client.px')]
        srv = os.path.join(ex, prefix + '_server.px')
        if os.path.exists(srv):
            pairs.append((prefix, os.path.join(ex, f), srv))
    return pairs


def build(repo, src, outdir, tag):
    log = os.path.join(outdir, tag + '.build.log')
    with open(log, 'w') as fh:
        r = subprocess.run([os.path.join(repo, 'tools', 'pxc'), 'build', src],
                           cwd=repo, stdout=fh, stderr=subprocess.STDOUT)
    if r.returncode != 0:
        raise RuntimeError('build 失败：%s（见 %s）' % (src, log))
    return os.path.join(os.path.dirname(src), 'build', os.path.basename(src)[:-3])


def run_pair(repo, name, cli_src, srv_src, workroot, break_server=False, verbose=False):
    w = os.path.join(workroot, name)
    os.makedirs(w, exist_ok=True)
    res = []
    try:
        srv_text = open(srv_src, encoding='utf-8', errors='replace').read()
        cli_text = open(cli_src, encoding='utf-8', errors='replace').read()
    except OSError as e:
        return False, 'READ-FAIL ' + str(e)

    # 端口计划：server + client 里出现的端口字面量，按首次出现顺序各分配一个
    order = []
    for t in (srv_text, cli_text):
        for m in PORT_RE.finditer(t):
            if m.group(0) not in order:
                order.append(m.group(0))
    if not order:
        return False, 'NO-PORT-LITERAL（示例里没找到 17xxx-19xxx 端口字面量）'
    alloc = {old: free_port() for old in order}
    if break_server:
        # 负控：只扰动 server 侧映射（client 仍连原分配端口）
        srv_map = {k: v + 1000 if v + 1000 < 65535 else v - 1000 for k, v in alloc.items()}
    else:
        srv_map = alloc

    def rewrite(text, mapping, dst):
        def sub(m):
            return str(mapping.get(m.group(0), m.group(0)))
        open(dst, 'w', encoding='utf-8').write(PORT_RE.sub(sub, text))

    try:
        rewrite(srv_text, srv_map, os.path.join(w, 'srv.px'))
        rewrite(cli_text, alloc, os.path.join(w, 'cli.px'))
        res.append('map=%s%s' % (','.join('%s->%d' % (k, alloc[k]) for k in order),
                                 ' (server 故意偏移)' if break_server else ''))
        srv_bin = build(repo, os.path.join(w, 'srv.px'), w, 'srv')
        cli_bin = build(repo, os.path.join(w, 'cli.px'), w, 'cli')
    except RuntimeError as e:
        return False, 'BUILD-FAIL ' + str(e)

    # 1) 起 server，等它真在监听（TCP/UDP 任一新端口）
    new_ports = set(srv_map.values())
    srv_out_path = os.path.join(w, 'srv.out')
    srv_out = open(srv_out_path, 'w')
    srv = subprocess.Popen([srv_bin], cwd=repo, stdout=srv_out, stderr=subprocess.STDOUT)
    ok_listen = False
    try:
        deadline = time.time() + 6   # 就绪等 6s 足够；再等会撞上 15s 自终的示例（m53_s1）
        while time.time() < deadline:
            if srv.poll() is not None:
                break
            txt = open(srv_out_path, encoding='utf-8', errors='replace').read() if os.path.exists(srv_out_path) else ''
            # 优先「日志里出现 listening <新端口>」（服务自己宣布就绪）；
            #   仅当日志迟迟不出现且 ss 已见端口时才退到 ss —— 实测 m50/m51：
            #   只靠 ss 会在 server 的 accept 循环尚未就绪时开跑 ⇒ Connection refused
            if any(('listening %d' % p) in txt for p in new_ports):
                ok_listen = True
                break
            time.sleep(0.2)

        if not ok_listen and (listening_ports() & new_ports) and srv.poll() is None:
            ok_listen = True   # fallback：不打印端口的示例（如 m53_s1 只写 ready 文件）
        # P1 是**辅助证据**，不是终点判据：真正的契约是「client 连得上且两端 rc=0」。
        #   实测 m53_s1（QUIC echo，只写 ready 文件、不打印端口）在 ss 与日志都拿不到
        #   就绪信号，但它端到端是好的 ⇒ 不能因为「就绪信号看不见」把好示例判死。
        #   反面的挡板仍在：源码里找不到端口字面量 ⇒ NO-PORT-LITERAL 直接失败。
        if ok_listen:
            res.append('P1-LISTEN-OK')
        else:
            res.append('P1-LISTEN-WEAK(依 rc 判端到端)')

        # 2) 跑 client
        try:
            cr = subprocess.run([cli_bin], cwd=repo, timeout=CLIENT_TIMEOUT,
                                stdout=open(os.path.join(w, 'cli.out'), 'w'),
                                stderr=subprocess.STDOUT)
            cli_rc = cr.returncode
        except subprocess.TimeoutExpired:
            cli_rc = 'timeout'
        # 3) 收尾 server：client 成功 ⇒ 它若还在监听就是常驻服务（px_serve 型），
        #    给 5s 收尾后 kill（不白等 35s）；client 失败 ⇒ 等久一点，拿它的完整输出。
        try:
            srv_rc = srv.wait(timeout=5 if cli_rc == 0 else SERVER_WAIT)
        except subprocess.TimeoutExpired:
            # 实测（2026-11-25）：px_serve 产物对 SIGTERM 不立刻退出 ⇒
            #   必须 TERM → 等 2s → KILL 兜底，否则 wait() 会无限挂住（配对门会卡死）
            srv.terminate()
            try:
                srv.wait(timeout=2)
            except subprocess.TimeoutExpired:
                srv.kill()
                srv.wait()
            if cli_rc == 0:
                srv_rc = 0
                res.append('SERVER-PERSISTENT-KILLED')
            else:
                srv_rc = 'timeout'
    finally:
        if srv.poll() is None:
            srv.terminate()
            try:
                srv.wait(timeout=2)
            except subprocess.TimeoutExpired:
                srv.kill()
                srv.wait()
        srv_out.close()

    res.append('P2-RC-OK' if (cli_rc == 0 and srv_rc == 0) else
               'P2-RC-FAIL cli=%s srv=%s' % (cli_rc, srv_rc))

    # 4) 端口释放
    released = False
    for _ in range(50):
        if not (listening_ports() & new_ports):
            released = True
            break
        time.sleep(0.1)
    res.append('P3-RELEASE-OK' if released else 'P3-RELEASE-FAIL')

    actual_ok = (cli_rc == 0 and srv_rc == 0 and released and ok_listen)
    ok = (actual_ok != bool(break_server))
    if break_server:
        res.append('N1-AS-EXPECTED-FAIL' if ok else 'N1-NOT-FAILED(负控未生效)')
    if verbose and not actual_ok:
        for f in ('srv.out', 'cli.out'):
            p = os.path.join(w, f)
            if os.path.exists(p):
                res.append('[%s] %s' % (f, open(p, encoding='utf-8', errors='replace').read()[-300:]))
    return ok, ' | '.join(res)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument('--repo', default='/data/code/puxian')
    ap.add_argument('--pair', default='')
    ap.add_argument('--list', action='store_true')
    ap.add_argument('--break-server', action='store_true')
    ap.add_argument('--verbose', action='store_true')
    ap.add_argument('--work', default='')
    a = ap.parse_args()

    repo = os.path.abspath(a.repo)
    if not os.path.exists(os.path.join(repo, 'tools', 'pxc')):
        print('ENV: 缺 %s/tools/pxc' % repo)
        return 3
    pairs = discover(repo)
    if a.list:
        for n, c, s in pairs:
            print('%s\t%s\t%s' % (n, os.path.relpath(c, repo), os.path.relpath(s, repo)))
        print('TOTAL=%d' % len(pairs))
        return 0
    if not pairs:
        print('ENV: 没发现任何 *_client.px ↔ *_server.px 配对')
        return 3

    want = [x.strip() for x in a.pair.split(',') if x.strip()]
    if want:
        pairs = [p for p in pairs if p[0] in want]
        missing = set(want) - {p[0] for p in pairs}
        if missing:
            print('ENV: 未找到配对 %s' % ','.join(sorted(missing)))
            return 3

    work = a.work or tempfile.mkdtemp(prefix='pxpair.')
    os.makedirs(work, exist_ok=True)
    print('PAIR-RUNNER 配对=%d work=%s negative=%s' % (len(pairs), work, a.break_server))
    ok_n = fail_n = 0
    for name, cli, srv in pairs:
        t0 = time.time()
        ok, detail = run_pair(repo, name, cli, srv, work, a.break_server, a.verbose)
        print('%s %-22s %5.1fs  %s' % ('✅' if ok else '❌', name, time.time() - t0, detail))
        ok_n += 1 if ok else 0
        fail_n += 0 if ok else 1
    print('PAIR-SUMMARY total=%d ok=%d fail=%d negative=%s' %
          (len(pairs), ok_n, fail_n, a.break_server))
    return 0 if fail_n == 0 else 1


if __name__ == '__main__':
    sys.exit(main())
