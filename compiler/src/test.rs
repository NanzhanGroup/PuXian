//! 普贤 (PuXian) 测试运行器（px test）
//!
//! 约定：顶层 `def test_xxx()` 无参函数即为测试用例。
//! 对每个测试独立构造 Program（全部顶层定义 + 该测试调用），
//! 用独立 Interpreter 运行，互不影响；失败不中断其余测试。
//!
//! 输出：PASS/FAIL + 耗时 + 汇总。有失败退出码 1。

use std::time::Instant;

use crate::ast::{self, Program, Stmt};
use crate::interp::Interpreter;
use crate::lexer::Lexer;
use crate::parser::Parser;

/// 单个测试结果
pub struct TestResult {
    pub name: String,
    pub passed: bool,
    pub ms: f64,
    pub error: Option<String>,
}

/// 运行测试；返回退出码（0 成功 / 1 有失败）
pub fn run_tests(file: &str, filter: Option<&str>) -> i32 {
    let src = match std::fs::read_to_string(file) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("错误: 无法读取文件 {}: {}", file, e);
            return 2;
        }
    };
    let prog = match parse_program(&src) {
        Ok(p) => p,
        Err(e) => {
            eprintln!("{}", e);
            return 1;
        }
    };
    let base_dir = std::path::Path::new(file)
        .parent()
        .map(|p| p.to_string_lossy().to_string())
        .unwrap_or_else(|| ".".to_string());
    let prog = crate::module::ModuleResolver::new(std::path::Path::new(&base_dir)).resolve(prog);

    // 收集测试函数
    let tests: Vec<String> = prog
        .items
        .iter()
        .filter_map(|s| match s {
            Stmt::FuncDef { name, params, .. } if name.starts_with("test_") && params.is_empty() => {
                Some(name.clone())
            }
            _ => None,
        })
        .collect();

    if tests.is_empty() {
        println!("未发现测试函数（约定：顶层 def test_xxx()）");
        return 0;
    }

    let filtered: Vec<&String> = match filter {
        Some(f) => tests
            .iter()
            .filter(|n| n.contains(f))
            .collect::<Vec<_>>(),
        None => tests.iter().collect(),
    };

    if filtered.is_empty() {
        println!("没有测试匹配过滤器 '{}'", filter.unwrap_or(""));
        return 0;
    }

    println!("运行 {} 个测试…\n", filtered.len());

    let mut results: Vec<TestResult> = Vec::new();
    for name in &filtered {
        let t0 = Instant::now();
        let r = run_one_test(&prog, name);
        let ms = t0.elapsed().as_secs_f64() * 1000.0;
        results.push(TestResult {
            name: name.to_string(),
            passed: r.is_ok(),
            ms,
            error: r.err(),
        });
    }

    // 输出
    let mut passed = 0usize;
    for r in &results {
        let mark = if r.passed { "PASS" } else { "FAIL" };
        println!("[{}] {} ({:.1} ms)", mark, r.name, r.ms);
        if let Some(e) = &r.error {
            println!("      {}", e);
        }
        if r.passed {
            passed += 1;
        }
    }
    println!(
        "\n结果: {}/{} 通过, {}/{} 失败",
        passed,
        results.len(),
        results.len() - passed,
        results.len()
    );

    if passed == results.len() {
        0
    } else {
        1
    }
}

/// 构造"仅该测试"的程序：全部顶层定义 + test 调用
fn run_one_test(prog: &Program, test_name: &str) -> Result<(), String> {
    let mut items: Vec<Stmt> = Vec::new();
    for stmt in &prog.items {
        match stmt {
            Stmt::FuncDef { .. }
            | Stmt::StructDef { .. }
            | Stmt::EnumDef { .. }
            | Stmt::TraitDef { .. }
            | Stmt::ImplDef { .. }
            | Stmt::Import { .. } => items.push(stmt.clone()),
            _ => {} // 顶层可执行语句（main() 等）不参与测试
        }
    }
    // 追加测试调用
    items.push(Stmt::ExprStmt {
        expr: ast::Expr::Call {
            callee: Box::new(ast::Expr::Var {
                name: test_name.to_string(),
                pos: crate::token::Pos::new(0, 0),
            }),
            args: Vec::new(),
            pos: crate::token::Pos::new(0, 0),
        },
        pos: crate::token::Pos::new(0, 0),
    });
    let test_prog = Program { items };

    let mut interp = Interpreter::new();
    interp
        .run_program(&test_prog)
        .map(|_| ())
        .map_err(|e| e.to_string())
}

/// 解析源码
fn parse_program(src: &str) -> Result<Program, String> {
    let tokens = Lexer::new(src).tokenize().map_err(|e| e.to_string())?;
    let mut p = Parser::new(tokens);
    p.parse_program().map_err(|e| e.to_string())
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_collect_tests() {
        let src = "def test_one():\n    assert(1 + 1 == 2, \"math\")\n\ndef test_two():\n    assert(len([1]) == 1)\n\ndef helper():\n    return 42\n";
        let prog = parse_program(src).unwrap();
        let names: Vec<String> = prog
            .items
            .iter()
            .filter_map(|s| match s {
                Stmt::FuncDef { name, params, .. } if name.starts_with("test_") && params.is_empty() => {
                    Some(name.clone())
                }
                _ => None,
            })
            .collect();
        assert_eq!(names, vec!["test_one", "test_two"]);
    }

    #[test]
    fn test_run_one_passing() {
        let src = "def test_ok():\n    assert(2 * 3 == 6)\n";
        let prog = parse_program(src).unwrap();
        assert!(run_one_test(&prog, "test_ok").is_ok());
    }

    #[test]
    fn test_run_one_failing() {
        let src = "def test_bad():\n    assert(1 == 2, \"boom\")\n";
        let prog = parse_program(src).unwrap();
        let r = run_one_test(&prog, "test_bad");
        assert!(r.is_err());
        assert!(r.unwrap_err().contains("boom"));
    }
}
