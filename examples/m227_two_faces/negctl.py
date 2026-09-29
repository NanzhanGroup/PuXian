#!/usr/bin/env python3
# ============================================================
# M227 负控锚点库 + 补丁/还原器
# ------------------------------------------------------------
# 锚点**与实际打的补丁同源**（由补丁脚本机械导出，不许手抄 —— 手抄锚点是本仓反复踩的坑：
#   M161/M164/M226/M227 各撞过一次「改了代码、旧门锚点失效」）。
#
# 用法：
#   negctl.py --selftest        自证：每条 NEW 在源码里**恰好 1 次**、OLD 为 0 次
#   negctl.py --revert A|B      忠实撤回该组（NEW → OLD）
#   negctl.py --apply  A|B      反向（OLD → NEW，复原用）
#   negctl.py --snapshot FILES  备份；--restore 从备份逐字节还原
# 判据：撤回后必须**判红**（H1 分叉回来），且**源逐字节还原**（`--restore` + `cmp`）。
# ============================================================
import argparse, json, os, shutil, sys

ROOT = os.environ.get('M227_ROOT', '/data/code/puxian')
SNAP = os.environ.get('M227_NEG_W', '/tmp/m227_neg')
EDITS = json.loads(r'''{
 "A": [
  {
   "file": "runtime/runtime.c",
   "old": "    if (args[0].type == PX_STR) {\n        if (args[1].type != PX_STR) return px_bool(false);\n        LXObject* h = args[0].as.obj;",
   "new": "    if (args[0].type == PX_STR) {\n        // M227（缺陷 338 · 「同名两门」）：修前**静默 `return px_bool(false)`** ——\n        //   同一次误用从**函数面**进得 `false`（**静默错值**），从**方法面**进\n        //   （`\"abc\".contains(7)`）得 `R1002: 方法 contains 参数 1 需要 string`。\n        //   子串只能与字符串比较（Python `'in <string>' requires string` 同为 TypeError）\n        //   ⇒ 三轨同码同文、响亮优于静默。\n        if (args[1].type != PX_STR)\n            px_error(\"R1002: contains 参数 2 需要 string，实际是 %s\", px_type_name(args[1]));\n        LXObject* h = args[0].as.obj;",
   "why": "缺陷 338 · bi_contains 子串类型检查"
  },
  {
   "file": "runtime/runtime.c",
   "old": "    px_error(\"R1002: contains 不支持类型 %s\", px_type_name(args[0]));\n    return px_null();\n}",
   "new": "    // M227（缺陷 337）：「同名两门」的能力对齐 —— 函数面 `contains({\"a\":1}, \"a\")`\n    //   修前响亮 `R1002 contains 不支持类型 dict`，而方法面 `({\"a\":1}).contains(\"a\")`\n    //   返回 true ⇒ **同一个操作两个门支持的类型集合不同**。字典按**键**判定\n    //   （= `d.has(k)` / `d.contains(k)`）；补齐函数面（删能力是退步）。\n    if (args[0].type == PX_DICT) {\n        if (args[1].type != PX_STR || !args[1].as.obj)\n            px_error(\"R1002: contains 参数 2 需要 string，实际是 %s\", px_type_name(args[1]));\n        return px_bool(px_dict_has(args[0], args[1].as.obj->as.str.data));\n    }\n    px_error(\"R1002: contains 不支持类型 %s\", px_type_name(args[0]));\n    return px_null();\n}",
   "why": "缺陷 337 · bi_contains 补 dict"
  },
  {
   "file": "runtime/runtime.c",
   "old": "        if (strcmp(name, \"len\") == 0) return px_int(px_len(obj));\n        if (strcmp(name, \"has\") == 0 || strcmp(name, \"contains\") == 0) {",
   "new": "        if (strcmp(name, \"len\") == 0) {\n            // M227（缺陷 341）：修前**完全没有 arity 检查** ⇒ `({\"a\":1}).len(1, 2)`\n            //   静默返回 1（而函数面 `len(d, 1, 2)` 响亮 `R1002 len 需要一个参数`）。\n            //   M226（缺陷 329）已把同族（`contains`/`has`/`remove` 的 `< 1`）统一为\n            //   **精确 arity**，本处是**漏网** —— 而 M226 的门**看不见它**：那道门是\n            //   **三轨对拍**，而三轨（解释/VM/C）的状态机**都**走这条静默路 ⇒ 一致 ⇒\n            //   无分叉。「**跨面对拍**」才照得出来（本轮方法论的核心）。\n            if (nargs != 0) px_error(\"R1005: 方法 len 不接受参数\");\n            return px_int(px_len(obj));\n        }\n        if (strcmp(name, \"has\") == 0 || strcmp(name, \"contains\") == 0) {",
   "why": "缺陷 341 · px_method dict.len arity"
  },
  {
   "file": "runtime/runtime.c",
   "old": "    if (obj.type == PX_TUPLE) {\n        if (strcmp(name, \"len\") == 0) {\n            if (nargs != 0) px_error(\"R1005: 方法 len 不接受参数\");\n            return px_int(px_len(obj));\n        }\n    }",
   "new": "    if (obj.type == PX_TUPLE) {\n        if (strcmp(name, \"len\") == 0) {\n            if (nargs != 0) px_error(\"R1005: 方法 len 不接受参数\");\n            return px_int(px_len(obj));\n        }\n        // M227（缺陷 339/340）：「同名两门」的能力对齐 —— 函数面 `contains(t, v)` /\n        //   `join(sep, t)` 从 M178（可迭代实参统一）起就支持 tuple，而方法面只有 `len`\n        //   ⇒ 同一个操作两个门支持的类型集合不同（`contains((1,2), 2)` 得 true、\n        //   `((1,2)).contains(2)` 响亮「类型 tuple 没有方法 'contains'」）。补齐方法面。\n        //   ⚠️ `str.join` **刻意不补**：方法面若收，`s.join(sep)` 的接收者是「序列」还是\n        //   「分隔符」会与 Python `str.join` 的约定**正好相反** ⇒ 是有理由的不对称（见门头）。\n        if (strcmp(name, \"contains\") == 0) {\n            if (nargs != 1) px_error(\"R1005: 方法 contains 需要 1 个参数\");\n            LXObject* o = obj.as.obj;\n            for (int i = 0; i < o->as.tuple.len; i++) {\n                if (px_eq(o->as.tuple.items[i], args[0]).as.b) return px_bool(true);\n            }\n            return px_bool(false);\n        }\n        if (strcmp(name, \"join\") == 0) {\n            if (nargs != 1) px_error(\"R1005: 方法 join 需要 1 个参数\");\n            if (args[0].type != PX_STR || !args[0].as.obj)\n                px_error(\"R1002: 方法 join 参数 1 需要 string\");\n            return call_with_self(\"join\", args[0], &obj, 1);\n        }\n    }",
   "why": "缺陷 339/340 · px_method tuple 补 contains/join"
  }
 ],
 "B": [
  {
   "file": "selfhost/icall.px",
   "old": "def i_dict_method(d, name, args, pos):\n    if name == \"len\":\n        return Ok(len(d.keys()))",
   "new": "def i_dict_method(d, name, args, pos):\n    if name == \"len\":\n        # M227（缺陷 341）：修前**没有 arity 检查** ⇒ `({\"a\":1}).len(1, 2)` 静默返回 1\n        #   （native `px_method` 同款漏网 —— M226 缺陷 329 的同族）。三轨**都**静默\n        #   ⇒ 「三轨对拍」门看不见；本轮「跨面对拍」才照出来。\n        if len(args) != 0:\n            return Err(i_r1005(\"方法 len 不接受参数\", pos))\n        return Ok(len(d.keys()))",
   "why": "缺陷 341 · i_dict_method len arity"
  },
  {
   "file": "selfhost/icall.px",
   "old": "    if t == \"tuple\":\n        if name == \"len\":\n            # M190（缺陷 213-k）：补参数校验（native 侧本轮同步补齐 tuple.len）。\n            if len(args) != 0:\n                return Err(i_r1005(\"方法 len 不接受参数\", pos))\n            return Ok(len(rcv))\n        return Err(i_r1007(\"类型 tuple 没有方法 '\" + name + \"'\", pos))",
   "new": "    if t == \"tuple\":\n        if name == \"len\":\n            # M190（缺陷 213-k）：补参数校验（native 侧本轮同步补齐 tuple.len）。\n            if len(args) != 0:\n                return Err(i_r1005(\"方法 len 不接受参数\", pos))\n            return Ok(len(rcv))\n        # M227（缺陷 339/340）：「同名两门」能力对齐 —— 函数面 `contains(t, v)` /\n        #   `join(sep, t)` 一直支持 tuple，方法面只有 len ⇒ 同一个操作两个门不同。\n        #   ⚠️ `str.join` 刻意不补（接收者角色会与 Python `str.join` 约定相反）。\n        if name == \"contains\":\n            if len(args) != 1:\n                return Err(i_r1005(\"方法 contains 需要 1 个参数\", pos))\n            var i = 0\n            while i < len(rcv):\n                if i_eq(rcv[i], args[0]):\n                    return Ok(true)\n                i += 1\n            return Ok(false)\n        if name == \"join\":\n            if len(args) != 1:\n                return Err(i_r1005(\"方法 join 需要 1 个参数\", pos))\n            let r = i_expect_str_arg(args, 0, \"join\", pos)\n            if r.is_err():\n                return Err(r.err())\n            let sep = r.unwrap()\n            let parts = []\n            var i = 0\n            while i < len(rcv):\n                parts.append(i_to_str(rcv[i]))\n                i += 1\n            return Ok(join(sep, parts))\n        return Err(i_r1007(\"类型 tuple 没有方法 '\" + name + \"'\", pos))",
   "why": "缺陷 339/340 · i_call_method tuple 补 contains/join"
  },
  {
   "file": "selfhost/ibuiltin.px",
   "old": "        let t0 = i_type_name(args[0])\n        if t0 == \"string\":\n            return Ok(contains(args[0], args[1]))",
   "new": "        let t0 = i_type_name(args[0])\n        # M227（缺陷 337/338）：dict 与 string 一律**转发 native**（一份实现、同码同文）——\n        #   修前 dict 落到末尾 `contains 不支持类型 dict`，而方法面 `d.contains(k)` 正常。\n        if t0 == \"string\" or t0 == \"dict\":\n            return Ok(contains(args[0], args[1]))",
   "why": "缺陷 337 · 解释轨函数面 contains 补 dict"
  }
 ]
}''')

ap = argparse.ArgumentParser()
ap.add_argument('group', nargs='?', default=None)
ap.add_argument('--selftest', action='store_true')
ap.add_argument('--revert', action='store_true')
ap.add_argument('--apply', action='store_true')
ap.add_argument('--snapshot', action='store_true')
ap.add_argument('--restore', action='store_true')
ap.add_argument('--files', default='runtime/runtime.c selfhost/icall.px selfhost/ibuiltin.px')
a = ap.parse_args()
FILES = a.files.split()


def path(rel):
    return os.path.join(ROOT, rel)


if a.snapshot:
    os.makedirs(SNAP, exist_ok=True)
    for rel in FILES:
        d = os.path.join(SNAP, rel)
        os.makedirs(os.path.dirname(d), exist_ok=True)
        shutil.copyfile(path(rel), d)
    print('SNAPSHOT-OK ' + ' '.join(FILES))
    sys.exit(0)

if a.restore:
    n = 0
    for rel in FILES:
        s = os.path.join(SNAP, rel)
        if os.path.exists(s):
            shutil.copyfile(s, path(rel)); n += 1
    print(f'RESTORE-OK {n} 个文件')
    sys.exit(0)

if a.selftest:
    bad = 0
    for g in ('A', 'B'):
        for i, e in enumerate(EDITS[g]):
            s = open(path(e['file']), encoding='utf-8').read()
            nnew = s.count(e['new'])
            # ⚠️ OLD 可能是 NEW 的**子串/后缀**（本轮的 337 就是：NEW 末尾原样保留 OLD）
            #    ⇒ 直接从「去掉 NEW 之后」的串里数 OLD 才准（首版数出 NEW×1 OLD×1 的假红）。
            nold = s.replace(e['new'], '', 1).count(e['old'])
            ok = (nnew == 1 and nold == 0)
            bad += 0 if ok else 1
            print(f"  {'OK ' if ok else '✖  '}[{g}{i}] NEW×{nnew} OLD×{nold}  {e['why']}")
    print(f"锚点自证 {'OK' if not bad else 'FAIL'}（{len(EDITS['A'])+len(EDITS['B'])} 条 · 失败 {bad}）")
    sys.exit(1 if bad else 0)

g = (a.group or 'A').upper()
if g not in EDITS:
    sys.exit(f'未知组 {g}')
fn = (lambda s, old, new: s.replace(new, old, 1)) if a.revert else (lambda s, old, new: s.replace(old, new, 1))
for e in EDITS[g]:
    s = open(path(e['file']), encoding='utf-8').read()
    open(path(e['file']), 'w', encoding='utf-8').write(fn(s, e['old'], e['new']))
print(f"{'REVERT' if a.revert else 'APPLY'}-OK {g}（{len(EDITS[g])} 处）")
