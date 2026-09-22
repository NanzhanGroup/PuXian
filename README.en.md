# PuXian (PX)

> **The face of Python + the types of Rust + the concurrency of Go + the heritage of C**
> An original language purpose-built for efficient AI programming, running in dual modes on Linux.

🌐 **English** | [简体中文](README.md)

PuXian (`.px`) is a self-hosted programming language built from scratch, driven by one core idea: **helping AI (large language models) write reliable programs efficiently**.

- **Syntax = a Python subset**: AI models are trained on massive Python corpora — the closer a language's syntax is to Python, the higher the accuracy of AI-generated code.
- **Types = Rust style**: gradual typing (no annotations → run immediately; add annotations → get performance), enums, pattern matching, Option/Result.
- **Concurrency = Go style**: `spawn` + `channel` + `select`, true coroutine concurrency.
- **Heritage = a C compiler**: the compilation backend emits C source code, which gcc turns into a **zero-dependency static binary** (supply-chain neutral, trivial to deploy).

---

## ⚖️ License

![License](https://img.shields.io/badge/License-Apache--2.0-blue.svg)

PuXian is open-sourced under the **Apache License 2.0** — anyone is free to use, modify, distribute, and commercially exploit it (including closed-source commercial use) with no license fees. Full terms are in [`LICENSE`](LICENSE) at the repository root.

Key points for users of this project:

- **Compiled output is not bound by the license**: programs and services you write with PuXian may be closed-source and commercialized freely; they do not constitute derivative works.
- When you modify and redistribute the source code, you must retain the copyright notice and a copy of this License (Apache is permissive — **your modifications are not required to be open-sourced**).
- **How to contribute**: in the early stage the project primarily accepts issues (bug reports / feature suggestions / usage feedback); code PRs are not open yet, and the roadmap is controlled by the core team.
- **Copyright © 2026 The PuXian Authors**

---

## ® Trademark

**达者同游 (Dazhe Tongyou)** is a registered trademark of **南瞻集团 (Nanzhan Group)**; **PuXian** is the project name of the 达者同游 (Dazhe Tongyou) team (not a registered trademark).
Without prior written permission, these names may not be used to identify products or services that are not officially released by the Dazhe Tongyou team.

---

## 🎉 Current Status: Self-Hosted · Semantically Consistent Across Three Rails · First-Class aarch64

Current version **`px 0.2.0`** (latest milestone tag `v0.2.0-m167`).

| Status | Description |
|---|---|
| ✅ **Self-hosting complete (M-B8)** | **The PuXian compiler is written in PuXian itself**: `lexer / parser / codegen / bc_emit / interp / value system` all rewritten in `.px`. The bootstrap proof shows A.c == B.c == B2.c, byte-for-byte identical. |
| ✅ **Rust version retired (M-B9a)** | Rust sources archived under `archive/rust-compiler/` (read-only). The new toolchain **`tools/px` requires no Rust at all**, running on top of the bootstrap binary. |
| ✅ **Two backends, three rails (since M91)** | Interpreter rail (`pxi`, tree-walking) · **VM bytecode rail (default for `px build`)** · C-text rail (`px build --c`, escape hatch). **The same source behaves identically on all three rails** — enforced byte-for-byte by the full gate suite (`selfhost/m116_gates.sh` + per-milestone gates). |
| ✅ **HTTP/2 policy landed · HTTP/3 capability self-proven (M180)** | **H2 is not supported (long-term)** — policy in [docs/HTTP2_DECISION.md](docs/HTTP2_DECISION.md): an `Upgrade: h2c` request is **ignored and served as HTTP/1.1** (previously it returned a fixed **demo page** ⇒ silently wrong content) · an `PRI * HTTP/2.0` preamble now gets **505 + explanation** · ALPN advertises `http/1.1` only (the unreachable ALPN-h2 branch was deleted as dead code) · the h2 demo frame layer moved behind `opts{"h2_demo": true}` (off by default) · **H3 is the only modern path** (`opts{"http3": true}` + automatic `Alt-Svc`; x86_64 builds link ngtcp2 — asking for H3 in a `--no-quic` build now **fails loudly**) · `PX_BUILD_FEATURES=1 px build …` prints a **capability line** (`quic=on/off`; silent by default) |
| ✅ **One truth for operators + two silent bad values removed (M179)** | Before: `1.0 * "x"` in the compiled rails read the **pointer bit pattern** out of the value union (UB) ⇒ VM rail `6.905411902715e-310` vs C rail `6.95199933934835e-310` (**two different garbage values on one machine**) · `1 / "x"` ⇒ `inf` · `1 < "x"` ⇒ **`true`** (type-name ordering) · `1.5 & 1` ⇒ `1` (truncation) · integer divide-by-zero carried **no code**. Now: arithmetic requires numbers · ordering requires **two numbers or the same type** · bitwise ops and index positions require `int` · out-of-range `R1003: 索引越界: i (len=n)` · divide-by-zero `R1006`; `sorted`/`min`/`max` keep the **total-order internal comparator** |
| ✅ **One definition of "iterable argument" (M178)** | `list` / `tuple` / generator / **string (by rune)** — `join`/`sorted`/`reversed`/`contains` share the exact iteration semantics with `min`/`max`/`sum`/`unique`/`flatten` (`px_as_list` = `px_len` + `px_iter_at`). Along the way a **bad value** was fixed: `reversed("中文")` used to reverse **bytes** ⇒ **invalid UTF-8**; it now reverses runes and returns str. Rejection messages are identical across all three rails |
| ✅ **HTTP reverse-proxy / static closed (M173)** | The `vhost(host, handler)` branch now performs gzip negotiation (previously vhost sites shipped text responses **uncompressed**, wasting 3–5× bandwidth) · pooled connections now **re-apply `timeout_ms` before reuse** (previously a smaller timeout was silently ignored). Both gaps were found by **external QA from production traffic** — see CHANGELOG M173. |
| ✅ **Tooling shares the same truth (M171)** | `px lint`'s scope model now **equals the language's** (frame-top declaration hoisting · assignment-style bindings declared in place · inner frames see outer frames · comprehension variables visible across the whole comprehension · `.px_modules` imports · host-injected globals registered). Repo-wide false `L002`: **3651 in 151 files → 801 in 21 files** (`stdlib/collections.px` **46 → 0**) — previously, using the new semantics failed CI. |
| ✅ **Semantic consistency closure (M159–M169)** | Every corner where "the host language left it open ⇒ our three rails must diverge" has been nailed down to **one truth**: evaluation order (lexical left-to-right) · container length change during iteration ⇒ `R1003` · strict unpacking · strict dict keys · `sorted` by value + stable · closure/generator capture rules. **No silent corners** (loud beats silent). |
| ✅ **First-class aarch64 (M159)** | `selfhost/native_bootstrap.sh` (self-bootstrap from **gcc only**, with self-proof) · host-arch-aware `tools/px` · new **`native-arm64` real-machine CI job** · release asset `puxian-bootstrap-aarch64-<tag>.tar.gz`. |
| ✅ **Fully self-hosted toolchain** | `build / run / lex / parse / fmt / lint / doc / test / bench / lsp / mcp / refs` are all implemented in PuXian (spec §12). |
| ✅ **CI integrated** | GitHub Actions: regression + bootstrap proof (C rail and BC rail) + example builds + **four-arch matrix** (x86_64 native / aarch64 / armv7 / riscv64) on every commit. |
| ✅ **Dogfooding done (M-B9b)** | First production application written in PuXian (HTTP + SQLite service), maintained in a separate private repository; infrastructure such as the mirror site and release chain is served by PuXian too. |

> Clone the repo and you can compile/run PuXian programs with `tools/px` right away — **no Rust installation required**.

---

## Quick Start

### Prerequisites (the only dependency: gcc + make)

```bash
# Linux (x86_64 / aarch64), make sure gcc is available
which gcc
# The repo ships its own bootstrap toolchain (bootstrap/pxc compiler + runtime C runtime) — clone and use
```

### Hello World

```python
# hello.px
def main():
    let msg = "hello, PuXian\n"
    print(msg |> to_upper())
```

```bash
./tools/px run hello.px              # script mode: interpreted, starts instantly
./tools/px build hello.px            # build mode: VM bytecode rail by default → static binary (--c for the C-text rail)
./hello/build/hello                   # run directly, zero dependencies (output in <dir>/build/)
```

### CLI Overview (`tools/px`)

| Command | Description |
|---|---|
| `px build <file.px>` | Compile to a static binary (outputs `<dir>/build/<name>`). **Since M91 the default artifact is the VM bytecode rail** (compiler_vm → BCModule image → linked with vm.o; `--c` / `PX_BUILD_ENGINE=c` is the escape hatch to the classic C-text rail, useful for pure compute hot spots). **Auto-pruned by referenced natives by default (M86-S2)**: unreferenced runtime modules are dropped at compile time (import-recursive; hello/pure CLI → ~2.7M, sqlite users keep it → ~3.8M; on parse failure falls back to full build) |
| `px build --full <file.px>` / `--max` | Full-capability build (M86-S2 escape hatch): skips auto-pruning, ≈9.0M baseline; combinable with explicit `--no-xxx` |
| `px build --min <file.px>` | Explicit minimal profile (M85): aggregates `--no-quic` + sqlite/ws/zip/xml/aes/rsa/ed25519/img/route/zlib/h2 → ≈2.7M; `--no-xxx` flags combine freely (explicit flag > auto; missing native → R1001) |
| `px build --target <arch>` | **High-level cross switch (M71-S2)**: `x86_64` / `aarch64` / `armv7` / `riscv64` (also `os-arch`); folds `--cc` plus target mbedtls/sqlite/zlib paths into one flag |
| `px build --lto <file.px>` | **LTO build profile (M104-S5)**: full-chain gcc `-flto` |
| `px build --c <file.px>` | Explicit **C-text rail** (`fn_*` → gcc): for pure-compute hot spots or when you want to read the generated C |
| `px build --print-plan <file.px>` | **Print the build plan (M159)**: engine rail / reference pruning / linked modules / CC / target arch — diagnose instead of guess |
| `px refs <file.px>` | Print referenced global/native names (M86-S1: extracted from the compiled C output, import-recursive; the reference collector behind `px build` auto-pruning) |
| `px run <file.px> [args...]` | Run in script mode |
| `px lex <file.px>` | Print the token stream (debugging; runs the PuXian lexer) |
| `px parse <file.px>` | Print the AST (debugging; runs the PuXian parser) |
| `px fmt <file.px> [-w] [--check] [--diff]` | Deterministic code formatting (self-hosted, M64a) |
| `px lint <file.px> [--json] [--strict]` | Static checks L001-L008 (self-hosted, M64b) |
| `px doc <file.px> [--output out.md]` | Generate Markdown docs from `##` comments (self-hosted, M64c) |
| `px test <file.px> [filter] [--list]` | Run top-level `def test_xxx()` tests (self-hosted, M64c) |
| `px bench <file.px> <func> [--count N] [--repeat R]` | Benchmarks (self-hosted, M64c) |
| `px lsp` | **LSP server** (self-hosted, M65): diagnostics / completion / definition / hover over stdio |
| `px mcp` | **MCP server** (self-hosted, M65): AI agent calls 8 tools (run/fmt/lint/test/bench/doc/ast/version) |
| `px --version` / `-v` | Print the version number |
| `px help` | Show help |

> **Current toolchain status (M64/M65 fully self-hosted)**: all 8 spec §12 tools
> `pkg / ast / fmt / lint / test / bench / doc / lsp / mcp` are implemented in PuXian
> itself (`.px` source → bootstrap binary → `px` subcommand). `pxc` remains as a
> compatibility alias (M86-S0). The Rust version is
> archived read-only in `archive/rust-compiler/`.

---

## Multi-Arch Cross-Compilation (M67: ARM64 / ARMv7 / RISC-V)

PuXian builds are **zero-dependency static single binaries**, a natural fit for Raspberry Pi /
gateways / edge boxes on Linux: cross-compile on an x86 dev machine and copy the binary to the
device — no runtime or dependencies to install. Officially supported: **x86_64 (native) +
aarch64 + armv7(armhf) + riscv64** (GC conservative scanning is arch-abstracted, see spec §8.21).

### 1. Get the musl cross toolchain (two ways per arch)

```bash
# Option A (recommended): official musl.cc tarball (no root needed)
#   aarch64 → aarch64-linux-musl-cross.tgz       ARM64 devices (RPi 4/5, gateways)
#   armv7   → armv7l-linux-musleabihf-cross.tgz  32-bit ARM (RPi 2/3, old industrial boxes)
#   riscv64 → riscv64-linux-musl-cross.tgz       RISC-V (VisionFive2, xiangshan/xuantie)
curl -LO https://musl.cc/aarch64-linux-musl-cross.tgz
tar xzf aarch64-linux-musl-cross.tgz
export PATH=$PWD/aarch64-linux-musl-cross/bin:$PATH

# Option B: docker (messense/musl-cross; images aarch64 / armv7l / riscv64)
docker run --rm -v $PWD:/src -w /src messense/musl-cross:aarch64 \
  sh -c 'aarch64-linux-musl-gcc --version'
```

> **Why not apt's `gcc-aarch64-linux-gnu`?** It is a glibc cross compiler: cross headers are
> missing (you would need `libc6-dev-arm64-cross`) and mixing with the repo's prebuilt **musl**
> static libs is an ABI risk. The official line only backs the musl path (musl.cc / docker).

### 2. Cross-compile with one command

```bash
# aarch64 cross static libs ship with the repo (runtime/mbedtls/lib-aarch64 +
# sqlite3-aarch64.o + zlib lib-aarch64) — grab a cross CC and build, no need to rebuild libs
./tools/px build --no-quic --cc aarch64-linux-musl-gcc \
  --mbedtls-lib runtime/mbedtls/lib-aarch64 \
  --sqlite-obj runtime/third_party/sqlite3/sqlite3-aarch64.o \
  your_app.px
# Output: your_app/build/your_app — ELF ARM aarch64 static single binary
# armv7 / riscv64: build the target libs first with tools/cross_multiarch.sh (same as CI):
#   tools/cross_multiarch.sh --arch armv7   --outdir /opt/px-multiarch/armv7
#   tools/cross_multiarch.sh --arch riscv64 --outdir /opt/px-multiarch/riscv64
# then pass --mbedtls-lib /opt/px-multiarch/<a>/mbedtls/lib-<a> --sqlite-obj .../sqlite3-<a>.o
# Note: riscv64 gets -no-pie automatically (musl static-pie RISC-V read-only-segment limit)
```

### 3. Verify & run (file + qemu)

```bash
file your_app/build/your_app     # ARM aarch64 (aarch64) / ARM EABI5 32-bit (armv7) / RISC-V (riscv64)
qemu-aarch64-static your_app/build/your_app    # no hardware? qemu-user runs it directly
qemu-arm-static ... / qemu-riscv64-static ...  # same for armv7 / riscv64
```

### 4. One-shot verification (same as CI 4-arch matrix)

```bash
# aarch64 first-class (3 use-cases; needs cross CC + qemu-aarch64-static)
bash examples/m67_aarch64/verify.sh
# 4-arch matrix: x86_64 native (incl. gc_stress concurrent GC stress) + aarch64/armv7/riscv64 qemu
bash examples/m67_multiarch/verify.sh                # all four
bash examples/m67_multiarch/verify.sh --arch aarch64 # single arch
```

> **Notes**: cross builds default to `--no-quic` (ngtcp2/openssl-quictls only have x86_64 prebuilt
> libs; edge scenarios don't need H3 and trimming is semantics-neutral). aarch64 libs ship in the
> repo; armv7/riscv64 libs are built by cross_multiarch.sh (script included in the release).
> musl emits static-pie by default — that is normal and runs under qemu. qemu-user is too slow for
> concurrent-GC stress → new-arch GC is verified by arch probe + native concurrency + real device
> (spec §8.21).

### 5. aarch64 official channel: bootstrap from source (M159)

```bash
# Needs gcc only — no prebuilt bootstrap binaries required (native target, or --cc for cross)
selfhost/native_bootstrap.sh
selfhost/native_bootstrap.sh --cc aarch64-linux-musl-gcc --target aarch64
# Self-proof: the freshly built pxc compiles selfhost/compiler.px, byte-identical to selfhost/golden/compiler.c
```

- `tools/px` **detects the host architecture automatically** (picks `lib-<arch>`, defaults to the C rail off x86_64, adds `--no-quic`); `--print-plan` prints the build plan (engine rail / pruning / CC / target) for diagnosis;
- CI gained a **`native-arm64` real-machine job**: self-bootstrap + self-proof + build + smoke + packaging on real aarch64;
- Since M159 releases ship a **`puxian-bootstrap-aarch64-<tag>.tar.gz`** companion asset (no preinstalled toolchain needed on the ARM box).

---

## Features at a Glance

| Dimension | Capabilities |
|---|---|
| 🏃 Three rails | Interpreter rail (`px run`, instant start) / **VM bytecode rail (default for `px build`)** / C-text rail (`px build --c`, pure-compute hot spots) — **the same source behaves identically on all three**; divergence is treated as a bug, not a "backend difference" |
| 🔀 Concurrency | `spawn` true concurrency, `channel` blocking communication, `select` random readiness + **concurrent GC** (stop-the-world full collection, thread-safe). |
| ⏱ Timers | `set_timeout` / `set_interval` / `clear_timer` (one-shot/periodic callbacks, variadic argument pass-through, concurrency primitives are safe inside callbacks). |
| 🧹 Memory | Build mode: C runtime with a conservative mark-and-sweep GC (cyclic references collectable, auto-triggered) + **slab allocator** (21 size-class slot reuse); interpreter side: **tracing GC** that collects cycles (list/dict/chan/**closure Func↔Env cycles**) + `gc()` forced collection. |
| 🧩 Modularity | `import std.*` / `import foo.bar` / `from foo import x` / relative-path imports. |
| 🌐 Networking | HTTP client (**HTTPS TLS 1.2/1.3** + gzip/chunked auto-decoding + **http/https connection-pool reuse** + **TLS session-ticket resumption** + **streaming gzip decode-as-you-download** + **Unix-socket HTTP client** (`http_unix(sock,path,method,...)` for local services / LLM gateways, auto Content-Length)) + **HTTP server** (`http_serve` with gzip/chunked/keep-alive/streaming + **`px_serve` server-side TLS**: `tls_server(cert,key[,hostname])` enables HTTPS/WSS/SSE-over-TLS + **TLS SNI multi-certificate selection by domain** + configurable request-body limits + 413 + large-body spooling to disk + **graceful shutdown** + **per-route rate limiting** (429 at route granularity) + **access-log file rotation** + **Alt-Svc advertisement** + **HTTP/3 three-stack unification** (`px_serve(...,{http3:true|{port?,cert?,key?}})` hosts H3/QUIC on the same port — HTTP/1.1+HTTP/2+HTTP/3 share the same vhost/route/rate-limit/access-log/static/.px pipeline; `h3_server_listen` standalone H3 listener; **aioquic third-party interop**)) + **WebSocket** (RFC 6455, heartbeat/timeouts, **one-line `ws://`/`wss://` connection**) + **SSE** server/client (**auto-reconnect on disconnect**, with Last-Event-ID) + **UDP** (udp_open/send/recv/close) + full-featured TCP. |
| 🛡 Crypto/Docs | **AES-CBC-PKCS7 / AES-GCM**, **RSA** (PKCS#1 v1.5), **XML** parse/escape/**generate** (xml_build), **zip** pack/unpack, **base64**, sha256 / xxhash, **SQLite** (open/exec/query/close, parameter binding + result sets). |
| 🔢 Language | Slice syntax `a[i:j]` / `a[i:j:k]` (stride/reverse, strings sliced by UTF-8 chars), **generator expressions** `(x for x in xs)` (**lazy**: single-level for delayed evaluation / `gen_next` item-by-item / for-in / `list()` conversion), bitwise ops + binary-data views (int_to_hex / bytes_to_hex / bit_count / bit_length), regex, lock primitives (mutex / rwlock), random file I/O + fsync, process/signal (os_spawn / os_wait / signal), **Result/Option error handling** (`Ok(x)`/`Err(e)`/`Some(x)` constructors, `?` error propagation — Err/None returns immediately, `!` forced unwrap, is_ok/is_err/unwrap methods; the single error channel in the spec), string interpolation `${expr}`, comprehensions, optional chaining `?.`, null coalescing `??`, pipeline `\|>`. |
| 🔌 Edge device | fd primitives `open`/`close`/`ioctl`/`os_errno` (ioctl arg three forms: int direct / bytes·str in-place in/out buffer, `_IOR` filled in place) + fd data path `read`/`write` (raw read(2)/write(2)) + **mmap live mapping** `mmap`/`munmap`/`mem_write` (MAP_SHARED framebuffer/shmem/DMA direct access, GC auto-munmap, in-place write into the mapping) + GPIO/I2C device examples + **aarch64 cross-compile** (`px build --no-quic` trimming + qemu-aarch64 verification identical to x86) — Linux edge devices (Raspberry Pi/gateway/box) as a single static binary, no runtime env needed |
| 🚀 Application platform | **`.px` script execution mechanism** (`px_serve`, a PHP/OpenResty-style application server: Cookie/Session/basic auth + server-side TLS + graceful shutdown; `px_exec`, a language-level embedding API) + **`.px` process pool** (build mode pre-forks worker interpreters that stay resident and are reused, PHP-FPM style; **hot-reload with automatic rolling restart on script/binary changes**) + route table & middleware (method+path patterns / `:id` params / `*` wildcards / middleware chains) + cron scheduling (6 fields) + JSON path (json_path / json_path_set). |
| 📚 Standard library | **13 public libraries** under `stdlib/` (collections / semver / webroute / yaml / pxml / lunar / gfx / png / edge / cookiejar / html / multipart / smtp; plus L1 helper modules such as strings / path / url / io / time_go / jsonx / go_json* / yaml* — **27 `.px` files** in total). `import std.<name>` and go; identical on both rails. API listing in [`docs/ECOSYSTEM.md`](docs/ECOSYSTEM.md) and the machine index `docs/ecosystem_index.json` |
| 🧭 Semantic consistency (M159–M169) | **No silent corners** (loud beats silent): evaluation order is always **lexical left-to-right** (including call arguments, not gcc's choice) · mutating a container while iterating it ⇒ `R1003` (**length snapshot** on loop entry) · strict unpacking (`for a, b in xs`, shape mismatch ⇒ `R1002`) · strict dict keys (non-string key in a constructor ⇒ `R1002`, no more silent data loss) · `sorted` uses **value comparison + stable sort** · closures capture by reference, generator captures are a **value snapshot** · **module-body bindings are module-level globals** (nested blocks included) and a closure body is a frame, just like a function. Every rule ships a gate that pins all three rails byte-for-byte |

---

## Bootstrapping

What makes PuXian most distinctive: **its compiler is written in itself**. A full bootstrap cycle was completed in 2026:

### Bootstrap Chain

```
selfhost/*.px (PuXian sources) ──compile──► bootstrap/pxc · pxc_vm (compiler binaries, checked in)
                                             │ compiles any .px
                                             ▼
                                       C source + runtime/ ──gcc──► static binary
```

| Component | Description |
|---|---|
| `bootstrap/pxc` / `pxc_vm` | The PuXian-written compilers (**C-text rail** / **VM bytecode rail**), static ELF, built from `selfhost/compiler.px`, **committed with the repo** |
| `bootstrap/pxi` / `pxi_vm` | The PuXian-written interpreters (source rail / bytecode rail) |
| `bootstrap/pxl` / `pxpar` | The PuXian-written lexer / parser (for debugging) |
| `bootstrap/pxfmt` … `pxmcp` | Self-hosted toolchain binaries (fmt / lint / check / doc / test / bench / lsp / mcp) — **14 checked-in binaries** in total |
| `selfhost/*.px` | **Compiler / interpreter sources (written in PuXian itself)**: 23 `.px` files — `compiler.px` (CLI) → `codegen.px` + `cg_*.px` (AST → C) and `bc_emit.px` (AST → bytecode) + `interp.px` + `i*.px` (tree-walking interpreter) |
| `selfhost/golden/compiler.c` | **C-rail golden** (**17,535 lines** of C): the one-shot artifact produced when the bootstrap compiler compiled itself |
| `selfhost/golden/compiler.bc.dump` | **BC-rail golden** (**37,296 lines** bytecode image): byte-for-byte replay target for the VM rail |
| `selfhost/bootstrap_prove.sh` | C-rail bootstrap proof: `bootstrap/pxc` compiles `compiler.px`, diffed byte-for-byte against the golden |
| `selfhost/bootstrap_prove_bc.sh` | **BC-rail bootstrap proof**: the compiler compiles itself → bytecode image diffed byte-for-byte against the golden |
| `selfhost/native_bootstrap.sh` | **aarch64 official channel (M159)**: native/cross self-bootstrap needing gcc only, **with self-proof** (the freshly built pxc compiles `compiler.px` again, byte-identical to the golden) |
| `selfhost/rebake_bin.sh` | Rebake of checked-in binaries + **source-chain fingerprint gate** (`--check-all` compares all 14 binaries against the *current* sources, `PXSRC-…` / `PXRT-…`) |
| `selfhost/m116_gates.sh` | **Full gate suite** (semantic gates + emission freeze + rebake trio + six-way diffcheck): run at the end of every milestone |

### The Classic Bootstrap Proof (plus the BC rail)

1. Compiler A (`bootstrap/pxc`) runs `build compiler.px` → produces B.c;
2. `B.c` is **byte-for-byte identical (17,535 lines, 0 differences)** to the golden `golden/compiler.c` → the C rail bootstraps;
3. Hardened loop: B.c is compiled by gcc into binary B → B compiles `compiler.px` again → B2.c, and **A.c == B.c == B2.c are all identical**;
4. **BC rail in parallel**: `bootstrap_prove_bc.sh` diffs the **bytecode image** the compiler produces for itself against `golden/compiler.bc.dump` (37,296 lines, 0 differences).

CI runs both proofs plus the **rebake fingerprint gate** on every commit (`.github/workflows/ci.yml`); on real aarch64 hardware the new **`native-arm64` job** runs `native_bootstrap.sh` (self-bootstrap + self-proof + build + smoke + packaging).

### Mini Subset (Language-Surface Lockdown)

During bootstrapping the language was locked to the **Mini subset** (`docs/MINI_SUBSET.md`, syntax baseline M40, a minimal Turing-complete surface): only bug fixes allowed, no new features. The PuXian-written compiler only needs to correctly compile this subset (its own sources already live inside it).

> **Known limitations**: in build mode `str(float)` truncates large-float %g precision (beyond 6 significant digits); the built version cannot parse source strings containing NUL; building the compiler itself takes ~3.5 min / 1.6 GB (the C runtime interprets the PuXian compiler logic). **pxi interpreter native reachability is aligned with build mode (M68 root fix, 2026-09)**: `ffi_call` dual-table lookup (ffi registry + global PX_NATIVE) plus interpreter auto-fallback — bare scripts with zero `extern def` can call all runtime builtins (`sqlite_*`/`http_serve`/`aes_*`/`os_pid`/…). See MINI_SUBSET §8–§13.

---

## Directory Layout

```
├── bootstrap/              # 14 checked-in static ELF binaries (pxc/pxc_vm compilers · pxi/pxi_vm interpreters · pxl/pxpar debug · pxfmt/pxlint/pxcheck/pxdoc/pxtest/pxbench/pxlsp/pxmcp toolchain)
├── tools/px               # User entry point (bash wrapper, zero Rust): build / run / lex / parse / fmt / lint / doc / test / bench / lsp / mcp / refs
├── selfhost/               # The bootstrapping project (the core!) — 23 .px files
│   ├── compiler.px         #   Full PuXian-written compiler CLI
│   ├── codegen.px + cg_*.px #   C-text rail emission (AST → C)
│   ├── bc_emit.px          #   VM bytecode rail emission (AST → bytecode)
│   ├── interp.px + i*.px   #   interpreter modules (tree-walking)
│   ├── lexer.px pxlexer.px parser.px  # lexer / parser
│   ├── value.px env.px module.px  # value system / scoping / module loading
│   ├── capability.px       #   capability self-check
│   ├── cases/ + golden/    #   differential cases and golden artifacts (compiler.c 17,535 lines · compiler.bc.dump 37,296 lines)
│   ├── cases_bad/          #   error cases
│   ├── diffcheck.sh · engine_parity.sh · emitc_freeze.sh   # six-way differential / three-rail parity / emission freeze gate
│   ├── bootstrap_prove.sh · bootstrap_prove_bc.sh          # C-rail / BC-rail bootstrap proofs
│   ├── native_bootstrap.sh #   gcc-only native/cross self-bootstrap + self-proof (aarch64)
│   ├── rebake_bin.sh       #   rebake checked-in binaries + source-chain fingerprint gate (--check-all / --check / --check-vm)
│   └── m116_gates.sh       #   full gate suite (semantics + freeze + rebake + diffcheck)
├── runtime/                # C runtime (runtime.c/h + aes/xml/zip/ws/rsa/ed25519/sqlite/route/h2/h3/quic/image/onnx + mbedtls + third_party), 367 natives
├── stdlib/                 # Standard library (27 .px files: 13 public libs — collections/cookiejar/edge/gfx/html/lunar/multipart/png/pxml/semver/smtp/webroute/yaml — plus L1 helpers strings/path/url/io/time_go/jsonx/…)
├── registry/               # Versioned library distribution (registry/<name>/<version>/<name>.px, 13 libs)
├── examples/               # 131 example directories / 120 single-file .px (hello / fib / match / concurrency / networking / TLS / SQLite / HTTP3 / edge / semantic gates ...)
├── packaging/              # Distribution packaging (rpm/dnf repos · make_release · tag_guard)
├── archive/rust-compiler/  # Rust compiler source archive (read-only; the pre-bootstrap implementation; git history preserved)
├── docs/                   # Documentation (index docs/README.md · spec · AI cheatsheet · ecosystem · roadmap ...)
└── .github/workflows/      # CI: regression + bootstrap proofs (C/BC) + examples + four-arch matrix + native aarch64 bootstrap + release + Tag Guard
```

---

## Documentation

| Document | Description |
|---|---|
| [docs/README.md](docs/README.md) | **Documentation index** (which doc covers what, reading paths by role, which files are historical archives) |
| [docs/spec.md](docs/spec.md) | **Language specification** (lexical / types / expressions / statements / concurrency / modules / **semantic consistency §17** / error codes §11 / toolchain §12) |
| [docs/PUXIAN_CHEATSHEET.md](docs/PUXIAN_CHEATSHEET.md) | **AI cheatsheet** (feed the whole file to an LLM and it writes correct `.px`: pitfalls · native roster · three-rail differences and unified rules) |
| [docs/ECOSYSTEM.md](docs/ECOSYSTEM.md) | Ecosystem overview (13 libraries, exported APIs / dogfood capability map / consumption paths / drift-proof machine index) |
| [docs/ECOSYSTEM_GAPS.md](docs/ECOSYSTEM_GAPS.md) | Library-authoring checklist + language-gap assessments (historical record) |
| [docs/DICT_STRICT_MIGRATION.md](docs/DICT_STRICT_MIGRATION.md) | **Strictness migration guide** (M163–M167: strict keys, iteration snapshot, unpacking shape, `items()`) |
| [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md) | **Mini subset spec** (the locked language surface of the self-hosted compiler: supported features / explicitly excluded / known limitations) |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Roadmap (completed milestones + future directions + language debt) |
| [docs/RELEASE_PROCESS.md](docs/RELEASE_PROCESS.md) | Release SOP (tag-driven automation / artifacts / missing-tag guard) |
| [docs/PXML.md](docs/PXML.md) | PXML configuration-language spec (behind `std.pxml`) |
| [docs/GAP_ANALYSIS.md](docs/GAP_ANALYSIS.md) | Capability gap analysis (edge devices / 2D-3D lines) |
| [CHANGELOG.md](CHANGELOG.md) | Changelog (notable changes per milestone, with defect numbers and verification evidence) |
| [CONTRIBUTING.md](CONTRIBUTING.md) | Contribution guide (build / test / PR guidelines) |
| [SECURITY.md](SECURITY.md) | Security vulnerability reporting policy |

---

## Milestone Progress

### Feature Development (M0–M40, the Rust era — all completed ✅)

| Stage | Highlights |
|---|---|
| M0–M9 | Requirements/design/spec → lexer+parser → interpreter → concurrency runtime → C codegen → standard library → AI toolchain (fmt/lint/test/bench/doc/ast) → LSP/MCP → GC value objects → package management/modularity |
| M10–M19 | HTTPS (TLS 1.2/1.3) → concurrent GC → random file I/O + fsync → lock primitives → sha256/xxhash → regex → HTTP server framework → `.px` script execution mechanism → timers → AES/XML/zip |
| M20–M29 | C runtime symbol unification → chunked/gzip/slicing/base64/SSE → slab allocator + tracing GC + WebSocket + bitwise ops → networking/storage/security wrap-up → XML generation + slice strides + connection pool → closure-cycle collection + process pool + TLS ticket resumption → `>>>` + WS heartbeat + remote registry → server-side TLS + request-body limits + Session → routing/timezone/cron/SQLite → JSON path + static cache headers + Range + access logs + request ID |
| M30–M40 | Server-side HTTPS connection pool + byte-order int↔bytes + comprehension completion + configurable fmt → sandbox + virtual hosts + rate limiting + HTTP/2 preflight + connection thread pool → one-line ws/wss + SSE reconnect + process-pool hot reload + generators → per-route rate limiting + TLS SNI + access-log rotation + QUIC research → lazy generators + configurable process pool + WS broadcast + event bus → gzip decompression + range comprehensions + minimal HTTP/2 server + multi-dimensional rate limiting → log enhancements + request context + WS heartbeat config + graceful shutdown → HTTP/2 over TLS + response compression + S3 → WS auto-reconnect + HTTP/2 multi-stream + UDP echo → **Result/Option as the single error channel** → **string interpolation `${expr}`** |

### Bootstrapping (M-B1 → M-B9b — all ✅)

| Milestone | Scope | Result |
|---|---|---|
| M-B1 | Capability gate + Mini subset + differential harness | capability self-check 110/110 |
| M-B2 | lexer rewritten in PuXian | token-stream differential 9/9 |
| M-B3 | parser rewritten in PuXian | AST differential 8/8 (dual mode) |
| M-B4 | parser error handling/recovery | error cases 23/23 (dual mode) |
| M-B5 | value/env/module value system | differential v01–v03 pass in both modes |
| M-B6 | codegen rewritten in PuXian | C-source differential 12/12 (dual mode) |
| M-B7 | interp rewritten in PuXian | stdout 8/8 + v01–v03 all PASS |
| M-B8 | **Bootstrap proof** | **A.c == B.c == B2.c byte-for-byte identical** 🎉 |
| M-B9a | Retire the Rust version + wire up CI + bootstrap chain | `tools/px` fully usable end-to-end; CI four jobs |
| M-B9b | First production application (dogfooding validation) | ✅ moved to a separate private repo |

### Native Development (M41–M67, post-bootstrap development in PuXian itself — all ✅)

| Milestone | Scope |
|---|---|
| M41 | Type-system debt cleared: edition / immutability / null-safety / definition-level generics |
| M42 | Explicit C library import (FFI platform leverage): `import "c/xxx"` + `extern def` |
| M43 | File-as-route (routegen generates route registration at build time, PHP-style framework form) |
| M44 | Language sugar: simplified enums (`type X const`) + list-append shorthand (`<-`) |
| M45 | Registry versioning: semver library + pxpkg + px.pkg.lock reproducible builds |
| M46–M52 | HTTP/3/QUIC full chain: QUIC transport → H3 semantic layer → QPACK (Huffman/static table/dynamic table/SETTINGS/multiplexing/decoder-stream ack) |
| M53 | **HTTP/3 three-stack WebServer**: px_serve http3 (HTTP/1.1+HTTP/2+HTTP/3 share the public pipeline) + Alt-Svc + **aioquic third-party interop** |
| M54 | **HTTP/3 productionization**: TLS 1.3 session resumption (1-RTT) + **0-RTT early data** (send before handshake) + connection migration (source-change resume) + BLOCKED_STREAMS flow-control negotiation (-206 / MAX_STREAMS) |
| M57 | **Edge-device-layer support (Linux userspace)**: fd primitives `open`/`close`/`ioctl`/`os_errno` (ioctl arg three forms: int direct / bytes·str in-place buffer) + `read`/`write` data path + **mmap/munmap live mapping** (MAP_SHARED, GC auto-munmap) + GPIO/I2C examples + **aarch64 cross-compile + qemu verify + `--no-quic` trimming** |
| M58 | **First dogfood real app "pxhwmond"** (hardware health monitor daemon, examples/m58_hwmond): multi-file import project + M57 fd-path /proc collection (CPU/mem/load/uptime/net + temp conditional degrade) + **mmap MAP_SHARED snapshot IPC** (external `--dump` live-read / ctl-channel bidirectional) + hand-written minimal HTTP status page (/healthz JSON + / HTML + 404, explicit headers) + run.sh crash self-heal wrapper + threshold alerts (log + webhook dry-run) + aarch64 cross-compile & qemu `--once` verify |
| M59 | Math & random filling: C libm 14 built-ins + 2 constants (sin/cos/tan/atan2/floor/ceil/round/log/log10/exp/random family/pi/e, splitmix64 cross-platform reproducible) |
| M60 | Edge-device deepening: `std.edge` stdlib (GPIO V2/I2C/serial/PWM, pure language) + sleep_us/now_us/fcntl/tty_config/fd_wait built-ins + PTY real-kernel loopback |
| M61 | External-library FFI proof (zlib) + pure-language 2D inner circle: `std.gfx`/`std.png` (Mandelbrot / snake demo, FFI compression pipeline) |
| M62 | Language-debt fixes L1–L7: float print `.0` alignment + codegen block-scope hoist + split empty segments + pxi bytes-family whitelist |
| M63 | Language-debt fixes L8–L11: pxi network API whitelist + float full-precision roundtrip + px --version |
| M64 | **Toolchain self-hosting restored**: `px fmt / lint / doc / test / bench` five tools self-hosted (keep-lexer base + whole-repo convergence, net -318 lines) |
| M65 | **LSP / MCP self-hosted**: `px lsp` (diagnostics/completion/definition/hover) + `px mcp` (8 tools for AI agents) — **spec §12 toolchain fully self-hosted** |
| M66 | **wsAgent runtime primitives + stdlib adoption** (qg-issue 01–06): unix_connect + os 5-tuple + os_capture/os_popen/os_kill group + write_file mode + zip_unpack password (zipcrypto/AES-256) → std.yaml / std.pxml / std.lunar (lunar calendar), spec §8.20 |
| M67 | **Multi-arch first-class support** (qg-issue 07): cross-compile docs section + `runtime/arch.h` GC arch abstraction (x86_64/aarch64 extracted + new **armv7/riscv64** mcontext) + `cross_multiarch.sh` + CI **4-arch matrix** (x86_64 native GC stress + aarch64/armv7/riscv64 qemu 3 use-cases), spec §8.21 |
| M68 | **pxi consistency completion — interpreter native reachability root fix** (docs/M68_PLAN.md): build mode reaches all 281 `px_set_global` runtime natives; pxi only knew a 129-name whitelist (155-gap → R1001 on bare scripts) → root fix = C-side `ffi_call` dual-table fallback (`px_global_native()` global PX_NATIVE single source) + pxi `i_eval_call` auto-fallback + distinguishable Err on unknown name (typo still R1001) → **zero-`extern def` bare scripts: pxi ≡ build artifacts** (capability 253 byte-identical dual-mode, diffcheck --all green), spec §9.3 |
| M69 | **Ecosystem launch: assetization + AI cheat-sheet + registry fetch loop** (docs/M69_PLAN.md): `docs/ECOSYSTEM.md` (9-lib overview / 119-example capability map / consumption paths) + `tools/gen_ecosystem.px` machine index (CI drift guard); fixed stdlib collections.group_by historical bug (`{}` literal = null, no `d[k]=v`); `docs/PUXIAN_CHEATSHEET.md` + `tools/gen_native_table.sh` (281 natives, single source = runtime) — AI self-test 3/3; `registry/` 9 official libs shipped in-repo + pxpkg fetch→import dual-mode e2e (`examples/m69_registry/verify.sh`, 11 asserts) + spec §8.6.3; `docs/ECOSYSTEM_GAPS.md` (G1-G4 assessed, M70 candidates logged) |
| M70 | **Language-gap fixes: multiline expressions + module top-level state** (docs/M70_PLAN.md): parser bracket-context newline tolerance (`skip_expr_ws` — multiline list/dict/call args incl. trailing comma/tuple/index slices/comprehensions; `brace_looks_like_dict` multiline-dict detection fix; **lexer untouched → zero golden drift**; line breaks allowed only inside `[` `(` `{`, statement boundary still = newline, continuation indent must fit the indent stack) + fmt multiline zero-change verification (`verify_fmt_multiline.sh`) → cg_module **exports non-Const top-level VarDecls on import** (module-level state slots, initialized once at importer startup; same-name conflict → user value wins; module `let` still immutable E3002) → full-capability bootstrap rebuild + capability 253 byte-identical dual-mode + diffcheck --all/--errors + bootstrap proof → spec §4.1/§5.1/§8.4 + MINI_SUBSET/CHEATSHEET/ECOSYSTEM_GAPS updated → tag v0.1.0-m70 auto-release |
| M71 | **Build-pipeline modernization + AI delivery one-stop** (docs/M71_PLAN.md): `px build` precompiled-runtime `.o` incremental cache (rebuild quic 14.7s→**0.94s** / no-quic 11.6s→**0.41s**) → `px build --target <arch>` high-level cross switch (one command folds 5 flags) → **MCP 9th tool `build`** (write→verify→deliver closed loop) → Release ships `sha256sums.txt` + `tools/install.sh` one-shot install (argv0 self-discovery + PX_STDLIB auto-inject, usable from any cwd) → ECOSYSTEM_GAPS F4 correction (compiled px build is ms-fast on large files ≈ grep) |
| M72 | **AI debug loop + runtime bytes support** (docs/M72_PLAN.md, qg-issue 9/10/13-R1): `print`/`println` **flush per line** (no more 8KB buffering under pipes/journald; existing .px needs zero changes) + new natives `flush()`/`print_err()` → **compiled-binary runtime errors carry the .px source line** (`运行时错误 [函数 行N]: msg` — one-shot AI locating) → **spawn-coroutine runtime errors are isolated by default** (scene printed + host keeps running; `PX_SPAWN_ISOLATE=0` reverts to exit) → **bytes natives**: `aes_gcm_encrypt_bytes/decrypt_bytes` (no utf8/NUL truncation; byte-compatible with Go crypto/aes-gcm) + `http_request` length-aware body (binary ciphertext uploads byte-exact) → Issue 9/10 archived done, 13-R1 ✅ (R2 ws-backup-px → M73) |

### Platformization \& Ecosystem (M83–M158 — all ✅)

| Milestone | Scope |
|---|---|
| M83–M94 | **Coroutine/scheduler completeness** (frame-coroutine M:N kernel, blocking primitives yield, preemption, timers folded into the scheduler loop) + **VM bytecode rail build-out** (M89 design → **since M91 the default `px build` artifact is the VM bytecode rail**) |
| M95–M103 | **Full-chain coroutine-ization** of server and client paths (http_serve / sse_serve / route / vhost / middleware / connection-level IDLE / client network IO) + px_serve concurrent TLS-handshake fix + .px process-pool coroutine-ization + Issue 29/30 language gaps |
| M104–M111 | Runtime performance \& memory paths (LTO profile · global-table O(1) name resolution · stable pointer slots for VM globals · amortized O(1) string indexing · slab + page reclamation) + server connection lifecycle family (TLS handshake no longer stalls the port) + response-header deny-list + serve allocation 13.5× blow-up root fix |
| M112–M123 | **VM rail (the user-facing default) semantics/FFI closure** + gate exit codes + checked-in binary rebake in CI + built-in roster as the single source of truth + service-process/env primitives + all defects surfaced by real-module .px porting |
| M125–M137 | Runtime **memory-safety trio** (allocation failure demoted to request-level 5xx · errors inside signal handlers no longer kill the process · lock audit on isolated rollback) + **api-server / token-cache fully ported to PuXian** (language defects 15–102 all fixed) |
| M138–M148 | **Go-fidelity family**: regex · `encoding/json` Indent/Compact/HTMLEscape byte-for-byte · HTTP client failure-cause classification + IPv6 · large request bodies + chunked I/O · cyclic-value comparison/render/JSON · float64 bit patterns + NaN semantics · fixed-point decimal text + `-0.0` · `/` fully IEEE-754 + Go `%v` float text |
| M149–M158 | **Service-readiness family**: TCP with timeouts \& distinguishable failures · digest/key-derivation + TLS client family · `tls_upgrade` · two allocation-rate cuts (literal pooling · compiler hot paths) + `join` byte semantics · strings with embedded NUL · **zero-dependency ONNX** (parse layer → tensors + 64 ops + topological executor) · interpreter function values → runtime native bridge |

### Semantic Consistency Closure (M159–M169 — all ✅)

> Shared root cause: **wherever the host language left things open, or implementations differ, our three rails (interpreter / VM / C) were bound to diverge.**
> The answer is not "follow one of them" but **pick one predictable, diagnosable rule that all three rails can share** — which is what "beyond Go" means here in practice: **no silent corners**.

| Milestone | Topic | Rule after closure |
|---|---|---|
| M159 | **aarch64 official channel** | `native_bootstrap.sh` (gcc-only self-bootstrap **with self-proof**) · host-arch-aware `tools/px` · `native-arm64` real-machine CI job · release asset |
| M160 | Lexical closures / `def` inside functions | Closures capture **by reference** (shared cell), identical on all rails (defects 159/160) |
| M161 | Generator capture (GenExp) | Generator lambda captures are a **value snapshot** (evaluated at creation); distinct from closures but the rules are explicit (defects 163/164) |
| M162 | `sorted` | **Value comparison + stable sort** (defect 166: interpreter compared rendered strings · 167: select-sort instability on the C rail) |
| M163 | Strict dict keys | Non-string key in a constructor (literal / comprehension) ⇒ `R1002` (the VM/C rails used to **silently drop data**; defects 168/169/171) |
| M164 | Iteration position semantics split from user indexing | `d[int]` always ⇒ `R1002 字典索引键必须是字符串`; `for k in d` uses a dedicated iteration entry point (defects 170/174/175) |
| M165 | **Evaluation order** | Expressions / call arguments / assignments are uniformly **lexical left-to-right** (C leaves it unspecified ⇒ the C rail used to follow gcc's right-to-left; defects 178/179) |
| M166 | Mutating a container while iterating | **Length snapshot** on loop entry; a length change ⇒ `R1003 迭代期间被迭代容器长度变化: n0 → n1` (defect 176) |
| M167 | Unified unpacking | Statement form `for a, b in xs` + `dict.items()`; shape/length mismatch ⇒ `R1002` (defects 180/182) |
| M168 | **Distribution portability** | openEuler officially supported (`install-rpm.sh` uses an explicit mapping table instead of `$releasever`) · release assets are now **fully static** (`check_bin_portability.sh` ships its own negative controls and red-flags the old package) · three disciplines: failures must name the next command / dependencies must be decidable / a gate you cannot read is not a gate |
| M169 | **Binding attribution (module body vs frame)** | Every binding in the module body (**including nested blocks**) is a module-level global (no block scope at module level) · frames capture by reference · **a closure body is a frame too** (hoist + boxing) · inside a frame: declarations ⇒ frame-local, assignments ⇒ global write iff the name is a module binding (defects 181/183/184) |
| M171 | **Lint scope model aligned with the language (tooling = same truth)** | Frame-top declaration hoisting · assignment-style bindings declared in place (lvalues never raise L002) · inner frames see outer frames (`used` written back to the owner) · comprehension variables visible across the whole comprehension · import candidates include `.px_modules` · builtin registry gains a 4th source ("host-injected globals") (defect 116) |
| M172 | Unified **channel** for runtime diagnostics | Same channel (stderr) + same error code + same message body across all three rails for the "undefined variable" family; the location prefix keeps each rail's best information, gates do not diff whole lines (defect 186) |
| M174 | `d.get(k[, default])` truth table | The default covers **only "key absent"**; key present with a `null` value ⇒ returns `null` (= Go two-value semantics / Python `dict.get`, defect 194) |
| M175 | Two ledger items | `len(bytes)` = **byte count** (`bytes_len` is now an equivalent alias) · `os_popen` gains a stderr destination option (`inherit`/`pipe`/`null`; byte-identical old behaviour when no opts are passed; defects 153 / 14) |
| M176 | **Zero-downtime binary swap** | `SO_REUSEPORT` (`opts{"reuse_port": true}` / `PX_REUSE_PORT=1`) · the gate measures **0 refused out of 700 probes** during a swap · the kernel requires **both sides to opt in** ⇒ the old binary must ship the switch too (Chenxi P1-5) |
| M177 | **One builtin surface for both engines** | `dict()` / `unique()` / `flatten()` (interpreter-only) added to runtime · `min`/`max` with a single iterable argument take the element extreme (the C/VM rails used to **silently return the generator object**) · `px_as_list` = `px_len` + `px_iter_at` (same iteration semantics as `for x in xs`) · registry gate gains criterion ⑦ (audit finding of defect 165) |
| M180 | **HTTP/2 policy landed · HTTP/3 capability self-proven** | Policy = [docs/HTTP2_DECISION.md](docs/HTTP2_DECISION.md): **h2 is not supported** (doing it properly means rewriting the frame loop + flow control + pipeline integration — architectural; H3 already rides the same handler pipeline at lower cost) · h2c upgrades are **ignored** and served as HTTP/1.1 (previously a demo page = silently wrong content) · h2 preambles ⇒ `505` · ALPN advertises `http/1.1` only · the h2 demo frame layer moved behind `opts{"h2_demo": true}` (default off) · H3 = `opts{"http3": true}` (x86_64 links ngtcp2 by default; asking for H3 in a `--no-quic` build fails loudly) |
| M179 | **One truth for the operator family** | Arithmetic requires **numbers** (`str * int` = repetition) · `< <= > >=` require **two numbers or the same type** (cross-type ⇒ `R1002 无法比较: <ta> vs <tb>`) · bitwise ops and index positions require **int** · out-of-range `R1003: 索引越界: i (len=n)` · integer divide-by-zero `R1006`; `sorted`/`min`/`max` use the **total-order internal comparator** (cross-type ordered by type name). Before, `num_val` read the union's pointer bit pattern (UB) ⇒ the compiled rails produced **silent bad values** (defects 195/196) |
| M178 | **One definition of "iterable argument"** | `list` / `tuple` / generator / **string (by rune)** — shared by `join`/`sorted`/`reversed`/`contains` and M177's five builtins; **`reversed(str)` fixed: it used to reverse by *byte* ⇒ invalid UTF-8** (a bad value) and now reverses runes and returns str; rejection messages are identical across rails; defect 195 newly registered |

> Details and migration guidance: [docs/spec.md §17](docs/spec.md) and [docs/DICT_STRICT_MIGRATION.md](docs/DICT_STRICT_MIGRATION.md); every rule ships a three-rail byte-identical gate plus negative controls (`examples/m159_*` … `examples/m167_*`).

---

## Examples

The `examples/` directory (**131 example directories / 120 single-file `.px`**) for quick hands-on:

```bash
# Interpreted run
./tools/px run examples/fib.px
./tools/px run examples/match.px
./tools/px run examples/m39_result.px
./tools/px run examples/m40_str_interp.px

# Build to a static binary
./tools/px build examples/fib.px && ./examples/build/fib
./tools/px build examples/m28_time_sqlite.px && ./examples/build/m28_time_sqlite
```

- `hello.px` — Hello World (pipeline operator)
- `fib.px` — Fibonacci / `match.px` — pattern matching
- `concurrent.px` — concurrency (spawn / channel / select)
- `m39_result.px` — Result/Option error handling (`?` / `!`)
- `m40_str_interp.px` — string interpolation `${expr}`
- `m28_time_sqlite.px` — time/zones + SQLite (CRUD / parameter binding)
- `m28_route.px` — route table + middleware chain (:id params / * wildcards)
- `m29_webprod.px` — WebServer production hardening (static cache headers / Range / request ID)
- `m30_comp.px` — comprehension syntax completion (multi-for / multi-if / DictComp)
- `m32_gen.px` — generator expressions (lazy evaluation)
- `m37_s3.px` — S3/MinIO object storage (AWS SigV4)
- `m38_h2_multi.px` — HTTP/2 multi-stream
- `m39_gc.px` — GC demo / `m22_tracing_gc.px` — cyclic-reference collection
- `m28_cron.px` — cron scheduling / `m33_route_rate_limit.px` — rate limiting
- `m46_quic_verify.sh` — QUIC transport-level loopback (handshake + hello-quic-42 → echo)
- `m47_h3_verify.sh` — HTTP/3 semantic-layer loopback (QPACK codec + HEADERS/DATA frames + request/response, GET /hello → 200)
- `m48_qpack_verify.sh` — QPACK full-codec byte-exact self-check (RFC Huffman official vectors + static-table indexing + fault tolerance, build/interpret dual mode)
- `m49_qpack_dyn_verify.sh` — QPACK dynamic table + SETTINGS session byte-exact self-check (dynamic-table reuse / compression gain / SETTINGS roundtrip, dual mode)
- `m50_h3_mux_verify.sh` — HTTP/3 multiplexing loopback (3 bidirectional streams on one connection, concurrent request/response one-to-one without crosstalk, dual mode)
- `m51_h3_qpack_wire_verify.sh` — QPACK session over real wire loopback (both ends open control/encoder/decoder QUIC unidirectional streams + SETTINGS negotiation; dynamic-table instructions travel over real unidirectional streams: request 1 inserts, request 2 reuses with zero additions, dual mode)
- `m52_qpack_decack_verify.sh` — QPACK decoder-stream ack over the wire (RFC 9204 §4.4: receiver auto-sends Section Ack after decoding dynamic field sections, sender consumes peer decoder stream to advance Known Received Count, safe encoder-table eviction; bidirectional closed loop, dual mode)
- `m53_s3_pipe_verify.sh` — HTTP/3 wired into the public HTTP pipeline (same-process px_serve + h3_server_listen dual stack; 4 QUIC connections × 5 requests byte-consistent with the HTTP/1.1 pipeline output)
- `m53_s4_pxserve_h3_verify.sh` — **HTTP/3 three-stack end-to-end** (px_serve http3: HTTP/1.1 TCP + HTTP/3 UDP in one service; self-built client + **aioquic third-party interop** 200; automatic Alt-Svc; graceful shutdown on SIGTERM)
- `m53_s5_pxi_h3_smoke.px` — self-check that the rebuilt pxi interpreter exposes h3_server_listen (id>0 PASS)
- `m57_s1_ioctl_verify.sh` — edge-device fd primitives (open/close/ioctl/os_errno: TCP-fd FIONREAD/FIONBIO + real-device conditional probing, dual mode)
- `m57_s2_mmap_verify.sh` — mmap live mapping (MAP_SHARED bidirectional visibility / offset sub-view / munmap semantics / GC auto-munmap 300 rounds, dual mode)
- `m57_s3_verify.sh` — device examples + real-kernel stand-in (GPIO_GET_CHIPINFO / I2C_SLAVE examples; loopback ifreq SIOCGIFADDR/FLAGS/HWADDR + PTY TIOCGPTN real-kernel ioctl hard assertions, dual mode)
- `m57_s4_cross_verify.sh` — **aarch64 cross-compile + qemu verify** (arm64 static binary 2.5MB edge-device ioctl identical to x86)
- `m57_s5_pxi_smoke.px` — rebuilt pxi exposes the 10 M57 builtins (open/read/ioctl in-place fill/write/mmap live mapping; interpret/compile outputs identical)
- `m58_hwmond/` — **M58 dogfood real app: pxhwmond hardware health monitor daemon** (multi-file import project: main/collect/shm/serve/notify; verify_s1–s4.sh per-substep self-checks; usage/deploy see `m58_hwmond/README.md`)
- `m159_hostarch/` … `m167_unpack/` — **semantic-consistency gates** (host-arch adaptation · closures · generator capture · stable `sorted` · strict dict keys · iteration position semantics · evaluation order · mutation during iteration · unpacking): each gate = byte-identical three rails + independently red negative controls
- ... full list in `examples/`

---

## Ecosystem

- **13 public standard libraries** (pure `.px`, `import std.*`): collections · semver · webroute · yaml · pxml · lunar · gfx · png · edge · cookiejar · html · multipart · smtp（plus L1 helper modules — **27 `.px` files** under `stdlib/`).
- **Overview**: [`docs/ECOSYSTEM.md`](docs/ECOSYSTEM.md) — library positioning and exported APIs / capability map of examples / consumption paths (import · pxpkg · copy the source) / drift-proof machine indexes (`tools/gen_ecosystem.px`, `tools/gen_native_table.sh`).
- **AI cheatsheet**: [`docs/PUXIAN_CHEATSHEET.md`](docs/PUXIAN_CHEATSHEET.md) — feed it whole to an LLM; includes the **367-native roster** (`docs/native_index.json`) and the three-rail facts table.
- **Package management**: `tools/pxpkg` (init / add / install + reproducible `px.px.lock`); **13 official libraries** ship in-repo under `registry/<name>/<version>/<name>.px` (fetch → import loop works end-to-end).

---

## Ecosystem & Collaboration

- **Out-of-repo private app**: PuXian's first real production user (HTTP + SQLite service, dogfooding validation); code maintained in a separate private repository.
- **Feedback & contribution**: when you find a problem, please attach a minimal reproduction case (a single `.px` + expected/actual output) and file an issue (labels: `M-B9b`); PRs to improve the project are welcome.

---

## 💡 Credits

PuXian is developed by the **达者同游 (Dazhe Tongyou) team**, with design and implementation assisted by **wsAgent (文殊智能体, the Wenshu agent)**:

| 中文名 | English Name | Type | Role |
|---|---|---|---|
| 本源 | Benyuan | Human | Founder / Chief Architect (direction & architecture decisions, final acceptance) |
| 东月 | Dongyue | wsAgent (AI agent) | Developer (implementation & coding) |
| 清歌 | Qingge | wsAgent (AI agent) | Designer / Issue Discovery (design assistance & problem finding) |

> This language was developed with the assistance of wsAgent (文殊智能体) — a language built for efficient AI programming, written with AI participation, dogfooding as its own proof. See [`AUTHORS.md`](AUTHORS.md) for full team details.
