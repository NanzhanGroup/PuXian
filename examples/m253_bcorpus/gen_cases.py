#!/usr/bin/env python3
# ============================================================
# M253 生成器：把「10 个 [B 类] native」的语料生成出来。
# 产物（写到 <out>/）：
#   drv.px      聚合驱动器 —— args()[1] 选例 ⇒ **一次编译覆盖全部例**
#   cases.tsv   label <TAB> mode <TAB> expect
#                 mode=v：stdout 必须逐字节等于 expect（`\n` 转义）
#                 mode=e：必须**响亮**（rc≠0 + 含 R1002）
#                 mode=-：只做三轨对拍，无期望值
# 真值来源（**独立于被测实现**）：
#   · AES  → Go crypto/aes + crypto/cipher（见 truth/gen_truth.go 的同一批向量）
#   · errno → Go syscall.Errno(n).Error() 的**全表**（0..140 + 越界）
#   · tz    → 由 verify.sh 用 TZ 环境 + 真值比对（Go time.Now().Zone()）
# ============================================================
import argparse, os, sys

ap = argparse.ArgumentParser()
ap.add_argument('--out', required=True)
a = ap.parse_args()

CASES = []          # (label, body_lines, mode, expect)


def C(label, body, mode='-', expect=''):
    CASES.append((label, body, mode, expect))


# ── 常量（驱动器开头统一声明）──
PRELUDE = [
    '    k16 = hex_to_bytes("30313233343536373839616263646566")',
    '    k24 = hex_to_bytes("303132333435363738396162636465663031323334353637")',
    '    k32 = hex_to_bytes("3031323334353637383961626364656630313233343536373839616263646566")',
    '    iv16 = hex_to_bytes("6162636465666768696a6b6c6d6e6f70")',
    '    nc12 = hex_to_bytes("6162636465666768696a6b6c")',
    '    P0 = bytes("")',
    '    P1 = bytes("A")',
    '    P15 = bytes("AAAAAAAAAAAAAAA")',
    '    P16 = bytes("AAAAAAAAAAAAAAAA")',
    '    PB7 = hex_to_bytes("000102feff807f")',
]

# ── Go 真值（AES）。键 = (族, 密钥名, 明文名)，值 = 密文 hex ──
G = {}
for ln in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'aes_truth.tsv'), encoding='utf-8'):
    ln = ln.strip()
    if ln and not ln.startswith('#'):
        fam, kn, pn, ct = ln.split('\t')
        G[(fam, kn, pn)] = ct

# ── Go 真值（errno 全表 0..140）──
ERRNO = {}
for ln in open(os.path.join(os.path.dirname(os.path.abspath(__file__)), 'errno_truth.tsv'), encoding='utf-8'):
    ln = ln.rstrip('\n')
    if ln and not ln.startswith('#'):
        n, txt = ln.split('\t', 1)
        ERRNO[int(n)] = txt

PMAP = {'p0': 'P0', 'p1': 'P1', 'p15': 'P15', 'p16': 'P16', 'pbin7': 'PB7'}
KMAP = {'k16': 'k16', 'k24': 'k24', 'k32': 'k32'}
HEXK = {'k16': '"0123456789abcdef"', 'k24': '"0123456789abcdef01234567"',
        'k32': '"0123456789abcdef0123456789abcdef"', 'iv': '"abcdefghijklmnop"'}


def esc(s):
    return s.replace('\\', '\\\\').replace('\n', '\\n').replace('\t', '\\t')


# ══════════ 一 已知向量（期望值 = Go 真值；三轨都必须等于它）══════════
for kn in ('k16', 'k24', 'k32'):
    for pn in ('p0', 'p1', 'p15', 'p16', 'pbin7'):
        C(f'v_cbc_b_{kn}_{pn}',
          [f'print("V=" + bytes_to_hex(aes_encrypt_bytes({PMAP[pn]}, {KMAP[kn]}, iv16)))'],
          'v', G[('CBC', kn, pn)])
for kn in ('k16', 'k24', 'k32'):
    for pn in ('p0', 'p1', 'pbin7'):
        C(f'v_gcm_b_{kn}_{pn}',
          [f'print("V=" + bytes_to_hex(aes_gcm_encrypt_bytes({PMAP[pn]}, {KMAP[kn]}, nc12)))'],
          'v', G[('GCM', kn, pn)])

# hex 文本版（参数序 (data, key, iv) —— 本轮同时**钉死**这条文档口径）
C('v_cbc_hex_p1', ['print("V=" + aes_encrypt("A", "0123456789abcdef", "abcdefghijklmnop"))'],
  'v', G[('CBC', 'k16', 'p1')])
C('v_cbc_hex_p0', ['print("V=" + aes_encrypt("", "0123456789abcdef", "abcdefghijklmnop"))'],
  'v', G[('CBC', 'k16', 'p0')])
C('v_cbc_hex_k32_p1', ['print("V=" + aes_encrypt("A", "0123456789abcdef0123456789abcdef", "abcdefghijklmnop"))'],
  'v', G[('CBC', 'k32', 'p1')])
C('v_gcm_hex_p0', ['print("V=" + aes_gcm_encrypt("", "0123456789abcdef", "abcdefghijkl"))'],
  'v', G[('GCM', 'k16', 'p0')])
C('v_gcm_hex_p1', ['print("V=" + aes_gcm_encrypt("A", "0123456789abcdef", "abcdefghijkl"))'],
  'v', G[('GCM', 'k16', 'p1')])

# AES-ECB 解密已知向量（ECB 是 2 参 (hex, key)）
C('v_ecb_hex_p16', [f'print("V=" + str(aes_decrypt_ecb("{G[("ECB", "k16", "p16")]}", "0123456789abcdef")))'],
  'v', 'AAAAAAAAAAAAAAAA')
C('v_ecb_hex_k32_p1', [f'print("V=" + str(aes_decrypt_ecb("{G[("ECB", "k32", "p1")]}", "0123456789abcdef0123456789abcdef")))'],
  'v', 'A')

# ══════════ 二 往返闭合（本轮缺陷 A 的回归位）══════════
PHEX = {'p0': '', 'p1': '41', 'pbin7': '000102feff807f'}   # 明文 hex（往返的期望值）
for kn in ('k16', 'k24', 'k32'):
    for pn in ('p0', 'p1', 'pbin7'):
        C(f'r_gcm_b_{kn}_{pn}_rt',
          [f'let p = aes_gcm_decrypt_bytes(aes_gcm_encrypt_bytes({PMAP[pn]}, {KMAP[kn]}, nc12), {KMAP[kn]}, nc12)',
           'if p == null:',
           '    print("V=NULL")',
           'else:',
           '    print("V=" + bytes_to_hex(p))'],
          'v', PHEX[pn])
# hex 版往返：GCM 密文 = 密文hex + taghex ⇒ 解回来应为原文
C('r_gcm_hex_p0_rt',
  ['let c = aes_gcm_encrypt("", "0123456789abcdef", "abcdefghijkl")',
   'let p = aes_gcm_decrypt(c, "0123456789abcdef", "abcdefghijkl")',
   'if p == null:',
   '    print("V=NULL")',
   'else:',
   '    print("V=[" + p + "]")'],
  'v', '[]')

# ══════════ 三 go_errno_string —— 全表 + 越界（Go 真值）══════════
body = ['i = 0', 'while i <= 140:', '    print(str(i) + "=" + go_errno_string(i))', '    i = i + 1']
C('v_errno_sweep', body, 'v', esc('\n'.join(f'{n}={ERRNO[n]}' for n in range(141))))

for lbl, val, txt in (
    ('neg', -1, 'errno -1'), ('neg22', -22, 'errno -22'),
    ('o141', 141, 'errno 141'), ('o4095', 4095, 'errno 4095'),
    ('big31', 2147483648, 'errno 2147483648'),           # 缺陷 B：修前 errno -2147483648
    ('big32', 4294967296, 'errno 4294967296'),           # 缺陷 B：修前 errno 0
    ('big32p1', 4294967297, 'errno 4294967297'),         # 缺陷 B：修前 operation not permitted
    ('big62', 4611686018427387904, 'errno 4611686018427387904'),
):
    C(f'v_errno_{lbl}', [f'print("V=" + go_errno_string({val}))'], 'v', txt)

# ══════════ 四 逐位置 × 错类型 / 错 arity（边界面：必须响亮）══════════
# 三元组 = (label, 调用表达式)
ERR_CASES = [
    # aes_encrypt_bytes(data, key, iv)
    ('e_encb_na0', 'aes_encrypt_bytes()'),
    ('e_encb_na2', 'aes_encrypt_bytes(P1, k16)'),
    ('e_encb_na4', 'aes_encrypt_bytes(P1, k16, iv16, 1)'),
    ('e_encb_d_int', 'aes_encrypt_bytes(1, k16, iv16)'),
    ('e_encb_k_int', 'aes_encrypt_bytes(P1, 1, iv16)'),
    ('e_encb_i_int', 'aes_encrypt_bytes(P1, k16, 1)'),
    ('e_encb_k15', 'aes_encrypt_bytes(P1, hex_to_bytes("303132333435363738396162636465"), iv16)'),
    ('e_encb_k0', 'aes_encrypt_bytes(P1, bytes(""), iv16)'),
    ('e_encb_i15', 'aes_encrypt_bytes(P1, k16, hex_to_bytes("6162636465666768696a6b6c6d6e6f"))'),
    ('e_encb_i17', 'aes_encrypt_bytes(P1, k16, hex_to_bytes("6162636465666768696a6b6c6d6e6f7071"))'),
    # aes_decrypt_bytes(ct, key, iv)
    ('e_decb_na2', 'aes_decrypt_bytes(P1, k16)'),
    ('e_decb_c_int', 'aes_decrypt_bytes(1, k16, iv16)'),
    ('e_decb_k15', 'aes_decrypt_bytes(P1, hex_to_bytes("303132333435363738396162636465"), iv16)'),
    # aes_gcm_encrypt_bytes / decrypt_bytes
    ('e_gcmeb_na2', 'aes_gcm_encrypt_bytes(P1, k16)'),
    ('e_gcmeb_i0', 'aes_gcm_encrypt_bytes(P1, k16, bytes(""))'),
    ('e_gcmeb_k15', 'aes_gcm_encrypt_bytes(P1, hex_to_bytes("303132333435363738396162636465"), nc12)'),
    ('e_gcmdb_na2', 'aes_gcm_decrypt_bytes(P1, k16)'),
    ('e_gcmdb_i0', 'aes_gcm_decrypt_bytes(P1, k16, bytes(""))'),
    # aes_decrypt_ecb(hex, key)
    ('e_ecb_na1', 'aes_decrypt_ecb("00")'),
    ('e_ecb_na3', 'aes_decrypt_ecb("00", "0123456789abcdef", 1)'),
    ('e_ecb_h_int', 'aes_decrypt_ecb(1, "0123456789abcdef")'),
    ('e_ecb_k_int', 'aes_decrypt_ecb("00", 1)'),
    ('e_ecb_k15', 'aes_decrypt_ecb("00", "0123456789abcde")'),
    # go_errno_string(errno)
    ('e_gerr_na0', 'go_errno_string()'),
    ('e_gerr_na2', 'go_errno_string(1, 2)'),
    ('e_gerr_str', 'go_errno_string("x")'),
    ('e_gerr_null', 'go_errno_string(null)'),
    ('e_gerr_true', 'go_errno_string(true)'),
    ('e_gerr_float', 'go_errno_string(2.9)'),
    # 0 参族
    ('e_tz_na1', 'tz_local(1)'),
    ('e_sid_na1', 'session_id(1)'),
    ('e_sd_na1', 'session_destroy(1)'),
]
for lbl, expr in ERR_CASES:
    C(lbl, [f'print("V=" + str({expr}))'], 'e', '')

# ══════════ 五 非值面（通道 / 上下文 / 返回值）══════════
C('p_pe_ret', ['print("V=" + str(print_err("PE", 1, true, null)))'], 'v', 'null')
C('p_pe_multi', ['print_err("a", "b")', 'print("V=ok")'], 'v', 'ok')
C('p_sid_none', ['print("V=" + str(session_id()))'], 'v', 'null')
C('p_sd_none', ['print("V=" + str(session_destroy()))'], 'v', 'false')
C('p_tz_int', ['let t = tz_local()', 'let ok = t >= (0 - 50400) and t <= 50400', 'print("V=" + str(ok))'], 'v', 'true')

# ══════════ 生成 drv.px ══════════
L = ['# M253 聚合驱动器（**由 gen_cases.py 生成，勿手改**）',
     '#   args()[1] = 例名；一次编译覆盖全部例（M199 的聚合范式）',
     'def main():',
     '    a = args()',
     '    if len(a) < 2:',
     '        print("USAGE")',
     '        exit(2)',
     '    lb = a[1]']
L += PRELUDE
for label, body, _m, _e in CASES:
    L.append(f'    if lb == "{label}":')
    # 约定：**body 行以「相对 8 空格基线」书写**（列 0 = 8 空格）
    L += [('        ' + b) if b.strip() else '' for b in body]
    L.append('        exit(0)')
L += ['    print("UNKNOWN=" + lb)', '    exit(3)']

os.makedirs(a.out, exist_ok=True)
open(os.path.join(a.out, 'drv.px'), 'w', encoding='utf-8').write('\n'.join(L) + '\n')
with open(os.path.join(a.out, 'cases.tsv'), 'w', encoding='utf-8') as f:
    for label, _b, mode, expect in CASES:
        f.write(f'{label}\t{mode}\t{expect}\n')

nv = sum(1 for c in CASES if c[2] == 'v')
ne = sum(1 for c in CASES if c[2] == 'e')
apis = ['aes_encrypt_bytes', 'aes_decrypt_bytes', 'aes_gcm_encrypt_bytes', 'aes_gcm_decrypt_bytes',
        'aes_decrypt_ecb', 'go_errno_string', 'print_err', 'session_id', 'session_destroy', 'tz_local']
drv = open(os.path.join(a.out, 'drv.px'), encoding='utf-8').read()
missing = [n for n in apis if n not in drv]
print(f'[M253] 例 {len(CASES)}（值 {nv} · 响亮 {ne} · 仅对拍 {len(CASES) - nv - ne}）'
      f' · drv.px {len(L)} 行 · 10 个 [B 类] API 缺席 {len(missing)} {missing}')
sys.exit(1 if missing else 0)
