#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════════════
# M309 门 · `pxpkg sync` 契约夹门（C3 · W4）
#
# 由来：用户 2026-10-16 11:33「那就 C1+C3」⇒ C3 = `pxpkg sync` 收尾。
#   W1（客户端）已在分支 `feat/pxpkg-sync-w1` 复核全绿
#   （/data/app/ws/data/notes/M309_W1_VERIFY.md：真对端 4 例 + e2e 4 步 + 负控 8 条），
#   但那份复核是**一次性手工脚本**（住 /tmp），**没有常设判据** ⇒ 本门把它固化。
#
# 为什么必须**离线**（本门的设计前提）：
#   ① 真对端（soft.xiusoft.cn）在外网，CI 上不可达 —— 判据会变成「网络好不好」；
#   ② 对端内容会变（`archive_sha256` 随每次发布更新）⇒ 判据会**随对端漂移**而假红；
#   ③ 契约的可判定性不该依赖对方是否在线。
#   ⇒ 本门**自造夹具**：本地 registry 树 → 确定性 tar.gz → version.json → 本地 http.server。
#      判据只依赖**契约本身**（§四 八条），不依赖对端。
#
# 层（每层独立有牙）：
#   [1] 静态：文件在位 · `pxpkg sync` 子命令挂载 · 零新依赖 · 用法契约
#   [2] 正控：首次同步 / 指纹落盘 / stage 残留 0 / 幂等 0 下载 / --dry-run 零副作用
#   [3] 原子性与失败语义：**8 条负控，每条都必须** rc=1 **且目标逐字节不动**
#       （这是 W1 的核心契约：「绝不把本地 registry 改成一半新一半旧」）
#   [4] 边界：相对路径 registry / 未知参数 / 缺参 ⇒ rc=2（用法错，与 rc=1 运行失败分开）
#   [5] 覆盖边界（如实登记）
#
# ⚠️ 纪律（本仓踩过的坑，逐条对齐）：
#   · 不写死开发机路径（缺陷 300）—— 一切从 `$(dirname "$0")` 推导；
#   · 提示文本用**单引号**（双引号里的反引号会触发命令替换 —— 本仓第 N 次）；
#   · 判据只看**代码行**（注释里的历史说明不算 —— M251 老坑）；
#   · 不写 `大产出 | grep -q`（P3 禁形：SIGPIPE + pipefail ⇒ 判据恒假，缺陷 487）；
#   · 清进程**按 PID**（`pkill -f` 会打到自己的 shell —— 本仓第 N 次）；
#   · 强就绪：必须**真发一次 GET 成功**才算就绪（M283 P2；M280 事故形态 = 文件出现但绑不上）。
# ══════════════════════════════════════════════════════════════════════
set -uo pipefail
. "$(dirname "$0")/../../selfhost/gate_lock.sh" || {
  echo '❌ [M276] 门级互斥锁 source 失败（selfhost/gate_lock.sh）' >&2; exit 2; }

HERE="$(cd "$(dirname "$0")" && pwd)"
ROOT="$(cd "$HERE/../.." && pwd)"
SYNC="$ROOT/tools/pxpkg_sync.sh"
PKG="$ROOT/tools/pxpkg"
W="${M309_W:-/tmp/m309_gate}"
PORT="${M309_PORT:-18743}"
rm -rf "$W"; mkdir -p "$W/srv" "$W/reg" "$W/logs"
PASS=0; FAIL=0
ok() { PASS=$((PASS+1)); printf '  ✅ %s\n' "$1"; }
bad(){ FAIL=$((FAIL+1)); printf '  ❌ %s\n' "$1"; }
chk(){ if [ "$2" = "$3" ]; then ok "$1（$2）"; else bad "$1（got=$2 want=$3）"; fi; }

echo '══ M309 门：pxpkg sync 契约夹门（C3·W4 · 离线夹具）══'

# ── 夹具：确定性 registry 树 + 归档 + version.json ────────────────────
# 确定性 = 同一次运行内两次造出的 sha256 相同（固定 mtime/owner/排序）
mk_fix() {
  local F="$W/fix" n v sha bytes
  rm -rf "$F"; mkdir -p "$F/registry"
  for p in alpha/0.1.0 beta/0.2.0 gamma/1.0.0; do
    n="${p%%/*}"; v="${p##*/}"
    mkdir -p "$F/registry/$n/$v"
    printf '// %s %s\nfunc %s() { return 42 }\n' "$n" "$v" "$n" > "$F/registry/$n/$v/$n.px"
  done
  printf '# fixture registry\n' > "$F/registry/README.md"
  tar --sort=name --mtime='2026-01-01 00:00:00' --owner=0 --group=0 --numeric-owner \
      -czf "$W/srv/reg-fix.tar.gz" -C "$F" registry 2>/dev/null
  sha="$(sha256sum "$W/srv/reg-fix.tar.gz" | awk '{print $1}')"
  bytes="$(wc -c < "$W/srv/reg-fix.tar.gz" | tr -d ' ')"
  cat > "$W/srv/version.json" <<EOF
{"version":"0.0.1-m309fix","commit":"0000000000000000000000000000000000000000","archive":"reg-fix.tar.gz","archive_sha256":"$sha","archive_bytes":$bytes,"files":4}
EOF
  printf '%s %s\n' "$sha" "$bytes" > "$W/srv/.fixture"
}
vj_set() {   # $1=键 $2=值（**纯数字串自动转 int** —— ⚠️ 勿用它写「全数字的 64 位 sha」：会被转成 JSON 数字）
  python3 - "$W/srv/version.json" "$1" "$2" <<'PY'
import json, sys
p, k, v = sys.argv[1], sys.argv[2], sys.argv[3]
d = json.load(open(p))
try: d[k] = int(v)
except ValueError: d[k] = v
json.dump(d, open(p, 'w'))
PY
}
stale() {    # 把本地指纹置「陈旧」（长度合法的假 sha）⇒ 强制走下载路径
  python3 - "$W/reg/registry/.pxpkg-sync.json" <<'PY'
import json, sys
p = sys.argv[1]
d = json.load(open(p)); d['archive_sha256'] = 'a' * 64
json.dump(d, open(p, 'w'))
PY
}
dirsig() { find "$1" -type f -print0 2>/dev/null | sort -z | xargs -0 sha256sum 2>/dev/null | sha256sum | cut -c1-16; }

# ── 起离线对端（python3 http.server；真就绪判据 = 真发一次 GET）────────
mk_fix || { echo '❌ 夹具构造失败' >&2; exit 2; }
( cd "$W/srv" && exec python3 -m http.server "$PORT" --bind 127.0.0.1 ) >"$W/logs/srv.log" 2>&1 &
SRV_PID=$!
cleanup() {
  if [ -n "${SRV_PID:-}" ]; then kill "$SRV_PID" 2>/dev/null; wait "$SRV_PID" 2>/dev/null; fi
  rm -rf "$W/stage" 2>/dev/null
}
trap cleanup EXIT
B="http://127.0.0.1:$PORT"
READY=0
for _ in $(seq 1 40); do
  if curl -fsS --max-time 2 "$B/version.json" >/dev/null 2>&1; then READY=1; break; fi
  sleep 0.25
done
if [ "$READY" = 1 ]; then ok '[前提] 离线对端就绪（真发一次 GET 成功）'; else bad '[前提] 离线对端未就绪'; fi

# ── [1] 静态 ────────────────────────────────────────────────────────
echo '[1] 静态'
[ -f "$SYNC" ] && ok '[1a] tools/pxpkg_sync.sh 在位' || bad '[1a] 缺 tools/pxpkg_sync.sh'
grep -qF 'tools/pxpkg_sync.sh' "$PKG" && ok '[1b] pxpkg 挂载 sync 子命令' || bad '[1b] pxpkg 未挂载 sync'
for c in curl sha256sum tar; do
  grep -qE "command -v $c" "$SYNC" && ok "[1c] 依赖检查在位：$c" || bad "[1c] 缺 $c 的检查"
done
if grep -nE '^[^#]*\b(wget|rsync|python3?)\b' "$SYNC" >/dev/null 2>&1; then
  bad '[1d] 出现新依赖调用（wget/rsync/python）'
else
  ok '[1d] 零新依赖（代码行无 wget/rsync/python 调用）'
fi
if grep -nE '^[^#]*\b(pkill|killall)\b' "$SYNC" >/dev/null 2>&1; then
  bad '[1e] 出现按名清进程（pkill/killall）'
else
  ok '[1e] 无按名清进程'
fi
bash "$SYNC" --help >/dev/null 2>&1; chk '[1f] --help rc' "$?" 0
bash "$SYNC" --help 2>&1 | grep -qF -- '--registry' && ok '[1g] 用法含 --registry' || bad '[1g] 用法缺 --registry'

# ── [2] 正控（离线夹具）──────────────────────────────────────────────
echo '[2] 正控'
REG="$W/reg/registry"
bash "$SYNC" --base "$B" --registry "$REG" >"$W/logs/first.log" 2>&1; rc=$?
chk '[2a] 首次同步 rc' "$rc" 0
if [ "$rc" != 0 ]; then sed -n '1,6p' "$W/logs/first.log" | sed 's/^/     ↳ /'; fi
NF="$(find "$REG" -type f 2>/dev/null | wc -l | tr -d ' ')"
[ "$NF" -ge 5 ] && ok "[2b] 文件数（$NF ≥ 5 = 4 夹具 + 指纹）" || bad "[2b] 文件数不足（$NF）"
chk '[2c] stage 残留' "$(ls -d "$W"/reg/.*stage* 2>/dev/null | wc -l | tr -d ' ')" 0
SHA_FIX="$(awk '{print $1}' "$W/srv/.fixture")"
grep -qF "$SHA_FIX" "$REG/.pxpkg-sync.json" 2>/dev/null && ok '[2d] 指纹记录 archive_sha256（与夹具一致）' || bad '[2d] 指纹缺失/不符'
sig0="$(dirsig "$REG")"
bash "$SYNC" --base "$B" --registry "$REG" >"$W/logs/second.log" 2>&1; rc2=$?
chk '[2e] 二次同步 rc' "$rc2" 0
grep -qF '已是最新' "$W/logs/second.log" && ok '[2f] 二次同步走幂等短路（0 下载）' || bad '[2f] 二次同步未命中幂等'
chk '[2g] 二次同步未改动目标' "$(dirsig "$REG")" "$sig0"
REG2="$W/reg2/registry"
bash "$SYNC" --base "$B" --registry "$REG2" --dry-run >"$W/logs/dry.log" 2>&1; rc3=$?
chk '[2h] --dry-run rc' "$rc3" 0
if [ -e "$REG2" ]; then bad '[2i] --dry-run 产生了副作用（目标被创建）'; else ok '[2i] --dry-run 零副作用'; fi

# ── [3] 原子性与失败语义（8 条负控 · 每条独立判红）─────────────────────
echo '[3] 原子性与失败语义（负控）'
neg_run() {   # $1=名 $2=期望 rc —— 判据 = rc 相符 **且** 目标逐字节不动
  local name="$1" want="$2" sb sa rc
  sb="$(dirsig "$REG")"
  bash "$SYNC" --base "$B" --registry "$REG" >"$W/logs/neg.log" 2>&1; rc=$?
  sa="$(dirsig "$REG")"
  if [ "$rc" = "$want" ] && [ "$sb" = "$sa" ]; then
    ok "[3] $name（rc=$rc · 目标逐字节未动）"
  else
    local extra='未动'; [ "$sb" = "$sa" ] || extra='**已改**'
    bad "[3] $name（rc=$rc / 期望 $want · 目标 $extra）"
  fi
}
Z64="$(printf '%064d' 0)"; Z64Z="$(printf 'z%.0s' $(seq 64))"; ZHEX="$(printf 'ab%.0s' $(seq 32))"

# N1 = 真正的「哈希不符」：值必须是**合法 64 位 hex 且非全数字**（全数字会被 vj_set 转成 JSON 数字，见 N9）
mk_fix; stale; vj_set archive_sha256 "$ZHEX"
neg_run 'N1 归档 sha256 不符（合法 64 位 hex，与归档不符）' 1

# N9 = 「字段类型不对」：全数字串被 vj_set 转成 JSON 数字 ⇒ 客户端 jget 取到空 ⇒ 报「缺字段」。
#   ⚠️ 本条是首跑实测的副产品：N1 原先用 $Z64（64 个 0）走的正是这条路径
#   ⇒ 红「对了」但**理由错**（红来自「缺字段」而非「哈希不符」），且 NC-A 撤校验后仍红。
mk_fix; stale; vj_set archive_sha256 "$Z64"
neg_run 'N9 archive_sha256 为 JSON 数字（类型不对）' 1

mk_fix; stale; vj_set archive 'gone.tar.gz'
neg_run 'N2 归档 404' 1

mk_fix; stale
cp "$W/srv/reg-fix.tar.gz" "$W/srv/tampered.tar.gz"
printf 'X' | dd of="$W/srv/tampered.tar.gz" bs=1 seek=50 conv=notrunc status=none
vj_set archive 'tampered.tar.gz'
vj_set archive_bytes "$(wc -c < "$W/srv/tampered.tar.gz" | tr -d ' ')"
neg_run 'N3 归档字节篡改（sha 不符）' 1

mk_fix; stale
mkdir -p "$W/noreg/other" && printf 'x\n' > "$W/noreg/other/x"
tar --sort=name --mtime='2026-01-01 00:00:00' --owner=0 --group=0 --numeric-owner \
    -czf "$W/srv/noreg.tar.gz" -C "$W/noreg" other 2>/dev/null
vj_set archive 'noreg.tar.gz'
vj_set archive_sha256 "$(sha256sum "$W/srv/noreg.tar.gz" | awk '{print $1}')"
vj_set archive_bytes "$(wc -c < "$W/srv/noreg.tar.gz" | tr -d ' ')"
neg_run 'N4 归档内无 registry/' 1

mk_fix; stale; vj_set archive_bytes 1
neg_run 'N5 archive_bytes 不符' 1

mk_fix; stale; mv "$W/srv/version.json" "$W/srv/vj.bak"
neg_run 'N6 version.json 404（目标根本没被碰）' 1
mv "$W/srv/vj.bak" "$W/srv/version.json"

mk_fix; stale; vj_set archive_sha256 "$Z64Z"
neg_run 'N7 sha256 非十六进制' 1

# 造「含越界路径」的归档（$1 = 越界成员名）。
#   两种形态的**拦截来源不同**（首跑实测，见 [5] 覆盖边界）：
#     绝对路径 `/x`  ⇒ **只有本判据**拦得住（GNU tar 只剥前导 `/` 且 rc=0）
#     相对越界 `../x` ⇒ GNU tar **自身也拒**（Member name contains '..' ⇒ rc≠0）= 双保险
mk_esc() {
  mk_fix; stale
  python3 - "$W/srv/esc.tar.gz" "$1" <<'PY'
import io, sys, tarfile
p, evil = sys.argv[1], sys.argv[2]
with tarfile.open(p, 'w:gz') as t:
    d = b'// evil\n'
    for name in (evil, 'registry/ok.px'):
        i = tarfile.TarInfo(name); i.size = len(d); i.mtime = 1767225600
        t.addfile(i, io.BytesIO(d))
PY
  vj_set archive 'esc.tar.gz'
  vj_set archive_sha256 "$(sha256sum "$W/srv/esc.tar.gz" | awk '{print $1}')"
  vj_set archive_bytes "$(wc -c < "$W/srv/esc.tar.gz" | tr -d ' ')"
}
mk_esc '/evil.txt'
neg_run 'N8 归档含绝对路径 /evil.txt（判据独家拦）' 1
mk_esc '../evil.txt'
neg_run 'N8b 归档含相对越界 ../evil.txt（tar 亦拦 · 双保险）' 1

# ── [4] 边界（用法契约 ⇒ rc=2，与运行失败 rc=1 分开）──────────────────
echo '[4] 边界'
bash "$SYNC" --base "$B" --registry relative/dir >/dev/null 2>&1; chk '[4a] 相对路径 registry ⇒ rc=2' "$?" 2
bash "$SYNC" --bogus >/dev/null 2>&1; chk '[4b] 未知参数 ⇒ rc=2' "$?" 2
bash "$SYNC" --base >/dev/null 2>&1; chk '[4c] --base 缺参 ⇒ rc=2' "$?" 2
bash "$SYNC" --base "$B" --registry "$W/reg/registry" --dry-run >/dev/null 2>&1; chk '[4d] 幂等态下 --dry-run 仍 rc=0' "$?" 0

# ── [6] 负控：证明 [3] 的判据**有牙**（只改 $W 里的副本，**不动仓库源**）──
#   本仓纪律（M199/M214/M286）：一套判据全绿只说明「没发现问题」，不说明「判据能发现问题」。
#   ⇒ 故意撤掉被判对象的一处能力，必须能观测到「同一故障不再被拦」。
echo '[6] 负控（判据自证）'
MUTD="$W/mut"; mkdir -p "$MUTD"
mutate() {   # $1=副本名 $2=锚点（行首起 · 全局必须恰 1 次）$3=替换整行的文本
  cp "$ROOT/tools/pxpkg_sync.sh" "$MUTD/$1"
  python3 - "$MUTD/$1" "$2" "$3" <<'PY'
import sys
p, anchor, repl = sys.argv[1], sys.argv[2], sys.argv[3]
s = open(p).read()
key = '\n' + anchor
n = s.count(key)
if n != 1:
    sys.stderr.write('锚点命中 %d 次（要求恰 1）\n' % n); sys.exit(3)
i = s.index(key) + 1
j = s.index('\n', i)
open(p, 'w').write(s[:i] + repl + s[j:])
PY
}
NC_A='[ "$got" = "$ASHA" ] || die'
NC_B="if grep -Eq '(^/|(^|/)\\.\\./)'"
if mutate ncA.sh "$NC_A" ':' 2>"$W/logs/ncA-mut.err"; then
  if cmp -s "$SYNC" "$MUTD/ncA.sh"; then bad '[6] NC-A 副本与源相同（未生效）'
  else ok '[6] NC-A 副本已变异（源未动）'; fi
  mk_fix; stale; vj_set archive_sha256 "$ZHEX"
  bash "$MUTD/ncA.sh" --base "$B" --registry "$REG" >/dev/null 2>&1; rc=$?
  if [ "$rc" = 0 ]; then ok '[6] NC-A 撤回归档哈希校验 ⇒ N1 同一故障**不再被拦**（rc=0，判据来源已证）'
  else bad "[6] NC-A 撤回校验后仍 rc=$rc（红不来自该判据）"; fi
else
  bad "[6] NC-A 打桩失败（锚点不唯一：$(cat "$W/logs/ncA-mut.err" 2>/dev/null)）"
fi
if mutate ncB.sh "$NC_B" 'if false; then' 2>"$W/logs/ncB-mut.err"; then
  mk_esc '/evil.txt'     # 必须用**绝对路径**形态：`../` 会被 tar 自己拦掉 ⇒ 证不到判据有牙
  bash "$MUTD/ncB.sh" --base "$B" --registry "$REG" >/dev/null 2>&1; rc=$?
  if [ "$rc" = 0 ]; then ok '[6] NC-B 撤回越界路径拦截 ⇒ N8 同一故障**不再被拦**（rc=0，判据来源已证）'
  else bad "[6] NC-B 撤回路径卫生后仍 rc=$rc（红不来自该判据）"; fi
else
  bad "[6] NC-B 打桩失败（锚点不唯一：$(cat "$W/logs/ncB-mut.err" 2>/dev/null)）"
fi

# ── [5] 覆盖边界 ────────────────────────────────────────────────────
echo '[5] 覆盖边界（如实登记）'
cat <<'TXT'
  · **离线夹具**，不是真对端：真对端连通性/内容由 W1 一次性复核覆盖
    （/data/app/ws/data/notes/M309_W1_VERIFY.md），**不由本门保证**（CI 上对端不可达）。
  · 本门覆盖的是**契约**：原子性（8 条负控 · 目标逐字节不动）· 幂等 · 落盘即验 ·
    失败语义 · 用法边界。**不覆盖**：网络中断（TCP 半开）· 对端慢（超时）·
    磁盘满（ENOSPC）· 并发两个 sync 同目标（闸门由调用方保证）。
  · **逐文件清单（§四 ②）未覆盖** —— 对端 `files_manifest` 取不到（F1：任何 `.sha256`
    一律 403）⇒ 本实现落在**归档级**校验与**归档级**增量。对端处置（推荐「清单并入归档」）
    后此处升级为逐文件校验，本层判据需同步扩面。
  · **首跑实测照出本门自身的两处夹具缺陷（已修，如实登记）**：
    ① `archive_sha256` 写成**全数字串**会被 `vj_set` 转成 JSON 数字 ⇒ 客户端报「缺字段」
       ⇒ 红**对了但理由错**（现由 N9 专门覆盖「类型不对」这条路径）；
    ② 越界夹具用 `../` 时 GNU tar **自己就会拒**（rc≠0）⇒ 撤判据后仍红 ⇒ 改**绝对路径**才证到牙。
  · 越界路径两种形态的拦截来源不同：绝对路径 `/x` **只有本判据**拦得住
    （GNU tar 只剥前导 `/` 且 rc=0）；相对 `../x` 由 **tar 自身**拦（`Member name contains '..'`）
    ⇒ 双保险（N8 / N8b 分别覆盖）。
  · `--force` 只在本门间接覆盖（N1–N8 用「置陈旧指纹」达成同样效果）；
    未单独断言 `--force` 的语义等价性。
TXT
ok '[5] 覆盖边界已登记'

echo
echo "══ 汇总：通过 $PASS / 失败 $FAIL ══"
if [ "$FAIL" = 0 ]; then echo 'M309-VERIFY-OK'; rm -rf "$W"; exit 0; fi
echo 'M309-VERIFY-FAIL'
exit 1
