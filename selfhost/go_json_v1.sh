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
#   ① 不检测、不猜 —— 一律显式设 `GOEXPERIMENT=nojsonv2`。
#      实测 go1.26.7 与 go1.27.3 **都接受**该取值（在 1.26 上是无害 no-op）。
#   ② 调用方已显式指定 `GOEXPERIMENT` 时**不覆盖**（允许做档位对比实验），
#      但门里会**响亮告警并判红**（见 go_json_v1_report）—— 不许静默漂移。
#   ③ 将来若某版 Go 删掉该实验项，`go build` 会报 `unknown GOEXPERIMENT nojsonv2`
#      直接红 ⇒ 被迫回来重新定口径 —— 要的就是这个「不许静默改基准」。
#
# 用法（门内，在编译 truth 之前）：
#   . "$(dirname "$0")/../../selfhost/go_json_v1.sh"
#   go_json_v1_report || exit 1        # 打印档位证据 + 断言「确实钉在 v1」
#   (cd truth && GOFLAGS=-mod=mod go build -o ../build/truth .)
# ============================================================
if [ -z "${GOEXPERIMENT:-}" ]; then
    export GOEXPERIMENT=nojsonv2
    GO_JSON_V1_SRC="go_json_v1.sh 默认钉死"
else
    case "$GOEXPERIMENT" in
        *nojsonv2*) GO_JSON_V1_SRC="调用方显式指定" ;;
        *)          GO_JSON_V1_SRC="调用方显式指定（⚠ 不含 nojsonv2）" ;;
    esac
fi

# 打印档位证据；没钉在 v1 上就返回 1（门据此判红，而不是默认档悄悄换掉基准）。
go_json_v1_report() {
    local tv
    tv=$(go env GOVERSION 2>/dev/null || echo "<无 go>")
    echo "   [json-v1] toolchain=$tv · GOEXPERIMENT=${GOEXPERIMENT:-<工具链默认>} · 来源=$GO_JSON_V1_SRC"
    case "${GOEXPERIMENT:-}" in
        *nojsonv2*) return 0 ;;
        *)
            echo "❌ [json-v1] GOEXPERIMENT 不含 nojsonv2 —— 基准会随工具链默认档漂移" >&2
            echo "   （M320 / 缺陷 553：Go 1.27 起 encoding/json 默认走 jsonv2，与我们复刻的 v1 口径分叉）" >&2
            return 1
            ;;
    esac
}
