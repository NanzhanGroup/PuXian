//! 模块解析器（M9）：import 的 AST 级合并
//!
//! 设计原则（spec §8.1-8.5）：
//! - 一个 `.px` 文件 = 一个模块；目录 = 包（含 `mod.px` 或与目录同名文件）
//! - `import` 不执行顶层语句，只注册定义（无副作用）
//! - 支持三类导入：
//!   1. `import std.collections` —— 标准库（stdlib 目录，.px 自举库）
//!   2. `import foo.bar` / `from foo import x` —— 用户点分包
//!   3. `import "path/to/file.px"` —— 相对路径导入（module 为单元素含 '/'）
//! - 模块定义插入主 AST 之前（Import 语句之后），运行时自然覆盖（用户优先）
//! - 同名定义去重：首个 std 模块优先；用户定义插在后面执行时覆盖
//! - 递归加载：模块内的 import 一并处理；已加载模块缓存避免重复

use std::collections::{HashMap, HashSet};
use std::path::{Path, PathBuf};

use crate::ast::{self, Program, Stmt};
use crate::lexer::Lexer;
use crate::parser::Parser;

/// 模块解析器
pub struct ModuleResolver {
    /// 用户模块搜索路径（当前文件目录 → .px_modules 等）
    search_paths: Vec<PathBuf>,
    /// stdlib 目录（PX_STDLIB → 二进制相对 → 编译期回退）
    stdlib_dir: Option<PathBuf>,
    /// 已加载模块缓存：规范名 → 源文件路径
    loaded: HashMap<String, PathBuf>,
}

/// 定位 stdlib 目录
/// 优先级：环境变量 PX_STDLIB → 可执行文件 ../stdlib（安装布局）→ CARGO_MANIFEST_DIR/../stdlib（开发布局）
pub fn stdlib_dir() -> Option<PathBuf> {
    if let Ok(dir) = std::env::var("PX_STDLIB") {
        if !dir.is_empty() {
            let p = PathBuf::from(dir);
            if p.exists() {
                return Some(p);
            }
        }
    }
    if let Ok(exe) = std::env::current_exe() {
        if let Some(parent) = exe.parent() {
            // bin/px → ../stdlib
            let p1 = parent.join("../stdlib");
            if p1.is_dir() {
                return Some(p1);
            }
            // target/release/px → ../../stdlib
            let p2 = parent.join("../../stdlib");
            if p2.is_dir() {
                return Some(p2);
            }
        }
    }
    let p = Path::new(env!("CARGO_MANIFEST_DIR")).join("../stdlib");
    if p.is_dir() {
        return Some(p);
    }
    None
}

/// 判断顶层语句是否为"定义类"（可被模块导出）
fn is_definition(stmt: &Stmt) -> bool {
    matches!(
        stmt,
        Stmt::FuncDef { .. }
            | Stmt::StructDef { .. }
            | Stmt::EnumDef { .. }
            | Stmt::TraitDef { .. }
            | Stmt::ImplDef { .. }
            | Stmt::VarDecl {
                  kind: ast::VarKind::Const, ..
              }
    )
}

impl ModuleResolver {
    /// 创建解析器：base_dir 为主文件所在目录（用户模块相对于此解析）
    pub fn new(base_dir: &Path) -> Self {
        let mut search_paths = vec![base_dir.to_path_buf()];
        // .px_modules（包管理器安装目录）
        let modules_dir = base_dir.join(".px_modules");
        let modules_dir_exist = modules_dir.is_dir();
        if modules_dir_exist {
            search_paths.push(modules_dir.clone());
        }
        // .px_modules 下每个依赖目录也加入（import mylib.util → .px_modules/mylib/util.px）
        if modules_dir_exist {
            if let Ok(entries) = std::fs::read_dir(&modules_dir) {
                for e in entries.flatten() {
                    if e.path().is_dir() {
                        search_paths.push(e.path());
                    }
                }
            }
        }
        ModuleResolver {
            search_paths,
            stdlib_dir: stdlib_dir(),
            loaded: HashMap::new(),
        }
    }

    /// 解析并合并程序的所有 import（AST 级）
    pub fn resolve(&mut self, prog: Program) -> Program {
        if prog.items.is_empty() {
            return prog;
        }
        let mut extra: Vec<Stmt> = Vec::new();
        // std 模块间同名去重（用户模块定义始终保留，靠执行序覆盖，用户优先）
        let mut std_defined: HashSet<String> = HashSet::new();
        // 主程序的 import 列表
        let imports: Vec<(Vec<String>, Vec<String>)> = prog
            .items
            .iter()
            .filter_map(|s| match s {
                Stmt::Import { module, names, .. } => Some((module.clone(), names.clone())),
                _ => None,
            })
            .collect();
        let base_dir = self.search_paths[0].clone();
        for (module, names) in imports {
            self.load_module(&module, &names, &base_dir, &mut extra, &mut std_defined);
        }
        if extra.is_empty() {
            return prog;
        }
        // 模块定义插到最前（Import 之后、任何语句之前）
        let mut new_items = Vec::with_capacity(extra.len() + prog.items.len());
        new_items.extend(extra);
        new_items.extend(prog.items);
        Program { items: new_items }
    }

    /// 加载单个模块：解析 → 递归子 import → 提取定义
    fn load_module(
        &mut self,
        module: &[String],
        names: &[String],
        base_dir: &Path,
        out: &mut Vec<Stmt>,
        std_defined: &mut HashSet<String>,
    ) {
        if module.is_empty() {
            return;
        }
        let is_std = module[0] == "std";
        let key = module.join(".");
        if self.loaded.contains_key(&key) {
            return;
        }
        let path = match self.find_module_path(module, base_dir) {
            Some(p) => p,
            None => {
                // std 模块缺失 → 静默跳过（可能是纯 builtin 实现）
                // 用户模块缺失 → 打印警告（不致命，保持既有行为兼容）
                if module[0] != "std" {
                    eprintln!("[module] 警告: 找不到模块 '{}'（已跳过）", key);
                }
                return;
            }
        };
        self.loaded.insert(key, path.clone());
        let src = match std::fs::read_to_string(&path) {
            Ok(s) => s,
            Err(_) => return,
        };
        let tokens = match Lexer::new(&src).tokenize() {
            Ok(t) => t,
            Err(_) => return,
        };
        let mut p = Parser::new(tokens);
        let sprog = match p.parse_program() {
            Ok(sp) => sp,
            Err(_) => return,
        };
        // 递归：模块内部 import（以模块所在目录为基准）
        let mod_dir = path.parent().map(|p| p.to_path_buf()).unwrap_or_default();
        let inner_imports: Vec<(Vec<String>, Vec<String>)> = sprog
            .items
            .iter()
            .filter_map(|s| match s {
                Stmt::Import { module, names, .. } => Some((module.clone(), names.clone())),
                _ => None,
            })
            .collect();
        for (im, inm) in inner_imports {
            self.load_module(&im, &inm, &mod_dir, out, std_defined);
        }
        // 提取定义类语句（spec §8.4：import 只注册定义，不执行顶层语句）
        // 选择性导入（from foo import x, y）：names 非空时只合并列出的名字
        let selective = !names.is_empty();
        for stmt in sprog.items {
            if let Stmt::Import { .. } = stmt {
                continue;
            }
            if !is_definition(&stmt) {
                continue;
            }
            let name = match &stmt {
                Stmt::FuncDef { name, .. } => Some(name.clone()),
                Stmt::StructDef { name, .. } => Some(name.clone()),
                Stmt::EnumDef { name, .. } => Some(name.clone()),
                Stmt::TraitDef { name, .. } => Some(name.clone()),
                Stmt::VarDecl {
                    kind: ast::VarKind::Const,
                    name,
                    ..
                } => Some(name.clone()),
                // impl 无名字：全量合并（去重由类型名 + trait 名近似控制）
                Stmt::ImplDef { type_name, trait_name, .. } => {
                    Some(format!("impl::{}::{:?}", type_name, trait_name))
                }
                _ => None,
            };
            match name {
                Some(n) => {
                    // 选择性导入：只保留显式列出的顶层符号（impl 无名字可匹配，跳过）
                    if selective {
                        if n.starts_with("impl::") || !names.iter().any(|x| x == &n) {
                            continue;
                        }
                    }
                    if is_std {
                        if std_defined.contains(&n) {
                            continue;
                        }
                        std_defined.insert(n);
                    }
                    out.push(stmt);
                }
                None => out.push(stmt),
            }
        }
    }

    /// 根据模块路径查找源文件
    fn find_module_path(&self, module: &[String], base_dir: &Path) -> Option<PathBuf> {
        if module.is_empty() {
            return None;
        }
        // 相对路径导入：import "path/to/file.px"（module 为单元素且含 '/' 或 '.px' 后缀）
        if module.len() == 1 && (module[0].contains('/') || module[0].contains(".px")) {
            let rel = Path::new(&module[0]);
            let p = if rel.is_absolute() {
                rel.to_path_buf()
            } else {
                base_dir.join(rel)
            };
            if p.is_file() {
                return Some(p);
            }
            return None;
        }
        // std.*
        if module[0] == "std" {
            if module.len() < 2 {
                return None;
            }
            let dir = self.stdlib_dir.clone()?;
            // std.a.b → stdlib/a/b.px
            let mut p = dir;
            for seg in &module[1..] {
                p.push(seg);
            }
            let f = p.with_extension("px");
            if f.is_file() {
                return Some(f);
            }
            // std.a → stdlib/a/mod.px
            let m = p.join("mod.px");
            if m.is_file() {
                return Some(m);
            }
            return None;
        }
        // 用户点分包：先搜当前模块所在目录（递归 import），再搜 search_paths（主目录 → .px_modules → 依赖子目录）
        for base in std::iter::once(base_dir).chain(self.search_paths.iter().map(|p| p.as_path())) {
            let mut p = base.to_path_buf();
            for seg in module {
                p.push(seg);
            }
            let f = p.with_extension("px"); // foo/bar.px
            if f.is_file() {
                return Some(f);
            }
            let m = p.join("mod.px"); // foo/bar/mod.px
            if m.is_file() {
                return Some(m);
            }
            // foo.px（单段模块）
            let single = base.join(format!("{}.px", module[0]));
            if module.len() == 1 && single.is_file() {
                return Some(single);
            }
        }
        None
    }
}

#[cfg(test)]
mod tests {
    use super::*;
    use std::fs;

    fn tmpdir() -> PathBuf {
        let d = std::env::temp_dir().join(format!("px_module_test_{}", std::process::id()));
        let _ = fs::remove_dir_all(&d);
        fs::create_dir_all(&d).unwrap();
        d
    }

    fn parse(src: &str) -> Program {
        let tokens = Lexer::new(src).tokenize().unwrap();
        Parser::new(tokens).parse_program().unwrap()
    }

    #[test]
    fn test_resolve_std_module() {
        // std.collections 应被合并（stdlib 目录存在）
        let prog = parse("import std.collections\ndef main():\n    print(unique([1,2,2,3]))\n");
        let mut r = ModuleResolver::new(Path::new("/tmp"));
        let merged = r.resolve(prog);
        let has_unique = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "unique")
        });
        assert!(has_unique, "std.collections 的 unique 应被合并");
    }

    #[test]
    fn test_resolve_user_module_dotted() {
        let d = tmpdir();
        fs::create_dir_all(d.join("mylib")).unwrap();
        fs::write(d.join("mylib/geom.px"), "def area(w, h):\n    return w * h\n").unwrap();
        let prog = parse("import mylib.geom\ndef main():\n    print(area(2, 3))\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        let has_area = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "area")
        });
        assert!(has_area, "mylib.geom 的 area 应被合并");
        let _ = fs::remove_dir_all(&d);
    }

    #[test]
    fn test_resolve_user_module_single_file() {
        let d = tmpdir();
        fs::write(d.join("utils.px"), "def twice(x):\n    return x * 2\n").unwrap();
        let prog = parse("import utils\ndef main():\n    print(twice(21))\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        let has_twice = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "twice")
        });
        assert!(has_twice, "utils.px 的 twice 应被合并");
        let _ = fs::remove_dir_all(&d);
    }

    #[test]
    fn test_resolve_relative_path() {
        let d = tmpdir();
        fs::create_dir_all(d.join("lib")).unwrap();
        fs::write(d.join("lib/util.px"), "def greet(name):\n    return \"hi \" + name\n").unwrap();
        let prog = parse("import \"lib/util.px\"\ndef main():\n    print(greet(\"px\"))\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        let has_greet = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "greet")
        });
        assert!(has_greet, "相对路径 lib/util.px 的 greet 应被合并");
        let _ = fs::remove_dir_all(&d);
    }

    #[test]
    fn test_resolve_recursive_import() {
        // inner.px import base.px；main import inner → base 也应合并
        let d = tmpdir();
        fs::write(d.join("base.px"), "def base_fn():\n    return 1\n").unwrap();
        fs::write(d.join("inner.px"), "import base\ndef inner_fn():\n    return base_fn() + 1\n").unwrap();
        let prog = parse("import inner\ndef main():\n    print(inner_fn())\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        let has_base = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "base_fn")
        });
        let has_inner = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "inner_fn")
        });
        assert!(has_base && has_inner, "递归 import 应合并 base_fn 与 inner_fn");
        let _ = fs::remove_dir_all(&d);
    }

    #[test]
    fn test_resolve_from_import_selective() {
        // from mylib import area：只合并 area，不合并其它定义
        let d = tmpdir();
        fs::create_dir_all(d.join("mylib")).unwrap();
        fs::write(
            d.join("mylib/geom.px"),
            "def area(w, h):\n    return w * h\n\ndef perimeter(w, h):\n    return 2 * (w + h)\n",
        )
        .unwrap();
        let prog = parse("from mylib.geom import area\ndef main():\n    print(area(2, 3))\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        let has_area = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "area")
        });
        let has_perimeter = merged.items.iter().any(|s| {
            matches!(s, Stmt::FuncDef { name, .. } if name == "perimeter")
        });
        assert!(has_area, "from import 应合并 area");
        assert!(!has_perimeter, "from import 不应合并未列出的 perimeter");
        let _ = fs::remove_dir_all(&d);
    }

    #[test]
    fn test_resolve_dedup_and_user_priority() {
        let d = tmpdir();
        // 用户定义与 std 同名：用户应生效（合并后用户 def 在后面覆盖）
        fs::write(d.join("mine.px"), "def unique(x):\n    return [\"user\"]\n").unwrap();
        let prog = parse("import std.collections\nimport mine\ndef main():\n    print(unique([1]))\n");
        let mut r = ModuleResolver::new(&d);
        let merged = r.resolve(prog);
        // unique 定义应出现两次（std.collections 一次 + 用户一次），用户在后覆盖
        let unique_pos: Vec<usize> = merged
            .items
            .iter()
            .enumerate()
            .filter(|(_, s)| matches!(s, Stmt::FuncDef { name, .. } if name == "unique"))
            .map(|(i, _)| i)
            .collect();
        assert_eq!(unique_pos.len(), 2, "std 与用户同名 def 都应保留（用户优先靠执行序）");
        let _ = fs::remove_dir_all(&d);
    }
}
