#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""patch_file.py —— 零转义 · 失败响亮的「文件打桩」原语（候选 M304）

【它治什么】
门的负控标准打桩法一直是 `sed -i 's|OLD|NEW|' <目标>`，而 sed 有两类**静默失效**：
  ① 元字符陷阱：替换文本里的 & \\ $ \\1 与分隔符会被 sed 解释 ⇒ 结果被改写，或干脆匹配不到。
     （本仓实例：M302 缺陷 523 —— `\\u` 未转义 ⇒ sed 把模式当字面 u ⇒ 打桩静默失效）
  ② 找不到也 rc=0：OLD 不在文件里时 sed 什么都不做、**一声不吭** ⇒ 负控被抽掉牙，门照样打 ✅
     （本仓实例：M285 的守卫 A1「打桩锚点在位」就是为了抓这个而建的 —— 那是**事后扫描**）

【它把四件事变成结构上不可能】
  · 字面量匹配（无正则、无转义、无分隔符）      ⇒ 治 ①
  · 找不到 ⇒ rc≠0 且**文件一字不动**            ⇒ 治 ②
  · --count N 强制命中数（默认 1）              ⇒ 治 M286 的 A4「打桩范围超出声明意图」
  · --spec JSON（多行 / 复杂文本走文件）        ⇒ 调用点真正做到零转义

【用法】
  patch_file.py --file F --old 'A' --new 'B' [--count 1] [--dry-run]
  patch_file.py --spec patch.json            # {"file":F,"patches":[{"old":..,"new":..,"count":1}]}
  patch_file.py --file F --old-file a.txt --new-file b.txt [--count 1]

【退出码】
  0 已应用（或 --dry-run 检查通过）      1 找不到 OLD
  2 命中数与 --count 不符                3 用法 / IO 错误
  4 文件不是 UTF-8 文本

【输出】stdout 一行机读摘要；stderr 只在出错时说话。
  PATCH-OK file=<F> patches=<n> bytes_delta=<±n>
  PATCH-FAIL file=<F> patch=<i> reason=<not-found|count-mismatch|usage> expected=<n> actual=<n>
"""

import argparse
import json
import os
import sys
import tempfile

RC_OK = 0
RC_NOTFOUND = 1
RC_COUNT = 2
RC_USAGE = 3
RC_ENCODING = 4

TOOL = "patch_file"


def die(msg, rc, extra=""):
    sys.stderr.write("%s: %s\n" % (TOOL, msg))
    if extra:
        sys.stderr.write(extra)
    sys.exit(rc)


def fail_line(path, idx, reason, expected=None, actual=None):
    parts = ["PATCH-FAIL", "file=%s" % path, "patch=%s" % idx, "reason=%s" % reason]
    if expected is not None:
        parts.append("expected=%s" % expected)
    if actual is not None:
        parts.append("actual=%s" % actual)
    sys.stdout.write(" ".join(parts) + "\n")


def read_text(path):
    try:
        with open(path, "rb") as fh:
            raw = fh.read()
    except OSError as exc:
        die("读文件失败：%s (%s)" % (path, exc), RC_USAGE)
    try:
        return raw.decode("utf-8")
    except UnicodeDecodeError as exc:
        die("文件不是 UTF-8 文本：%s (%s)" % (path, exc), RC_ENCODING)


def read_arg_file(path):
    try:
        with open(path, "rb") as fh:
            return fh.read().decode("utf-8")
    except OSError as exc:
        die("读参数文件失败：%s (%s)" % (path, exc), RC_USAGE)
    except UnicodeDecodeError as exc:
        die("参数文件不是 UTF-8：%s (%s)" % (path, exc), RC_ENCODING)


def atomic_write(path, text):
    """同目录临时文件 + os.replace ⇒ 要么全新、要么全旧，绝不半截。"""
    directory = os.path.dirname(os.path.abspath(path))
    fd, tmp = tempfile.mkstemp(dir=directory, prefix="." + TOOL + ".", suffix=".tmp")
    try:
        with os.fdopen(fd, "wb") as fh:
            fh.write(text.encode("utf-8"))
        st = os.stat(path)
        os.chmod(tmp, st.st_mode & 0o7777)
        try:
            os.chown(tmp, st.st_uid, st.st_gid)
        except PermissionError:
            pass
        os.replace(tmp, path)
    except BaseException:
        try:
            os.unlink(tmp)
        except OSError:
            pass
        raise


def load_spec(args):
    """返回 (file, [(old, new, count), ...])。"""
    if args.spec:
        try:
            with open(args.spec, "rb") as fh:
                spec = json.loads(fh.read().decode("utf-8"))
        except OSError as exc:
            die("读 spec 失败：%s (%s)" % (args.spec, exc), RC_USAGE)
        except (ValueError, UnicodeDecodeError) as exc:
            die("spec 不是合法 JSON/UTF-8：%s (%s)" % (args.spec, exc), RC_USAGE)
        if isinstance(spec, dict):
            target = spec.get("file") or args.file
            items = spec.get("patches", [])
        else:
            target = args.file
            items = spec
        if not target:
            die("spec 里没有 file，且命令行也没给 --file", RC_USAGE)
        if not isinstance(items, list) or not items:
            die("spec 里 patches 为空或不是数组", RC_USAGE)
        out = []
        for i, it in enumerate(items):
            if not isinstance(it, dict) or "old" not in it or "new" not in it:
                die("spec.patches[%d] 缺 old/new" % i, RC_USAGE)
            out.append((it["old"], it["new"], int(it.get("count", 1))))
        return target, out

    if not args.file:
        die("必须给 --file（或 --spec）", RC_USAGE)
    if args.old is None and args.old_file is None:
        die("必须给 --old 或 --old-file", RC_USAGE)
    old = args.old if args.old is not None else read_arg_file(args.old_file)
    if args.new is not None:
        new = args.new
    elif args.new_file is not None:
        new = read_arg_file(args.new_file)
    else:
        die("必须给 --new 或 --new-file", RC_USAGE)
    if old == "":
        die("--old 不能是空串（空串匹配处处命中，无意义）", RC_USAGE)
    return args.file, [(old, new, int(args.count))]


def main():
    ap = argparse.ArgumentParser(add_help=True, description="零转义、失败响亮的文件打桩原语")
    ap.add_argument("--file")
    ap.add_argument("--old")
    ap.add_argument("--new")
    ap.add_argument("--old-file")
    ap.add_argument("--new-file")
    ap.add_argument("--count", type=int, default=1, help="要求 OLD 恰好出现 N 次（默认 1；-1 = 至少 1 次）")
    ap.add_argument("--spec", help="JSON：{\"file\":F,\"patches\":[{\"old\":..,\"new\":..,\"count\":1}]}")
    ap.add_argument("--dry-run", action="store_true", help="只检查、不写文件")
    args = ap.parse_args()

    path, patches = load_spec(args)
    text = read_text(path)
    orig = text
    applied = 0

    for idx, (old, new, want) in enumerate(patches, start=1):
        actual = text.count(old)
        if actual == 0:
            fail_line(path, idx, "not-found", want, actual)
            die("第 %d 处：文件中未找到 OLD（文件一字未改）" % idx, RC_NOTFOUND)
        if want >= 0 and actual != want:
            fail_line(path, idx, "count-mismatch", want, actual)
            die("第 %d 处：命中 %d 次，要求恰好 %d 次（文件一字未改）" % (idx, actual, want), RC_COUNT)
        text = text.replace(old, new)
        applied += 1

    changed = text != orig
    delta = len(text) - len(orig)

    if args.dry_run:
        sys.stdout.write(
            "PATCH-OK file=%s patches=%d bytes_delta=%+d dry_run=1 changed=%d\n"
            % (path, applied, delta, 1 if changed else 0)
        )
        return RC_OK

    if changed:
        atomic_write(path, text)
    sys.stdout.write(
        "PATCH-OK file=%s patches=%d bytes_delta=%+d dry_run=0 changed=%d\n"
        % (path, applied, delta, 1 if changed else 0)
    )
    return RC_OK


if __name__ == "__main__":
    sys.exit(main())
