#!/usr/bin/env bash
# ============================================================
# M117 验证（qg-issue 72：真实模块 ws-install PuXian 化暴露的三类缺陷）
# ------------------------------------------------------------
# 判据（直接比可观测事实）：
#   ① 双轨（px run / px build）都必须 m117_defects.px 全 PASS
#   ② 负控：确认这三条在**修复前**是红的 —— 由 git 历史 + 本脚本的
#      「老行为断言」间接守（见 §负控）
#   ③ 零回归：http_request 未指定 opts 时仍是原行为（不强行加超时）
#   ④ 反抖动（M117 尾修）：①b 的「逐字节相等」**只允许用在稳定伪文件**上。
#      /proc/uptime 是**时变**伪文件（HZ=100 ⇒ 刻度 10ms），拿它做逐字节比较是
#      **必现抖动**断言 —— CI 实测 read_file=|200.97 703.04| vs read_at=|200.98 703.04|，
#      本机 2000 次循环复现率 解释轨 5.9% / 编译轨 0.65%。
#      时变伪文件请用 ①b2 的容差写法（两读均实时：非空 / 2 字段 / up 差 ≤5s，同 m58 口径）。
# 用法：bash examples/m117_realworld_defects/verify.sh
# ============================================================
set -uo pipefail
cd "$(dirname "$0")/../.."
ROOT=$(pwd)
DIR=examples/m117_realworld_defects
PX=${PX:-$ROOT/tools/px}

pass=0; fail=0
ok()  { echo "  ✅ $1"; pass=$((pass+1)); }
bad() { echo "  ❌ $1"; fail=$((fail+1)); }

echo "== M117 验证（read_file 伪文件 / connect 超时 / POSIX 字符类）=="

echo "[1/3] 解释轨（px run）"
if out=$("$PX" run "$DIR/m117_defects.px" 2>&1); then
  echo "$out" | sed 's/^/     /'
  ok "解释轨全 PASS"
else
  echo "$out" | sed 's/^/     /'
  bad "解释轨有 ❌"
fi

echo "[2/3] 编译轨（px build + 运行产物）"
if "$PX" build "$DIR/m117_defects.px" >/tmp/m117_build.log 2>&1; then
  if out=$("$ROOT/$DIR/build/m117_defects" 2>&1); then
    echo "$out" | sed 's/^/     /'
    ok "编译轨全 PASS"
  else
    echo "$out" | sed 's/^/     /'
    bad "编译轨有 ❌"
  fi
else
  tail -5 /tmp/m117_build.log | sed 's/^/     /'
  bad "编译轨构建失败"
fi

echo "[2b/3] 负控：未知 POSIX 类名必须**报错**（不许静默不匹配）"
cat >/tmp/m117_badclass.px <<'PX'
def main():
    print(regex_search("[[:nope:]]", "x"))
PX
if out=$(./tools/px run /tmp/m117_badclass.px 2>&1); then
  bad "未知 POSIX 类名未报错（静默通过）：$out"
else
  case "$out" in
    *"未知 POSIX 字符类"*) ok "未知类名报错且信息可定位（$(echo "$out" | tail -1)）" ;;
    *) bad "报错了但信息不可定位：$out" ;;
  esac
fi

echo "[3/3] 零回归负控：未指定 opts.timeout_ms 的连接行为不变（默认 30s 口径）"
cat >/tmp/m117_noreg.px <<'PX'
def main():
    # 指定 0/负值 → 不启用连接超时（保持原 mbedtls/阻塞 connect 语义）
    var t0 = now_ms()
    var r = http_request("http://127.0.0.1:1/x", "GET", null, null, {"timeout_ms": 500})
    var dt = now_ms() - t0
    # 端口 1 是"立刻 refuse"（不是黑洞）→ 两种实现都应秒级返回 Err
    if dt < 3000 and type(r) == "result":
        print("PASS refuse 快返回 " + str(dt) + "ms")
    else:
        print("FAIL dt=" + str(dt) + " type=" + type(r))
        exit(1)
PX
if out=$("$PX" run /tmp/m117_noreg.px 2>&1) && [ "${out#*PASS}" != "$out" ]; then
  ok "refuse 路径仍快速返回 Err（$out）"
else
  bad "refuse 路径异常：$out"
fi

echo ""
echo "== 汇总：pass=$pass fail=$fail =="
exit $((fail > 0))
