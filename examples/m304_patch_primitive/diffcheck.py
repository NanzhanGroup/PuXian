#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""M304 差分驱动器 —— 打桩原语 ⇄ wst-edit 2.1.0，外加 sed 对照。

三层，每层给一行机读结论（verify.sh 只认这些行）：
  DIFF  n=<例数> pass=<n> fail=<n>      两边**语义类 + 落盘字节 + 权限**全等
  SED   applicable=<n> differ=<n> silent=<n>    sed 的失效现场（形状层证据）
  SHAPE case=<名> class=<...>            逐例明细（只给 sed 层，便于人读）

语义类归一（两边 rc 口径不同，比的是**语义**不是数字）：
  OK / NOTFOUND / COUNT / USAGE

⚠️ wst-edit 只在**东月所在机器**上存在（/data/app/ws/tools/wst-edit），
   CI runner 上没有 ⇒ 用 `--wedit -` 显式跳过（**响亮记一行 SKIP，不静默**，M215 先例）。
   sed 层到处都有 ⇒ 它是 CI 上永远有牙的那一层。
"""
import argparse
import hashlib
import json
import os
import shutil
import subprocess
import sys

sys.path.insert(0, os.path.dirname(os.path.abspath(__file__)))
from cases import CASES, SED_APPLICABLE  # noqa: E402

PATCH_RC = {0: "OK", 1: "NOTFOUND", 2: "COUNT", 3: "USAGE", 4: "ENCODING"}


def sha_file(p):
    with open(p, "rb") as fh:
        return hashlib.sha256(fh.read()).hexdigest()[:16]


def sha_of_text(text):
    return hashlib.sha256(text.encode("utf-8")).hexdigest()[:16]


def classof_wedit(rc, out, err):
    blob = (out + "\n" + err).replace(" ", "")
    if rc == 0 and '"ok":true' in blob:
        return "OK"
    if "未找到" in blob or "notfound" in blob.lower():
        return "NOTFOUND"
    if "命中" in blob and ("不符" in blob or "期望" in blob or "expect" in blob.lower()):
        return "COUNT"
    return "OTHER(rc=%d)" % rc


def run_patch(binpath, path, ops):
    if len(ops) == 1:
        o = ops[0]
        cmd = [sys.executable, binpath, "--file", path, "--old", o["old"],
               "--new", o["new"], "--count", str(o["count"])]
    else:
        sp = path + ".spec.json"
        with open(sp, "w", encoding="utf-8") as fh:
            json.dump({"file": path,
                       "patches": [{"old": o["old"], "new": o["new"], "count": o["count"]}
                                   for o in ops]}, fh)
        cmd = [sys.executable, binpath, "--spec", sp]
    p = subprocess.run(cmd, capture_output=True, text=True)
    return PATCH_RC.get(p.returncode, "OTHER(rc=%d)" % p.returncode), p.stdout, p.stderr


def run_wedit(binpath, path, ops):
    if len(ops) == 1:
        o = ops[0]
        payload = {"file": path, "old_text": o["old"], "new_text": o["new"],
                   "expect": o["count"]}
    else:
        payload = {"edits": [{"file": path, "old_text": o["old"], "new_text": o["new"],
                              "expect": o["count"]} for o in ops]}
    p = subprocess.run([binpath], input=json.dumps(payload), capture_output=True, text=True)
    return classof_wedit(p.returncode, p.stdout, p.stderr), p.stdout, p.stderr


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--patch", required=True, help="被测打桩原语（patch_file.py）")
    ap.add_argument("--wedit", default="-", help="wst-edit 路径；'-' = 跳过该层")
    ap.add_argument("--work", default="/tmp/m304_diff")
    ap.add_argument("--verbose", action="store_true")
    a = ap.parse_args()

    shutil.rmtree(a.work, ignore_errors=True)
    os.makedirs(a.work)

    npass = nfail = 0
    ref = {}
    for name, content, ops in CASES:
        da, db = os.path.join(a.work, "a"), os.path.join(a.work, "b")
        for d in (da, db):
            shutil.rmtree(d, ignore_errors=True)
            os.makedirs(d)
        fa, fb = os.path.join(da, "f.txt"), os.path.join(db, "f.txt")
        for f in (fa, fb):
            with open(f, "w", encoding="utf-8") as fh:
                fh.write(content)
            os.chmod(f, 0o640)

        ca, oa, ea = run_patch(a.patch, fa, ops)
        sa, ma = sha_file(fa), oct(os.stat(fa).st_mode & 0o777)
        ref[name] = (ca, sa, ma)

        if a.wedit == "-":
            continue
        cb, ob, eb = run_wedit(a.wedit, fb, ops)
        sb, mb = sha_file(fb), oct(os.stat(fb).st_mode & 0o777)
        same = (ca == cb) and (sa == sb) and (ma == mb)
        npass += same
        nfail += (not same)
        if not same:
            print("DIFF-MISMATCH case=%s patch=(%s,%s,%s) wedit=(%s,%s,%s)"
                  % (name, ca, sa, ma, cb, sb, mb))
            if a.verbose:
                print("   patch out=%r err=%r" % (oa[-200:], ea[-200:]))
                print("   wedit out=%r err=%r" % (ob[-200:], eb[-200:]))

    if a.wedit == "-":
        print("DIFF  SKIPPED（本机无 wst-edit ⇒ 差分层未执行 · 只跑了 sed 对照层）")
    else:
        print("DIFF  n=%d pass=%d fail=%d" % (len(CASES), npass, nfail))

    # ── sed 对照：证明「这四类形状确实存在」，不是纸面推理 ──
    differ = silent = 0
    for name, content, ops in SED_APPLICABLE:
        dc = os.path.join(a.work, "c")
        shutil.rmtree(dc, ignore_errors=True)
        os.makedirs(dc)
        fc = os.path.join(dc, "f.txt")
        with open(fc, "w", encoding="utf-8") as fh:
            fh.write(content)
        o = ops[0]
        p = subprocess.run(["sed", "-i", "s|" + o["old"] + "|" + o["new"] + "|", fc],
                           capture_output=True, text=True)
        sc = sha_file(fc)
        want = ref[name][1]
        d = (sc != want)
        # 「静默失效」的精确形状 = **本该响亮失败**（原语判 NOTFOUND）而 sed **rc=0 且一字未改**
        #   ⇒ 调用点看不出任何异常，负控被抽掉牙。注意：此时字节与正确结果**相同**，
        #     所以只比字节是看不见它的 —— 必须比**语义类**。
        silent_noop = (p.returncode == 0 and sc == sha_of_text(content)
                       and ref[name][0] == "NOTFOUND")
        differ += d
        silent += silent_noop
        print("SHAPE case=%s rc=%d class=%s" % (name, p.returncode,
                                                "DIFFER" if d else "same"))
        if silent_noop:
            print("SHAPE-SILENT case=%s（无命中却 rc=0 —— sed 的静默失效）" % name)
    print("SED   applicable=%d differ=%d silent=%d"
          % (len(SED_APPLICABLE), differ, silent))

    return 0 if nfail == 0 else 1


if __name__ == "__main__":
    sys.exit(main())
