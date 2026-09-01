//! 普贤 (PuXian) MCP 服务器（px mcp）
//!
//! MCP（Model Context Protocol）实现，JSON-RPC 2.0 over stdio：
//!   - initialize / initialized
//!   - tools/list：暴露普贤工具链（run / fmt / lint / test / bench / doc / ast / version）
//!   - tools/call：执行对应工具
//!
//! 零依赖：帧解析复用 lsp.rs，JSON 序列化用自研 json.rs。
//! 工具执行通过子进程调用自身二进制，避免污染协议 stdout 通道。

use std::io;
use std::process::Command;

use crate::json::Json;

const MCP_VERSION: &str = "2024-11-05";

/// MCP 工具定义
struct ToolDef {
    name: &'static str,
    description: &'static str,
    params: &'static [(&'static str, &'static str, bool)], // (名, 描述, 必填)
}

const TOOLS: &[ToolDef] = &[
    ToolDef {
        name: "run",
        description: "运行普贤（PuXian）脚本：执行 .px 源码文件或直接传入源码字符串",
        params: &[("file", "普贤源码文件路径", false), ("code", "普贤源码字符串", false)],
    },
    ToolDef {
        name: "fmt",
        description: "格式化普贤代码（file 或 code 二选一）",
        params: &[("file", "普贤源码文件路径", false), ("code", "普贤源码字符串", false)],
    },
    ToolDef {
        name: "lint",
        description: "静态检查普贤代码（file 或 code 二选一），输出 L001-L008 诊断",
        params: &[("file", "普贤源码文件路径", false), ("code", "普贤源码字符串", false)],
    },
    ToolDef {
        name: "test",
        description: "运行普贤测试用例（def test_*）",
        params: &[("file", "普贤源码文件路径", true)],
    },
    ToolDef {
        name: "bench",
        description: "基准测试普贤函数",
        params: &[("file", "普贤源码文件路径", true), ("func", "要测试的函数名", true)],
    },
    ToolDef {
        name: "doc",
        description: "从 ## 文档注释生成 Markdown API 文档",
        params: &[("file", "普贤源码文件路径", true)],
    },
    ToolDef {
        name: "ast",
        description: "查看普贤源码的 AST 树形结构",
        params: &[("file", "普贤源码文件路径", false), ("code", "普贤源码字符串", false)],
    },
    ToolDef {
        name: "version",
        description: "获取普贤编译器版本信息",
        params: &[],
    },
];

// ==================== JSON 辅助 ====================

fn obj() -> Json {
    Json::obj()
}

// ==================== 工具执行 ====================

/// 定位 CLI 可执行文件。
/// 优先顺序：
/// 1. `CARGO_BIN_EXE_px`（cargo 测试时注入）
/// 2. 当前 exe（正式运行时即 px 本身）
/// 3. 测试 harness（deps/px-xxxx）时回退到同目录下的 `px` 二进制
fn cli_exe() -> std::path::PathBuf {
    if let Ok(p) = std::env::var("CARGO_BIN_EXE_px") {
        let pb = std::path::PathBuf::from(p);
        if pb.exists() {
            return pb;
        }
    }
    let exe = std::env::current_exe().unwrap_or_else(|_| std::path::PathBuf::from("px"));
    let name = exe.file_name().map(|n| n.to_string_lossy().to_string()).unwrap_or_default();
    // 测试二进制形如 px-<hash>（在 target/release/deps/ 下），真实的 px 在同级 target/release/ 下
    if name.starts_with("px-") {
        if let Some(deps_dir) = exe.parent() {
            if let Some(release_dir) = deps_dir.parent() {
                let cand = release_dir.join("px");
                if cand.exists() {
                    return cand;
                }
            }
        }
    }
    exe
}

/// 通过子进程调用自身二进制执行 CLI 工具，捕获输出返回
fn run_cli(args: &[&str], stdin_text: Option<&str>) -> (String, bool) {
    let exe = cli_exe();
    let mut cmd = Command::new(&exe);
    cmd.args(args);
    if stdin_text.is_some() {
        use std::process::Stdio;
        cmd.stdin(Stdio::piped());
        cmd.stdout(Stdio::piped());
        cmd.stderr(Stdio::piped());
    }
    match cmd.output() {
        Ok(out) => {
            let mut text = String::from_utf8_lossy(&out.stdout).to_string();
            if !out.stderr.is_empty() {
                if !text.is_empty() {
                    text.push('\n');
                }
                text.push_str(&String::from_utf8_lossy(&out.stderr));
            }
            let is_err = !out.status.success();
            (text, is_err)
        }
        Err(e) => (format!("执行失败: {}", e), true),
    }
}

/// 写临时文件（MCP 传入 code 时）
fn write_temp(code: &str) -> Result<String, String> {
    let dir = std::env::temp_dir();
    let path = dir.join(format!("px_mcp_{}_{}.px", std::process::id(), std::time::SystemTime::now().duration_since(std::time::UNIX_EPOCH).map(|d| d.as_nanos()).unwrap_or(0)));
    std::fs::write(&path, code).map_err(|e| format!("写临时文件失败: {}", e))?;
    Ok(path.to_string_lossy().to_string())
}

/// 从工具参数中取 file 或 code；返回 (参数列表, 错误)
fn resolve_file_arg(params: &Json) -> Result<Vec<String>, String> {
    if let Some(f) = params.get_str("file") {
        Ok(vec![f.to_string()])
    } else if let Some(code) = params.get_str("code") {
        let path = write_temp(code)?;
        Ok(vec![path])
    } else {
        Err("缺少参数：需要 file 或 code".to_string())
    }
}

/// 执行 MCP 工具调用，返回 MCP 标准 result
fn call_tool(name: &str, params: &Json) -> Json {
    let (text, is_err): (String, bool) = match name {
        "version" => {
            let (t, e) = run_cli(&["--version"], None);
            (t, e)
        }
        "run" => match resolve_file_arg(params) {
            Ok(f) => run_cli(&["run", &f[0]], None),
            Err(e) => (e, true),
        },
        "fmt" => match resolve_file_arg(params) {
            Ok(f) => run_cli(&["fmt", &f[0]], None),
            Err(e) => (e, true),
        },
        "lint" => match resolve_file_arg(params) {
            Ok(f) => run_cli(&["lint", &f[0]], None),
            Err(e) => (e, true),
        },
        "test" => match params.get_str("file") {
            Some(f) => run_cli(&["test", f], None),
            None => ("缺少参数：file".to_string(), true),
        },
        "bench" => {
            let file = params.get_str("file");
            let func = params.get_str("func");
            match (file, func) {
                (Some(f), Some(fn_name)) => run_cli(&["bench", f, fn_name], None),
                _ => ("缺少参数：需要 file 和 func".to_string(), true),
            }
        }
        "doc" => match params.get_str("file") {
            Some(f) => run_cli(&["doc", f], None),
            None => ("缺少参数：file".to_string(), true),
        },
        "ast" => match resolve_file_arg(params) {
            Ok(f) => run_cli(&["ast", &f[0]], None),
            Err(e) => (e, true),
        },
        _ => (format!("未知工具: {}", name), true),
    };

    let mut content = obj();
    content.set("type", Json::Str("text".into()));
    content.set("text", Json::Str(text));
    let mut result = obj();
    result.set("content", Json::Arr(vec![content]));
    result.set("isError", Json::Bool(is_err));
    result
}

/// 构建 tools/list 结果
fn tools_list() -> Json {
    let mut tools = Json::arr();
    for t in TOOLS {
        let mut tj = obj();
        tj.set("name", Json::Str(t.name.into()));
        tj.set("description", Json::Str(t.description.into()));
        // inputSchema（JSON Schema 简化）
        let mut schema = obj();
        schema.set("type", Json::Str("object".into()));
        let mut props = obj();
        for (pname, pdesc, _required) in t.params {
            let mut p = obj();
            p.set("type", Json::Str("string".into()));
            p.set("description", Json::Str((*pdesc).into()));
            props.set(pname, p);
        }
        schema.set("properties", props);
        let required_list: Vec<Json> = t.params.iter().filter(|(_, _, req)| *req).map(|(n, _, _)| Json::Str((*n).into())).collect();
        schema.set("required", Json::Arr(required_list));
        tj.set("inputSchema", schema);
        tools.push(tj);
    }
    let mut result = obj();
    result.set("tools", tools);
    result
}

// ==================== 主循环 ====================

/// MCP 服务器入口（阻塞，直到 exit / EOF）
pub fn run_mcp() -> i32 {
    let stdin = io::stdin();
    let mut reader = stdin.lock();
    let stdout = io::stdout();
    let mut writer = stdout.lock();

    loop {
        let msg = match crate::lsp::read_message(&mut reader) {
            Ok(Some(m)) => m,
            Ok(None) => break,
            Err(e) => {
                eprintln!("[px-mcp] 读取消息失败: {}", e);
                break;
            }
        };

        let method = msg.get_str("method").unwrap_or("").to_string();
        let id = msg.get("id").cloned();
        let is_notification = id.is_none();

        let mut response: Option<Json> = None;

        match method.as_str() {
            "initialize" => {
                let mut result = obj();
                result.set("protocolVersion", Json::Str(MCP_VERSION.into()));
                let mut capabilities = obj();
                let mut tools_cap = obj();
                tools_cap.set("listChanged", Json::Bool(false));
                capabilities.set("tools", tools_cap);
                result.set("capabilities", capabilities);
                let mut server_info = obj();
                server_info.set("name", Json::Str("px-mcp".into()));
                server_info.set("version", Json::Str(env!("CARGO_PKG_VERSION").into()));
                result.set("serverInfo", server_info);
                response = Some(result);
            }
            "notifications/initialized" => { /* 忽略 */ }
            "tools/list" => {
                response = Some(tools_list());
            }
            "tools/call" => {
                let name = msg.get("params").and_then(|p| p.get_str("name")).unwrap_or("").to_string();
                let params = msg.get("params").and_then(|p| p.get("arguments")).cloned().unwrap_or(Json::obj());
                response = Some(call_tool(&name, &params));
            }
            "shutdown" => {
                response = Some(obj());
            }
            "exit" => break,
            _ => {
                if !is_notification {
                    let mut err = obj();
                    err.set("code", Json::Int(-32601));
                    err.set("message", Json::Str(format!("方法未找到: {}", method)));
                    let mut resp = obj();
                    resp.set("jsonrpc", Json::Str("2.0".into()));
                    if let Some(idv) = id {
                        resp.set("id", idv);
                    }
                    resp.set("error", err);
                    let _ = crate::lsp::write_message(&mut writer, &resp);
                    continue;
                }
            }
        }

        if let Some(r) = response {
            let mut resp = obj();
            resp.set("jsonrpc", Json::Str("2.0".into()));
            if let Some(idv) = id {
                resp.set("id", idv);
            } else {
                resp.set("id", Json::Null);
            }
            resp.set("result", r);
            if let Err(e) = crate::lsp::write_message(&mut writer, &resp) {
                eprintln!("[px-mcp] 写响应失败: {}", e);
                break;
            }
        }
    }
    0
}

// ==================== 测试 ====================

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_tools_list_shape() {
        let r = tools_list();
        let tools = r.get("tools").unwrap();
        if let Json::Arr(items) = tools {
            assert!(!items.is_empty());
            for t in items {
                assert!(t.get_str("name").is_some());
                assert!(t.get("inputSchema").is_some());
            }
        } else {
            panic!("tools 应为数组");
        }
    }

    #[test]
    fn test_call_version() {
        let mut params = obj();
        let r = call_tool("version", &params);
        assert_eq!(r.get("isError"), Some(&Json::Bool(false)));
        let content = r.get("content").unwrap();
        if let Json::Arr(items) = content {
            let text = items[0].get_str("text").unwrap_or("");
            assert!(text.contains("px"));
        }
    }

    #[test]
    fn test_call_unknown_tool() {
        let mut params = obj();
        let r = call_tool("nope", &params);
        assert_eq!(r.get("isError"), Some(&Json::Bool(true)));
    }

    #[test]
    fn test_call_lint_missing_arg() {
        let mut params = obj();
        let r = call_tool("lint", &params);
        assert_eq!(r.get("isError"), Some(&Json::Bool(true)));
    }

    #[test]
    fn test_call_run_with_code() {
        // 临时文件方式执行一段普贤代码
        let mut params = obj();
        params.set("code", Json::Str("print(\"mcp-ok\")\n".into()));
        let r = call_tool("run", &params);
        let content = r.get("content").unwrap();
        if let Json::Arr(items) = content {
            let text = items[0].get_str("text").unwrap_or("");
            assert!(text.contains("mcp-ok"), "输出: {}", text);
        } else {
            panic!("content 应为数组");
        }
    }
}
