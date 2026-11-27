#!/usr/bin/env bash
# ============================================================
# check_m448_png957.sh —— std.png 必须能真压缩，且**默认路径不许变**（缺陷 957）
# ------------------------------------------------------------
# 为什么有它：
#   缺陷 957 报「stdlib/png.px 只有 stored 无压缩，产物比真压缩大 3–10 倍」。
#   修法必须同时满足两个方向相反的要求：
#     ① 显式 opts.level>0 时要真的压（体积显著变小，且解码回像素逐字节一致）；
#     ② **默认（不传 opts）必须与 M61-S2 的 stored 产物逐字节相同** ——
#        否则所有既有调用点、既有金标 PNG、跨架构 sha256 对拍会一起漂移。
#   ⇒ 只测「变小」会漏掉 ②，只测「不变」会漏掉 ①，两边都要有牙。
#
# 判据：
#   C1 默认三参调用 ⇒ sha256 == 固定锚（与 M61-S2 stored 产物一致；锚值见 ANCHOR_STORED）
#   C2 level=6 产物：python3 独立解码（逐 chunk CRC + zlib.decompress）成功，
#      且像素与 C1 逐字节一致、raw 长度 == h*(1+3w)（无 filter 行）
#   C3 体积比 stored/compressed ≥ 3.0（缺陷 957 的原始判据）
#   C4 兼容/回退：显式 {"level":0} 与缺省逐字节相同；越界 level（99 / -1 / 字符串 "6"）
#      一律回退 stored，产物与 C1 逐字节相同（不产出坏文件、不崩）
#   N1 负控（有牙）：把 png.px 复制一份并把 level 分支改成永不走压缩，
#      同一探针的 C3 必须失败（体积比 ≈1）⇒ 证明 C3 不是恒真
#   G1 前置：tools/pxc / python3 / stdlib/png.px 缺失 ⇒ rc=3（不降级成 0）
#
# 用法：check_m448_png957.sh [--repo DIR] [--quick]
#   --quick 跳过 N1（省一次编译，排障用；常规门必跑）
# 退出码：0 全过 / 1 有违例 / 3 环境不成立
# ============================================================
set -u

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
QUICK=0
while [ $# -gt 0 ]; do
  case "$1" in
    --repo) ROOT="$2"; shift 2 ;;
    --quick) QUICK=1; shift ;;
    *) echo "❌ 未知参数：$1" >&2; exit 3 ;;
  esac
done

PXC="$ROOT/tools/pxc"
PNG="$ROOT/stdlib/png.px"
ANCHOR_STORED="7bee77b872740064865cdd001b8d1da922073c357753b00a756ad9eaf60f5b6b"   # 96x64 固定图，stored 产物 sha256
#   锚的来历：用 python 按 M61-S2 口径**独立构造**（sig+IHDR+IDAT stored(0x7801/adler32)+IEND，
#   逐 chunk CRC-32）得到的同一份字节 ⇒ 与「改前实现的实测产物」sha256 相同（改前实现已实测两次，
#   另一张图 + examples/m61_gfx/demo_mandelbrot.px 产物与仓库金标 mandelbrot.png 逐字节一致）。

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "=== 缺陷 957 · std.png 真压缩（M448）==="

# ---- G1 前置 ----
[ -x "$PXC" ]   || { echo "❌ G1 缺 tools/pxc ⇒ rc=3"; exit 3; }
[ -f "$PNG" ]   || { echo "❌ G1 缺 stdlib/png.px ⇒ rc=3"; exit 3; }
command -v python3 >/dev/null 2>&1 || { echo "❌ G1 缺 python3 ⇒ rc=3"; exit 3; }
ok "G1 前置：tools/pxc / stdlib/png.px / python3 就位"

TMP="$(mktemp -d "/tmp/m448png957.$$.XXXXXX")" || { echo "❌ 无法建临时目录 ⇒ rc=3"; exit 3; }
trap 'rm -rf "$TMP"' EXIT

# ---- 探针源码（同一个图像数据，只用 LIB 区分被测实现）----
probe() { # $1=lib 导入行  $2=输出前缀
  cat > "$TMP/$2.px" <<EOF
$1
var px = []
var y = 0
while y < 64:
    var x = 0
    while x < 96:
        var r = 0x10
        var g = 0x10
        var b = 0x28
        if y < 16:
            r = 0x22
            g = 0x44
            b = 0x88
        elif y < 24:
            r = (x * 255) // 95
            g = (y * 255) // 63
            b = ((x + y) * 255) // 158
        elif y < 61 and x >= 44 and x < 76:
            if ((x // 4) + (y // 4)) % 2 == 0:
                r = 0xff
                g = 0xcc
                b = 0x22
            else:
                r = 0x44
                g = 0x66
                b = 0xff
        px.append((r * 65536) + (g * 256) + b)
        x = x + 1
    y = y + 1
var s0 = png_encode(96, 64, px)
var s0b = png_encode(96, 64, px, {"level": 0})
var s6 = png_encode(96, 64, px, {"level": 6})
var s99 = png_encode(96, 64, px, {"level": 99})
var sm1 = png_encode(96, 64, px, {"level": -1})
var ss = png_encode(96, 64, px, {"level": "6"})
write_bytes("$TMP/$2-default.png", s0)
write_bytes("$TMP/$2-lv0.png", s0b)
write_bytes("$TMP/$2-lv6.png", s6)
write_bytes("$TMP/$2-lv99.png", s99)
write_bytes("$TMP/$2-lvm1.png", sm1)
write_bytes("$TMP/$2-lvstr.png", ss)
print("STORED", bytes_len(s0), "LV6", bytes_len(s6))
EOF
}

build_run() { # $1=输出前缀
  if ! "$PXC" build "$TMP/$1.px" > "$TMP/$1.build.log" 2>&1; then
    bad "编译失败（$1）：$(tail -2 "$TMP/$1.build.log" | tr '\n' ' ')"
    return 1
  fi
  if ! ( cd "$TMP" && "./build/$1" > "$TMP/$1.run.log" 2>&1 ); then
    bad "运行失败（$1）：$(tail -2 "$TMP/$1.run.log" | tr '\n' ' ')"
    return 1
  fi
  return 0
}

# ---- 正控：真 stdlib/png.px ----
probe 'import std.png' real
if build_run real; then
  ok "C0 探针编译+运行（import std.png）"

  # C1 默认路径锚
  got="$(sha256sum "$TMP/real-default.png" | cut -d' ' -f1)"
  if [ "$got" = "$ANCHOR_STORED" ]; then
    ok "C1 默认三参调用 = M61-S2 stored 锚（sha256 ${got:0:16}…）"
  else
    bad "C1 默认产物漂移：got ${got:0:16}… want ${ANCHOR_STORED:0:16}…（默认路径不许变）"
  fi

  # C2/C3/C4 由 python 独立校验（解码 + 逐 chunk CRC + 像素对拍 + 体积比 + 回退一致）
  if python3 - "$TMP" <<'PY'
import struct, sys, zlib, hashlib, os
D = sys.argv[1]
def load(p):
    d = open(p, 'rb').read()
    assert d[:8] == b'\x89PNG\r\n\x1a\n', "签名错"
    off, idat, ihdr, nch = 8, b'', None, 0
    while off < len(d):
        ln = struct.unpack('>I', d[off:off+4])[0]
        typ = d[off+4:off+8]; data = d[off+8:off+8+ln]
        crc = struct.unpack('>I', d[off+8+ln:off+12+ln])[0]
        assert crc == zlib.crc32(typ+data) & 0xffffffff, "chunk CRC 错 %r" % typ
        nch += 1
        if typ == b'IHDR': ihdr = struct.unpack('>IIBBBBB', data)
        if typ == b'IDAT': idat += data
        off += 12 + ln
    assert ihdr[:2] == (96, 64) and ihdr[2] == 8 and ihdr[3] == 2 and ihdr[6] == 0, ihdr
    w, h = ihdr[0], ihdr[1]
    raw = zlib.decompress(idat)
    assert len(raw) == h * (1 + 3*w), "raw 长度错 %d" % len(raw)
    for y in range(h):
        assert raw[y*(1+3*w)] == 0, "filter 非 0"
    return w, h, raw, nch, len(idat)

f0 = os.path.join(D, 'real-default.png')
f6 = os.path.join(D, 'real-lv6.png')
w, h, raw0, n0, id0 = load(f0)
w6, h6, raw6, n6, id6 = load(f6)
assert raw0 == raw6, "C2 像素不一致"
print("  ✅ C2 level=6 产物 python 独立解码通过（chunk CRC ×%d）· 像素与 stored 逐字节一致" % n0)
ratio = float(id0) / id6
assert ratio >= 3.0, "C3 体积比不足：%.2fx" % ratio
print("  ✅ C3 体积比 stored/compressed = %.1fx（≥3.0；IDAT %d→%d 字节）" % (ratio, id0, id6))

base = hashlib.sha256(open(f0,'rb').read()).hexdigest()
for name, lbl in [('lv0','{"level":0}'), ('lv99','{"level":99}'), ('lvm1','{"level":-1}'), ('lvstr','{"level":"6"}')]:
    p = os.path.join(D, 'real-%s.png' % name)
    got = hashlib.sha256(open(p,'rb').read()).hexdigest()
    assert got == base, "C4 %s 未回退到 stored 字节" % lbl
print("  ✅ C4 level=0 显式 / 99 / -1 / \"6\" 全部回退为 stored 且逐字节相同（不产坏文件、不崩）")
PY
  then :; else bad "C2/C3/C4 python 判据未过（见上）"; fi
fi

# ---- N1 负控：把 level 分支阉掉 ⇒ C3 必须失败 ----
if [ "$QUICK" = "0" ]; then
  cp "$PNG" "$TMP/brokenmod.px"
  python3 - "$TMP/brokenmod.px" <<'PY'
import sys
p = sys.argv[1]; s = open(p).read()
old = "    if level > 0:"
assert s.count(old) == 1, "负控注入点找不到（判据需跟着实现走）"
open(p, 'w').write(s.replace(old, "    if level > 900:"))
PY
  if [ $? -ne 0 ]; then
    bad "N1 负控注入失败（实现变了，判据要同步）"
  else
    probe 'import "brokenmod.px"' neg
    if build_run neg; then
      if python3 - "$TMP" <<'PY'
import struct, sys, zlib
D = sys.argv[1]
def idat(p):
    d = open(p,'rb').read(); off = 8; out = b''
    while off < len(d):
        ln = struct.unpack('>I', d[off:off+4])[0]
        if d[off+4:off+8] == b'IDAT': out += d[off+8:off+8+ln]
        off += 12 + ln
    return out
a = idat(D + '/neg-default.png'); b = idat(D + '/neg-lv6.png')
ratio = float(len(a)) / len(b)
if ratio >= 3.0:
    print("  ✗ 负控仍压到 %.2fx ⇒ level 分支没被阉 / C3 恒真" % ratio)
    sys.exit(1)
print("  ✓ 负控体积比 %.2fx（<3.0，正控口径会判红）" % ratio)
PY
      then ok "N1 负控有牙：阉掉 level 分支后体积比不达标 ⇒ C3 真能红"
      else bad "N1 负控未变红（等同 C3 无牙）"
      fi
    fi
  fi
else
  echo "  ⏭ N1 负控已跳过（--quick）"
fi

echo "--- 汇总：pass=$pass fail=$fail ---"
if [ "$fail" -eq 0 ]; then
  echo "M448_PNG957-OK"
  exit 0
fi
echo "M448_PNG957-FAIL"
exit 1
