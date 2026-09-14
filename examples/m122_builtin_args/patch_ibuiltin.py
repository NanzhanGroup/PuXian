#!/usr/bin/env python3
# M122 · 给 selfhost/ibuiltin.px 的 49 个「直调宿主 native」分支补 arity 校验
# 文案逐字对齐 runtime.c 的 px_error（C 轨），错码用 i_r1002（参数错）
# 仅插入「该分支没有 len(args) 校验」的 return 行之前；已校验/复杂逻辑分支不动
import re, sys

SRC = "/data/code/puxian/selfhost/ibuiltin.px"

# name -> (校验条件, 文案)
RULES = {
  # 零参
  "dict":            ("len(args) != 0", "dict 不需要参数"),
  "now_sec":         ("len(args) != 0", "now_sec 不需要参数"),
  "tz_local":        ("len(args) != 0", "tz_local 不需要参数"),
  "now":             ("len(args) != 0", "now 不需要参数"),
  "os_self_path":    ("len(args) != 0", "os_self_path 不需要参数"),
  # 单参
  "exists":          ("len(args) != 1", "exists 需要一个路径参数"),
  "remove":          ("len(args) != 1", "remove 需要一个路径参数"),
  "list_dir":        ("len(args) != 1", "list_dir 需要一个路径参数"),
  "file_size":       ("len(args) != 1", "file_size 需要一个路径参数"),
  "read_bytes":      ("len(args) != 1", "read_bytes 需要一个路径参数"),
  "json_parse":      ("len(args) != 1", "json_parse 需要一个字符串参数"),
  "json_stringify":  ("len(args) != 1", "json_stringify 需要一个参数"),
  "sha256":          ("len(args) != 1", "sha256 需要一个参数"),
  "http_get":        ("len(args) != 1", "http_get 需要 (url) 参数"),
  "os_wait":         ("len(args) != 1", "os_wait 需要 (pid) 参数"),
  "os_remove_all":   ("len(args) != 1", "os_remove_all 需要 (path) 参数"),
  "os_random_hex":   ("len(args) != 1", "os_random_hex 需要 (nbytes) 参数"),
  "os_file_sha256":  ("len(args) != 1", "os_file_sha256 需要 (path) 参数"),
  "unix_connect":    ("len(args) != 1", "unix_connect 需要 (socket_path) 参数"),
  "env":             ("len(args) != 1", "env 需要一个变量名"),
  "sleep":           ("len(args) != 1", "sleep 需要一个参数"),
  "hex_to_int":      ("len(args) != 1", "hex_to_int 需要一个参数"),
  "bytes":           ("len(args) != 1", "bytes 需要一个参数"),
  "bytes_len":       ("len(args) != 1", "bytes_len 需要一个参数"),
  "bytes_to_str":    ("len(args) != 1", "bytes_to_str 需要一个参数"),
  "bytes_to_hex":    ("len(args) != 1", "bytes_to_hex 需要一个参数"),
  "hex_to_bytes":    ("len(args) != 1", "hex_to_bytes 需要一个参数"),
  "bytes_base64":    ("len(args) != 1", "bytes_base64 需要一个参数"),
  "base64_to_bytes": ("len(args) != 1", "base64_to_bytes 需要一个参数"),
  "base64_encode":   ("len(args) != 1", "base64_encode 需要一个参数"),
  "base64_decode":   ("len(args) != 1", "base64_decode 需要一个参数"),
  # 双参
  "write_bytes":     ("len(args) != 2", "write_bytes 需要 (路径, bytes) 参数"),
  "regex_match":     ("len(args) != 2", "regex_match 需要 2 个参数: (pattern, text)"),
  "regex_find":      ("len(args) != 2", "regex_find 需要 2 个参数: (pattern, text)"),
  "regex_search":    ("len(args) != 2", "regex_search 需要 2 个参数: (pattern, text)"),
  "regex_find_all":  ("len(args) != 2", "regex_find_all 需要 2 个参数: (pattern, text)"),
  "regex_split":     ("len(args) != 2", "regex_split 需要 2 个参数: (pattern, text)"),
  "json_path":       ("len(args) != 2", "json_path 需要 (json, path) 参数"),
  "os_spawn_capture":("len(args) != 2", "os_spawn_capture 需要 (cmd, args) 参数"),
  "os_rename":       ("len(args) != 2", "os_rename 需要 (old_path, new_path) 参数"),
  "os_capture":      ("len(args) != 2", "os_capture 需要 (cmd, args) 参数"),
  "os_popen":        ("len(args) != 2", "os_popen 需要 (cmd, args) 参数"),
  "bytes_get":       ("len(args) != 2", "bytes_get 需要 (bytes, index) 参数"),
  "bytes_find":      ("len(args) != 2", "bytes_find 需要 (bytes, sub) 参数"),
  "int_to_hex":      ("len(args) != 2", "int_to_hex 需要 (n, width) 参数"),
  # 三参
  "regex_replace":   ("len(args) != 3", "regex_replace 需要 3 个参数: (pattern, text, repl)"),
  "json_path_set":   ("len(args) != 3", "json_path_set 需要 (json, path, value) 参数"),
  "bytes_set":       ("len(args) != 3", "bytes_set 需要 (bytes, index, value) 参数"),
  # 区间
  "bytes_to_int":    ("len(args) < 1 or len(args) > 3", "bytes_to_int 需要 (bytes[, endian[, signed]]) 参数"),
  "exit":            ("len(args) > 1", "exit 需要 0-1 个参数"),
}

lines = open(SRC, encoding="utf-8").read().split("\n")
out = []
cur = None
cur_body = []
stats = {"patched": 0, "skipped_has_check": 0, "skipped_not_direct": 0}

def flush_branch():
    global cur, cur_body
    if cur is None:
        return
    name = cur
    body = "\n".join(cur_body)
    rule = RULES.get(name)
    if rule is None:
        out.extend(cur_body)
    elif "len(args)" in body:
        stats["skipped_has_check"] += 1
        out.extend(cur_body)
    else:
        cond, msg = rule
        # 在首个 `return Ok(` 行前插入校验（该分支直调宿主，缩进 8/12）
        inserted = False
        new_body = []
        for ln in cur_body:
            if not inserted and ln.strip().startswith("return Ok("):
                new_body.append(f"        if {cond}:")
                new_body.append(f'            return Err(i_r1002("{msg}", pos))')
                new_body.append(ln)
                inserted = True
                stats["patched"] += 1
            else:
                new_body.append(ln)
        if not inserted:
            stats["skipped_not_direct"] += 1
        out.extend(new_body)
    cur, cur_body = None, []

for ln in lines:
    m = re.match(r'\s*if name == "([^"]+)":', ln)
    if m:
        flush_branch()
        cur = m.group(1)
        cur_body = [ln]
    elif cur is not None:
        cur_body.append(ln)
    else:
        out.append(ln)
flush_branch()

open(SRC, "w", encoding="utf-8").write("\n".join(out))
print(f"patched={stats['patched']} skipped_has_check={stats['skipped_has_check']} skipped_not_direct={stats['skipped_not_direct']}")