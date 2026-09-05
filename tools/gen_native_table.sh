#!/usr/bin/env bash
# ============================================================
# gen_native_table.sh —— native 全量清单生成器（M69-S2）
# ------------------------------------------------------------
# 从 runtime/*.c 提取全部 `px_set_global("name", px_native(...))`
# 注册的 C native 名（单一事实源 = runtime 注册表），生成
# docs/native_index.json：{generator, source, count, names}。
# 口径：完整编译 281 项（含 QUIC/H3 族；--no-quic 裁剪后不含）。
# 幂等：同一 runtime 快照重跑输出字节一致。
# CI 对账（防漂移）：重跑本脚本 + git diff --exit-code docs/native_index.json；
#   人工表（docs/PUXIAN_CHEATSHEET.md §native）引用的计数须 == count。
# 运行：bash tools/gen_native_table.sh （仓库根目录）
# ============================================================
set -euo pipefail
cd "$(dirname "$0")/.."

names_file=$(mktemp)
grep -h 'px_set_global("' \
  runtime/runtime.c runtime/runtime_quic.c runtime/runtime_h3.c \
  runtime/runtime_h3_qpack_dyn.c runtime/runtime_ws.c \
  | sed -n 's/.*px_set_global("\([A-Za-z_][A-Za-z0-9_]*\)", px_native.*/\1/p' \
  | LC_ALL=C sort -u > "$names_file"   # LC_ALL=C 固定字节序排序：sort 输出不受 runner locale 影响（否则 en_US.UTF-8 与 C locale 排序不同 → CI 防漂移假红）

python3 - "$names_file" <<'PY'
import json, sys
names = sorted(l.strip() for l in open(sys.argv[1]) if l.strip())  # sorted 按码点 = 与 LC_ALL=C sort 一致，双保险防 locale 漂移
out = {"generator": "tools/gen_native_table.sh",
       "source": "runtime/*.c px_set_global(px_native)",
       "count": len(names), "names": names}
json.dump(out, open("docs/native_index.json", "w"), ensure_ascii=False, indent=1)
PY
count=$(wc -l < "$names_file")
rm -f "$names_file"
echo "written docs/native_index.json ($count natives)"
