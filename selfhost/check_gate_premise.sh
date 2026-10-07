#!/usr/bin/env bash
# selfhost/check_gate_premise.sh —— 判据的「第 0 维：前提」守卫（M283 · 第 161 轮）
#
# 立论（三次同族事故，全都不在产品代码）：
#   · M280「六轮读数全 0」—— 一条判据都没错，**结论全错**（残留进程占端口 ⇒ 服务没起来）
#   · M282「提交前绿、提交后红」（缺陷 491）—— 扫描面把**未提交的文件**漏在外面
#   · 缺陷 487「干跑通过 ≠ 同步能过」（`tar | grep -q` + pipefail 恒假）
#   ⇒ 共同形状：**判据自己的前提没有被验证过**，只是被**假定**过。
#
# 四类判据（全部静态可判；各配自证 + 反向判据）：
#   P1 端口互斥  被 >=2 个门共用的端口必须**显式登记**（承认 + 清理理由），未登记判红
#   P2 强就绪    等服务的门必须探到**可服务**（连得上/能应答），不许只等「日志出现 READY」
#   P3 禁形      `大产出 | grep -q` 在 pipefail 下**恒假**（SIGPIPE）—— 缺陷 487
#   P4 前提可得  门依赖的**外部文件前提**（读取但门自己不创建）必须显式登记
#
# 用法：check_gate_premise.sh [--root DIR] [--self-test] [--json] [--list] [--audit]
# 退出码：0 干净 · 1 有违例 · 2 用法错 · 3 登记过期 / 派生失败（拒绝放行）
#
# 纪律（沿 M212 缺陷 288）：**候选清单从源码派生 · 派生失败拒放行 · 规模下限锚点**。

set -uo pipefail

SELF="${BASH_SOURCE[0]}"
SCRIPT_DIR="$(cd "$(dirname "$SELF")" && pwd)"
ROOT="$(cd "$SCRIPT_DIR/.." && pwd)"
MODE=check
JSON=0
LIST=0
AUDIT=0

usage() {
  cat <<'U'
用法: check_gate_premise.sh [--root DIR] [--self-test] [--json] [--list] [--audit]
  --root DIR    仓库根（默认：本脚本的上一级）
  --self-test   只跑内置夹具自证（不扫真仓）
  --json        机器可读输出
  --list        列出全部候选（含已登记的）
  --audit       只报**登记过期**（允许表里有、实测已不在）
退出码: 0 干净 · 1 有违例 · 2 用法错 · 3 登记过期/派生失败
U
}

while [ $# -gt 0 ]; do
  case "$1" in
    --root)      ROOT="${2:-}"; shift 2 ;;
    --root=*)    ROOT="${1#*=}"; shift ;;
    --self-test) MODE=selftest; shift ;;
    --json)      JSON=1; shift ;;
    --list)      LIST=1; shift ;;
    --audit)     AUDIT=1; shift ;;
    -h|--help)   usage; exit 0 ;;
    *) echo "未知选项: $1" >&2; usage >&2; exit 2 ;;
  esac
done
[ -n "$ROOT" ] && [ -d "$ROOT" ] || { echo "root 不存在: $ROOT" >&2; exit 2; }

ALLOW_FILE="$ROOT/selfhost/premise_allow.tsv"

# ───────────────────────── 通用 ─────────────────────────
# 扫描面：已跟踪 ∪ 未跟踪未忽略（M282 缺陷 491：只看 git ls-files 会让新写的文件隐形）
filelist() {
  git -C "$ROOT" ls-files 2>/dev/null
  git -C "$ROOT" ls-files --others --exclude-standard 2>/dev/null
}

# 允许表读取：kind<TAB>subject<TAB>reason<TAB>date<TAB>basis
allow_has() { # $1=kind $2=subject
  [ -f "$ALLOW_FILE" ] || return 1
  awk -F'\t' -v k="$1" -v s="$2" \
    '!/^#/ && NF>=4 && $1==k && $2==s {found=1} END{exit !found}' "$ALLOW_FILE"
}
allow_subjects() { # $1=kind
  [ -f "$ALLOW_FILE" ] || return 0
  awk -F'\t' -v k="$1" '!/^#/ && NF>=4 && $1==k {print $2}' "$ALLOW_FILE"
}

VIOL=0
EXPIRE=0
declare -a MSGS=()

emit() { MSGS+=("$1"); }

# ───────────────────────── P1 端口互斥 ─────────────────────────
# 精确口径（首版 `\b(1[0-9]{4}|[2-9][0-9]{3})\b` 命中 4096/2026/8192 等非端口 ⇒ 已废弃）
PORT_RE_ADDR='(127\.0\.0\.1|localhost|0\.0\.0\.0|::1):[0-9]{2,5}'
PORT_RE_CALL='(px_serve|tcp_listen|udp_bind|listen|bind)[^)]{0,80}[0-9]{4,5}'
PORT_RE_OPT='(PORT|port|--port)[=: ][^0-9]{0,3}[0-9]{2,5}'

ports_in_file() { # $1=file -> 每行一个端口号（排序去重）
  { grep -hoE "$PORT_RE_ADDR" "$1" 2>/dev/null
    grep -hoE "$PORT_RE_CALL" "$1" 2>/dev/null
    grep -hoE "$PORT_RE_OPT"  "$1" 2>/dev/null
  } | grep -oE '[0-9]{2,5}$' | sort -n -u
}

gate_dirs() { # 门 = examples/*/verify.sh 所在目录（相对 ROOT，排序 ⇒ 可复现）
  ( cd "$ROOT" && ls -d examples/*/verify.sh 2>/dev/null | sed 's|/verify.sh$||' | sort )
}

scan_p1() {
  local tmp; tmp="$(mktemp)"; local n=0
  local g
  while IFS= read -r g; do
    [ -n "$g" ] || continue
    n=$((n+1))
    local f p
    for f in "$ROOT/$g"/*.sh "$ROOT/$g"/*.px; do
      [ -f "$f" ] || continue
      for p in $(ports_in_file "$f"); do printf '%s\t%s\n' "$p" "$g" >> "$tmp"; done
    done
  done < <(gate_dirs)
  P1_GATES=$n
  # 端口 -> 门集合
  local p cnt gates_line
  while IFS= read -r p; do
    cnt="$(awk -F'\t' -v P="$p" '$1==P{print $2}' "$tmp" | sort -u | wc -l)"
    [ "$cnt" -ge 2 ] || continue
    gates_line="$(awk -F'\t' -v P="$p" '$1==P{print $2}' "$tmp" | sort -u | tr '\n' ' ')"
    P1_TOTAL=$((P1_TOTAL+1))
    if [ "$AUDIT" = 0 ] && ! allow_has P1 "$p"; then
      VIOL=$((VIOL+1))
      emit "P1 端口 $p 被 $cnt 个门共用，**未登记**：$gates_line"
    elif [ "$LIST" = 1 ]; then
      emit "P1 端口 $p x$cnt（已登记）：$gates_line"
    fi
  done < <(awk -F'\t' '{print $1}' "$tmp" | sort -n -u)
  # 过期判据：登记了但实测已不冲突
  [ "$AUDIT" = 1 ] || [ "$LIST" = 1 ] || true
  while IFS= read -r s; do
    [ -n "$s" ] || continue
    local c2
    c2="$(awk -F'\t' -v P="$s" '$1==P{print $2}' "$tmp" | sort -u | wc -l)"
    if [ "$c2" -lt 2 ]; then
      EXPIRE=$((EXPIRE+1))
      emit "P1 登记过期：端口 $s 已不再冲突（实测 $( [ "$c2" = 0 ] && echo 0 || echo "$c2" ) 个门）⇒ 请复核并更新 $ALLOW_FILE"
    fi
  done < <(allow_subjects P1)
  rm -f "$tmp"
}

# ───────────────────────── P2 强就绪 ─────────────────────────
# 弱 = 门里有「等 READY 字样」**且**全文无任何连接探测
# 弱就绪的**精确形态**：用 grep **主动等待** READY/ready/listening 字样（= M280 的事故形态）。
# ⚠️ 不判「等 ready **文件**」：那是另一种就绪方式，且后续仍需自证（如 m280 的 [2a] 真监听检查）。
# 首版把 `started|已启动` 也算 ⇒ `m162_sorted_order` 等**假阳**（M283 实测：11 条里多数是这类）。
READY_RE='(grep|egrep)[^|;&]*[[:space:]](READY|ready|listening)'
PROBE_RE='(curl[[:space:]]|curl$|nc[[:space:]]+-z|/dev/tcp|tcp_connect|wait_port|check_port|probe_|http_get|tcp_send|tcp_recv)'

scan_p2() {
  local g
  while IFS= read -r g; do
    [ -n "$g" ] || continue
    local f blob="" has_ready=0 has_probe=0
    for f in "$ROOT/$g"/*.sh "$ROOT/$g"/*.px; do
      [ -f "$f" ] || continue
      # 只看**代码行**（跳过纯注释）—— 注释里的留证原文不算判据面（同 P3 口径）。
      # M283 实测：本门自己的注释含字面关键词 ⇒ 被自己的 P2 判红。
      blob="$(grep -vE '^[[:space:]]*#' "$f" 2>/dev/null)"
      grep -qE "$READY_RE" <<<"$blob" && has_ready=1
      grep -qE "$PROBE_RE" <<<"$blob" && has_probe=1
    done
    [ "$has_ready" = 1 ] || continue
    [ "$has_probe" = 0 ] || continue
    P2_TOTAL=$((P2_TOTAL+1))
    if [ "$AUDIT" = 0 ] && ! allow_has P2 "$g"; then
      VIOL=$((VIOL+1))
      emit "P2 弱就绪：$g 用 grep 等 READY 字样、无连接探测 ⇒ 服务可能没起来而判据全绿"
    elif [ "$LIST" = 1 ]; then
      emit "P2 弱就绪候选（已登记）：$g"
    fi
  done < <(gate_dirs)
  while IFS= read -r s; do
    [ -n "$s" ] || continue
    local f blob="" r=0 p=0
    for f in "$ROOT/$s"/*.sh "$ROOT/$s"/*.px; do
      [ -f "$f" ] || continue
      blob="$(grep -vE '^[[:space:]]*#' "$f" 2>/dev/null)"
      grep -qE "$READY_RE" <<<"$blob" && r=1
      grep -qE "$PROBE_RE" <<<"$blob" && p=1
    done
    if [ "$r" = 0 ] || [ "$p" = 1 ]; then
      EXPIRE=$((EXPIRE+1))
      emit "P2 登记过期：$s 已不再是弱就绪候选 ⇒ 请复核 $ALLOW_FILE"
    fi
  done < <(allow_subjects P2)
}

# ───────────────────────── P3 禁形 ─────────────────────────
# `大产出 | grep -q` + pipefail ⇒ grep 命中即退出 ⇒ 左侧收 SIGPIPE(141) ⇒ 管道非零 ⇒ 判据恒假
# ⚠️ 口径校准（M283 实测）：首版判「**非**白名单左侧」⇒ 真仓 **86** 条违例，
#   逐条核对后 **85 条是假阳**（`tail -1` / `sed -n 范围` / `echo "$VAR"` / `grep -A1` 都输出有界）。
#   ⇒ 改成**危险清单式**：只判「输出量不可判定、且已知可能大」的左侧命令。
#   覆盖边界（如实）：`echo "$VAR"` / `printf '%s' "$VAR"` 理论上也可能超管道缓冲（64KB），
#   但内容不可静态判定 ⇒ 不判；真出事由各门自证。
DANGER_LEFT_RE='^[[:space:]]*([A-Za-z_][A-Za-z0-9_]*=[^;|]*[[:space:]]+)*([A-Za-z0-9_./-]*/)?(cat|tar|readelf|nm|strings|objdump|ldd|zipinfo|unzip|dnf|yum|rpm|curl|wget|ssh|scp|git|find|du|sort|uniq|px|pxc)([[:space:]]|$)|^[[:space:]]*\./'
# `file` 特例：**单**文件 ⇒ 1 行输出（安全，如 tools/build_zlib.sh:87/89）；
#              **通配/多**文件 ⇒ 行数 ∝ 文件数（不可判定）⇒ 危险（如 tools/cross_multiarch.sh:138）。
DANGER_FILE_RE='^[[:space:]]*file[[:space:]]+[^|]*\*'

scan_p3() {
  local f rel
  while IFS= read -r rel; do
    [ -n "$rel" ] || continue
    case "$rel" in *.sh) ;; *) continue ;; esac
    f="$ROOT/$rel"
    [ -f "$f" ] || continue
    grep -q 'pipefail' "$f" 2>/dev/null || continue
    local ln
    ln="$(grep -nE '\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q' "$f" 2>/dev/null || true)"
    [ -n "$ln" ] || continue
    while IFS= read -r line; do
      [ -n "$line" ] || continue
      local num="${line%%:*}" body="${line#*:}"
      # 只判代码行（注释行豁免；本仓注释里大量留证原文）
      case "$(printf '%s' "$body" | sed 's/^[[:space:]]*//')" in
        '#'*) continue ;;
      esac
      # 取管道左侧**最后一段命令**（剥 if/&&/||/;/then/do/case 标签 前缀），再判是否属危险清单
      local left; left="$(printf '%s' "$body" \
        | sed -E 's/\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q.*$//' \
        | sed -E 's/.*(\|\||&&|[;{}]|then|do)[[:space:]]*//' \
        | sed -E 's/^[[:space:]]*(if|elif|while|until|return)[[:space:]]+//' \
        | sed -E 's/^[[:space:]]*[A-Za-z_][A-Za-z0-9_*|-]*\)[[:space:]]*//')"
      local danger=0
      printf '%s' "$left" | grep -qE "$DANGER_LEFT_RE" && danger=1
      printf '%s' "$left" | grep -qE "$DANGER_FILE_RE" && danger=1
      [ "$danger" = 1 ] || continue
      local key="$rel:$num"
      P3_TOTAL=$((P3_TOTAL+1))
      if [ "$AUDIT" = 0 ] && ! allow_has P3 "$key"; then
        VIOL=$((VIOL+1))
        emit "P3 禁形：$key 「$(printf '%s' "$left" | cut -c1-60) | grep -q」在 pipefail 下可恒假"
      elif [ "$LIST" = 1 ]; then
        emit "P3 禁形候选（已登记）：$key"
      fi
    done <<< "$ln"
  done < <(filelist | sort -u)
  # 过期：登记的 key 已不在实测集合 —— 用一次重扫判定
  local s
  while IFS= read -r s; do
    [ -n "$s" ] || continue
    local sf="${s%%:*}" sn="${s##*:}"
    if [ ! -f "$ROOT/$sf" ] || ! sed -n "${sn}p" "$ROOT/$sf" 2>/dev/null | grep -qE '\|[[:space:]]*grep[[:space:]]+-[a-zA-Z]*q'; then
      EXPIRE=$((EXPIRE+1))
      emit "P3 登记过期：$s 已不再是禁形 ⇒ 请复核 $ALLOW_FILE"
    fi
  done < <(allow_subjects P3)
}

# ───────────────────────── P4 前提可得 ─────────────────────────
# 门**读取**了某个绝对路径文件，但门自己**不创建**它 ⇒ 外部前提 ⇒ 必须登记
# 只认「读取位置」：cat / read_file / [ -f X ] / source / . X / grep … X
READFILE_RE='(\[\[?[[:space:]]+-[efr][[:space:]]+|cat[[:space:]]+|read_file\(|source[[:space:]]+|grep[[:space:]]+-[a-zA-Z]*[[:space:]]+\S+[[:space:]]+)(/tmp/[A-Za-z0-9_./-]+|/var/[A-Za-z0-9_./-]+)'
CREATEFILE_RE='(mkdir[[:space:]]+-p[[:space:]]+|touch[[:space:]]+|>+[[:space:]]*|mktemp|install[[:space:]]+-d)'
# ⚠️ **构建工具（selfhost/devbuild.sh）的产物命名空间** —— 门调用该工具后读取其产物属**白盒验证**，
#    不是外部前提（与它并列的 pxcdev/pxidev 早已豁免；M287s1 把**整族**补齐）。
#    依据（devbuild.sh:209）：out="/tmp/${name}dev" · fp="/tmp/devbuild_${name}.fp" · mf="/tmp/devbuild_${name}.src"
#      · 契约产物  /tmp/<件>dev · /tmp/<件>dev_vm（+ 后缀变体）
#      · 辅助产物  /tmp/devbuild_<件>.{fp,src,c,err,o} 与 *.log
#    ⚠️ 首版只列 pxcdev|pxidev 两条字面量 ⇒ 漏掉辅助产物 ⇒ 新门读 .fp 即误报（m283 判红抓住）。
#    ⚠️ 正则刻意**窄**（不含 `/` 与多级路径）⇒ 嵌套路径 /tmp/m283_prem_absent/x.txt 不会被误放。
#    自证 ⑩（豁免生效）· ⑪（豁免不越界）守窄性；m283 [7f]/[7g] 锚定依据仍在位。
BUILD_TOOL_OUT_RE='^/tmp/(devbuild_[A-Za-z0-9_]+(\.[A-Za-z0-9_]+)?|[A-Za-z0-9_]+dev(_vm)?(\.[A-Za-z0-9_]+)?)$'

scan_p4() {
  local tmp; tmp="$(mktemp)"
  local g
  while IFS= read -r g; do
    [ -n "$g" ] || continue
    local f blob="" created="" reads=""
    for f in "$ROOT/$g"/*.sh "$ROOT/$g"/*.px; do
      [ -f "$f" ] || continue
      blob="$(cat "$f" 2>/dev/null)"
      reads+="$(grep -oE "$READFILE_RE" <<<"$blob" 2>/dev/null | grep -oE '/(tmp|var)/[A-Za-z0-9_./-]+' || true)"$'\n'
      # ⚠️ 「创建」**只**认 mkdir/touch/install/重定向 —— 若把「文件里出现过的所有 /tmp 路径」
      #    都当创建（首版做法），则夹具里的 `cat /tmp/X` 会把 X 记成「自己创建过」⇒ 整类漏报
      #    （M283 自证 ④ 当场判红抓回）。
      created+="$(grep -oE '(mkdir|touch|install)[[:space:]]+-[a-zA-Z]*[[:space:]]+[^;&|]*' <<<"$blob" 2>/dev/null | grep -oE '/(tmp|var)/[A-Za-z0-9_./-]+' || true)"$'\n'
      created+="$(grep -oE '>[[:space:]]*/(tmp|var)/[A-Za-z0-9_./-]+' <<<"$blob" 2>/dev/null | grep -oE '/(tmp|var)/[A-Za-z0-9_./-]+' || true)"$'\n'
    done
    # ⚠️ 判据校准（M283 实测）：首版「读但未创建」把三类**假阳**都算成外部前提 ——
    #   ① 门启动的**子进程**产出（服务日志：门把路径传给服务，由服务写）
    #   ② 门内**变量形式**的创建（字面量判据看不见）
    #   ③ **构建工具**产物（devbuild 的 /tmp/<件>dev 与 /tmp/devbuild_*）—— 整族豁免，见 BUILD_TOOL_OUT_RE
    #   ⇒ 加**命名空间过滤**：路径含本门标识（m150 / issue28 / 目录名）⇒ 视为门自己的产物。
    local gid gnum
    gid="$(basename "$g")"
    gnum="$(sed -nE 's/^((m|issue)[0-9]+).*/\1/p' <<<"$gid")"
    local p
    while IFS= read -r p; do
      [ -n "$p" ] || continue
      grep -qxF "$p" <<<"$created" && continue
      if [ -n "$gnum" ] && grep -qF "$gnum" <<<"$p"; then continue; fi
      if grep -qF "$gid" <<<"$p"; then continue; fi
      [[ "$p" =~ $BUILD_TOOL_OUT_RE ]] && continue
      printf '%s|%s\n' "$g" "$p" >> "$tmp"
    done < <(printf '%s' "$reads" | sort -u | grep -v '^$' || true)
  done < <(gate_dirs)
  # ⚠️ tmp 用「|」分隔（与允许表的 subject 同格式）—— 首版用 tab ⇒ 过期检查的 grep -qxF 恒不中
  #    ⇒ 登记项被**永久**判「过期」（自证 ⑤ 当场判红抓回）。
  local key
  while IFS= read -r key; do
    [ -n "$key" ] || continue
    P4_TOTAL=$((P4_TOTAL+1))
    if [ "$AUDIT" = 0 ] && ! allow_has P4 "$key"; then
      VIOL=$((VIOL+1))
      emit "P4 外部前提 ${key%%|*} 依赖 ${key##*|}：未登记（缺须响亮 SKIP，不许静默绿）"
    elif [ "$LIST" = 1 ]; then
      emit "P4 外部前提 ${key%%|*} 依赖 ${key##*|}：已登记"
    fi
  done < "$tmp"
  while IFS= read -r s; do
    [ -n "$s" ] || continue
    grep -qxF "$s" "$tmp" || { EXPIRE=$((EXPIRE+1)); emit "P4 登记过期：$s 已不再是外部前提 ⇒ 请复核 $ALLOW_FILE"; }
  done < <(allow_subjects P4)
  rm -f "$tmp"
}

# ───────────────────────── 自证 ─────────────────────────
selftest() {
  local W; W="$(mktemp -d)"; local ok=0 ng=0
  trap 'rm -rf "$W"' RETURN
  mkdir -p "$W/examples/g_port_a" "$W/examples/g_port_b" "$W/examples/g_port_c" \
           "$W/examples/g_ready" "$W/examples/g_pipe" "$W/examples/g_prem" "$W/selfhost"
  git -C "$W" init -q 2>/dev/null || true
  # ① 端口冲突：a/b 同用 39001 ⇒ 必须命中 P1
  printf 'PORT=39001\n' > "$W/examples/g_port_a/verify.sh"
  printf '127.0.0.1:39001\n' > "$W/examples/g_port_b/verify.sh"
  printf 'PORT=39002\n' > "$W/examples/g_port_c/verify.sh"
  # ② 弱就绪：等 READY、无探测 ⇒ 必须命中 P2
  printf '#!/bin/bash\ngrep -q READY srv.log\n' > "$W/examples/g_ready/verify.sh"
  # ③ 禁形：readelf | grep -q + pipefail ⇒ 必须命中 P3（printf 左侧 ⇒ 必须**不**命中）
  printf '#!/bin/bash\nset -o pipefail\nreadelf -d x | grep -q NEEDED\nprintf %%s "%%s" "$v" | grep -q yes\n' > "$W/examples/g_pipe/verify.sh"
  # ④ 外部前提：读 /tmp/m283_prem_absent/x.txt 但不创建 ⇒ 必须命中 P4
  printf '#!/bin/bash\nif [ -f /tmp/m283_prem_absent/x.txt ]; then cat /tmp/m283_prem_absent/x.txt; fi\n' > "$W/examples/g_prem/verify.sh"
  # ⑩ 构建工具产物：读 devbuild 的产物（不创建）⇒ 必须**不**判 P4（豁免族完整 —— M287s1）
  mkdir -p "$W/examples/g_tool"
  printf '#!/bin/bash\n[ -f /tmp/devbuild_pxc.fp ] && cat /tmp/devbuild_pxc.fp\n' > "$W/examples/g_tool/verify.sh"
  # ⑪ 窄性反向判据：**非**构建工具产物 ⇒ 仍必须判 P4（豁免不许越界）
  mkdir -p "$W/examples/g_other"
  printf '#!/bin/bash\ncat /tmp/other_tool.state\n' > "$W/examples/g_other/verify.sh"
  : > "$W/selfhost/premise_allow.tsv"
  local out rc
  chk(){ if [ "$2" = "$3" ]; then ok=$((ok+1)); echo "  ✅ $1"; else ng=$((ng+1)); echo "  ❌ $1 （actual=[$2] want=[$3]）"; fi; }
  # ①–④ 走**真实判红路径**（不带 --list）
  out="$(bash "$SELF" --root "$W" 2>&1)"; rc=$?
  chk "① 端口冲突命中 P1"           "$(grep -c '^P1 .*39001' <<<"$out" || true)" "1"
  chk "② 弱就绪命中 P2"             "$(grep -c '^P2 .*g_ready' <<<"$out" || true)" "1"
  chk "③ 禁形命中 P3（readelf 行）" "$(grep -c '^P3 .*g_pipe/verify.sh:3' <<<"$out" || true)" "1"
  chk "③b printf 左侧不判（白名单）" "$(grep -c '^P3 .*g_pipe/verify.sh:4' <<<"$out" || true)" "0"
  chk "④ 外部前提命中 P4"           "$(grep -c '^P4 .*g_prem' <<<"$out" || true)" "1"
  chk "⑩ 构建工具产物豁免（不判 P4）"   "$(grep -c '^P4 .*g_tool'  <<<"$out" || true)" "0"
  chk "⑪ 非工具产物仍判 P4（豁免不越界）" "$(grep -c '^P4 .*g_other' <<<"$out" || true)" "1"
  chk "⑨ 未登记时 rc=1"             "$rc" "1"
  # 反向判据：登记后必须**不**判红
  printf 'P1\t39001\tr\t2026-10-07\tM283\nP2\texamples/g_ready\tr\t2026-10-07\tM283\nP3\texamples/g_pipe/verify.sh:3\tr\t2026-10-07\tM283\nP4\texamples/g_prem|/tmp/m283_prem_absent/x.txt\tr\t2026-10-07\tM283\nP4\texamples/g_other|/tmp/other_tool.state\tr\t2026-10-08\tM287s1\n' > "$W/selfhost/premise_allow.tsv"
  out="$(bash "$SELF" --root "$W" 2>&1)"; rc=$?
  chk "⑤ 全部登记后 rc=0（反向判据）" "$rc" "0"
  # 判据自伤：把允许表清空 ⇒ 必须重新判红（证明 ⑤ 的绿来自登记，不是判据恒绿）
  : > "$W/selfhost/premise_allow.tsv"
  out="$(bash "$SELF" --root "$W" 2>&1)"; rc=$?
  chk "⑥ 判据自伤（清空允许表 ⇒ rc=1）" "$rc" "1"
  # 派生失败拒放行：root 不存在 ⇒ rc=2
  bash "$SELF" --root /nonexistent-xyz >/dev/null 2>&1; rc=$?
  chk "⑦ root 不存在 ⇒ rc=2（拒绝放行）" "$rc" "2"
  # 用法：未知选项 ⇒ rc=2
  bash "$SELF" --frobnicate >/dev/null 2>&1; rc=$?
  chk "⑧ 未知选项 ⇒ rc=2" "$rc" "2"
  echo "自证：通过 $ok / 失败 $ng"
  [ "$ng" = 0 ] || return 1
}

# ───────────────────────── main ─────────────────────────
P1_GATES=0; P1_TOTAL=0; P2_TOTAL=0; P3_TOTAL=0; P4_TOTAL=0

if [ "$MODE" = selftest ]; then
  selftest; exit $?
fi

if ! git -C "$ROOT" rev-parse --git-dir >/dev/null 2>&1; then
  echo "派生失败：$ROOT 不是 git 仓库 ⇒ 拒绝放行（不得静默回退）" >&2
  exit 3
fi

scan_p1
scan_p2
scan_p3
scan_p4

if [ "$JSON" = 1 ]; then
  printf '{"violations":%d,"expired":%d,"p1":{"gates":%d,"shared":%d},"p2":%d,"p3":%d,"p4":%d}\n' \
    "$VIOL" "$EXPIRE" "$P1_GATES" "$P1_TOTAL" "$P2_TOTAL" "$P3_TOTAL" "$P4_TOTAL"
else
  for m in "${MSGS[@]}"; do echo "$m"; done
  echo "──"
  echo "扫描：门 $P1_GATES 个 · P1 共用端口 $P1_TOTAL · P2 弱就绪 $P2_TOTAL · P3 禁形 $P3_TOTAL · P4 前提 $P4_TOTAL"
  echo "违例 $VIOL · 登记过期 $EXPIRE"
fi

[ "$EXPIRE" -gt 0 ] && exit 3
[ "$VIOL" -gt 0 ] && exit 1
exit 0
