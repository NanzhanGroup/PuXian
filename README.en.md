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

## 🎉 Current Status: The Compiler Is Self-Hosted

| Status | Description |
|---|---|
| ✅ **Self-hosting complete (M-B8)** | **The PuXian compiler is written in PuXian itself**: the five core components — `lexer / parser / codegen / interp / value system` — have all been rewritten in `.px`. The bootstrap proof shows A.c == B.c == B2.c, byte-for-byte identical. |
| ✅ **Rust version retired (M-B9a)** | Rust sources archived under `archive/rust-compiler/` (read-only). The new toolchain **`tools/pxc` requires no Rust at all**, running on top of the bootstrap binary. |
| ✅ **CI integrated** | GitHub Actions: every commit automatically runs regression + bootstrap proof + example compilation |
| ✅ **Dogfooding done (M-B9b)** | Wrote the first production application in PuXian (HTTP + SQLite service); now maintained in a separate private repository. |

> Clone the repo and you can compile/run PuXian programs with `tools/pxc` right away — **no Rust installation required**.

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
./tools/pxc run hello.px              # script mode: interpreted, starts instantly
./tools/pxc build hello.px            # build mode: emits C → gcc static binary
./hello/build/hello                   # run directly, zero dependencies (output in <dir>/build/)
```

### CLI Overview (`tools/pxc`)

| Command | Description |
|---|---|
| `pxc build <file.px>` | Compile to a static binary (outputs `<dir>/build/<name>`) |
| `pxc run <file.px> [args...]` | Run in script mode |
| `pxc lex <file.px>` | Print the token stream (debugging; runs the PuXian lexer) |
| `pxc parse <file.px>` | Print the AST (debugging; runs the PuXian parser) |
| `pxc fmt <file.px> [-w] [--check] [--diff]` | Deterministic code formatting (self-hosted, M64a) |
| `pxc lint <file.px> [--json] [--strict]` | Static checks L001-L008 (self-hosted, M64b) |
| `pxc doc <file.px> [--output out.md]` | Generate Markdown docs from `##` comments (self-hosted, M64c) |
| `pxc test <file.px> [filter] [--list]` | Run top-level `def test_xxx()` tests (self-hosted, M64c) |
| `pxc bench <file.px> <func> [--count N] [--repeat R]` | Benchmarks (self-hosted, M64c) |
| `pxc lsp` | **LSP server** (self-hosted, M65): diagnostics / completion / definition / hover over stdio |
| `pxc mcp` | **MCP server** (self-hosted, M65): AI agent calls 8 tools (run/fmt/lint/test/bench/doc/ast/version) |
| `pxc --version` / `-v` | Print the version number |
| `pxc help` | Show help |

> **Current toolchain status (M64/M65 fully self-hosted)**: all 8 spec §12 tools
> `pkg / ast / fmt / lint / test / bench / doc / lsp / mcp` are implemented in PuXian
> itself (`.px` source → bootstrap binary → `pxc` subcommand). The Rust version is
> archived read-only in `archive/rust-compiler/`.

---

## Features at a Glance

| Dimension | Capabilities |
|---|---|
| 🏃 Dual modes | Script mode (interpreted, instant start) / build mode (emits C → gcc static binary, near-C performance). `run` and `build` output are byte-for-byte identical. |
| 🔀 Concurrency | `spawn` true concurrency, `channel` blocking communication, `select` random readiness + **concurrent GC** (stop-the-world full collection, thread-safe). |
| ⏱ Timers | `set_timeout` / `set_interval` / `clear_timer` (one-shot/periodic callbacks, variadic argument pass-through, concurrency primitives are safe inside callbacks). |
| 🧹 Memory | Build mode: C runtime with a conservative mark-and-sweep GC (cyclic references collectable, auto-triggered) + **slab allocator** (21 size-class slot reuse); interpreter side: **tracing GC** that collects cycles (list/dict/chan/**closure Func↔Env cycles**) + `gc()` forced collection. |
| 🧩 Modularity | `import std.*` / `import foo.bar` / `from foo import x` / relative-path imports. |
| 🌐 Networking | HTTP client (**HTTPS TLS 1.2/1.3** + gzip/chunked auto-decoding + **http/https connection-pool reuse** + **TLS session-ticket resumption** + **streaming gzip decode-as-you-download** + **Unix-socket HTTP client** (`http_unix(sock,path,method,...)` for local services / LLM gateways, auto Content-Length)) + **HTTP server** (`http_serve` with gzip/chunked/keep-alive/streaming + **`px_serve` server-side TLS**: `tls_server(cert,key[,hostname])` enables HTTPS/WSS/SSE-over-TLS + **TLS SNI multi-certificate selection by domain** + configurable request-body limits + 413 + large-body spooling to disk + **graceful shutdown** + **per-route rate limiting** (429 at route granularity) + **access-log file rotation** + **Alt-Svc advertisement** + **HTTP/3 three-stack unification** (`px_serve(...,{http3:true|{port?,cert?,key?}})` hosts H3/QUIC on the same port — HTTP/1.1+HTTP/2+HTTP/3 share the same vhost/route/rate-limit/access-log/static/.px pipeline; `h3_server_listen` standalone H3 listener; **aioquic third-party interop**)) + **WebSocket** (RFC 6455, heartbeat/timeouts, **one-line `ws://`/`wss://` connection**) + **SSE** server/client (**auto-reconnect on disconnect**, with Last-Event-ID) + **UDP** (udp_open/send/recv/close) + full-featured TCP. |
| 🛡 Crypto/Docs | **AES-CBC-PKCS7 / AES-GCM**, **RSA** (PKCS#1 v1.5), **XML** parse/escape/**generate** (xml_build), **zip** pack/unpack, **base64**, sha256 / xxhash, **SQLite** (open/exec/query/close, parameter binding + result sets). |
| 🔢 Language | Slice syntax `a[i:j]` / `a[i:j:k]` (stride/reverse, strings sliced by UTF-8 chars), **generator expressions** `(x for x in xs)` (**lazy**: single-level for delayed evaluation / `gen_next` item-by-item / for-in / `list()` conversion), bitwise ops + binary-data views (int_to_hex / bytes_to_hex / bit_count / bit_length), regex, lock primitives (mutex / rwlock), random file I/O + fsync, process/signal (os_spawn / os_wait / signal), **Result/Option error handling** (`Ok(x)`/`Err(e)`/`Some(x)` constructors, `?` error propagation — Err/None returns immediately, `!` forced unwrap, is_ok/is_err/unwrap methods; the single error channel in the spec), string interpolation `${expr}`, comprehensions, optional chaining `?.`, null coalescing `??`, pipeline `\|>`. |
| 🔌 Edge device | fd primitives `open`/`close`/`ioctl`/`os_errno` (ioctl arg three forms: int direct / bytes·str in-place in/out buffer, `_IOR` filled in place) + fd data path `read`/`write` (raw read(2)/write(2)) + **mmap live mapping** `mmap`/`munmap`/`mem_write` (MAP_SHARED framebuffer/shmem/DMA direct access, GC auto-munmap, in-place write into the mapping) + GPIO/I2C device examples + **aarch64 cross-compile** (`pxc build --no-quic` trimming + qemu-aarch64 verification identical to x86) — Linux edge devices (Raspberry Pi/gateway/box) as a single static binary, no runtime env needed |
| 🚀 Application platform | **`.px` script execution mechanism** (`px_serve`, a PHP/OpenResty-style application server: Cookie/Session/basic auth + server-side TLS + graceful shutdown; `px_exec`, a language-level embedding API) + **`.px` process pool** (build mode pre-forks worker interpreters that stay resident and are reused, PHP-FPM style; **hot-reload with automatic rolling restart on script/binary changes**) + route table & middleware (method+path patterns / `:id` params / `*` wildcards / middleware chains) + cron scheduling (6 fields) + JSON path (json_path / json_path_set). |
| 📚 Standard library | `stdlib/collections.px` (sorted/reversed/map/filter/reduce/unique/group_by) + built-in registration whitelist (see MINI_SUBSET §2.5). |

---

## Bootstrapping

What makes PuXian most distinctive: **its compiler is written in itself**. A full bootstrap cycle was completed in 2026:

### Bootstrap Chain

```
selfhost/*.px (PuXian sources) ──compile──► bootstrap/pxc (compiler binary, checked in)
                                             │ compiles any .px
                                             ▼
                                       C source + runtime/ ──gcc──► static binary
```

| Component | Description |
|---|---|
| `bootstrap/pxc` | The PuXian-written compiler (static binary, built from `selfhost/compiler.px`, committed with the repo) |
| `bootstrap/pxi` | The PuXian-written interpreter (built from `selfhost/interp.px`) |
| `bootstrap/pxl` / `pxpar` | The PuXian-written lexer / parser (for debugging) |
| `selfhost/compiler.px` | **Compiler source (written in PuXian itself)**: imports the codegen.px chain (pxlexer → parser → cg_module → codegen) |
| `selfhost/golden/compiler.c` | Bootstrap golden file (6003 lines of C): the one-shot artifact produced when the bootstrap compiler compiled itself |
| `selfhost/bootstrap_prove.sh` | Bootstrap proof: `bootstrap/pxc` compiles `compiler.px` and diffs against the golden file byte-for-byte |

### The Classic Bootstrap Proof

1. Compiler A (`bootstrap/pxc`) runs `build compiler.px` → produces B.c;
2. `B.c` is **byte-for-byte identical (6002 lines, 0 differences)** to the golden `golden/compiler.c` → bootstrapping holds;
3. Hardened loop: B.c is compiled by gcc into binary B → B compiles `compiler.px` again → B2.c, and **A.c == B.c == B2.c are all identical**.

CI runs this proof automatically on every commit (`.github/workflows/ci.yml`).

### Mini Subset (Language-Surface Lockdown)

During bootstrapping the language was locked to the **Mini subset** (`docs/MINI_SUBSET.md`, syntax baseline M40, a minimal Turing-complete surface): only bug fixes allowed, no new features. The PuXian-written compiler only needs to correctly compile this subset (its own sources already live inside it).

> **Known limitations**: in build mode `str(float)` truncates large-float %g precision (beyond 6 significant digits); the built version cannot parse source strings containing NUL; building the compiler itself takes ~3.5 min / 1.6 GB (the C runtime interprets the PuXian compiler logic); the built interpreter only exposes the Mini-subset built-in whitelist (`sqlite_*`/`http_*` and other production builtins are available in build mode only). See MINI_SUBSET §8–§12.

---

## Directory Layout

```
├── bootstrap/              # Bootstrap binaries (pxc compiler / pxi interpreter / pxl lexer / pxpar parser, static ELF)
├── tools/pxc               # User entry point: build / run / lex / parse / --version (bash wrapper, zero Rust dependency)
├── selfhost/               # The bootstrapping project (the core!)
│   ├── compiler.px         #   Full PuXian-written compiler CLI (imports the full codegen.px chain)
│   ├── codegen.px + cg_*.px #   codegen modules (AST → C)
│   ├── interp.px + i*.px   #   interpreter modules (tree-walking)
│   ├── lexer.px pxlexer.px #   lexer
│   ├── parser.px           #   parser
│   ├── value.px env.px module.px  # value system / scoping / module loading
│   ├── capability.px       #   capability self-check (110/110)
│   ├── cases/ + golden/    #   differential test cases (s01-s09 + v01-v03) and golden artifacts
│   ├── cases_bad/          #   error cases (lex 14 + parse 9)
│   └── diffcheck.sh / bootstrap_prove.sh  # differential harness / bootstrap proof
├── runtime/                # C runtime (runtime.c/h + aes/xml/zip/ws/rsa/sqlite/route/h2/h3/quic + mbedtls + third_party)
├── stdlib/                 # Standard library (collections.px)
├── examples/               # 80+ examples (hello / fib / match / concurrency / networking / TLS / SQLite / comprehensions ...)
├── archive/rust-compiler/  # Rust compiler source archive (read-only; the pre-bootstrap implementation; git history preserved)
├── docs/                   # Documentation (spec / Mini subset / ROADMAP)
└── .github/workflows/ci.yml # CI: regression + bootstrap proof + example compilation
```

---

## Documentation

| Document | Description |
|---|---|
| [docs/spec.md](docs/spec.md) | Language specification (lexical / syntax / semantics / standard library) |
| [docs/MINI_SUBSET.md](docs/MINI_SUBSET.md) | **Mini subset spec** (the locked language surface of the self-hosted compiler: supported features / explicitly excluded / known limitations) |
| [docs/ROADMAP.md](docs/ROADMAP.md) | Roadmap (completed milestones + future directions) |
| [CHANGELOG.md](CHANGELOG.md) | Changelog (notable changes per milestone) |
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
| M-B9a | Retire the Rust version + wire up CI + bootstrap chain | `tools/pxc` fully usable end-to-end; CI four jobs |
| M-B9b | First production application (dogfooding validation) | ✅ moved to a separate private repo |

### Native Development (M41–M65, post-bootstrap development in PuXian itself — all ✅)

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
| M63 | Language-debt fixes L8–L11: pxi network API whitelist + float full-precision roundtrip + pxc --version |
| M64 | **Toolchain self-hosting restored**: `pxc fmt / lint / doc / test / bench` five tools self-hosted (keep-lexer base + whole-repo convergence, net -318 lines) |
| M65 | **LSP / MCP self-hosted**: `pxc lsp` (diagnostics/completion/definition/hover) + `pxc mcp` (8 tools for AI agents) — **spec §12 toolchain fully self-hosted** |

---

## Examples

The `examples/` directory (80+ examples) for quick hands-on:

```bash
# Interpreted run
./tools/pxc run examples/fib.px
./tools/pxc run examples/match.px
./tools/pxc run examples/m39_result.px
./tools/pxc run examples/m40_str_interp.px

# Build to a static binary
./tools/pxc build examples/fib.px && ./examples/build/fib
./tools/pxc build examples/m28_time_sqlite.px && ./examples/build/m28_time_sqlite
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
- ... full list in `examples/`

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
