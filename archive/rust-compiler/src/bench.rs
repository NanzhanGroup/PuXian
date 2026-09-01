//! 普贤 (PuXian) 基准测试工具（px bench）
//!
//! 用法：px bench <file.px> <func> [--count N] [--repeat R]
//!   - 对目标函数独立构造 Program（全部顶层定义 + 单次调用）运行
//!   - --count N：迭代次数（默认 1000）
//!   - --repeat R：重复轮数（默认 3），取每轮耗时后报告 min/avg/max
//!
//! 注：每次迭代新建 Interpreter（干净环境，符合语言无全局持久化语义），
//! 测量 wall-clock 耗时。

use std::time::Instant;

use crate::ast::{self, Program, Stmt};
use crate::interp::Interpreter;
use crate::lexer::Lexer;
use crate::parser::Parser;

pub struct BenchOptions {
    pub count: usize,
    pub repeat: usize,
}

impl Default for BenchOptions {
    fn default() -> Self {
        BenchOptions {
            count: 1000,
            repeat: 3,
        }
    }
}

/// 运行基准；返回退出码
pub fn run_bench(file: &str, func: &str, opts: &BenchOptions) -> i32 {
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

    // 校验目标函数存在
    let found = prog.items.iter().any(|s| match s {
        Stmt::FuncDef { name, .. } => name == func,
        _ => false,
    });
    if !found {
        eprintln!("错误: 未找到函数 '{}'", func);
        return 1;
    }

    // 构造单次调用程序
    let call_prog = build_call_program(&prog, func);

    // 预热
    let mut warm = Interpreter::new();
    if warm.run_program(&call_prog).is_err() {
        // 预热失败继续（可能在等 IO）
    }

    println!(
        "基准: {} (函数 {}) — {} 次迭代 × {} 轮\n",
        file, func, opts.count, opts.repeat
    );

    let mut rounds: Vec<f64> = Vec::new();
    for round in 1..=opts.repeat {
        let t0 = Instant::now();
        for _ in 0..opts.count {
            let mut interp = Interpreter::new();
            if let Err(e) = interp.run_program(&call_prog) {
                eprintln!("第 {} 轮运行错误: {}", round, e);
                return 1;
            }
        }
        let secs = t0.elapsed().as_secs_f64();
        rounds.push(secs);
        println!(
            "第 {} 轮: 总 {:.3} s, 平均 {:.6} ms/次 ({:.0} 次/s)",
            round,
            secs,
            secs / opts.count as f64 * 1000.0,
            opts.count as f64 / secs
        );
    }

    let min = rounds.iter().cloned().fold(f64::INFINITY, f64::min);
    let max = rounds.iter().cloned().fold(f64::NEG_INFINITY, f64::max);
    let avg = rounds.iter().sum::<f64>() / rounds.len() as f64;
    println!(
        "\n汇总: min {:.3} s | avg {:.3} s | max {:.3} s | 平均 {:.6} ms/次",
        min,
        avg,
        max,
        avg / opts.count as f64 * 1000.0
    );
    0
}

fn build_call_program(prog: &Program, func: &str) -> Program {
    let mut items: Vec<Stmt> = Vec::new();
    for stmt in &prog.items {
        match stmt {
            Stmt::FuncDef { .. }
            | Stmt::StructDef { .. }
            | Stmt::EnumDef { .. }
            | Stmt::TraitDef { .. }
            | Stmt::ImplDef { .. }
            | Stmt::Import { .. } => items.push(stmt.clone()),
            _ => {}
        }
    }
    items.push(Stmt::ExprStmt {
        expr: ast::Expr::Call {
            callee: Box::new(ast::Expr::Var {
                name: func.to_string(),
                pos: crate::token::Pos::new(0, 0),
            }),
            args: Vec::new(),
            pos: crate::token::Pos::new(0, 0),
        },
        pos: crate::token::Pos::new(0, 0),
    });
    Program { items }
}

fn parse_program(src: &str) -> Result<Program, String> {
    let tokens = Lexer::new(src).tokenize().map_err(|e| e.to_string())?;
    let mut p = Parser::new(tokens);
    p.parse_program().map_err(|e| e.to_string())
}
