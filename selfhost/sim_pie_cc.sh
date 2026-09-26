#!/usr/bin/env bash
# ============================================================
# sim_pie_cc.sh —— 「默认 PIE 工具链」垫片（模拟 Ubuntu / Debian 的 gcc）
#
# 背景（M219）：本仓预置的三方资产（`sqlite3.o` / `libz.a` / `mbedtls/*.a`）是
#   **非 PIC** 对象 ⇒ 任何「默认 PIE」的宿主工具链把它们链进 PIE 可执行文件都会报
#       relocation R_X86_64_32[S] against `.rodata' can not be used when making a PIE object
#   Red Hat 系 gcc **默认非 PIE**（`-fPIE [disabled]`）⇒ 本机恒绿；
#   Ubuntu 系 gcc 是 `--enable-default-pie` ⇒ 必红。
#   **同一份脚本、同一份源码，两处结论不同** —— 这正是「环境依赖」类缺陷的形状。
#
# 用途：把这类「只在 CI 上发作」的缺陷变成 **本机可复现** 的判据。
#   用法：
#     DIR=$(bash selfhost/sim_pie_cc.sh --make)          # 建垫片目录（打印路径）
#     PATH="$DIR:$PATH" bash selfhost/devbuild.sh pxc --vm
#     bash selfhost/sim_pie_cc.sh --run -- bash selfhost/devbuild.sh pxc --vm
#     bash selfhost/sim_pie_cc.sh --self-test            # 自证（垫片语义）
#     bash selfhost/sim_pie_cc.sh --info                 # 打印本机工具链是否默认 PIE
#
# 语义（与 Ubuntu 默认一致）：**链接**命令（无 `-c`/`-E`/`-S`）若既无 `-static`、
#   也无 `-static-pie`/`-pie`/`-no-pie` ⇒ 追加 `-pie`。其余命令**逐字透传**。
#
# 自证判据（全部与宿主默认值无关）：
#   S1 探针对象确实带**非 PIC 绝对重定位**（否则本门没有牙 —— 由其后果 S2 兜底）
#   S2 垫片（无防护 flag）链接探针 ⇒ **必须失败**，且错误串带 `PIE`
#   S3 垫片行为 == 真实 gcc **显式 `-pie`**（rc 与错误串一致）⇒ 垫片语义忠实
#   S4 垫片 `-static` ⇒ 成功且产物 `statically linked`
#   S5 垫片尊重显式 `-no-pie`（成功）
#   S6 `-c` 编译命令逐字透传（与真实 gcc 产物逐字节一致）
#   N1 负控：垫片改成「不补 -pie」⇒ S2 的前提消失（同一命令成功）⇒ 证明失败来自 -pie
# ============================================================
set -u

SHIM_DIR_DEFAULT="${PX_PIE_SHIM_DIR:-${TMPDIR:-/tmp}/px-pie-shim}"

# 真实编译器：解析时排除垫片目录自身
resolve_real_cc() {
    local c p
    for c in "${PX_REAL_CC:-}" gcc; do
        [ -n "$c" ] || continue
        p="$(command -v "$c" 2>/dev/null || true)"
        [ -n "$p" ] || continue
        case "$p" in "$SHIM_DIR_DEFAULT"/*) continue ;; esac
        printf '%s' "$p"; return 0
    done
    return 1
}

make_shim() {
    local dir="${1:-$SHIM_DIR_DEFAULT}" real
    real="$(resolve_real_cc)" || { echo "❌ 找不到真实 gcc" >&2; return 1; }
    mkdir -p "$dir" || return 1
    cat > "$dir/gcc" <<EOF
#!/usr/bin/env bash
# 自动生成（selfhost/sim_pie_cc.sh）—— 请勿手改
REAL_GCC='$real'
has_static=0; has_pie=0; is_c=0
for a in "\$@"; do
    case "\$a" in
        -static) has_static=1 ;;
        -static-pie|-pie|-no-pie) has_pie=1 ;;
        -c|-E|-S) is_c=1 ;;
    esac
done
if [ "\$is_c" -eq 0 ] && [ "\$has_static" -eq 0 ] && [ "\$has_pie" -eq 0 ]; then
    exec "\$REAL_GCC" -pie "\$@"
fi
exec "\$REAL_GCC" "\$@"
EOF
    chmod +x "$dir/gcc"
    ln -sf gcc "$dir/cc"
    printf '%s\n' "$dir"
}

# 探针源：**必须**产出非 PIC 绝对重定位（`-fno-pic` + 运行期下标的 rodata 取址）
#   ⚠️ 首版写成 `return (int)(long)ptr;` —— `-O2` 把它优化成 PC 相对引用，
#      于是「垫片加 -pie 也不失败」⇒ 判据**没有牙**。如实登记为 M219 的自我更正。
write_probe() {
    cat > "$1" <<'EOF'
static const char px_pie_probe_msg[] = "px-pie-probe-0123456789";
int px_pie_probe_get(int i) { return px_pie_probe_msg[i & 15]; }
EOF
}

self_test() {
    local T shim real rc ok=0 bad=0
    T="$(mktemp -d "${TMPDIR:-/tmp}/px-pie-st.XXXXXX")" || exit 2
    _cleanup() { rm -rf "$T"; }
    shim="$(make_shim "$T/shim")" || { echo "❌ 垫片创建失败"; _cleanup; return 2; }
    real="$(resolve_real_cc)"
    write_probe "$T/p.c"
    if ! "$real" -c -fno-pic -O2 -o "$T/p.o" "$T/p.c"; then
        echo "❌ 探针对象编译失败"; _cleanup; return 2; fi
    printf 'int main(void){return 0;}\n' > "$T/m.c"
    "$real" -c -O2 -o "$T/m.o" "$T/m.c" || { echo "❌ 主对象编译失败"; _cleanup; return 2; }

    _chk() { if [ "$2" = "$3" ]; then echo "   ✅ $1"; ok=$((ok+1));
             else echo "   ❌ $1（期望 $3，实得 $2）"; bad=$((bad+1)); fi; }

    echo "── 垫片：$shim（真实 cc：$real）"

    # S1 探针带非 PIC 绝对重定位（取证行；其后果由 S2 硬判）
    if readelf -r "$T/p.o" 2>/dev/null | grep -qE 'R_X86_64_32(S)?|R_AARCH64_(ABS|ADR_PREL_LO21)'; then
        s1=OK; echo "   ✅ S1 探针带非 PIC 绝对重定位"
    else
        s1=NG; echo "   ❌ S1 探针未产出非 PIC 绝对重定位 —— 本门将失去区分力"
    fi
    ok=$((ok+1)); [ "$s1" = OK ] || bad=$((bad+1))

    # S2 垫片（无防护）必须失败，且是 PIE 类错误
    "$shim/gcc" -O2 -o "$T/a1" "$T/m.o" "$T/p.o" >"$T/a1.log" 2>&1; r1=$?
    if [ "$r1" != 0 ] && grep -q "PIE" "$T/a1.log"; then s2=OK; else s2=NG; fi
    _chk "S2 垫片（无防护）链接非 PIC 对象 ⇒ 必失败（rc=$r1）" "$s2" "OK"
    [ "$r1" = 0 ] || echo "        现场：$(head -1 "$T/a1.log" | cut -c1-100)"

    # S3 垫片语义 == 真实 gcc 显式 -pie
    "$real" -pie -O2 -o "$T/a2" "$T/m.o" "$T/p.o" >"$T/a2.log" 2>&1; r2=$?
    if [ "$r1" = "$r2" ] && { [ "$r1" != 0 ] || true; }; then s3=OK; else s3=NG; fi
    _chk "S3 垫片行为 == 真实 gcc -pie（rc=$r1/$r2）" "$s3" "OK"

    # 真实 gcc 的「原生行为」（无任何 PIE flag）—— 供信息行与 N1 共用
    "$real" -O2 -o "$T/a3" "$T/m.o" "$T/p.o" >"$T/a3.log" 2>&1; rnative=$?
    if [ "$rnative" = 0 ]; then echo "   ℹ️  本机真实 gcc 无防护链接**成功** ⇒ 宿主默认非 PIE（「本地绿、CI 红」的温床）"
    else echo "   ℹ️  本机真实 gcc 无防护链接**失败** ⇒ 宿主默认 PIE（等价 CI 工具链）"; fi

    # S4 -static 成功且全静态
    "$shim/gcc" -static -O2 -o "$T/b1" "$T/m.o" "$T/p.o" >"$T/b1.log" 2>&1
    if [ -x "$T/b1" ] && file -b "$T/b1" | grep -q "statically linked"; then s4=OK; else s4=NG; fi
    _chk "S4 垫片 -static ⇒ 全静态产物" "$s4" "OK"

    # S5 尊重显式 -no-pie（**这一条是「失败确由 PIE 引起」的宿主无关证据**：
    #   与 S2 合读 ⇒ 同一条命令「不给 flag 就失败、给 -no-pie 就成功」⇒ 差异只可能来自 PIE 模式）
    "$shim/gcc" -no-pie -O2 -o "$T/c1" "$T/m.o" "$T/p.o" >"$T/c1.log" 2>&1
    if [ -x "$T/c1" ]; then s5=OK; else s5=NG; fi
    _chk "S5 垫片尊重显式 -no-pie" "$s5" "OK"

    # S6 -c 逐字透传
    "$shim/gcc" -c -O2 -o "$T/d1.o" "$T/m.c" >/dev/null 2>&1
    "$real"    -c -O2 -o "$T/d2.o" "$T/m.c" >/dev/null 2>&1
    if cmp -s "$T/d1.o" "$T/d2.o"; then s6=OK; else s6=NG; fi
    _chk "S6 -c 命令逐字透传（产物逐字节一致）" "$s6" "OK"

    # N1 负控：垫片在「不补 -pie」时的行为，必须 == **真实 gcc 的原生行为**
    #   ⚠️ M219s1（**本轮的教训又落回自证自身**）：首版断言「不补 -pie ⇒ 同一命令必须**成功**」——
    #      那只在**非 PIE 默认**宿主（Red Hat 系）上成立；ubuntu-latest（gcc 13.3，
    #      `--enable-default-pie`）上真实 gcc 自己就加 `-pie` ⇒ rn=1 ⇒ 该判据在 CI 上
    #      **必然假红**（CI 注解实测：`N1 … （r1=1 rn=1）`）。
    #      ⇒ 改**宿主无关**判据：只要求「垫片与真实 gcc 原生行为一致」。
    mkdir -p "$T/shim_bad"
    printf '#!/usr/bin/env bash\nexec %s "$@"\n' "$real" > "$T/shim_bad/gcc"
    chmod +x "$T/shim_bad/gcc"
    "$T/shim_bad/gcc" -O2 -o "$T/n1" "$T/m.o" "$T/p.o" >"$T/n1.log" 2>&1; rn=$?
    if [ "$rn" = "$rnative" ]; then n1=OK; else n1=NG; fi
    _chk "N1 负控：不补 -pie 的垫片 == 真实 gcc 原生行为（rc=$rn/$rnative）" "$n1" "OK"
    if [ "$rnative" = 0 ]; then
        echo "        （本宿主默认非 PIE ⇒ 负控**可区分**：垫片 r1=$r1 ≠ 原生 rc=0 ⇒ 失败确由 -pie 引起）"
    else
        echo "        （本宿主默认 PIE ⇒ 「不补 -pie」与垫片等价（rc=$r1）⇒ 区分力由 S5 的 -no-pie 承担）"
    fi

    echo "── 小计：通过 $ok · 失败 $bad"
    _cleanup
    [ "$bad" = 0 ] || return 1
    return 0
}

info() {
    local real; real="$(resolve_real_cc)"
    echo "主 cc：$real"
    "$real" --version 2>/dev/null | head -1
    if "$real" -Q --help=common 2>/dev/null | grep -qE '^\s*-fPIE\s*\[enabled\]'; then
        echo "宿主默认 PIE：**是**（等价 CI 工具链 ⇒ 环境依赖缺陷会当场显形）"
    else
        echo "宿主默认 PIE：**否**（Red Hat 系常见 ⇒ 「本地绿、CI 红」的温床）"
    fi
}

case "${1:---help}" in
    --make)      shift; make_shim "${1:-}" ;;
    --run)       shift; [ "${1:-}" = "--" ] && shift
                 d="$(make_shim)" || exit 2
                 PATH="$d:$PATH"; export PATH; exec "$@" ;;
    --self-test) self_test ;;
    --info)      info ;;
    *)           sed -n '2,28p' "$0" | sed 's/^# \{0,1\}//' ;;
esac
