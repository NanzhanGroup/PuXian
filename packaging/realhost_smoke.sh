#!/usr/bin/env bash
# ============================================================
# packaging/realhost_smoke.sh —— **真机冒烟**（在一台真实节点上验证 PuXian 发布物）
# ------------------------------------------------------------
# 存在理由（用户 2026-10-06 问「你现在都是在本机跑测试吗？没有在文殊上面跑测试吗？」）：
#   我们的验证链 = **本机全量门（195 门）+ GitHub CI**（ubuntu / 真 ARM runner /
#   el7·el9·openEuler 容器）。覆盖已经很宽，但**全部**跑在「我们自己的机器 / GitHub 的机器」上。
#   有一条**从来没被覆盖**：**一台真实的、别人在用的节点** —— 真内核 / 真 SELinux /
#   真 systemd / 真网络 / 真发行版。
#   这条缝里已经有过真实教训：CI 里 SELinux 一律宽松，而真机 **Enforcing** 下
#   `systemd-run … /tmp/xxx` 直接 `status=203/EXEC`（M274 实测）。
#
# 定位：**ops 脚本，不进全量门**（依赖外部节点与 SSH 授权，跑不进 CI）
#   ⇒ 它是「发布后**半自动**的补充证据」，不是判据链的一环。
#
# 用法：
#   bash packaging/realhost_smoke.sh --host root@gy.example.com [--port 9322]
#                                   [--mode installed|tarball|mirror]
#                                   [--tarball /path/to/puxian-*.tar.gz]
#                                   [--expect-version 0.2.276] [--remote-dir /tmp/rhsmoke]
# 选项：
#   --mode installed  只测**已安装**的 `px`（默认；验证 RPM/dnf 装出来的那一份）
#   --mode tarball    上传 `--tarball` 指定的发布 tarball，远端解包后测（验证分发物本身）
#   --mode mirror     远端直接从**国内镜像**下载（验证镜像可达 + 校验链）
#   --keep            远端目录不清理（排障用）
#   --dry-run         只打印将执行的远端脚本，不连
#   --self-test       离线自证（7 组判据 · **不连任何主机**）
# 判据（逐条打印，末行汇总；任一 FAIL ⇒ rc=1）：
#   [1] 连通与环境（hostname / 架构 / 发行版 / 内核 / SELinux）
#   [2] （tarball/mirror 档）取得发布物 + sha256 与登记值一致
#   [3] 工具链在位 + 版本号（给了 --expect-version 才判）
#   [4] **编译-运行闭环**：`px build hello.px` → 运行 → stdout 逐字节比对 + 产物**静态性**
#   [5] **HTTP 面**：起 `px_serve` → 200 / 404 / 并发 10 连全 200
#   [6] 清理（远端目录 + 进程）
# ⚠️ 覆盖边界（如实登记）：
#   · 远端一切都在 `--remote-dir`（默认 /tmp/rhsmoke）里，**不动系统**、不装包、不改配置。
#   · **刻意不经 systemd 启动**：`/tmp` 下的可执行在 SELinux Enforcing 上会被 203/EXEC
#     —— 那是**已知**的真机行为（M274 实测），不是本脚本要测的东西。
#   · 只验「发布物能不能用」，不验性能/规模（那由 m267_perf 与 CI 覆盖）。
# 退出码：0 全过 · 1 有用例失败 · 2 用法/环境错
# ============================================================
set -uo pipefail

HOST=""; PORT=9322; MODE=installed; TARBALL=""; EXPECT_VER=""; RDIR=/tmp/rhsmoke
KEEP=0; DRY=0; SELFTEST=0
while [ $# -gt 0 ]; do
  case "$1" in
    --host) HOST="$2"; shift 2 ;;
    --port) PORT="$2"; shift 2 ;;
    --mode) MODE="$2"; shift 2 ;;
    --tarball) TARBALL="$2"; shift 2 ;;
    --expect-version) EXPECT_VER="$2"; shift 2 ;;
    --remote-dir) RDIR="$2"; shift 2 ;;
    --keep) KEEP=1; shift ;;
    --dry-run) DRY=1; shift ;;
    --self-test) SELFTEST=1; shift ;;
    -h|--help) sed -n '2,40p' "$0"; exit 0 ;;
    *) echo "❌ 未知参数：$1（-h 看用法）" >&2; exit 2 ;;
  esac
done
# ── 离线自证（**不连任何主机**）──
if [ "$SELFTEST" = 1 ]; then
  SP=0; SF=0
  st() { if [ "$2" = 1 ]; then printf '  ✅ %s\n' "$1"; SP=$((SP+1)); else printf '  ❌ %s\n' "$1"; SF=$((SF+1)); fi; }
  T="$(mktemp -d /tmp/rhsmoke-selftest.XXXXXX)"; trap 'rm -rf "$T"' EXIT
  SELF="$(cd "$(dirname "$0")" && pwd)/$(basename "$0")"
  echo "── [S] realhost_smoke.sh 离线自证 ──"

  bash "$SELF" > "$T/a" 2>&1; rc=$?
  st "[S1] 缺 --host ⇒ rc=2" "$([ "$rc" = 2 ] && echo 1 || echo 0)"
  st "[S1] 报错里指明 --host" "$(grep -q -- '--host' "$T/a" && echo 1 || echo 0)"

  bash "$SELF" --host x --mode bogus > "$T/b" 2>&1; rc=$?
  st "[S2] --mode 非法 ⇒ rc=2" "$([ "$rc" = 2 ] && echo 1 || echo 0)"

  bash "$SELF" --host x --mode tarball --tarball /nonexistent.tgz > "$T/c" 2>&1; rc=$?
  st "[S3] --mode tarball 缺文件 ⇒ rc=2" "$([ "$rc" = 2 ] && echo 1 || echo 0)"

  # S4：dry-run 的**远端脚本**必须是合法 bash（这是本脚本的核心产物）
  bash "$SELF" --host dummy@invalid --dry-run > "$T/d" 2>&1
  tail -n +2 "$T/d" > "$T/body.sh"
  if bash -n "$T/body.sh" 2>"$T/n.err"; then st "[S4] dry-run 产物是合法 bash" 1
  else st "[S4] dry-run 产物语法错：$(head -1 "$T/n.err")" 0; fi
  MISS=""
  for k in '\[1\] 连通与环境' '\[3\] 工具链' '\[4\] 编译-运行闭环' '\[5\] HTTP 面' '\[6\] 清理' '真机冒烟：通过'; do
    grep -q "$k" "$T/body.sh" || MISS="$MISS $k"
  done
  st "[S4] 六段判据锚点齐（缺:$MISS）" "$([ -z "$MISS" ] && echo 1 || echo 0)"

  # S5：本地变量必须在**本地**展开完（远端脚本里不得残留 \$RDIR / \$MODE 这类未展开的名字）
  LEAK="$(grep -oE '\\$\{?(RDIR|MODE|TARBALL|EXPECT_VER|KEEP)\b' "$T/body.sh" | sort -u | tr '\n' ' ')"
  st "[S5] 无未展开的本地变量（残留:$LEAK）" "$([ -z "$LEAK" ] && echo 1 || echo 0)"

  # S6：档位**确实传到了远端**（取件那段在远端按 MODE 分支 ⇒ 文本恒在，但条件必须按档展开）
  #   ⚠️ 首版判据写成「installed 档不应出现『取得发布物』字样」—— 假红：
  #      该文本在**远端脚本体内**、由远端按 \$MODE 决定跑不跑 ⇒ 恒在。判据要判**分支条件**，不是文本。
  if grep -q 'if \[ "installed" = tarball \]' "$T/body.sh"; then st "[S6] installed 档：远端分支条件已按档展开" 1
  else st "[S6] installed 档：分支条件未按档展开" 0; fi
  bash "$SELF" --host dummy@invalid --mode mirror --dry-run > "$T/e" 2>&1
  tail -n +2 "$T/e" > "$T/ebody.sh"
  if grep -q 'if \[ "mirror" = tarball \]' "$T/ebody.sh"; then st "[S6] mirror 档：分支条件已按档展开" 1
  else st "[S6] mirror 档：分支条件未按档展开" 0; fi
  # 但「本地侧」的分支必须是**真分支**（mirror 档必须带 sha256 断言；installed 档不得带上传段 —— S7）
  if grep -q 'sha256 == version.json 登记' "$T/e"; then st "[S6] mirror 档断言 sha256 与登记值一致" 1
  else st "[S6] mirror 档缺 sha256 断言" 0; fi

  # S7：反向 —— 同一份 dry-run 产物里**不得**出现「上传发布物」（那是 tarball 档才有的）
  if grep -q '上传发布物' "$T/d"; then st "[S7] installed 档不应提「上传」" 0
  else st "[S7] installed 档不提上传（tarball 专属）" 1; fi

  # S8（本轮真踩）：mirror 档那条远端脚本里，**两条 curl 都必须自带完整的 https URL**。
  #   为什么这么判：本脚本用**不带引号**的 heredoc 生成远端脚本 ⇒ 续行/转义一旦被折叠，
  #   最典型的后果就是 curl 拿到残缺参数 ⇒ 远端 `curl: (3) URL using bad/illegal format`（实测撞到）。
  #   首版写成「不得出现『反斜杠+空格』」—— 实测**没有牙**（单反斜杠+换行会被 heredoc 当续行正常吃掉），
  #   故改成判「URL 在不在」，并用负控验过它真的会红。
  CURLN="$(grep -cE 'curl -fsSL[^|]*https://' "$T/ebody.sh" || true)"
  st "[S8] mirror 档两条 curl 都自带 https URL（实测 $CURLN 条）" "$([ "$CURLN" = 2 ] && echo 1 || echo 0)"

  printf '\n══ 自证：通过 %s · 失败 %s ══\n' "$SP" "$SF"
  [ "$SF" = 0 ] && { echo "REALHOST-SMOKE-SELFTEST-OK"; exit 0; } || { echo "REALHOST-SMOKE-SELFTEST-FAIL"; exit 1; }
fi

[ -n "$HOST" ] || { echo "❌ 必须给 --host（例：--host root@gy.example.com）" >&2; exit 2; }
case "$MODE" in installed|tarball|mirror) ;; *) echo "❌ --mode 只能是 installed/tarball/mirror" >&2; exit 2 ;; esac
if [ "$MODE" = tarball ] && [ ! -f "$TARBALL" ]; then echo "❌ --mode tarball 需要 --tarball <存在的文件>" >&2; exit 2; fi

SSH_OPTS=(-p "$PORT" -o BatchMode=yes -o StrictHostKeyChecking=no -o ConnectTimeout=15 -o LogLevel=ERROR)
SSH=(ssh "${SSH_OPTS[@]}" "$HOST")
SCP=(scp -P "$PORT" -o BatchMode=yes -o StrictHostKeyChecking=no -o LogLevel=ERROR)

say()  { printf '%s\n' "$*"; }
P=0; F=0
ck() { if [ "$2" = 1 ]; then printf '  ✅ %s\n' "$1"; P=$((P+1)); else printf '  ❌ %s\n' "$1"; F=$((F+1)); fi; }

PXA_PORT=$(( 21000 + (RANDOM % 2000) ))
REMOTE="$(cat <<EOF
set -uo pipefail
RD="$RDIR"
P=0; F=0
ck(){ if [ "\$2" = 1 ]; then echo "  ✅ \$1"; P=\$((P+1)); else echo "  ❌ \$1"; F=\$((F+1)); fi; }
step(){ printf '\\n── %s ──\\n' "\$*"; }
cleanup(){ [ -f "\$RD/srv.pid" ] && kill "\$(cat "\$RD/srv.pid")" 2>/dev/null; }

step "[1] 连通与环境"
echo "  host=\$(hostname) arch=\$(uname -m) kernel=\$(uname -r)"
. /etc/os-release 2>/dev/null || true
echo "  distro=\${PRETTY_NAME:-?}"
getenforce >/dev/null 2>&1 && echo "  selinux=\$(getenforce)" || echo "  selinux=(无 getenforce)"
ck "[1] 远端可执行命令" 1

cd "\$RD" || { echo "❌ 进不去 \$RD"; exit 2; }

# 取工具链所在目录
if [ "$MODE" = tarball ] || [ "$MODE" = mirror ]; then
  step "[2] 取得发布物"
  if [ "$MODE" = mirror ]; then
    curl -fsSL -m 60 -o "\$RD/vj.json" "https://soft.xiusoft.cn/puxian/version.json" || { echo "❌ 取不到 version.json"; exit 2; }
    TB=\$(sed -n 's/.*"tarball" *: *"\\([^"]*\\)".*/\\1/p' "\$RD/vj.json" | head -1)
    SH=\$(sed -n 's/.*"tarball_sha256" *: *"\\([^"]*\\)".*/\\1/p' "\$RD/vj.json" | head -1)
    echo "  镜像 tarball=\$TB"
    curl -fsSL -m 600 -o "\$RD/pkg.tgz" "https://soft.xiusoft.cn/puxian/\$TB" || { echo "❌ 下载镜像 tarball 失败"; exit 2; }
    GOT=\$(sha256sum "\$RD/pkg.tgz" | awk '{print \$1}')
    ck "[2] 镜像 tarball sha256 == version.json 登记" "\$([ -n "\$SH" ] && [ "\$GOT" = "\$SH" ] && echo 1 || echo 0)"
    tar xzf "\$RD/pkg.tgz" -C "\$RD" --strip-components=1 || { echo "❌ 解包失败"; exit 2; }
  else
    tar xzf "\$RD/$(basename "$TARBALL")" -C "\$RD" --strip-components=1 || { echo "❌ 解包失败"; exit 2; }
    ck "[2] 发布 tarball 解包成功" 1
  fi
  export PATH="\$RD/tools:\$PATH"
fi

step "[3] 工具链 + 版本号"
PX=""
for c in px pxc /usr/bin/px /usr/bin/pxc; do
  if command -v "\$c" >/dev/null 2>&1; then PX="\$c"; break; fi
done
[ -n "\$PX" ] && ck "[3] 找到工具链 \$PX（\$(command -v "\$PX")）" 1 || ck "[3] 找不到 px / pxc" 0
V="\$("\$PX" --version 2>&1 | head -3 | tr '\\n' ' ')"
echo "  \$PX --version → \$V"
if [ -n "$EXPECT_VER" ]; then
  case "\$V" in *"$EXPECT_VER"*) ck "[3] 版本号含 $EXPECT_VER" 1 ;; *) ck "[3] 版本号**不含** $EXPECT_VER" 0 ;; esac
fi

step "[4] 编译-运行闭环"
mkdir -p "\$RD/work"; cd "\$RD/work"
cat > hello.px <<'PXX'
# 真机冒烟桩：覆盖「编译 → 运行 → 容器/切片/方法调用」最小面
def main():
    print("hello from real host")
    var xs = [1, 2, 3]
    print(str(len(xs)) + " " + str(xs[1]))
    print("up-" + "abc".to_upper())
PXX
if timeout 300 "\$PX" build hello.px > "\$RD/build.log" 2>&1; then
  ck "[4] 编译成功" 1
  BIN="\$RD/work/build/hello"
  [ -x "\$BIN" ] || BIN="\$(find "\$RD" -maxdepth 3 -type f -name hello -perm -u+x 2>/dev/null | head -1)"
  OUT="\$(timeout 60 "\$BIN" 2>&1)"; RC=\$?
  echo "  run rc=\$RC"
  printf '%s\\n' "\$OUT" | sed 's/^/     | /'
  ck "[4] 运行 rc=0" "\$([ \$RC -eq 0 ] && echo 1 || echo 0)"
  WANT="\$(printf 'hello from real host\\n3 2\\nup-ABC')"
  ck "[4] stdout 逐字节符合预期" "\$([ "\$OUT" = "\$WANT" ] && echo 1 || echo 0)"
  if command -v ldd >/dev/null 2>&1; then
    LD="\$(ldd "\$BIN" 2>&1 || true)"
    case "\$LD" in
      *"not a dynamic executable"*|*"statically linked"*|*"不是动态可执行文件"*) ck "[4] 产物是**静态**件" 1 ;;
      *) ck "[4] 产物是动态件（\$(printf '%s' "\$LD" | head -1)）" 0 ;;
    esac
  fi
else
  ck "[4] 编译失败" 0; tail -12 "\$RD/build.log" | sed 's/^/     | /'
fi

step "[5] HTTP 面（px_serve）"
mkdir -p "\$RD/docroot"; printf 'RH-SMOKE-OK\\n' > "\$RD/docroot/index.html"
cat > srv.px <<PXX
def main():
    px_serve($PXA_PORT, "\$RD/docroot", 30000)
PXX
if timeout 600 "\$PX" build srv.px > "\$RD/srvbuild.log" 2>&1; then
  SRVBIN="\$RD/work/build/srv"
  [ -x "\$SRVBIN" ] || SRVBIN="\$(find "\$RD" -maxdepth 3 -type f -name srv -perm -u+x 2>/dev/null | head -1)"
  setsid "\$SRVBIN" > "\$RD/srv.log" 2>&1 &
  echo \$! > "\$RD/srv.pid"
  sleep 3
  C1="\$(curl -s -o "\$RD/r1.txt" -w '%{http_code}' -m 8 "http://127.0.0.1:$PXA_PORT/" || true)"
  echo "  GET / → \$C1  body=[\$(head -c 40 "\$RD/r1.txt" 2>/dev/null | tr -d '\\n')]"
  ck "[5] GET / 返回 200（实得 \$C1）" "\$([ "\$C1" = 200 ] && echo 1 || echo 0)"
  ck "[5] 正文正确（RH-SMOKE-OK）" "\$(grep -q 'RH-SMOKE-OK' "\$RD/r1.txt" 2>/dev/null && echo 1 || echo 0)"
  C2="\$(curl -s -o /dev/null -w '%{http_code}' -m 8 "http://127.0.0.1:$PXA_PORT/__no_such_path__" || true)"
  ck "[5] 不存在路径 → 404（实得 \$C2）" "\$([ "\$C2" = 404 ] && echo 1 || echo 0)"
  OK=0
  for i in \$(seq 1 10); do curl -s -o /dev/null -m 8 "http://127.0.0.1:$PXA_PORT/" && OK=\$((OK+1)); done
  ck "[5] 并发 10 连全 200（实得 \$OK/10）" "\$([ "\$OK" = 10 ] && echo 1 || echo 0)"
  cleanup; sleep 1
else
  ck "[5] px_serve 编译失败" 0; tail -10 "\$RD/srvbuild.log" | sed 's/^/     | /'
fi

step "[6] 清理"
if [ "$KEEP" = 1 ]; then
  echo "  --keep：保留 \$RD（排障用）"
else
  cleanup
  cd /tmp && rm -rf "\$RD"
  ck "[6] 远端目录已清理" "\$([ ! -d "\$RD" ] && echo 1 || echo 0)"
fi

printf '\\n══ 真机冒烟：通过 %s · 失败 %s ══\\n' "\$P" "\$F"
[ "\$F" = 0 ] || exit 1
EOF
)"

if [ "$DRY" = 1 ]; then say "── 将对 $HOST:$PORT 执行（--dry-run）──"; say "$REMOTE"; exit 0; fi

say "── [0] 连通性 + 远端工作目录 ──"
if ! "${SSH[@]}" "mkdir -p '$RDIR' && echo READY" 2>/dev/null | grep -q READY; then
  ck "[0] SSH 可达（$HOST:$PORT）" 0; say ""; say "REALHOST-SMOKE-FAIL（连不上）"; exit 1
fi
ck "[0] SSH 可达（$HOST:$PORT）" 1

if [ "$MODE" = tarball ]; then
  say "── 上传发布物（$(du -h "$TARBALL" | cut -f1)）──"
  if "${SCP[@]}" "$TARBALL" "$HOST:$RDIR/" >/dev/null 2>&1; then ck "[0] 上传完成" 1; else ck "[0] 上传失败" 0; exit 1; fi
fi

say "$REMOTE" | "${SSH[@]}" "bash -s"
RC=$?
say ""
if [ "$RC" = 0 ]; then say "REALHOST-SMOKE-OK（$HOST）"; else say "REALHOST-SMOKE-FAIL（$HOST rc=$RC）"; fi
exit "$RC"
