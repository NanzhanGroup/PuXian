#!/usr/bin/env bash
# ============================================================
# packaging/selftest_pages_links.sh —— 落地页链接判据（离线回归）
# ------------------------------------------------------------
# 背景（2026-09-16 实测，东月）：
#   镜像站 https://soft.xiusoft.cn/puxian 与 https://soft.wsai.chat/puxian
#   **无尾斜杠时不会 301 补斜杠**，直接 200 返回落地页。此时浏览器把
#     href="version.json"            → 站点根 /version.json
#     href="rpm/PUXIAN-GPG-KEY.asc"  → 站点根 /rpm/PUXIAN-GPG-KEY.asc（404）
#   而 /version.json **返回 200**，内容却是**文殊 App 的版本文件**
#   （{"version":"1.7.141", ...}）⇒ 用户点「镜像自证文件」拿到的是**别的版本号**
#   —— 不报错、静默错，比 404 更危险。
# 判据 ⇒ 落地页所有站内链接必须**根相对（以 / 开头）**或**绝对 http(s)**，
#        禁止裸相对路径；两份目录（源站 / 镜像）与有无尾斜杠都能正确解析。
# 用法：bash packaging/selftest_pages_links.sh   （退出码 0 = 全过）
# ============================================================
set -uo pipefail

HTML="$(cd "$(dirname "$0")" && pwd)/pages/index.html"
[ -f "$HTML" ] || { echo "❌ 找不到 $HTML"; exit 2; }

PASS=0; FAIL=0
ok(){ echo "✅ $1"; PASS=$((PASS+1)); }
no(){ echo "❌ $1"; FAIL=$((FAIL+1)); }

echo "== 落地页链接判据（$HTML）=="

# ① 禁裸相对：href/src 的值必须以 / 或 http(s):// 或 # 或 mailto: 开头
bad="$(grep -o -E '(href|src)="[^"]*"' "$HTML" \
        | sed -E 's/^(href|src)="//; s/"$//' \
        | grep -v -E '^(/|https?://|#|mailto:)' || true)"
if [ -z "$bad" ]; then
  ok "① 无裸相对链接（全为 根相对 / 绝对 / 锚点）"
else
  no "① 发现裸相对链接（无尾斜杠 URL 下会解析到站点根）：$(echo "$bad" | tr '\n' ' ')"
fi

# ② 两个关键自证链接必须根相对
for k in "/puxian/version.json" "/puxian/rpm/PUXIAN-GPG-KEY.asc"; do
  if grep -q "href=\"$k\"" "$HTML"; then ok "② 含根相对链接 $k"; else no "② 缺根相对链接 $k"; fi
done

# ③ 安装命令是 shell 文本，必须保留绝对 URL
if grep -q 'curl -fsSL -o install-rpm.sh https://' "$HTML"; then
  ok "③ install 命令保留绝对 URL"
else
  no "③ install 命令的绝对 URL 丢了"
fi

echo "== 结果：通过 $PASS / 失败 $FAIL =="
[ "$FAIL" = 0 ] || exit 1
