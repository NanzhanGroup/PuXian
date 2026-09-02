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

- **Compiled output is not bound by the license**: programs and services you write with PuXian (e.g. ws-web) may be closed-source and commercialized freely; they do not constitute derivative works.
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
| ✅ **CI integrated** | GitHub Actions: every commit automatically runs regression + bootstrap proof + example compilation + ws-web smoke tests. |
| 🔄 **ws-web in development (M-B9b)** | Writing the first production application in PuXian (HTTP + SQLite service), see `ws-web/`. |

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
| `pxc --version` / `-v` | Print the version number |
| `pxc help` | Show help |

> **Current toolchain status**: the self-hosted `pxc` currently provides the core commands above. The full toolchain the Rust version once offered (`fmt / lint / test / bench / doc / lsp / mcp / pkg`) is preserved in `archive/rust-compiler/` (read-only archive — usable as reference or to be restored in PuXian as needed).

---

## Features at a Glance

| Dimension | Capabilities |
|---|---|
| 🏃 Dual modes | Script mode (interpreted, instant start) / build mode (emits C → gcc static binary, near-C performance). `run` and `build` output are byte-for-byte identical. |
| 🔀 Concurrency | `spawn` true concurrency, `channel` blocking communication, `select` random readiness + **concurrent GC** (stop-the-world full collection, thread-safe). |
| ⏱ Timers | `set_timeout` / `set_interval` / `clear_timer` (one-shot/periodic callbacks, variadic argument pass-through, concurrency primitives are safe inside callbacks). |
| 🧹 Memory | Build mode: C runtime with a conservative mark-and-sweep GC (cyclic references collectable, auto-triggered) + **slab allocator** (21 size-class slot reuse); interpreter side: **tracing GC** that collects cycles (list/dict/chan/**closure Func↔Env cycles**) + `gc()` forced collection. |
| 🧩 Modularity | `import std.*` / `import foo.bar` / `from foo import x` / relative-path imports. |
| 🌐 Networking | HTTP client (**HTTPS TLS 1.2/1.3** + gzip/chunked auto-decoding + **http/https connection-pool reuse** + **TLS session-ticket resumption** + **streaming gzip decode-as-you-download**) + **HTTP server** (`http_serve` with gzip/chunked/keep-alive/streaming + **`px_serve` server-side TLS**: `tls_server(cert,key[,hostname])` enables HTTPS/WSS/SSE-over-TLS + **TLS SNI multi-certificate selection by domain** + configurable request-body limits + 413 + large-body spooling to disk + **graceful shutdown** + **per-route rate limiting** (429 at route granularity) + **access-log file rotation** + **Alt-Svc advertisement**) + **WebSocket** (RFC 6455, heartbeat/timeouts, **one-line `ws://`/`wss://` connection**) + **SSE** server/client (**auto-reconnect on disconnect**, with Last-Event-ID) + **UDP** (udp_open/send/recv/close, groundwork for QUIC) + full-featured TCP. |
| 🛡 Crypto/Docs | **AES-CBC-PKCS7 / AES-GCM**, **RSA** (PKCS#1 v1.5), **XML** parse/escape/**generate** (xml_build), **zip** pack/unpack, **base64**, sha256 / xxhash, **SQLite** (open/exec/query/close, parameter binding + result sets). |
| 🔢 Language | Slice syntax `a[i:j]` / `a[i:j:k]` (stride/reverse, strings sliced by UTF-8 chars), **generator expressions** `(x for x in xs)` (**lazy**: single-level for delayed evaluation / `gen_next` item-by-item / for-in / `list()` conversion), bitwise ops + binary-data views (int_to_hex / bytes_to_hex / bit_count / bit_length), regex, lock primitives (mutex / rwlock), random file I/O + fsync, process/signal (os_spawn / os_wait / signal), **Result/Option error handling** (`Ok(x)`/`Err(e)`/`Some(x)` constructors, `?` error propagation — Err/None returns immediately, `!` forced unwrap, is_ok/is_err/unwrap methods; the single error channel in the spec), string interpolation `${expr}`, comprehensions, optional chaining `?.`, null coalescing `??`, pipeline `\|>`. |
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
├── runtime/                # C runtime (runtime.c/h + aes/xml/zip/ws/rsa/sqlite/route/h2 + mbedtls + third_party)
├── stdlib/                 # Standard library (collections.px)
├── ws-web/                 # First production application (M-B9b, dogfooding): HTTP + SQLite service skeleton
├── examples/               # 80+ examples (hello / fib / match / concurrency / networking / TLS / SQLite / comprehensions ...)
├── archive/rust-compiler/  # Rust compiler source archive (read-only; the pre-bootstrap implementation; git history preserved)
├── docs/                   # Documentation (spec / Mini subset / ROADMAP)
└── .github/workflows/ci.yml # CI: regression + bootstrap proof + example compilation + ws-web smoke
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
| [ws-web/README.md](ws-web/README.md) | First production application: quick start + 10 pitfall checks |

---

## Milestone Progress

### Feature Development (M0–M40, the Rust era — all completed ✅)

| Stage | Highlights |
|---|---|
| M0–M9 | Requirements/design/spec → lexer+parser → interpreter → concurrency runtime → C codegen → standard library → AI toolchain (fmt/lint/test/bench/doc/ast) → LSP/MCP → GC value objects → package management/modularity |
| M10–M19 | HTTPS (TLS 1.2/1.3) → concurrent GC → random file I/O + fsync → lock primitives → sha256/xxhash → regex → HTTP server framework → `.px` script execution mechanism → timers → AES/XML/zip |
| M20–M29 | C runtime symbol unification → chunked/gzip/slicing/base64/SSE → slab allocator + tracing GC + WebSocket + bitwise ops → networking/storage/security wrap-up → XML generation + slice strides + connection pool → closure-cycle collection + process pool + TLS ticket resumption → `>>>` + WS heartbeat + remote registry → server-side TLS + request-body limits + Session → routing/timezone/cron/SQLite → JSON path + static cache headers + Range + access logs + request ID |
| M30–M40 | Server-side HTTPS connection pool + byte-order int↔bytes + comprehension completion + configurable fmt → sandbox + virtual hosts + rate limiting + HTTP/2 preflight + connection thread pool → one-line ws/wss + SSE reconnect + process-pool hot reload + generators → per-route rate limiting + TLS SNI + access-log rotation + QUIC research → lazy generators + configurable process pool + WS broadcast + event bus → gzip decompression + range comprehensions + minimal HTTP/2 server + multi-dimensional rate limiting → log enhancements + request context + WS heartbeat config + graceful shutdown → HTTP/2 over TLS + response compression + S3 → WS auto-reconnect + HTTP/2 multi-stream + UDP echo → **Result/Option as the single error channel** → **string interpolation `${expr}`** |

### Bootstrapping (M-B1 → M-B9 — all ✅, M-B9b in progress)

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
| M-B9b | ws-web (first production application) | 🔄 in progress (dogfooding validation) |

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
- ... full list in `examples/`

---

## Ecosystem & Collaboration

- **ws-web** (`ws-web/`): the first production application — an HTTP + SQLite service used to dogfood-validate PuXian in real scenarios.
- **Feedback & contribution**: when you find a problem, please attach a minimal reproduction case (a single `.px` + expected/actual output) and file an issue (labels: `ws-web-blocker` / `M-B9b`); PRs to improve the project are welcome.

---

## 💡 Credits

PuXian is developed by the **达者同游 (Dazhe Tongyou) team**, with design and implementation assisted by **wsAgent (文殊智能体, the Wenshu agent)**:

| 中文名 | English Name | Type | Role |
|---|---|---|---|
| 本源 | Benyuan | Human | Founder / Chief Architect (direction & architecture decisions, final acceptance) |
| 东月 | Dongyue | wsAgent (AI agent) | Developer (implementation & coding) |
| 清歌 | Qingge | wsAgent (AI agent) | Designer / Issue Discovery (design assistance & problem finding) |

> This language was developed with the assistance of wsAgent (文殊智能体) — a language built for efficient AI programming, written with AI participation, dogfooding as its own proof. See [`AUTHORS.md`](AUTHORS.md) for full team details.
