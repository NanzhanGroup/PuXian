#!/usr/bin/env bash
# ============================================================
# selfhost/go_json_v1.sh —— 把 Go 的 `encoding/json` 钉在 **v1 口径**（M320 / 缺陷 553）
# ------------------------------------------------------------
# 为什么要有它（实测，不是推测）：
#   Go **1.27**（2026-10）起 `encoding/json` 的默认实现换成 **jsonv2** ——
#   GOROOT 里两套实现并存、按 build tag 二选一：
#     `indent.go`   带 `//go:build !goexperiment.jsonv2`
#     `v2_indent.go` 带 `//go:build goexperiment.jsonv2`
#   JSON v1 退到 `GOEXPERIMENT=nojsonv2` 背后。分叉实测（本机 go1.27.3 vs 1.26.7，
#   同一个语料、同一份门）：
#     · `Compact` 错误对象的 `Offset` —— v1 **恒 0**，v2 变成「输入长度」
#     · 错误文案 —— `after array element`→`after array value`
#                    `in string escape code`→`invalid escape sequence \x in string`
#                    `looking for beginning of value`→`invalid character '\ufeff'…`
#                    `invalid character '[' exceeded max depth`→`exceeded max depth`
#                    `in literal true (…)`→`unexpected end of JSON input`
#   实测 m142 的 **696 行**真值里 **112 行**在这两档之间不同。
#   ⇒ 基准**跟着工具链默认档漂移**：同一条门，换台机器（Go 版本不同）红绿不同。
#
#   PuXian 复刻的是 **JSON v1** 口径（`stdlib/go_json_scan.px` / `go_json_indent.px`，
#   缺陷 111/112/115 那批），所以凡是拿 `encoding/json` 的**文本或错误文案**当基准的门，
#   都必须**显式钉死**口径，不能吃工具链默认档。
#
# 纪律：
#   ① 有 jsonv2 实验项的工具链（Go ≥ 1.27）—— 一律显式设 `GOEXPERIMENT=nojsonv2`。
#   ② 没有 jsonv2 实验项的工具链（Go < 1.27）—— **不能**设 `nojsonv2`（旧 `go` 连
#      `go env` 都会炸：`go: unknown GOEXPERIMENT jsonv2`）；此时 encoding/json 只有
#      v1 实现，口径天然在 v1。**门必须把这一档如实报出来并自证**（见 ④），
#      而不是「设置失败 ⇒ 判红」—— CI runner 自带的就是 pre-1.27 工具链。
#   ③ 调用方已显式指定 `GOEXPERIMENT` 时**不覆盖**（允许做档位对比实验），
#      但门里会**响亮告警并判红**（见 go_json_v1_report）—— 不许静默漂移。
#   ④ 「天然 v1」不是空口：报告的 natural 档会**再探一次**实验项是否存在；
#      若工具链已升到 1.27+（实验项有了）却没人钉 nojsonv2 ⇒ 判红。
#      将来某版 Go 若删掉该实验项，探针会自动回落到 natural 档并打印取证行。
#
# 用法（门内，在编译 truth 之前）：
#   . "$(dirname "$0")/../../selfhost/go_json_v1.sh"
#   go_json_v1_report || exit 1        # 打印档位证据 + 断言「确实钉在 v1」
#   (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth .)
# ============================================================
# ── 探测：本工具链认不认 jsonv2 这个实验项（Go 1.27 才引入）──
#   ⚠️ 取版本/探针都必须「清空 GOEXPERIMENT 后再调 go」：给旧工具链塞未知实验项，
#      连 `go env` 都会报 unknown GOEXPERIMENT ⇒ 会被误写成 <无 go>，把版本这一维看丢。
_go_json_has_v2_experiment() {
    GOEXPERIMENT=jsonv2 go env GOVERSION >/dev/null 2>&1
}

if [ -n "${GOEXPERIMENT:-}" ]; then
    # 调用方显式指定 ⇒ 不覆盖（档位对比实验用），报告里判红责任在调用方
    case "$GOEXPERIMENT" in
        *nojsonv2*) GO_JSON_V1_SRC="调用方显式指定" ; GO_JSON_V1_MODE=explicit ;;
        *)          GO_JSON_V1_SRC="调用方显式指定（⚠ 不含 nojsonv2）" ; GO_JSON_V1_MODE=explicit ;;
    esac
elif _go_json_has_v2_experiment; then
    export GOEXPERIMENT=nojsonv2
    GO_JSON_V1_SRC="go_json_v1.sh 默认钉死"
    GO_JSON_V1_MODE=pin
else
    # 工具链里没有 jsonv2 实验项 ⇒ encoding/json 只有 v1 实现，口径天然在 v1。
    GO_JSON_V1_SRC="工具链无 jsonv2 实验项（pre-1.27）⇒ 只有 v1 实现"
    GO_JSON_V1_MODE=natural
fi

# 打印档位证据；没钉在 v1 上就返回 1（门据此判红，而不是默认档悄悄换掉基准）。
go_json_v1_report() {
    local tv
    tv=$(GOEXPERIMENT= go env GOVERSION 2>/dev/null || echo "<无 go>")
    echo "   [json-v1] toolchain=$tv · GOEXPERIMENT=${GOEXPERIMENT:-<工具链默认>} · 来源=$GO_JSON_V1_SRC"
    case "${GO_JSON_V1_MODE:-pin}" in
        natural)
            # 自证「天然 v1」不是空口：此时工具链**必须**仍然不认 jsonv2 实验项。
            # 认了（已升到 1.27+）⇒ 默认档可能已切 v2 ⇒ 必须回来显式钉住。
            if _go_json_has_v2_experiment; then
                case "${GOEXPERIMENT:-}" in
                    *nojsonv2*)
                        echo "❌ [json-v1] 档位自相矛盾：判定为 natural（无实验项）却探测到实验项存在（GOEXPERIMENT=$GOEXPERIMENT）—— 探测结果不稳定，基准不可信" >&2 ;;
                    *)
                        echo "❌ [json-v1] 工具链已支持 jsonv2 实验项，但 GOEXPERIMENT 未钉 nojsonv2 —— 默认档可能已切 v2" >&2 ;;
                esac
                return 1
            fi
            return 0
            ;;
        *)
            case "${GOEXPERIMENT:-}" in
                *nojsonv2*) return 0 ;;
                *)
                    echo "❌ [json-v1] GOEXPERIMENT 不含 nojsonv2 —— 基准会随工具链默认档漂移" >&2
                    echo "   （M320 / 缺陷 553：Go 1.27 起 encoding/json 默认走 jsonv2，与我们复刻的 v1 口径分叉）" >&2
                    return 1
                    ;;
            esac
            ;;
    esac
}
