#!/usr/bin/env bash
# ══════════════════════════════════════════════════════════════
# examples/m267_perf/s5_remote_baseline.sh
#   —— S5「一次性基准机」：在**租用机**上跑 m267 性能基线采集，并把档回传固化
#
# 一句话：把「租一台机 → 上去跑基线 → 把档存回仓库 → 销毁实例」里的**中间三段**
#         做成一条命令，且每一步都有判据。
#
# 为什么需要它（M305 → M307）：
#   · 2026-10-12 dongyue 被云厂商**冷迁移**（AMD EPYC 7K62 → Intel Xeon E5-2682 v4，
#     落后约 3 代）后，m267 门恒报 4 项红 —— 而那**不是回归**（C 轨原生产物同样慢
#     1.4–2.2×）⇒ **绝对值不可跨机**，已被实测钉死。
#   · M305 的解法 = `baselines/<cpu-signature>.tsv`（每机一档）。但**档从哪来**仍靠人手：
#     ssh 上去、下包、装依赖、跑采集、拷回来。本脚本就是那一步的自动化。
#   · 反过来：**dongyue 自己不能当基准机** —— 它既跑生产又跑门（负载会抖，M305 那次
#     就是活例），而且云厂商可以随时换掉它下面那颗 CPU。基准机必须是**一次性的、干净的**。
#
# 用法：
#   bash examples/m267_perf/s5_remote_baseline.sh --host <ip> [选项]
# 选项：
#   --host HOST       目标机（必填）
#   --port N          ssh 端口（默认 22）
#   --user U          登录用户（默认 root）
#   --key PATH        ssh 私钥（默认 $PX_S5_KEY → ~/.ssh/id_ed25519 → ~/.ssh/id_rsa）
#   --workdir DIR     远端工作目录（默认 /opt/puxian-s5）
#   --tag TAG         引导包版本（默认从镜像 version.json 取最新发布 tag）
#   --reps N          独立采集组数（默认 3 · 与 M305 建 Xeon 档同口径）
#   --no-onnx         跳过 ONNX 两项的补测（默认补）
#   --skip-setup      跳过「落包 + 装依赖 + 自举」（工作目录已就绪时复跑采集）
#   --arch-hint A     仅 --dry-run 用：假定远端架构（x86_64|aarch64）
#   --dry-run         只打印将要执行的步骤
#
# 前置：
#   ① 本机能 ssh 到目标机；② 目标机能出网（走国内镜像；**不需要 GitHub**）；
#   ③ 目标机有 gcc / python3 / taskset / strings / sha256sum / curl。
#
# 边界（如实登记）：
#   · **不负责「租」与「销毁」** —— 那是云 API/控制台的事；本脚本只做「跑 + 存」。
#     租机规格建议：8C16G **独享/计算型**（⚠️ 不要突发型/共享型积分 CPU —— 积分耗尽
#     会把基准数字毁掉），本地 NVMe ≥100 GiB（IOPS 直接影响 startup/build 两项），
#     OS 与 CI 同档（Rocky 9 / Ubuntu 22.04–24.04）。
#   · **不自动 commit / tag / Release** —— 档回传后由人按仓库纪律核对后再提交。
#   · ONNX 两项（onnx_mm512 / onnx_mm1024）**不在** `collect_baseline_v2.sh` 的 LOADS 里
#     （采集器口径如此）⇒ 由本脚本第 6 步**单独补测后追加**，与 M303 建 EPYC 档同做法。
#   · aarch64 的**官方引导包不含 VM 轨件**（`selfhost/build_native_tools.sh` 主动移除：
#     非 x86_64 包内带 VM 件会让用户拿到 `Exec format error`）⇒ 需要 VM 轨数据时先**本机自举**
#     （第 2c 步）。M307 实测：自举后 VM/C 双轨 stdout 逐字节一致，可放心采；
#     自举件与发布件**语义相同**（同一 compiler.px + 同一 runtime/vm.c），档头注如实登记。
#   · 目标机签名（档名）**从回传的 raw.env.cpu_model 归一化得到**，不用 `machine_sig()` 的
#     本机值 —— 否则会把工作站的签名写到目标机的档上（这正是 M305 的档机制要防的事）。
# ══════════════════════════════════════════════════════════════
set -uo pipefail

ROOT="$(cd "$(dirname "$0")/../.." && pwd)"
DIR="$ROOT/examples/m267_perf"
MIRROR="${PX_S5_MIRROR:-https://soft.xiusoft.cn/puxian}"

HOST=""; PORT=22; USER_="root"; KEY=""
WORKDIR="/opt/puxian-s5"; TAG=""; REPS=3
DO_ONNX=1; SKIP_SETUP=0; DRY=0; ARCH_HINT="x86_64"

while [ $# -gt 0 ]; do
    case "$1" in
        --host) HOST="$2"; shift 2 ;;
        --port) PORT="$2"; shift 2 ;;
        --user) USER_="$2"; shift 2 ;;
        --key)  KEY="$2"; shift 2 ;;
        --workdir) WORKDIR="$2"; shift 2 ;;
        --tag)  TAG="$2"; shift 2 ;;
        --reps) REPS="$2"; shift 2 ;;
        --no-onnx) DO_ONNX=0; shift ;;
        --skip-setup) SKIP_SETUP=1; shift ;;
        --arch-hint) ARCH_HINT="$2"; shift 2 ;;
        --dry-run) DRY=1; shift ;;
        -h|--help) sed -n '2,50p' "$0"; exit 0 ;;
        *) echo "未知参数：$1" >&2; exit 2 ;;
    esac
done
[ -n "$HOST" ] || { echo "❌ 缺 --host（用法见 --help）" >&2; exit 2; }

# ── 私钥发现（不猜、找不到就响亮）─────────────────────────────────
if [ -z "$KEY" ]; then
    for k in "${PX_S5_KEY:-}" "$HOME/.ssh/id_ed25519" "$HOME/.ssh/id_rsa"; do
        [ -n "$k" ] && [ -r "$k" ] && { KEY="$k"; break; }
    done
fi
[ -n "$KEY" ] || { echo "❌ 找不到可读的 ssh 私钥（--key / \$PX_S5_KEY / ~/.ssh/id_*）" >&2; exit 2; }

SSHOPTS=(-p "$PORT" -i "$KEY" -o StrictHostKeyChecking=no -o ConnectTimeout=12 -o BatchMode=yes)
SCP_OPTS=(-P "$PORT" -i "$KEY" -o StrictHostKeyChecking=no)

say()  { printf '\n══ %s ══\n' "$*"; }
step() { printf '\n── [%s] %s\n' "$1" "$2"; }
die()  { echo "❌ $*" >&2; exit 1; }
runr() { if [ "$DRY" = 1 ]; then echo "   DRY ssh :: $1"; return 0; fi; ssh "${SSHOPTS[@]}" "$USER_@$HOST" "$1"; }
copy_to()   { if [ "$DRY" = 1 ]; then echo "   DRY scp -> $2"; return 0; fi; scp "${SCP_OPTS[@]}" "$1" "$USER_@$HOST:$2"; }
copy_from() { if [ "$DRY" = 1 ]; then echo "   DRY scp <- $1"; return 0; fi; scp "${SCP_OPTS[@]}" "$USER_@$HOST:$1" "$2"; }

# 远端仓库根（每次现场求值 —— 不写死解包出来的目录名）
RD_CD='d=$(find . -maxdepth 1 -type d -name "puxian*" | head -1); [ -n "$d" ] || { echo NO-PKG-ROOT; exit 3; }; cd "$d"'

say "S5 一次性基准机 —— host=$HOST port=$PORT user=$USER_ workdir=$WORKDIR reps=$REPS onnx=$DO_ONNX dry=$DRY"

# ══ 0 前置检查 ═════════════════════════════════════════════════
step 0 "前置检查（连通 / 架构 / 工具链）"
if [ "$DRY" = 1 ]; then
    RARCH="$ARCH_HINT"; echo "   远端架构：$RARCH（dry-run 假定；真跑时取 uname -m）"
else
    ssh "${SSHOPTS[@]}" "$USER_@$HOST" true || die "ssh 不通：$USER_@$HOST:$PORT（检查密钥/安全组）"
    RARCH=$(ssh "${SSHOPTS[@]}" "$USER_@$HOST" 'uname -m' | tail -1)
    [ -n "$RARCH" ] || die "取不到远端架构"
    echo "   远端架构：$RARCH"
    MISS=""
    for t in gcc python3 taskset strings sha256sum curl find; do
        ssh "${SSHOPTS[@]}" "$USER_@$HOST" "command -v $t >/dev/null" 2>/dev/null || MISS="$MISS $t"
    done
    [ -n "$MISS" ] && die "远端缺工具：$MISS"
fi
case "$RARCH" in
    x86_64|aarch64) : ;;
    *) die "不支持的架构：$RARCH（本仓性能基线只覆盖 x86_64 / aarch64）" ;;
esac

# ══ 1 落包（镜像 + sha256 强校验）═══════════════════════════════
step 1 "落包（国内镜像 · sha256 强校验）"
VJ=$(curl -fsSL "$MIRROR/version.json" 2>/dev/null) || die "取不到 $MIRROR/version.json"
[ -n "$TAG" ] || TAG=$(printf '%s' "$VJ" | sed -n 's/.*"tag" *: *"\([^"]*\)".*/\1/p' | head -1)
[ -n "$TAG" ] || die "解析不出 tag（version.json 形状变了？）"
if [ "$RARCH" = aarch64 ]; then
    PKG_REL=$(printf '%s' "$VJ" | sed -n 's/.*"bootstrap_aarch64_tarball" *: *"\([^"]*\)".*/\1/p' | head -1)
    PKG_SHA=$(printf '%s' "$VJ" | sed -n 's/.*"bootstrap_aarch64_sha256" *: *"\([^"]*\)".*/\1/p' | head -1)
else
    PKG_REL=$(printf '%s' "$VJ" | sed -n 's/.*"tarball" *: *"\([^"]*\)".*/\1/p' | head -1)
    PKG_SHA=$(printf '%s' "$VJ" | sed -n 's/.*"tarball_sha256" *: *"\([^"]*\)".*/\1/p' | head -1)
fi
[ -n "$PKG_REL" ] && [ -n "$PKG_SHA" ] || die "version.json 缺 $RARCH 的包字段"
echo "   tag=$TAG"; echo "   包=$PKG_REL"; echo "   期望 sha256=$PKG_SHA"
if [ "$SKIP_SETUP" = 0 ]; then
    runr "mkdir -p $WORKDIR && cd $WORKDIR && rm -rf pkg.tgz puxian-* && \
          curl -fsSL -o pkg.tgz '$MIRROR/$PKG_REL' && \
          echo '$PKG_SHA  pkg.tgz' | sha256sum -c - && tar xzf pkg.tgz && $RD_CD && echo PKGROOT-\$d" \
        || die "落包失败（下载 / sha256 / 解包）"
fi

# ══ 2 依赖 + 套件 + （aarch64）自举 VM 件 ═══════════════════════
if [ "$SKIP_SETUP" = 0 ]; then
    step 2 "装依赖（静态 libc —— PuXian 产物一律 -static）"
    runr 'if command -v dnf >/dev/null 2>&1; then
             dnf install -y glibc-static >/dev/null 2>&1 || dnf install -y --enablerepo=crb glibc-static >/dev/null 2>&1 || true
          elif command -v apt-get >/dev/null 2>&1; then
             apt-get update -qq >/dev/null 2>&1; apt-get install -y libc6-dev >/dev/null 2>&1 || true
          fi
          cd /tmp && printf "int main(){return 0;}\n" > .s5s.c && gcc -static -o .s5s .s5s.c 2>/dev/null && echo STATIC-LINK-OK || echo STATIC-LINK-FAIL'
    # 判据：真能静态链一个最小程序（"装了包"不算）—— 失败不中止，留给人看（可能是别的发行版包名）

    step 2b "传采集套件（仓库 → 远端）"
    TMPB=$(mktemp -d)
    SET=( examples/m267_perf docs/PERF_BASELINE_METRICS_SPEC.md )
    # `selfhost/*.sh` 与 `*.py`：**门**要用（M276 起每个门都 source `gate_lock.sh`）
    #   ⇒ 不传的话「在租来的机上跑 verify.sh」第一步就失败（M307 实测：
    #      `No such file or directory: selfhost/gate_lock.sh`）。x86_64 / aarch64 都要。
    for f in "$ROOT"/selfhost/*.sh "$ROOT"/selfhost/*.py; do
        [ -f "$f" ] && SET+=( "${f#"$ROOT"/}" )
    done
    if [ "$RARCH" = aarch64 ]; then
        # 自举 VM 件还需要 selfhost/*.px 与 tools/*.px（远端包内没有 selfhost/）
        for f in "$ROOT"/selfhost/*.px "$ROOT"/tools/*.px; do
            [ -f "$f" ] && SET+=( "${f#"$ROOT"/}" )
        done
    fi
    tar czf "$TMPB/s5-suite.tgz" -C "$ROOT" "${SET[@]}" || die "打包失败"
    echo "   套件：$(( ${#SET[@]} )) 个条目 · $(stat -c %s "$TMPB/s5-suite.tgz") 字节"
    copy_to "$TMPB/s5-suite.tgz" "$WORKDIR/s5-suite.tgz" || die "传套件失败"
    runr "cd $WORKDIR && $RD_CD && tar xzf ../s5-suite.tgz && echo SUITE-OK" || die "解套件失败"
    rm -rf "$TMPB"

    if [ "$RARCH" = aarch64 ]; then
        step 2c "自举 VM 轨编译器（aarch64 官方包内无 VM 件 ⇒ 本机自举；见头注「边界」）"
        runr "cd $WORKDIR && $RD_CD && RT=\$PWD/runtime && \
              ./bootstrap/pxc --emit-c selfhost/compiler.px > /tmp/s5vm.c 2>/tmp/s5vm.err || { tail -3 /tmp/s5vm.err; exit 4; } && \
              CACHE=\$PWD/.rtcache/ccd85d85eda7bab9; [ -d \"\$CACHE\" ] || CACHE=\$(./tools/px rtcache 2>/dev/null | tail -1); \
              [ -d \"\$CACHE\" ] || { echo NO-RTCACHE; exit 5; }; \
              objs=\$(ls \$CACHE/*.o | tr '\n' ' ') && \
              gcc -c -O2 -I\$CACHE -I\$RT /tmp/s5vm.c -o /tmp/s5vm.o && \
              gcc -static -O2 -pthread -o bootstrap/pxc_vm /tmp/s5vm.o \$objs \
                  \$RT/mbedtls/lib-aarch64/libmbedtls.a \$RT/mbedtls/lib-aarch64/libmbedx509.a \$RT/mbedtls/lib-aarch64/libmbedcrypto.a \
                  \$RT/third_party/zlib/lib-aarch64/libz.a \$RT/third_party/sqlite3/sqlite3-aarch64.o -lm -ldl -lpthread && \
              ./bootstrap/pxc_vm --version && echo SELFBOOT-VM-OK" \
            || die "aarch64 VM 件自举失败（远端 /tmp/s5vm.err）"
    fi
fi

# ══ 3 采集 ═════════════════════════════════════════════════════
step 3 "采集 $REPS 组（钉核 taskset -c 3 · 每负载双轨 ×5 轮取 min + 200 次启动 + 热构建）"
runr "cd $WORKDIR && $RD_CD && rm -rf /tmp/m267_perf_v2 && \
      bash examples/m267_perf/collect_baseline_v2_driver.sh $REPS" || die "采集失败"

# ══ 4 回传原始数据（**不用远端 glob** —— scp 走 SFTP，不展开通配符）════
step 4 "回传原始数据"
OUT="$ROOT/examples/m267_perf/_s5_incoming"
mkdir -p "$OUT"
runr "cd $WORKDIR && $RD_CD && mkdir -p /tmp/s5out && rm -f /tmp/s5out/* && \
      cp examples/m267_perf/raw_baseline_v2*.json examples/m267_perf/raw_baseline_v2*.md /tmp/s5out/ && ls /tmp/s5out/" \
    || die "远端打包原始数据失败"
for f in raw_baseline_v2.json raw_baseline_v2.md raw_baseline_v2.candidate.tsv; do
    copy_from "/tmp/s5out/$f" "$OUT/$f" 2>/dev/null || true
done
copy_from "/tmp/s5out/raw_baseline_v2.rep1.json" "$OUT/raw_baseline_v2.rep1.json" 2>/dev/null || true
copy_from "/tmp/s5out/raw_baseline_v2.rep2.json" "$OUT/raw_baseline_v2.rep2.json" 2>/dev/null || true
copy_from "/tmp/s5out/raw_baseline_v2.rep3.json" "$OUT/raw_baseline_v2.rep3.json" 2>/dev/null || true
[ -s "$OUT/raw_baseline_v2.json" ] || [ "$DRY" = 1 ] || die "回传后 raw_baseline_v2.json 为空"

# ══ 5 固化（**在本机**做：freeze 要用 git 取 commit id）════════════
step 5 "固化档（freeze_baseline_v2.py → baselines/<目标机签名>.tsv）"
. "$DIR/machine_sig.sh"
RSIG=$(python3 - "$OUT/raw_baseline_v2.json" <<'PY' 2>/dev/null
import json,re,sys
cpu=json.load(open(sys.argv[1]))["repetitions"][0]["env"]["cpu_model"]
s=cpu.lower()
s=re.sub(r'\(r\)|\(tm\)','',s); s=re.sub(r'[0-9]+-core','',s); s=re.sub(r'@.*','',s)
s=re.sub(r'\b(cpu|processor)\b','',s); s=re.sub(r'[^a-z0-9]+','-',s); s=s.strip('-')
print(re.sub(r'-+','-',s))
PY
)
[ -n "$RSIG" ] || RSIG="unknown-$RARCH"
OBASE="$DIR/baselines/$RSIG.tsv"
echo "   本机（工作站）签名 = $(machine_sig)（仅参考）"
echo "   目标机签名 = $RSIG  ⇒ 档 = $OBASE"
if [ "$DRY" = 0 ] && [ -f "$OBASE" ]; then die "档已存在：$OBASE —— 覆盖前请人工确认（本脚本不自动覆盖已发布档）"; fi
if [ "$DRY" = 0 ]; then
    python3 "$DIR/freeze_baseline_v2.py" \
        --raw "$OUT/raw_baseline_v2.json" \
        --baseline "$OBASE" \
        --machine-sig "$RSIG" \
        --out-json "$OUT/baseline_v2.json" \
        --out-md "$OUT/baseline_v2.md" || die "固化失败"
    echo "   ⇒ 已产 $OBASE"
fi

# ══ 6 ONNX 两项补测并追加（口径同 M303）═════════════════════════
if [ "$DO_ONNX" = 1 ]; then
    step 6 "ONNX 两项补测（不在采集器 LOADS 内）"
    runr "cd $WORKDIR && $RD_CD && PX=\$PWD/tools/px && \
          rm -rf /tmp/s5onnx && mkdir -p /tmp/s5onnx/models && \
          python3 examples/m267_perf/tools/gen_perf_models.py /tmp/s5onnx/models >/dev/null && \
          cp examples/m267_perf/src/onnx_mm512.px examples/m267_perf/src/onnx_mm1024.px /tmp/s5onnx/ && \
          cd /tmp/s5onnx && \$PX build onnx_mm512.px >/dev/null 2>&1 && \$PX build onnx_mm1024.px >/dev/null 2>&1 && \
          for g in 1 2 3; do
            b512=999; for i in 1 2 3; do
              s=\$(date +%s.%N); taskset -c 3 ./build/onnx_mm512 /tmp/s5onnx/models/onnx_mm512.onnx >/dev/null 2>&1; e=\$(date +%s.%N)
              dd=\$(echo \"\$e \$s\" | awk '{printf \"%.4f\",\$1-\$2}'); b512=\$(echo \"\$b512 \$dd\" | awk '{print (\$2<\$1)?\$2:\$1}')
            done
            b1024=999; for i in 1 2; do
              s=\$(date +%s.%N); taskset -c 3 ./build/onnx_mm1024 /tmp/s5onnx/models/onnx_mm1024.onnx >/dev/null 2>&1; e=\$(date +%s.%N)
              dd=\$(echo \"\$e \$s\" | awk '{printf \"%.4f\",\$1-\$2}'); b1024=\$(echo \"\$b1024 \$dd\" | awk '{print (\$2<\$1)?\$2:\$1}')
            done
            echo S5ONNX \$g \$b512 \$b1024
          done" > "$OUT/s5onnx.log" 2>&1 || true
    LINES=$(grep '^S5ONNX ' "$OUT/s5onnx.log" 2>/dev/null || true)
    if [ "$DRY" = 1 ]; then
        echo "   DRY: 将补测 onnx_mm512 / onnx_mm1024（3 组取 min）并追加到档"
    elif [ -n "$LINES" ]; then
        M512=$(printf '%s\n' "$LINES" | awk '{print $3}' | sort -n | head -1)
        M1024=$(printf '%s\n' "$LINES" | awk '{print $4}' | sort -n | head -1)
        {
            echo ""
            echo "# ── M307（S5）追加 ONNX 两行（单独补测 · 3 组取 min；采集器 LOADS 不含 onnx —— 见 s5_remote_baseline.sh 头注）"
            printf '%-14s %-8s %-6s %-10s %s\n' onnx_mm512  onnx s "$M512"  "ONNX MatMul 512³ ×5 —— 常规内核区（无跨页崩塌）"
            printf '%-14s %-8s %-6s %-10s %s\n' onnx_mm1024 onnx s "$M1024" "ONNX MatMul 1024³ ×1 —— 跨页崩塌探针（路线 S3 的靶子）"
        } >> "$OBASE"
        echo "   ⇒ 追加 onnx_mm512=$M512  onnx_mm1024=$M1024"
    else
        echo "   ⚠️ ONNX 补测无读数（见 $OUT/s5onnx.log）—— 档里**暂缺**这两行，如实登记，勿凭空填"
    fi
fi

# ══ 7 自校验 + 汇总 ═══════════════════════════════════════════
step 7 "自校验与汇总"
bash "$DIR/check_spec_consistency.sh" 2>&1 | tail -8 || true
if [ "$DRY" = 0 ] && [ -f "$OBASE" ]; then
    echo; echo "── 档 $OBASE ──"; cat "$OBASE"
    echo; echo "── 与既有档对照（本档 ÷ 其它档；>1 = 本机更慢）──"
    for f in "$DIR/baselines"/*.tsv; do
        [ "$f" = "$OBASE" ] && continue
        printf '   %-30s' "$(basename "$f" .tsv)"
        grep -v '^#' "$OBASE" | while read -r n k u v rest; do
            o=$(awk -v n="$n" '$1==n{print $4}' "$f")
            [ -n "$o" ] && awk -v n="$n" -v a="$v" -v b="$o" 'BEGIN{ if (b+0>0) printf "%s=%.2fx ", n, a/b }'
        done
        echo
    done
fi
echo
echo "✅ S5 流程结束。后继（人工，按仓库纪律）："
echo "   ① 核对 $OBASE 与 $OUT/*（raw / md / s5onnx.log）"
echo "   ② 写 CHANGELOG 与 docs/PERF_BASELINE_V2.md 的档记录"
echo "   ③ 提交；按需 packaging/make_tag.sh --milestone <N> --push"
echo "   ④ 销毁实例（**先确认 $OUT 已回传**）"
