#!/usr/bin/env python3
# ============================================================
# M197 判据 [S8]：**全内置 0 参探针** ⇄ **原生 arity 文案**（动态 · 解释轨）
# ------------------------------------------------------------
# 为什么需要它（M196 的 [S7] 的**盲区**）：
#   [S7] 是**静态**对拍「解释轨的字面量」⇄「原生的字面量/模板」；
#   但解释轨有一大类文案是**用变量拼出来的**（`cname + " 需要 1 个参数"`、
#   `"方法 " + name + " 需要 1 个参数"` …）—— 字面量的 head 是**变量**，
#   静态器只能看到 `" 需要 1 个参数"` 这种**无 head** 的碎片 ⇒ **看不见**。
#   M197 实测：正是这一类藏了 5 条 arity 文案分歧（sqrt/sin/cos/tan/log）
#   + 3 处**守卫顺序**错（os_spawn/os_kill/os_exec 修前 0 参撞 R1003 越界）。
# 判据：对**每一个内置名**（单一事实源 = `selfhost/interp.px` 的 names 列表）跑 `fn()`：
#   ① 必须得到 **R1002**（0 参 = 个数错）；
#   ② 文案必须**逐字命中**原生侧该函数的某条「arity 类」文案
#      （含 `需要`/`不接受` 且含 `参数`/`个`/`(`）。
#   ⚠️ 0 参**合法**或**有副作用**的内置列入 SKIP（表内给理由）。
# 用法：python3 sweep_arity.py [--root <仓库根>]     退出码 0=绿 1=红
# ============================================================
import os, re, subprocess, sys, importlib.util

ROOT = "/data/code/puxian"
for i, a in enumerate(sys.argv):
    if a == "--root" and i + 1 < len(sys.argv):
        ROOT = sys.argv[i + 1]

W = "/tmp/m197_probe"
os.makedirs(W, exist_ok=True)

SKIP = {
    "print": "0 参合法", "flush": "0 参合法", "print_err": "0 参合法",
    "now": "0 参合法", "now_ms": "0 参合法", "now_us": "0 参合法",
    "now_sec": "0 参合法", "now_ns": "0 参合法", "args": "0 参合法",
    "env": "0 参合法（无参 = 环境字典）", "list": "0 参合法（空列表）",
    "tuple": "0 参合法（空元组）", "dict": "0 参合法（空字典）",
    "random": "0 参合法", "hostname": "0 参合法", "tz_local": "0 参合法",
    "os_self_path": "0 参合法", "os_errno": "0 参合法", "object_id": "0 参合法",
    "isatty": "0 参合法", "exit": "有副作用（退出进程）",
    "unwrap": "0 参可能合法", "unwrap_err": "0 参可能合法",
    "onnx_op_names": "0 参合法（返回算子名单）", "onnx_model_open": "0 参会读文件",
    "read_file": "0 参走原生 arity（已由 [S7] 覆盖）",
    "file_stat": "0 参走原生 arity（已由 [S7] 覆盖）",
}


def collect_native():
    """原生侧：函数名 → 文案集合"""
    NAT = re.compile(r'px_error\(\s*"(R\d{4}):\s*([^"]*)"')
    out = {}
    for f in sorted(os.listdir(os.path.join(ROOT, "runtime"))):
        if not f.endswith(".c"):
            continue
        for line in open(os.path.join(ROOT, "runtime", f), encoding="utf-8"):
            st = line.lstrip()
            if st.startswith("//") or st.startswith("*") or st.startswith("/*"):
                continue
            for mm in NAT.finditer(line):
                head = re.match(r"^([A-Za-z_][A-Za-z0-9_]*)", mm.group(2).strip())
                if not head:
                    continue
                out.setdefault(head.group(1), set()).add(mm.group(2).strip())
    return out


def nat_arity(nat, fn):
    out = []
    for msg in nat.get(fn, ()):
        if ("需要" in msg or "不接受" in msg) and ("参数" in msg or "个" in msg or "(" in msg):
            out.append(msg)
    return sorted(set(out))


def main():
    nat = collect_native()
    src = open(os.path.join(ROOT, "selfhost/interp.px"), encoding="utf-8").read()
    mm = re.search(r'let names = \[(.*?)\]', src, re.S)
    if not mm:
        print("❌ 找不到内置名册（selfhost/interp.px 的 names 列表形态变了）")
        return 1
    names = re.findall(r'"([^"]+)"', mm.group(1))
    fails, nskip, nchk = [], 0, 0
    for fn in names:
        if fn in SKIP:
            nskip += 1
            continue
        p = os.path.join(W, fn + ".px")
        open(p, "w", encoding="utf-8").write('print("%s()")\n%s()\n' % (fn, fn))
        try:
            r = subprocess.run([ROOT + "/bootstrap/pxi", p], cwd=W,
                               capture_output=True, text=True, timeout=20)
        except subprocess.TimeoutExpired:
            fails.append("%s：0 参探针超时" % fn)
            continue
        out = r.stdout + r.stderr
        got = ""
        for line in out.splitlines():
            if "错误" in line or "error" in line.lower():
                got = line.strip()
                break
        m2 = re.search(r'\[(R\d{4})\]\s*(?:[0-9]+:[0-9]+:\s*)?(.*)$', got) or \
             re.search(r'\]\s*(R\d{4}):\s*(.*)$', got)
        code, text = (m2.group(1), m2.group(2)) if m2 else ("", got)
        nchk += 1
        if code != "R1002":
            fails.append("%s：0 参得到 %s（应为 R1002）——%s" % (fn, code or "(无码)", text[:60]))
            continue
        na = nat_arity(nat, fn)
        if na and text not in na:
            fails.append("%s：0 参文案「%s」不在原生 arity 文案里（原生：%s）"
                         % (fn, text[:50], " | ".join(x[:44] for x in na[:3])))
    print("── [S8] 全内置 0 参探针：共 %d 个（抽查 %d · 跳过 %d）· 不一致 **%d** ──"
          % (len(names), nchk, nskip, len(fails)))
    for f in fails:
        print("   ❌", f)
    return 1 if fails else 0


if __name__ == "__main__":
    sys.exit(main())
