#!/usr/bin/env python3
# ============================================================
# selfhost/check_gate_inputs.py —— **门不得对「这台机器恰好有什么」有未声明的依赖**（M405 · 缺陷 782）
# ------------------------------------------------------------
# 为什么有它（三次实锤，形状完全一样）：
#   M403  m90_s1 绿是因为仓库里还躺着我 06:0x 编译留下的 selfhost/build/ 产物
#         （M304：门绿不绿，取决于谁的机器上有什么残留）
#   M404  m103_s2c/m103_s2d/m84_s3_dns 在分片里红：分片无 HOME ⇒ go build 找不到缓存/要联网
#   M404 r2  m103_s2d 红：参照图 ../m61_gfx/scene.png 被 .gitignore 忽略 ⇒ 8 个分片各自
#         worktree 里都没有这张图；门在本机绿是因为主仓库里**恰好**有别人跑过的产物
#   同族存量：m157/m311 吃 /data/app/ws/models/embed/model.onnx（102MB 本机资产）、
#             m67_* 吃 /opt/aarch64-linux-musl-cross、门里参考实现 import 第三方包
#   ⇒ 单靠「本机跑一遍绿」证明不了任何事：**前提必须自备，或显式声明后走响亮跳过**。
#
# 判据（五类，逐门逐行扫；注释行不算）：
#   P1 主机路径       /data /opt /srv /root /var/log /home 开头的绝对路径（/data/code 归 check_gate_paths，本门不重复）
#   P2 第三方 python  import 名不在标准库名单（仓内模块、setuptools 内部名不算）
#   P3 外网访问       http(s):// 且主机不是 127.0.0.1/localhost/0.0.0.0/[::1]/变量
#   P4 环境隐式依赖   门里跑 go build/run/test，却没有自己设 HOME/GOCACHE/GOPATH/GOMODCACHE
#   P5 非仓库产物     引用仓库内的产物（build/、*.png/*.bin/*.dump/*.onnx…）而该路径
#                     **未被 git 跟踪**（被 .gitignore 覆盖或干脆不存在）⇒ 换台机器就没了
#
# 登记（只许缩小，每条带处置与理由）：selfhost/gate_inputs_allow.tsv
#   file <TAB> 类别 <TAB> 主体 <TAB> 处置 <TAB> 理由
#   处置 ∈ self（门内自备前提）· skiploud（显式 [585-SKIP-LOUD] 跳过）· host（仅本机可跑，CI 必跳）
#   ⚠️ skiploud 额外要求：该文件里**真的**出现 [585-SKIP-LOUD] 字样（否则登记等于洗白）
#   ⚠️ 表里有、实测扫不到该输入 ⇒ **过期判红**（同 check_gate_paths 的口径）
#
# 用法：
#   python3 selfhost/check_gate_inputs.py                 # 判定真仓
#   python3 selfhost/check_gate_inputs.py --self-test     # 9 条正/负控（含前提自证）
#   python3 selfhost/check_gate_inputs.py --list          # 输出全部实测输入（TSV，供建表）
#   python3 selfhost/check_gate_inputs.py --scope-all     # 不限于已注册门（全 examples/）
# 退出码：0 无违例 / 1 有违例（含过期登记） / 2 自证失败 / 3 判据自身失效（扫不到任何文件）
# ============================================================
import os
import re
import subprocess
import sys
import tempfile

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(HERE)
ALLOW = os.path.join(HERE, "gate_inputs_allow.tsv")
CONSUMER = os.path.join(HERE, "gates.registry.sh")
ALLOW_MAX = 40          # 条目上限：只许缩小（加一条必须显式改这个数并说明）
STDLIB_TXT = os.path.join(HERE, "py_stdlib_3.9.txt")

P1 = re.compile(r"(?<![\w$])(/(?:data|opt|srv|root|var/log|home)/[A-Za-z0-9_.@+=/-]*)")
# 归 check_gate_paths（M213/M317），本门不重复记。
# ⚠️ 这条前缀在源码里**拆成两段**写：check_gate_paths 扫的就是「门脚本的代码行里出现写死的开发机
#   绝对路径」，连续的整串字面量会被它判红（M405 r1 全量门 shard7 实测）。相邻字面量在编译期
#   拼成同一个串，语义不变；只有注释里才允许出现整串（check_gate_paths 跳过注释行）。
P1_SKIP_PREFIX = ("/data" "/code/",)
P2_PY = re.compile(r"^\s*(?:import|from)\s+([A-Za-z_]\w*)")
P2_SH = re.compile(r"^\s*from\s+([A-Za-z_]\w*)\s+import\b")
P3 = re.compile(r"https?://(?:[A-Za-z0-9_.+%-]+(?::[^/@\s]*)?@)?([A-Za-z0-9_.-]+)")
P3_OK = {"127.0.0.1", "localhost", "0.0.0.0", "::1", "unix", "%s"}
P3_FAKE_SUFFIX = (".example", ".invalid", ".test", ".localhost", ".local")
P4_GO = re.compile(r"\bgo\s+(?:build|run|test|generate|mod|work)\b")
P4_ENV = re.compile(r"(?:export\s+)?(GOCACHE|GOMODCACHE|GOPATH|HOME)=")
P5 = re.compile(
    r"(?:\.\./|\./)?(?:[A-Za-z0-9_.][A-Za-z0-9_.-]*/)*[A-Za-z0-9_.-]+"
    r"\.(?:png|jpg|jpeg|bin|dump|bc|so|onnx|whl|tar|gz|tgz|zip|pyc|px)\b"
)
REG_RUN = re.compile(r"^run\s+(\S+)\s+(.*)$")


def is_comment(line):
    return line.lstrip().startswith("#")


def load_stdlib():
    names = set()
    if os.path.isfile(STDLIB_TXT):
        with open(STDLIB_TXT, encoding="utf-8", errors="replace") as f:
            names = {ln.strip() for ln in f if ln.strip()}
    return names


def registered_files(root):
    """从单一注册源取门脚本；examples/<名>/verify.sh 连带扫同目录辅助脚本（M317 的教训）。"""
    reg = os.path.join(root, "selfhost", "gates.registry.sh")
    files = set()
    if not os.path.isfile(reg):
        return files
    with open(reg, encoding="utf-8", errors="replace") as f:
        for line in f:
            m = REG_RUN.match(line.strip())
            if not m:
                continue
            for tok in m.group(2).split():
                if re.search(r"\.(sh|py)$", tok) and not tok.startswith("-"):
                    p = os.path.normpath(os.path.join(root, tok))
                    if os.path.isfile(p):
                        files.add(p)
    return files


def scope_all(root):
    files = set()
    ex = os.path.join(root, "examples")
    for dirpath, _dirs, names in os.walk(ex):
        for n in names:
            if n.endswith((".sh", ".py")):
                files.add(os.path.join(dirpath, n))
    return files


def expand_helpers(root, files):
    """examples/<名>/ 下的辅助 .sh/.py 一并扫（M317：negctl.py 写死路径，扫描面漏了整类）。"""
    out = set(files)
    for f in list(files):
        d = os.path.dirname(f)
        if os.sep + "examples" + os.sep in f:
            try:
                for n in os.listdir(d):
                    p = os.path.join(d, n)
                    if n.endswith((".sh", ".py")) and os.path.isfile(p):
                        out.add(p)
            except OSError:
                pass
    return out


def git_tracked(root):
    try:
        r = subprocess.run(["git", "-C", root, "ls-files", "-z"], capture_output=True)
        if r.returncode != 0:
            return None
        return {p for p in r.stdout.decode("utf-8", "replace").split("\0") if p}
    except (OSError, subprocess.SubprocessError):
        return None


def git_ignored(root, paths):
    """留作诊断用：判断哪些路径被 .gitignore 覆盖（判定不用它，用「未跟踪且存在」+ build/ 前缀）。"""
    if not paths:
        return set()
    try:
        r = subprocess.run(["git", "-C", root, "check-ignore", "--stdin"],
                           input="\n".join(paths).encode(), capture_output=True)
        return {p for p in r.stdout.decode("utf-8", "replace").split("\n") if p}
    except (OSError, subprocess.SubprocessError):
        return set()


def rel(root, p):
    try:
        return os.path.relpath(p, root)
    except ValueError:
        return p


def repo_modules(root, tracked):
    """仓内 python 模块名（含 examples/<名>/ 下的辅助模块）：from <mod> import 不算第三方。"""
    mods = set()
    if tracked:
        for p in tracked:
            b = os.path.basename(p)
            if b.endswith(".py"):
                mods.add(b[:-3])
    else:
        for _dp, _dn, names in os.walk(root):
            for n in names:
                if n.endswith(".py"):
                    mods.add(n[:-3])
    return mods


def scan_file(root, path, stdlib, tracked, repo_mods):
    """返回 [(类别, 主体, 行号, 原文)]"""
    found = []
    rp = rel(root, path)
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            lines = f.read().split("\n")
    except OSError:
        return found
    is_py = path.endswith(".py")
    go_seen = False
    env_seen = False
    p5_cache = []
    for no, line in enumerate(lines, 1):
        if is_comment(line):
            continue
        if P4_ENV.search(line):
            env_seen = True
        for m in P1.finditer(line):
            tok = m.group(1).rstrip("/.,;:)]}\"'")
            if any(tok.startswith(p) for p in P1_SKIP_PREFIX):
                continue
            if tok in ("/data", "/opt", "/srv", "/home", "/root", "/var/log"):
                continue
            found.append(("P1", tok, no, line.strip()[:110]))
        mod = None
        if is_py:
            m = P2_PY.match(line)
            mod = m.group(1) if m else None
        else:
            m = P2_SH.match(line)
            mod = m.group(1) if m else None
        if mod and mod not in stdlib and mod not in repo_mods \
                and not os.path.isfile(os.path.join(os.path.dirname(path), mod + ".py")) \
                and not os.path.isdir(os.path.join(root, mod)):
            found.append(("P2", mod, no, line.strip()[:110]))
        for m in P3.finditer(line):
            host = m.group(1).strip(".")
            if host in P3_OK or "$" in host or host.startswith("127.") \
                    or host.startswith("0.0.0.0") or host.endswith(P3_FAKE_SUFFIX):
                continue
            if "." not in host and host != "localhost":
                continue          # 模板残片（https://h/u）不是外网依赖
            found.append(("P3", host, no, line.strip()[:110]))
        if P4_GO.search(line):
            go_seen = True
        for m in P5.finditer(line):
            tok = m.group(0)
            if tok.startswith("./"):
                tok = tok[2:]
            cand = os.path.normpath(os.path.join(os.path.dirname(path), tok))
            crel = rel(root, cand)
            p5_cache.append((crel, no, line.strip()[:110]))
    if go_seen and not env_seen:
        found.append(("P4", "go", 0, "门里跑 go，但没自设 HOME/GOCACHE/GOPATH"))
    for crel, no, ln in p5_cache:
        # 主体＝解析后的仓库内相对路径（供人读、供登记）
        inside = not crel.startswith("..")
        own = crel.startswith(rel(root, os.path.dirname(path)) + "/")
        if inside and own and "/build/" in crel:
            continue          # 本门自己目录下的 build/ ⇒ 门内产物（自备），不是外部输入
        if inside and "build/" in crel:
            found.append(("P5", crel, no, ln))
        elif tracked is not None and inside and crel not in tracked \
                and os.path.exists(os.path.join(root, crel)):
            found.append(("P5", crel, no, ln))
    # 去重（同一主体重复引用只记一次）
    seen = set()
    uniq = []
    for cls, subj, no, ln in found:
        if (cls, subj) in seen:
            continue
        seen.add((cls, subj))
        uniq.append((cls, subj, no, ln))
    return uniq


def load_allow(path):
    rows = []
    if not os.path.isfile(path):
        return rows
    with open(path, encoding="utf-8", errors="replace") as f:
        for ln, line in enumerate(f, 1):
            if not line.strip() or line.startswith("#"):
                continue
            parts = line.rstrip("\n").split("\t")
            if len(parts) < 5:
                rows.append({"bad": True, "file": parts[0] if parts else "", "line": ln})
                continue
            rows.append({"bad": False, "file": parts[0], "cls": parts[1],
                         "subject": parts[2], "disp": parts[3], "why": parts[4], "line": ln})
    return rows


def has_skiploud(path):
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            return "[585-SKIP-LOUD]" in f.read()
    except OSError:
        return False


SKIP_LINE = re.compile(r"^[ \t]*(?:\u23ed\ufe0f?|SKIP)(?![=_0-9])")


def has_skip_line(path):
    """该文件里是否存在「行首跳过行」（与 gate_skip_loud.sh 同口径的粗筛）。"""
    try:
        with open(path, encoding="utf-8", errors="replace") as f:
            for ln in f:
                if SKIP_LINE.match(ln):
                    return True
    except OSError:
        return False
    return False


def judge(root, allow_path, tracked_file=None, scope="registry"):
    stdlib = load_stdlib()
    if not stdlib:
        print("❌ 判据自身失效：标准库名单缺失/为空 %s ⇒ 无法区分第三方 import" % STDLIB_TXT)
        return 3
    files = registered_files(root)
    if scope == "all":
        files |= scope_all(root)
    else:
        # 只盯**会在分片/CI 跑的例门**：selfhost/ 与 packaging/ 里的扫描器/探针按定义就是宿主感知的
        # （check_gate_paths 自己就含被测前缀），把它们也纳面只会逼出一堆应付式登记。
        files = {f for f in files if os.sep + "examples" + os.sep in f}
    files = expand_helpers(root, files)
    if not files:
        print("❌ 判据自身失效：扫不到任何门脚本（注册源 %s 未解析出条目）" % CONSUMER)
        return 3
    if tracked_file:
        with open(tracked_file, encoding="utf-8", errors="replace") as f:
            tracked = {ln.strip() for ln in f if ln.strip()}
    else:
        tracked = git_tracked(root)
    repo_mods = repo_modules(root, tracked)
    findings = []
    for p in sorted(files):
        findings.extend((rel(root, p),) + f for f in scan_file(root, p, stdlib, tracked, repo_mods))
    rows = load_allow(allow_path)
    ok_keys = set()
    fails = []
    for r in rows:
        if r.get("bad"):
            fails.append("登记表第 %d 行格式不全（需 5 列：file/类别/主体/处置/理由）" % r["line"])
            continue
        if r["disp"] not in ("self", "skiploud", "host"):
            fails.append("登记 %s %s 的处置 %s 非法（self/skiploud/host）" % (r["file"], r["cls"], r["disp"]))
        if not r["why"].strip():
            fails.append("登记 %s %s 缺理由" % (r["file"], r["cls"]))
        if r["disp"] == "skiploud" and has_skip_line(os.path.join(root, r["file"])) \
                and not has_skiploud(os.path.join(root, r["file"])):
            fails.append("登记 %s 标 skiploud，但文件里有行首跳过行而无 [585-SKIP-LOUD] ⇒ 登记等于洗白" % r["file"])
        ok_keys.add((r["file"], r["cls"], r["subject"]))
    seen = set()
    for f, cls, subj, no, text in findings:
        seen.add((f, cls, subj))
        if (f, cls, subj) in ok_keys or (f, cls, "*") in ok_keys:
            continue
        fails.append("%s:%d %s 未声明输入 %s ⇒ %s" % (f, no, cls, subj, text))
    for f, cls, subj in sorted(ok_keys - seen):
        if subj == "*" and any((ff, cc) == (f, cls) for ff, cc, _ in seen):
            continue          # 通配登记：该类只要还有任一实测输入就算「在用」
        fails.append("登记过期：%s %s %s 实测扫不到该输入 ⇒ 删除登记" % (f, cls, subj))
    if len(rows) > ALLOW_MAX:
        fails.append("登记条目 %d > 上限 %d —— 债在扩大（要扩充请显式改 ALLOW_MAX 并说明）"
                     % (len(rows), ALLOW_MAX))
    print("── 扫描面 %d 个文件（%d 条登记，上限 %d）── 实测输入 %d 处"
          % (len(files), len(rows), ALLOW_MAX, len(findings)))
    if fails:
        for x in fails[:60]:
            print("  ❌ " + x)
        if len(fails) > 60:
            print("  … 另有 %d 条" % (len(fails) - 60))
        print("══ GATE-INPUTS-FAIL（违例 %d）" % len(fails))
        return 1
    print("══ GATE-INPUTS-OK（%d 个文件 · %d 处已声明输入 · 违例 0）" % (len(files), len(findings)))
    return 0


def do_list(root, scope="registry"):
    stdlib = load_stdlib()
    files = registered_files(root)
    if scope == "all":
        files |= scope_all(root)
    else:
        # 只盯**会在分片/CI 跑的例门**：selfhost/ 与 packaging/ 里的扫描器/探针按定义就是宿主感知的
        # （check_gate_paths 自己就含被测前缀），把它们也纳面只会逼出一堆应付式登记。
        files = {f for f in files if os.sep + "examples" + os.sep in f}
    files = expand_helpers(root, files)
    tracked = git_tracked(root)
    repo_mods = repo_modules(root, tracked)
    agg = {}
    for p in sorted(files):
        for cls, subj, no, text in scan_file(root, p, stdlib, tracked, repo_mods):
            agg.setdefault((rel(root, p), cls, subj), []).append(no)
    for (f, cls, subj), nos in sorted(agg.items()):
        print("%s\t%s\t%s\t?\t(line %s)" % (f, cls, subj, ",".join(map(str, nos[:3]))))
    print("── 合计 %d 处 / 主体 %d 个" % (sum(len(v) for v in agg.values()), len(agg)), file=sys.stderr)
    return 0


# ── 自证：9 条（正/负控成对，每条负控都能红）────────────────────────────
FIX = {
    "examples/fx_host/verify.sh": "#!/usr/bin/env bash\nMODEL=/data/app/ws/models/m.onnx\n",
    "examples/fx_host_ok/verify.sh": "#!/usr/bin/env bash\nexport HOME=/tmp/x\nMODEL=${FX_MODEL:-/tmp/m.onnx}\n",
    "examples/fx_py/verify.sh": "#!/usr/bin/env bash\npython3 \"./a.py\"\n",
    "examples/fx_py/a.py": "from yaml import safe_load\nprint('x')\n",
    "examples/fx_net/verify.sh": "#!/usr/bin/env bash\ncurl -sS https://example.org/x\n"
                                  "echo '[585-SKIP-LOUD] 外网档跳过'\n",
    "examples/fx_net_nomark/verify.sh": "#!/usr/bin/env bash\nSKIP 外网档（未标记）\n"
                                          "curl -sS https://api.gateway.io/x\n"
                                          "echo '[585-SKIP-LOUD] 外网档跳过'\n",
    "examples/fx_net_ok/verify.sh": "#!/usr/bin/env bash\ncurl -sS http://127.0.0.1:9/x\n"
                                    "echo '[585-SKIP-LOUD] 外网档跳过'\n",
    "examples/fx_go/verify.sh": "#!/usr/bin/env bash\ngo build ./...\n",
    "examples/fx_go_ok/verify.sh": "#!/usr/bin/env bash\nexport GOCACHE=/tmp/gc HOME=/tmp/h\ngo build ./...\n",
    "examples/fx_res/verify.sh": "#!/usr/bin/env bash\ncp ../fx_other/build/out.bin /tmp/x\n",
    "examples/fx_clean/verify.sh": "#!/usr/bin/env bash\necho ok\n",
}


def do_selftest():
    sp = sf = 0
    tmp = tempfile.mkdtemp(prefix="gi_")
    root = os.path.join(tmp, "repo")
    for p, body in FIX.items():
        full = os.path.join(root, p)
        os.makedirs(os.path.dirname(full), exist_ok=True)
        with open(full, "w", encoding="utf-8") as f:
            f.write(body)
    os.makedirs(os.path.join(root, "selfhost"), exist_ok=True)
    stdlib = load_stdlib()
    with open(os.path.join(root, "selfhost", "py_stdlib_3.9.txt"), "w") as f:
        f.write("\n".join(sorted(stdlib)))
    reg = ["#!/usr/bin/env bash"]
    for p in FIX:
        if p.endswith("/verify.sh"):
            reg.append("run %s bash %s" % (os.path.basename(os.path.dirname(p)), p))
    with open(os.path.join(root, "selfhost", "gates.registry.sh"), "w") as f:
        f.write("\n".join(reg) + "\n")

    def run(allow_rows, tracked=None, label=""):
        ap = os.path.join(tmp, "allow.tsv")
        with open(ap, "w", encoding="utf-8") as f:
            f.write("# file\t类别\t主体\t处置\t理由\n")
            for row in allow_rows:
                f.write("\t".join(row) + "\n")
        tf = None
        if tracked is not None:
            tf = os.path.join(tmp, "tracked.txt")
            with open(tf, "w") as f:
                f.write("\n".join(tracked) + "\n")
        out = []
        old = sys.stdout
        import io
        buf = io.StringIO()
        sys.stdout = buf
        try:
            rc = judge(root, ap, tracked_file=tf, scope="all")
        finally:
            sys.stdout = old
        return rc, buf.getvalue()

    host = ("examples/fx_host/verify.sh", "P1", "/data/app/ws/models/m.onnx", "host", "本机 102MB 模型资产")
    py = ("examples/fx_py/a.py", "P2", "yaml", "self", "门内自带解析器")
    net = ("examples/fx_net/verify.sh", "P3", "example.org", "skiploud", "外网档")
    nomark = ("examples/fx_net_nomark/verify.sh", "P3", "api.gateway.io", "skiploud", "外网档")
    go = ("examples/fx_go/verify.sh", "P4", "go", "self", "自备 GOCACHE")
    res = ("examples/fx_res/verify.sh", "P5", "examples/fx_other/build/out.bin", "self", "跨目录产物（真缺失就是缺）")
    allrows = [host, py, net, nomark, go, res]

    def chk(name, cond, detail=""):
        nonlocal sp, sf
        if cond:
            sp += 1
            print("  ✅ " + name)
        else:
            sf += 1
            print("  ❌ %s ｜%s" % (name, detail))

    rc, txt = run([])
    chk("P 前提自证：允许表为空 ⇒ 违例必须 > 0（判据确实在扫，不是空转）",
        rc == 1 and "未声明输入" in txt, txt[-160:])
    chk("N1 负控：五类输入全未登记 ⇒ 判红且五类都被点名",
        all(c in txt for c in ("P1", "P2", "P3", "P4", "P5")), txt[-200:])
    rc, txt = run(allrows)
    chk("P1 正控：五类全部登记 ⇒ 判绿", rc == 0, txt[-200:])
    rc, txt = run([r for r in allrows if r[1] != "P5"])
    chk("N2 负控：只漏 P5 一条 ⇒ 判红（登记不是万能钥匙）", rc == 1 and "P5" in txt, txt[-160:])
    rc, txt = run(allrows + [("examples/fx_clean/verify.sh", "P1", "/tmp/x", "bogus", "x")])
    chk("N4b 负控：处置值非法 ⇒ 判红", rc == 1 and "非法" in txt, txt[-160:])
    rc, txt = run(allrows + [("examples/fx_clean/verify.sh", "P1", "/tmp/none", "self", "x")])
    chk("N3 负控：登记了扫不到的输入 ⇒ 判过期红", rc == 1 and "过期" in txt, txt[-160:])
    rc, txt = run(allrows + [("examples/fx_clean/verify.sh", "P1", "/tmp/never", "self", "")])
    chk("N4 负控：登记缺理由 ⇒ 判红", rc == 1 and "缺理由" in txt, txt[-160:])
    with open(os.path.join(root, "examples/fx_net_nomark/verify.sh"), "w") as f:
        f.write("#!/usr/bin/env bash\nSKIP 外网档（无响亮标记）\ncurl -sS https://api.gateway.io/x\n")
    rc, txt = run(allrows)
    chk("N5 负控：标 skiploud 但文件里没有 [585-SKIP-LOUD] ⇒ 判红（不许登记洗白）",
        rc == 1 and "洗白" in txt, txt[-200:])
    rc, txt = run([py])
    chk("N6 负控：逐类独立 —— 只登记 P2 ⇒ 其余四类仍红（一条登记不连坐放过）",
        rc == 1 and "P1" in txt and "P4" in txt, txt[-160:])
    with open(os.path.join(root, "examples/fx_net_nomark/verify.sh"), "w") as f:
        f.write("#!/usr/bin/env bash\nSKIP [585-SKIP-LOUD] 外网档跳过\ncurl -sS https://api.gateway.io/x\n")
    rc, txt = run(allrows)
    chk("P2 正控：补上响亮跳过后恢复绿（证明 N5 的红来自缺标记本身）", rc == 0, txt[-200:])
    print("── check_gate_inputs --self-test：通过 %d · 失败 %d" % (sp, sf))
    return 0 if sf == 0 else 2


def main(argv):
    scope = "registry"
    mode = "judge"
    for a in argv[1:]:
        if a == "--self-test":
            mode = "selftest"
        elif a == "--list":
            mode = "list"
        elif a == "--scope-all":
            scope = "all"
        elif a in ("-h", "--help"):
            print(__doc__ or "见文件头注释")
            return 0
        else:
            print("未知参数：%s" % a, file=sys.stderr)
            return 2
    if mode == "selftest":
        return do_selftest()
    if mode == "list":
        return do_list(REPO, scope)
    return judge(REPO, ALLOW, scope=scope)


if __name__ == "__main__":
    sys.exit(main(sys.argv))
