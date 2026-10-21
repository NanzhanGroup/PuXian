#!/usr/bin/env python3
# ══════════════════════════════════════════════════════════════════════
# shard_verdict.py —— 「并行结果 == 串行结果」的逐门裁决（M304 步3）
#
# 判据四条（任一不满足 ⇒ RED）：
#   ① 分片里**没有红门**（状态 != OK）
#   ② 串行有、分片无 ⇒ 缺门（漏跑）
#   ③ 分片有、串行无 ⇒ 只允许**本轮新增的那一门**
#   ④ 同一门的 (状态, rc) 必须**逐门一致**；唯一豁免 = 串行档里"修复前跑的"那一门
#      （m208：基线是 FAIL、本轮修复后应 OK —— 这条是**预期内**的差异，不是漂移）
#
# 用法：shard_verdict.py <串行基线 times.tsv> <分片 times.tsv 合并>
# 退出码：0 = GREEN · 1 = RED
# ══════════════════════════════════════════════════════════════════════
import sys

NEW_GATES = {'m309_pxpkg_sync',        # M309 C3·W4：pxpkg sync 契约夹门（串行基线早于它）
             'm304_patch_primitive',        # M304：打桩原语门
             'spec_consistency',            # M305：性能基线口径 ⇄ 实现 的机器核对（补注册；串行基线 TSV 早于它）
             'selfwake_guard',              # 用户令 2026-10-12：禁止自建平行告警/唤醒通道 的护栏门
             'm306_onnx_quant',             # M306：ONNX 量化算子面（DequantizeLinear/QuantizeLinear/DynamicQuantizeLinear/MatMulInteger）
             'm308_onnx_ops',
             'm315_onnx_ort_kernel'}   # M315：ORT kernel 真值方向门（串行基线 /tmp/m314/gates.times.tsv 早于它）               # M308：ONNX 推理算子补齐 S4（Gelu/Silu/RMSNorm/ScatterND/Cos/Sin + Trilu）
                                            #   ⭐ M309 阶段A' 实测补登：该门在 **registry 与 ci.yml 双向都在位**
                                            #      （gates.registry.sh:1526 / ci.yml:1289，check_gate_registry 全绿），
                                            #      只是**串行基线 TSV 早于它** ⇒ 旧「期望集」漏了它，会误报「多余门」。
                                            #      申报口径不变：只登记**基线之后新增的正式门**。
EXPECT_FIXED = {'m208_vm_c_local_roots',      # 基线里 FAIL、本轮修复后应为 OK
                'm131_stream_post',            # M314 修：固定端口被遗留 mock_mysql 占住（假红）
                'm312_gc_lock_order',          # M314 修：固定端口被上轮遗留 build/srv 占住（假红）
                'emitc_freeze'}                # M314 修：M313 语料扩展后重定基


def load(p, prefer_run=False):
    """prefer_run=True：同一门出现多行时（分片的 times.tsv 会为**未选中**的门写 SKIP），
    取「真正跑过」的那一行。首版没做这件事 ⇒ 630 行 SKIP 把 209 门全部覆盖成 SKIP
    ⇒ 误报「分片红门 210 / 不一致 209」（判据读错了对象，2026-10-12 实测抓回）。"""
    d = {}
    for ln in open(p, encoding='utf-8', errors='ignore'):
        f = ln.rstrip('\n').split('\t')
        if len(f) < 2:
            continue
        rec = (f[1], f[3] if len(f) > 3 else '-')
        if prefer_run:
            if f[0] not in d or (d[f[0]][0] == 'SKIP' and rec[0] != 'SKIP'):
                d[f[0]] = rec
        else:
            d[f[0]] = rec
    return d


ser = load(sys.argv[1])
par = load(sys.argv[2], prefer_run=True)

red = sorted(k for k, v in par.items() if v[0] != 'OK')
miss = sorted(set(ser) - set(par))
extra = sorted(set(par) - set(ser))
bad_extra = [g for g in extra if g not in NEW_GATES]
miss_new = sorted(g for g in NEW_GATES if g not in par)
diff = []
for k in sorted(ser):
    if k in par and ser[k] != par[k]:
        if k in EXPECT_FIXED and par[k][0] == 'OK':
            continue                               # 预期内的修复
        diff.append((k, ser[k], par[k]))

ok = (not red) and (not miss) and (not bad_extra) and (not miss_new) and (not diff)

print('== 分片 ⇄ 串行 逐门比对 ==')
print('串行基线 %d 门 · 分片 %d 门' % (len(ser), len(par)))
print('① 分片红门            : %d %s' % (len(red), red[:8]))
print('② 缺失（串行有分片无）: %d %s' % (len(miss), miss[:8]))
print('③ 多余（分片有串行无）: %d %s（期望仅 %s）' % (len(extra), extra[:8], sorted(NEW_GATES)))
print('④ 状态/rc 不一致      : %d' % len(diff))
for k, s, p in diff[:10]:
    print('     %s: 串行=%s 分片=%s' % (k, s, p))
print('SHARD-VERDICT-' + ('GREEN' if ok else 'RED'))
sys.exit(0 if ok else 1)
