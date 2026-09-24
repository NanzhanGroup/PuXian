#!/usr/bin/env python3
# ============================================================
# M199 [S10-b] 探针**自动生成器**：从 native 的逐参守卫声明，生成「全参类型正确、只有第 P 位是毒值」
#   的探针；并合成**一个**驱动器程序（按 args()[1] 分派）—— 一次编译即可取到编译轨对全量用例的行为。
# ------------------------------------------------------------
# 为什么这样设计（本轮实证）：
#   若「一例一次 build」，136 例 × ~200s ≈ 7.5 小时 —— 不可行（M198 的 14 例已接近上限）。
#   把全部用例合成**一个**驱动器（每条 `print(...)` 独立分支）后：**两次编译**（VM/C）即可覆盖全部用例，
#   随后的 120+ 次执行各 ~10ms。⇒ 「逐参类型面」从此可以**全量**度量，而不是抽样。
# 为什么毒值取 dict：它不是 int/str/bytes/list/bool/null，对任何类型守卫都应判错；且 `{}` 是字面量，
#   不会引入额外变量（`px_dict()` 在语言层不存在 —— 首版踩过：136 例全部 R1001 未定义变量）。
# ============================================================
import json, re, os, sys, argparse

ap = argparse.ArgumentParser()
ap.add_argument('--root', default='.')
ap.add_argument('--out', default='/tmp/m199sweep')
ap.add_argument('--sweep', default='/tmp/m199sweep/sweep.json')
a = ap.parse_args()
os.chdir(a.root)

G = json.load(open(a.sweep))['guards']
POISON = '{}'
TYPEVAL = {'INT': '7', 'STR': '"s"', 'STRBYTES': '"s"', 'BYTES': 'bytes("s")',
           'LIST': '[1]', 'DICT': '{}', 'DUR': '7', 'IDX': '0', 'NULL': 'null', 'TYPETEST': '7',
           'BOOL': 'true', 'FLOAT': '1.5', 'FUNC': 'null'}

# 副作用 / 危险族：不做动态（只做静态判据）。刻意**收窄**到真正危险的前缀 ——
#   过宽会把 `exit`/`udp_open`/`tls_server`/`ws_heartbeat` 这类「守卫先于副作用」的函数也排掉，
#   而它们恰恰是本轮缺陷（237）的所在。
SKIP_RE = re.compile(r'^(os_spawn|os_exec|os_kill|proc_|sandbox_|panic|write_file|append_file|write_at|'
                     r'write_bytes|truncate_file|remove|rename|chmod|chown|mkdir|rmdir|unlink|'
                     r'dns_|tcp_|udp_send|udp_recv|quic_|h3_|tls_connect|tls_upgrade|tls_read|tls_write|'
                     r'tls_close|http_|ws_serve|ws_send|ws_close|ws_read|sse_|serve|listen|accept|'
                     r'connect|send|recv|read_file|read_bytes|read_at|open|close|mmap|munmap|ioctl|'
                     r'fcntl|flock|fd_wait|input|bus_|sqlite|ffi|dlopen|set_timeout|set_interval|'
                     r'clear_timer|cron|s3_|mail|smtp|random_seed|env_set|env_unset|ctx_set|'
                     r'session_set|session_del|session_destroy|mem_write|kcp_)')


# 静态扫描器给出的「声明」可能是类型名（PX_INT→INT）或**守卫 helper 名**（px_arg_int / px_req_int_idx）
HELPER2TYPE = {'px_arg_int': 'INT', 'px_req_int': 'INT', 'px_req_int_idx': 'IDX',
               'px_arg_str': 'STR', 'px_req_str': 'STR', 'px_arg_strbytes': 'STRBYTES',
               'px_arg_bytes': 'BYTES', 'px_req_bytes': 'BYTES', 'px_arg_list': 'LIST',
               'px_arg_dict': 'DICT', 'px_arg_dur_ms': 'DUR', 'px_arg_dur_ns': 'DUR'}


def val_for(types):
    for t in types:
        t2 = HELPER2TYPE.get(t, t)
        if t2 in TYPEVAL:
            return TYPEVAL[t2]
    return None


cases, skipped = [], []
for name, g in sorted(G.items()):
    gi = {int(p): ts for p, ts in g.items()}
    if not gi:
        continue
    top = max(gi)
    if not all(p in gi for p in range(0, top + 1)):
        skipped.append(name + '(声明不连续)')
        continue
    if SKIP_RE.match(name):
        skipped.append(name)
        continue
    vals = [val_for(gi[p]) for p in range(0, top + 1)]
    if any(v is None for v in vals):
        skipped.append(name + '(类型无样本)')
        continue
    for p in range(0, top + 1):
        args = list(vals)
        args[p] = POISON
        cases.append((f'{name}__p{p}', name, p, f'print({name}({", ".join(args)}))',
                      '+'.join(gi[p])))

# ── 合法调用回归（LEGAL.tsv）：正常路径必须三轨一致（M199 [4]）──
legal = []
lg = os.path.join(os.path.dirname(os.path.abspath(__file__)), 'LEGAL.tsv')
if os.path.exists(lg):
    for ln in open(lg, encoding='utf-8'):
        ln = ln.strip()
        if not ln or ln.startswith('#'):
            continue
        lbl, src = ln.split('@@', 1)
        legal.append((lbl, src))

os.makedirs(a.out, exist_ok=True)
with open(f'{a.out}/autocases.tsv', 'w') as f:
    for label, name, p, src, ts in cases:
        f.write(f'{label}@@{src}\n')
    for lbl, src in legal:                 # 合法侧追加（标签以 ok 开头，供门 [4] 断言）
        f.write(f'{lbl}@@{src}\n')

# ── 合成驱动器 ──
L = ['let a = args()',
     'if len(a) < 2:',
     '    print("NOCASE")',
     'else:',
     '    let c = a[1]',
     '    var hit = false']
for label, name, p, src, ts in cases:
    L.append('    if c == "%s":' % label)
    L.append('        hit = true')
    L.append('        ' + src)
for lbl, src in legal:
    L.append('    if c == "%s":' % lbl)
    L.append('        hit = true')
    L.append('        ' + src)
L.append('    if hit == false:')
L.append('        print("NOCASE")')
open(f'{a.out}/drv.px', 'w').write('\n'.join(L) + '\n')

json.dump({'cases': [[c[0], c[1], c[2], c[4]] for c in cases], 'skipped': skipped},
          open(f'{a.out}/autocases.json', 'w'), ensure_ascii=False, indent=1)
print(f'合法回归用例 {len(legal)} 条')
print(f'生成探针 {len(cases)} 条（函数 {len(set(c[1] for c in cases))} 个）· 跳过 {len(skipped)} 个（副作用族/声明不连续/无样本）')
print('驱动器 → %s/drv.px（%d 行）' % (a.out, len(L)))
