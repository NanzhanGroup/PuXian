#!/usr/bin/env bash
# ============================================================
# M61 S2 verify —— gfx.px + png.px + 双 demo（B 线核心可见性）
# ------------------------------------------------------------
# 验证点：
#   A. pxc build demo_mandelbrot.px / demo_scene.px（import std.gfx + std.png）
#   B. 运行 → PNG 落盘（mandelbrot.png / scene.png，均 640x480）
#   C. python3 独立解码（stdlib zlib，无第三方依赖）：
#      - PNG 签名 / chunk 遍历 / 每 chunk CRC32 全校验（zlib.crc32 独立确认）
#      - IHDR 640x480 / 8bit RGB / IDAT zlib 解压 / filter=0 行结构 / 长度精确
#      - 像素抽查：scene 太阳中心黄+底色+边框蓝；mandelbrot M 集内部黑+外部有色+标题有白字
#   D. 行数约束（gfx.px / png.px / 双 demo < 500）
# 说明：视觉复核由工作流 send_file 发图给用户眼见
# ============================================================
set -u
cd "$(dirname "$0")/../.."          # 仓库根（demo 相对路径写盘依赖 cwd=根）
PXC=./tools/pxc
D=examples/m61_gfx
B=$D/build
fail() { echo "M61-S2 FAIL: $*" >&2; exit 1; }
ok()   { echo "  ✅ $*"; }

echo "== A. 编译双 demo =="
$PXC build "$D/demo_mandelbrot.px" >/dev/null 2>&1 || fail "demo_mandelbrot 编译失败"
$PXC build "$D/demo_scene.px" >/dev/null 2>&1 || fail "demo_scene 编译失败"
ok "双 demo 编译（import std.gfx/std.png 静态链接）"

echo "== B. 运行产出 PNG =="
O1=$("$B/demo_mandelbrot" 2>&1) || fail "demo_mandelbrot 运行失败: $O1"
echo "$O1"
O2=$("$B/demo_scene" 2>&1) || fail "demo_scene 运行失败: $O2"
echo "$O2"
[ -f "$D/mandelbrot.png" ] || fail "未产出 mandelbrot.png"
[ -f "$D/scene.png" ] || fail "未产出 scene.png"
ok "PNG 落盘"

echo "== C. python3 独立解码（zlib 全校验 + 像素抽查）=="
if command -v python3 >/dev/null 2>&1; then
    python3 - "$D" <<'PY'
import struct, zlib, sys
D = sys.argv[1]

def load(fn):
    data = open(fn, 'rb').read()
    assert data[:8] == b'\x89PNG\r\n\x1a\n', '签名错误'
    off = 8
    w = h = None
    idat = b''
    nchunk = 0
    while off < len(data):
        ln, = struct.unpack('>I', data[off:off+4])
        typ = data[off+4:off+8]
        chunk = data[off+8:off+8+ln]
        crc, = struct.unpack('>I', data[off+8+ln:off+12+ln])
        assert crc == (zlib.crc32(typ + chunk) & 0xffffffff), f'{fn} chunk {typ} CRC 错误'
        if typ == b'IHDR':
            w, h = struct.unpack('>II', chunk[:8])
            assert chunk[8:13] == b'\x08\x02\x00\x00\x00', '须 8bit RGB filter0'
        elif typ == b'IDAT':
            idat += chunk
        off += 12 + ln
        nchunk += 1
    assert off == len(data), '尾部多余'
    assert w == 640 and h == 480, f'尺寸 {w}x{h}'
    raw = zlib.decompress(idat)
    assert len(raw) == h * (1 + 3 * w), f'行数据长度 {len(raw)}'
    return w, h, raw

def px(raw, w, x, y):
    row = raw[y * (1 + 3 * w):(y + 1) * (1 + 3 * w)]
    assert row[0] == 0, f'行 {y} filter 应 0'
    return row[1 + 3 * x:1 + 3 * x + 3]

def rgb(t):
    return (t[0], t[1], t[2])

ok = True
def chk(name, cond, msg):
    global ok
    if cond:
        print(f"  ✅ {name}")
    else:
        ok = False
        print(f"  ✗ {name}: {msg}")

# scene：确定性绘制元素
w, h, raw = load(f"{D}/scene.png")
chk("scene 太阳中心黄", rgb(px(raw, w, 320, 220)) == (255, 204, 34), str(rgb(px(raw, w, 320, 220))))
chk("scene 底部底色", rgb(px(raw, w, 10, 460)) == (16, 16, 40), str(rgb(px(raw, w, 10, 460))))
chk("scene 左边框蓝", rgb(px(raw, w, 4, 240)) == (68, 102, 255), str(rgb(px(raw, w, 4, 240))))
# scene 精灵区：黄色棋盘（130,260）起点应有黄像素
found_yellow = any(rgb(px(raw, w, 130 + xx, 260 + yy)) == (255, 255, 0) for yy in range(0, 8, 2) for xx in range(0, 8, 2))
chk("scene blit 精灵黄点", found_yellow, "精灵区无黄色")

# mandelbrot：M 集内部黑 / 外部 escape 有色 / 标题白字
w, h, raw = load(f"{D}/mandelbrot.png")
chk("mandelbrot M集内部黑(0,0)", sum(rgb(px(raw, w, 427, 240))) == 0, str(rgb(px(raw, w, 427, 240))))
chk("mandelbrot 外部有色(-2,-1.19)", sum(rgb(px(raw, w, 2, 2))) > 0, str(rgb(px(raw, w, 2, 2))))
has_white = False
for yy in range(8, 15):
    for xx in range(8, 140):
        if rgb(px(raw, w, xx, yy)) == (255, 255, 255):
            has_white = True
chk("mandelbrot 标题白字", has_white, "标题区无白像素")

if ok:
    print("  ✅ PNG 全 chunk CRC 校验 + zlib 解压 + 像素抽查全过（第三方独立确认编码正确）")
else:
    sys.exit(1)
PY
else
    echo "  ⚠️ 无 python3，跳过解码（PNG 结构 xxd 快速核验）"
    for png in mandelbrot scene; do
        F="$D/$png.png"
        [ "$(head -c 8 "$F" | xxd -p)" = "89504e470d0a1a0a" ] || fail "$png.png 签名错误"
        xxd -p "$F" | tr -d '\n' | grep -q "49454e44" || fail "$png.png 缺 IEND"
        ok "$png.png 签名/IEND OK"
    done
fi

echo "== D. 行数约束 =="
for f in stdlib/gfx.px stdlib/png.px "$D/demo_mandelbrot.px" "$D/demo_scene.px"; do
    LC=$(wc -l < "$f")
    [ "$LC" -lt 500 ] || fail "$f 超 500 行: $LC"
    ok "$f $LC 行 < 500"
done

echo "M61-S2 verify PASS"
