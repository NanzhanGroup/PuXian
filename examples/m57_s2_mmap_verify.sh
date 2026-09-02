#!/usr/bin/env bash
# ============================================================
# M57 S2：fd 数据通道 + mmap 设备映射 端到端验证
#   语言新内建：read(fd,n)→bytes / write(fd,data)→int /
#              mmap(fd,len[,off])→bytes / munmap(bytes)→bool /
#              mem_write(mmap_bytes,off,data)→int（就地写映射区）
# 验证点（全部在 .px 内 assert，失败即非 0 退出）：
#   A. fd read/write：TCP 环回 write/read、文件 fd 顺序写、非法 fd -1+EBADF
#   B. mmap MAP_SHARED：整视图、offset 页对齐子视图、双向活映射可见
#      （mem_write→read_at / write_at→视图）、mem_write 超长截断
#   C. mmap 视图作 ioctl 就地 buffer（FIONREAD 内核写映射区）
#   D. munmap 显式解除（len=0 / 重复 false / 非映射 false）
#   E. GC sweep 自动 munmap（300 轮循环 + gc() 不崩）
#   F. mmap 失败路径 -1 + errno=EBADF
# ============================================================
set -u
cd "$(dirname "$0")/.."
PXC=./tools/pxc
B=examples/build

echo "== build =="
$PXC build examples/m57_s2_mmap.px >/dev/null 2>&1 || { echo "FAIL: m57_s2_mmap.px 编译失败"; exit 1; }

echo "== run =="
OUT=$($B/m57_s2_mmap 2>&1)
RC=$?
echo "$OUT"
echo "== result =="
if [ $RC -ne 0 ]; then
    echo "❌ M57-S2: mmap 自检退出码 $RC"
    exit 1
fi
echo "$OUT" | grep -q "A: TCP write/read 环回 ok" || { echo "❌ M57-S2: A1 TCP 环回未出现"; exit 1; }
echo "$OUT" | grep -q "A: 文件 fd 顺序写 8192B ok" || { echo "❌ M57-S2: A2 文件写未出现"; exit 1; }
echo "$OUT" | grep -q "B: 整视图 8192B ok" || { echo "❌ M57-S2: B1 整视图未出现"; exit 1; }
echo "$OUT" | grep -q "B: offset=4096 页对齐子视图 ok" || { echo "❌ M57-S2: B2 子视图未出现"; exit 1; }
echo "$OUT" | grep -q "B: mem_write→read_at 可见" || { echo "❌ M57-S2: B3a 写回可见未出现"; exit 1; }
echo "$OUT" | grep -q "B: write_at→视图可见 ok" || { echo "❌ M57-S2: B3b 文件写→视图未出现"; exit 1; }
echo "$OUT" | grep -q "C: FIONREAD 写入 mmap 映射区" || { echo "❌ M57-S2: C ioctl buffer 未出现"; exit 1; }
echo "$OUT" | grep -q "D: munmap 解除语义 ok" || { echo "❌ M57-S2: D munmap 未出现"; exit 1; }
echo "$OUT" | grep -q "E: 300 轮 mmap + gc() 自动回收无崩 ok" || { echo "❌ M57-S2: E GC 回收未出现"; exit 1; }
echo "$OUT" | grep -q "F: mmap(非法fd) -> -1 errno=EBADF ok" || { echo "❌ M57-S2: F errno 未出现"; exit 1; }
echo "$OUT" | grep -q "M57-S2 mmap OK" || { echo "❌ M57-S2: 未达 OK 行"; exit 1; }
rm -f /tmp/m57_s2_map.bin
echo "✅ M57-S2: fd 数据通道 + mmap 设备映射 PASS"
