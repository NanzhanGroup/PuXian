//! 包管理器（M9）：px pkg 子命令
//!
//! 支持：
//! - `px pkg init [--name NAME]`         初始化项目（生成 px.toml）
//! - `px pkg add <path> [--name ALIAS]`  添加本地依赖（复制到 .px_modules/ 并记录）
//! - `px pkg install`                    按 px.toml 安装依赖（本地路径复制，幂等）
//! - `px pkg list`                       列出已安装依赖
//! - `px pkg remove <name>`              移除依赖
//!
//! px.toml 格式（零依赖手写解析，不引入 toml crate）：
//! ```toml
//! [package]
//! name = "myapp"
//! version = "0.1.0"
//!
//! [dependencies]
//! mylib = "../mylib"
//! ```

use std::collections::BTreeMap;
use std::fs;
use std::path::{Path, PathBuf};

pub const PKG_FILE: &str = "px.toml";
pub const MODULES_DIR: &str = ".px_modules";

/// 解析 px.toml → (package_name, version, dependencies)
/// 返回值：name, version, deps(name → path)
pub fn parse_manifest(src: &str) -> (String, String, BTreeMap<String, String>) {
    let mut name = String::new();
    let mut version = String::new();
    let mut deps: BTreeMap<String, String> = BTreeMap::new();
    let mut section = String::new();
    for line in src.lines() {
        let line = line.trim();
        if line.is_empty() || line.starts_with('#') {
            continue;
        }
        if line.starts_with('[') && line.ends_with(']') {
            section = line[1..line.len() - 1].trim().to_string();
            continue;
        }
        if let Some((k, v)) = line.split_once('=') {
            let k = k.trim().to_string();
            let v = v.trim().trim_matches('"').to_string();
            match section.as_str() {
                "package" => {
                    if k == "name" {
                        name = v;
                    } else if k == "version" {
                        version = v;
                    }
                }
                "dependencies" => {
                    deps.insert(k, v);
                }
                _ => {}
            }
        }
    }
    (name, version, deps)
}

/// 序列化 px.toml
pub fn serialize_manifest(name: &str, version: &str, deps: &BTreeMap<String, String>) -> String {
    let mut s = String::new();
    s.push_str("[package]\n");
    s.push_str(&format!("name = \"{}\"\n", name));
    s.push_str(&format!("version = \"{}\"\n", version));
    if !deps.is_empty() {
        s.push_str("\n[dependencies]\n");
        for (k, v) in deps {
            s.push_str(&format!("{} = \"{}\"\n", k, v));
        }
    }
    s
}

/// 读取项目清单（不存在返回 None）
fn read_manifest(dir: &Path) -> Option<(String, String, BTreeMap<String, String>)> {
    let path = dir.join(PKG_FILE);
    let src = fs::read_to_string(&path).ok()?;
    Some(parse_manifest(&src))
}

/// 复制目录（递归）
fn copy_dir(src: &Path, dst: &Path) -> std::io::Result<()> {
    fs::create_dir_all(dst)?;
    for entry in fs::read_dir(src)? {
        let entry = entry?;
        let ft = entry.file_type()?;
        let from = entry.path();
        let to = dst.join(entry.file_name());
        if ft.is_dir() {
            copy_dir(&from, &to)?;
        } else {
            fs::copy(&from, &to)?;
        }
    }
    Ok(())
}

/// 从源路径计算依赖名：目录名 / 文件名去 .px
fn dep_name_from_path(src: &Path, alias: Option<&str>) -> String {
    if let Some(a) = alias {
        return a.to_string();
    }
    let base = if src.is_dir() {
        src.file_name().map(|s| s.to_string_lossy().to_string())
    } else {
        src.file_stem().map(|s| s.to_string_lossy().to_string())
    };
    base.unwrap_or_else(|| "dep".to_string())
}

/// px pkg init：生成 px.toml
pub fn pkg_init(dir: &Path, name: Option<&str>) -> Result<String, String> {
    let manifest_path = dir.join(PKG_FILE);
    if manifest_path.exists() {
        return Err(format!("{} 已存在，跳过初始化", PKG_FILE));
    }
    let pkg_name = name
        .map(|s| s.to_string())
        .unwrap_or_else(|| {
            dir.file_name()
                .map(|s| s.to_string_lossy().to_string())
                .unwrap_or_else(|| "px_app".to_string())
        });
    let content = serialize_manifest(&pkg_name, "0.1.0", &BTreeMap::new());
    fs::write(&manifest_path, content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    Ok(format!("已初始化 {}（包名: {}）", PKG_FILE, pkg_name))
}

/// px pkg add <path>：复制依赖到 .px_modules/<name> 并写入 px.toml
pub fn pkg_add(dir: &Path, src_path: &str, alias: Option<&str>) -> Result<String, String> {
    let src = PathBuf::from(src_path);
    if !src.exists() {
        return Err(format!("依赖路径不存在: {}", src_path));
    }
    let name = dep_name_from_path(&src, alias);
    let (pkg_name, version, mut deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;

    // 复制到 .px_modules/<name>
    let modules_dir = dir.join(MODULES_DIR);
    let dst = modules_dir.join(&name);
    fs::create_dir_all(&modules_dir).map_err(|e| format!("创建 {} 失败: {}", MODULES_DIR, e))?;
    if src.is_dir() {
        copy_dir(&src, &dst).map_err(|e| format!("复制目录失败: {}", e))?;
    } else {
        fs::create_dir_all(&dst).map_err(|e| format!("创建目录失败: {}", e))?;
        fs::copy(&src, dst.join(src.file_name().unwrap())).map_err(|e| format!("复制文件失败: {}", e))?;
    }

    // 写入 px.toml
    deps.insert(name.clone(), src_path.to_string());
    let content = serialize_manifest(&pkg_name, &version, &deps);
    fs::write(dir.join(PKG_FILE), content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    Ok(format!("已添加依赖 {} <- {}", name, src_path))
}

/// px pkg install：按 px.toml 安装依赖（幂等：已存在则跳过）
pub fn pkg_install(dir: &Path) -> Result<String, String> {
    let (pkg_name, version, deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    if deps.is_empty() {
        return Ok("没有依赖需要安装".to_string());
    }
    let modules_dir = dir.join(MODULES_DIR);
    fs::create_dir_all(&modules_dir).map_err(|e| format!("创建 {} 失败: {}", MODULES_DIR, e))?;
    let mut installed = Vec::new();
    for (name, path_str) in &deps {
        let dst = modules_dir.join(name);
        if dst.exists() {
            installed.push(format!("{}（已存在，跳过）", name));
            continue;
        }
        let src = PathBuf::from(path_str);
        if !src.exists() {
            return Err(format!("依赖 {} 的源路径不存在: {}", name, path_str));
        }
        if src.is_dir() {
            copy_dir(&src, &dst).map_err(|e| format!("安装 {} 失败: {}", name, e))?;
        } else {
            fs::create_dir_all(&dst).map_err(|e| format!("创建目录失败: {}", e))?;
            fs::copy(&src, dst.join(src.file_name().unwrap())).map_err(|e| format!("复制失败: {}", e))?;
        }
        installed.push(name.clone());
    }
    Ok(format!(
        "已按 {}（包名 {} v{}）安装 {} 个依赖：{}",
        PKG_FILE,
        pkg_name,
        version,
        installed.len(),
        installed.join(", ")
    ))
}

/// px pkg list：列出依赖
pub fn pkg_list(dir: &Path) -> Result<String, String> {
    let (pkg_name, version, deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    let modules_dir = dir.join(MODULES_DIR);
    let mut out = String::new();
    out.push_str(&format!("项目: {} v{}\n", pkg_name, version));
    out.push_str(&format!("清单: {} 个依赖\n", deps.len()));
    for (name, path_str) in &deps {
        let installed = modules_dir.join(name).exists();
        out.push_str(&format!(
            "  - {} <- {} [{}]\n",
            name,
            path_str,
            if installed { "已安装" } else { "未安装" }
        ));
    }
    if deps.is_empty() {
        out.push_str("  （空）\n");
    }
    // 列出 .px_modules 内容
    if modules_dir.is_dir() {
        out.push_str(&format!("{} 目录:\n", MODULES_DIR));
        if let Ok(entries) = fs::read_dir(&modules_dir) {
            for e in entries.flatten() {
                let name = e.file_name().to_string_lossy().to_string();
                let t = if e.path().is_dir() { "dir" } else { "file" };
                out.push_str(&format!("  - {} [{}]\n", name, t));
            }
        }
    } else {
        out.push_str(&format!("{} 目录不存在（尚未安装）\n", MODULES_DIR));
    }
    Ok(out)
}

/// px pkg remove <name>：移除依赖
pub fn pkg_remove(dir: &Path, name: &str) -> Result<String, String> {
    let (pkg_name, version, mut deps) =
        read_manifest(dir).ok_or_else(|| format!("未找到 {}，请先运行 px pkg init", PKG_FILE))?;
    if !deps.contains_key(name) {
        return Err(format!("依赖 {} 不在清单中", name));
    }
    deps.remove(name);
    let content = serialize_manifest(&pkg_name, &version, &deps);
    fs::write(dir.join(PKG_FILE), content).map_err(|e| format!("写入 {} 失败: {}", PKG_FILE, e))?;
    let dst = dir.join(MODULES_DIR).join(name);
    if dst.exists() {
        fs::remove_dir_all(&dst).map_err(|e| format!("删除 {} 失败: {}", name, e))?;
    }
    Ok(format!("已移除依赖 {}", name))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn tmpdir() -> PathBuf {
        let d = std::env::temp_dir().join(format!("px_pkg_test_{}", std::process::id()));
        let _ = fs::remove_dir_all(&d);
        fs::create_dir_all(&d).unwrap();
        d
    }

    #[test]
    fn test_manifest_roundtrip() {
        let src = "[package]\nname = \"demo\"\nversion = \"0.2.0\"\n\n[dependencies]\nmylib = \"../mylib\"\nfoo = \"./foo\"\n";
        let (n, v, deps) = parse_manifest(src);
        assert_eq!(n, "demo");
        assert_eq!(v, "0.2.0");
        assert_eq!(deps.get("mylib").unwrap(), "../mylib");
        assert_eq!(deps.get("foo").unwrap(), "./foo");
        let s2 = serialize_manifest(&n, &v, &deps);
        let (n2, v2, deps2) = parse_manifest(&s2);
        assert_eq!(n2, n);
        assert_eq!(v2, v);
        assert_eq!(deps2, deps);
    }

    #[test]
    fn test_pkg_init_and_add_list_remove() {
        let d = tmpdir();
        // 源依赖
        let srcdir = d.join("_src_lib");
        fs::create_dir_all(srcdir.join("sub")).unwrap();
        fs::write(srcdir.join("core.px"), "def core():\n    return 1\n").unwrap();
        fs::write(srcdir.join("sub/extra.px"), "def extra():\n    return 2\n").unwrap();

        let pkg = pkg_init(&d, Some("demo")).unwrap();
        assert!(pkg.contains("demo"));

        let add = pkg_add(&d, srcdir.to_str().unwrap(), Some("mylib")).unwrap();
        assert!(add.contains("mylib"));

        // 校验复制
        assert!(d.join(MODULES_DIR).join("mylib/core.px").exists());
        assert!(d.join(MODULES_DIR).join("mylib/sub/extra.px").exists());

        let list = pkg_list(&d).unwrap();
        assert!(list.contains("mylib"));

        let rm = pkg_remove(&d, "mylib").unwrap();
        assert!(rm.contains("已移除"));
        assert!(!d.join(MODULES_DIR).join("mylib").exists());
        let _ = fs::remove_dir_all(&d);
    }
}
