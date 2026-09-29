#!/usr/bin/env python3
# ============================================================
# M226 门 · 负控补丁器（**忠实撤回**：把本轮修复逐字换回修前原文）
# ------------------------------------------------------------
# 纪律（M220 教训）：负控必须「**忠实撤回**」而不是「另造一个错」——
#   首版若把标记追加进被发射的字符串字面量，得到的是**别的**缺陷，
#   判红就证明不了本轮修的东西有用。
#   本文件里每条 `new` 都是**修前原文**（逐字来自本轮 patch 前的源码）。
#
# 用法：negctl.py --apply A|B   # A = runtime.c（VM/C 轨）· B = icall.px（解释轨）
#       negctl.py --selftest    # 自证：每条锚点在**当前**源码里恰好命中 1 次
# ============================================================
import argparse, os, sys

ROOT = os.environ.get('M226_ROOT', '.')
A_PAIRS = []   # (file, 当前(已修)文本, 修前文本)
B_PAIRS = []

# ── A：runtime/runtime.c（编译两轨共用）──
RT = 'runtime/runtime.c'

A_PAIRS.append((RT, '''        if (strcmp(name, "contains") == 0) {
            // M226（缺陷 329）：修前是 `nargs < 1` ⇒ `[1,2].contains(2,1,2)` 静默忽略多余实参
            //   （解释轨是精确「恰 1」）⇒ 三轨分叉。M190 已把这一族统一为「精确 arity」，
            //   本处是**漏网**：`< 1` 与 `!= 1` 只差在「多了」这一侧，正是最容易被忽略的方向。
            if (nargs != 1) px_error("R1005: 方法 contains 需要 1 个参数");''',
'''        if (strcmp(name, "contains") == 0) {
            if (nargs < 1) px_error("R1005: 方法 contains 需要 1 个参数");'''))

A_PAIRS.append((RT, '''        // M226（缺陷 328/329/333）：修前**完全不查实参** —— 三个后果：
        //   ① 0 参 ⇒ 直接读 `args[0]` = **越界读**（VM 轨实测 `[1,2].join()` SIGSEGV rc=139 core）；
        //   ② `join("-", 1, 2)` 静默忽略多余实参（解释轨是精确「恰 1」）；
        //   ③ 分隔符错类型时用的是**转发内置**的文案（`join 分隔符需要 string`），
        //      与解释轨 `方法 join 参数 1 需要 string` 不一致。
        if (strcmp(name, "join") == 0) {
            if (nargs != 1) px_error("R1005: 方法 join 需要 1 个参数");
            if (args[0].type != PX_STR || !args[0].as.obj)
                px_error("R1002: 方法 join 参数 1 需要 string");
            return call_with_self("join", args[0], &obj, 1);
        }''',
'''        if (strcmp(name, "join") == 0) return call_with_self("join", args[0], &obj, 1);'''))

A_PAIRS.append((RT, '''            // M226（缺陷 335）：修前是 `nargs < 1` ⇒ `get("a", 1, 2)` 静默忽略第 3 个实参。
            //   签名是 get(键[, 默认值]) ⇒ 上界 2（与 `set` 的精确 arity 同口径）。
            if (nargs < 1 || nargs > 2) px_error("R1005: 方法 get 需要 1-2 个参数");''',
'''            if (nargs < 1) px_error("R1005: 方法 get 需要 1 个参数");'''))

A_PAIRS.append((RT, '''        // M226（缺陷 332）：`put` 是 `set` 的**别名**（解释轨 `i_dict_method` 的
        //   `if name == "set" or name == "put"` 分支一直有），而编译轨**清单里没有** ⇒
        //   `d.put("b", 1)` 在编译轨响亮 `R1007 类型 dict 没有方法 'put'`。
        //   ⇒ 补齐编译侧（删能力是退步 —— 同 M190 对 list.index/reverse/sort 的处置）。
        if (strcmp(name, "set") == 0 || strcmp(name, "put") == 0) {''',
'''        if (strcmp(name, "set") == 0) {'''))

A_PAIRS.append((RT, '''            // M226（缺陷 329）：同 list.contains —— `< 1` 放过多余实参
            if (nargs != 1) px_error("R1005: 方法 %s 需要 1 个参数", name);''',
'''            if (nargs < 1) px_error("R1005: 方法 %s 需要 1 个参数", name);'''))

A_PAIRS.append((RT, '''            // M226（缺陷 329）：`< 1` ⇒ 多余实参静默（解释轨精确「恰 1」）
            if (nargs != 1) px_error("R1005: 方法 remove 需要 1 个参数");
            if (args[0].type != PX_STR || !args[0].as.obj)
                px_error("R1002: 方法 remove 参数 1 需要 string");
            LXObject* o = obj.as.obj;
            const char* key = args[0].as.obj->as.str.data;
            LXValue v = px_null();
            int m226_found = 0;
            for (int i = 0; i < o->as.dict.len; i++) {
                if (strcmp(o->as.dict.keys[i], key) == 0) {
                    v = o->as.dict.vals[i];
                    m226_found = 1;''',
'''            if (nargs < 1) px_error("R1005: 方法 remove 需要 1 个参数");
            if (args[0].type != PX_STR || !args[0].as.obj)
                px_error("R1002: 方法 remove 参数 1 需要 string");
            LXObject* o = obj.as.obj;
            const char* key = args[0].as.obj->as.str.data;
            LXValue v = px_null();
            for (int i = 0; i < o->as.dict.len; i++) {
                if (strcmp(o->as.dict.keys[i], key) == 0) {
                    v = o->as.dict.vals[i];'''))

A_PAIRS.append((RT, '''            // M226（缺陷 334）：缺键 ⇒ **响亮**。解释轨一直报 R1008（`字典没有键 'k'`），
            //   编译轨此前静默返回 null ⇒ 同一份代码三轨两种命运（静默方向最危险：调用方
            //   以为删掉了、其实没有）。口径按本仓既定原则「**响亮优于静默**」+ 与 `d[k]`
            //   的 R1008 完全同形。
            if (!m226_found) px_error("R1008: 字典没有键 '%s'", key);
            return v;''',
'''            return v;'''))

# ── B：selfhost/icall.px（解释轨）──
IC = 'selfhost/icall.px'

B_PAIRS.append((IC, '''    # M226（缺陷 331）：`to_upper` / `to_lower` 是 `upper` / `lower` 的**别名**
    #   （native `px_method` 的 `strcmp(name,"upper")==0 || strcmp(name,"to_upper")==0`
    #   一直如此），而解释轨**清单里没有** ⇒ `"abc".to_upper()` 解释轨响亮
    #   `R1007 类型 string 没有方法 'to_upper'`、编译轨正常 ⇒ 方法清单不对称。
    #   ⇒ 补齐解释侧；**报错文案一律用规范名**（`方法 upper …`）—— 与 native 逐字同文。
    if name == "upper" or name == "to_upper":
        if len(args) != 0:
            return Err(i_r1005("方法 upper 不接受参数", pos))
        return Ok(s.upper())
    if name == "lower" or name == "to_lower":
        if len(args) != 0:
            return Err(i_r1005("方法 lower 不接受参数", pos))
        return Ok(s.lower())''',
'''    if name == "upper":
        if len(args) != 0:
            return Err(i_r1005("方法 upper 不接受参数", pos))
        return Ok(s.upper())
    if name == "lower":
        if len(args) != 0:
            return Err(i_r1005("方法 lower 不接受参数", pos))
        return Ok(s.lower())'''))

B_PAIRS.append((IC, '''        # M226（缺陷 330/335）：**缺参必须先报缺参**（R1005）—— 修前直接进
        #   `i_expect_str_arg(args, 0, …)`，0 参时它报 R1002「参数 1 需要 string」
        #   而编译轨报 R1005「需要 1 个参数」⇒ 同一次误用三轨两种 R 码（且解释轨的
        #   说法把「没给」说成「类型不对」，指不到真因）。上界同 `set`（get(键[, 默认值])）。
        if len(args) > 2 or len(args) < 1:
            return Err(i_r1005("方法 get 需要 1-2 个参数", pos))
        let r = i_expect_str_arg(args, 0, "get", pos)''',
'''        let r = i_expect_str_arg(args, 0, "get", pos)'''))

B_PAIRS.append((IC, '''        # M226（缺陷 330）：先查 arity（缺参 ⇒ R1005」，同编译轨）
        if len(args) != 1:
            return Err(i_r1005("方法 " + name + " 需要 1 个参数", pos))
        let r = i_expect_str_arg(args, 0, name, pos)''',
'''        let r = i_expect_str_arg(args, 0, name, pos)'''))

B_PAIRS.append((IC, '''        # M226（缺陷 330）：先查 arity（缺参 ⇒ R1005，同编译轨）
        if len(args) != 1:
            return Err(i_r1005("方法 remove 需要 1 个参数", pos))
        let r = i_expect_str_arg(args, 0, "remove", pos)''',
'''        let r = i_expect_str_arg(args, 0, "remove", pos)'''))

SETS = {'A': A_PAIRS, 'B': B_PAIRS}


def apply_set(key):
    n = 0
    for f, cur, old in SETS[key]:
        p = os.path.join(ROOT, f)
        s = open(p, encoding='utf-8').read()
        c = s.count(cur)
        if c != 1:
            sys.exit(f'❌ 负控 {key}：{f} 锚点命中 {c} 次（要求 1）')
        open(p, 'w', encoding='utf-8').write(s.replace(cur, old, 1))
        n += 1
    print(f'✓ 负控 {key}：忠实撤回 {n} 处')


ap = argparse.ArgumentParser()
ap.add_argument('--apply', choices=['A', 'B'])
ap.add_argument('--selftest', action='store_true')
a = ap.parse_args()
if a.selftest:
    bad = 0
    for k, pairs in SETS.items():
        for f, cur, _ in pairs:
            p = os.path.join(ROOT, f)
            s = open(p, encoding='utf-8').read()
            c = s.count(cur)
            if c != 1:
                print(f'❌ 自证失败：{k}/{f} 锚点命中 {c} 次')
                bad += 1
    print(f'自证 {"OK" if not bad else "FAIL"}：锚点共 {len(A_PAIRS)+len(B_PAIRS)} 处，'
          f'A={len(A_PAIRS)} B={len(B_PAIRS)}')
    sys.exit(1 if bad else 0)
if a.apply:
    apply_set(a.apply)
