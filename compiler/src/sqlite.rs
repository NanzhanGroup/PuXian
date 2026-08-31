//! M28 P1：SQLite 绑定（清歌场景 B：registry / knowledge / 任务库）
//!
//! API（Rust 解释器 / C 编译双模式一致）：
//! - `sqlite_open(path) -> int`：打开/创建数据库（":memory:" 内存库）；返回句柄
//! - `sqlite_exec(db, sql) -> int`：执行无结果 SQL（DDL/DML），返回受影响行数
//! - `sqlite_query(db, sql[, params]) -> list[dict]`：查询，返回结果集（全部行）
//!   params：list → `?` 位置绑定；dict → `:name`/`$name`/`@name` 命名绑定
//! - `sqlite_close(db) -> bool`：关闭连接
//! - `sqlite_escape(s) -> str`：字符串字面量转义（拼接 SQL 防注入）
//!
//! 类型映射：NULL→null / INTEGER→int / REAL→float / TEXT→str / BLOB→bytes
//! 句柄为自增 id，内部注册表持有连接；Rust 端 rusqlite（bundled 编译 sqlite3.c），
//! C 端 runtime_sqlite.c 用同一 sqlite3 amalgamation → 双模式行为一致。

use std::collections::HashMap;
use std::sync::atomic::{AtomicI64, Ordering};
use std::sync::{Mutex, OnceLock};

use crate::value::Value;

static SQLITE_SEQ: AtomicI64 = AtomicI64::new(0);
static SQLITE_DBS: OnceLock<Mutex<HashMap<i64, rusqlite::Connection>>> = OnceLock::new();

fn dbs() -> &'static Mutex<HashMap<i64, rusqlite::Connection>> {
    SQLITE_DBS.get_or_init(|| Mutex::new(HashMap::new()))
}

/// sqlite_open(path) → int handle | null（打开失败返回 null，不抛错）
pub fn sqlite_open(path: &str) -> Value {
    match rusqlite::Connection::open(path) {
        Ok(conn) => {
            let id = SQLITE_SEQ.fetch_add(1, Ordering::SeqCst) + 1;
            dbs().lock().unwrap().insert(id, conn);
            Value::Int(id)
        }
        Err(e) => {
            eprintln!("[sqlite] 打开失败 {}: {}", path, e);
            Value::Null
        }
    }
}



/// sqlite_exec(db, sql[, params]) → int 受影响行数 | null（执行出错返回 null）
pub fn sqlite_exec(db: i64, sql: &str, params: Option<&Value>) -> Value {
    let map = dbs().lock().unwrap();
    let conn = match map.get(&db) {
        Some(c) => c,
        None => {
            eprintln!("[sqlite] 无效句柄 {}", db);
            return Value::Null;
        }
    };
    match params {
        None => match conn.execute_batch(sql) {
            Ok(()) => Value::Int(0),
            Err(e) => {
                eprintln!("[sqlite] exec 出错: {}", e);
                Value::Null
            }
        },
        Some(p) => {
            let mut stmt = match conn.prepare(sql) {
                Ok(s) => s,
                Err(e) => {
                    eprintln!("[sqlite] prepare 出错: {}", e);
                    return Value::Null;
                }
            };
            if let Err(e) = bind_params(&mut stmt, Some(p)) {
                eprintln!("[sqlite] 参数绑定出错: {}", e);
                return Value::Null;
            }
            match stmt.raw_execute() {
                Ok(n) => Value::Int(n as i64),
                Err(e) => {
                    eprintln!("[sqlite] exec 出错: {}", e);
                    Value::Null
                }
            }
        }
    }
}

/// sqlite_query(db, sql[, params]) → list[dict] | null
pub fn sqlite_query(db: i64, sql: &str, params: Option<&Value>) -> Value {
    let map = dbs().lock().unwrap();
    let conn = match map.get(&db) {
        Some(c) => c,
        None => {
            eprintln!("[sqlite] 无效句柄 {}", db);
            return Value::Null;
        }
    };
    let mut stmt = match conn.prepare(sql) {
        Ok(s) => s,
        Err(e) => {
            eprintln!("[sqlite] prepare 出错: {}", e);
            return Value::Null;
        }
    };
    // 参数绑定
    let bind_res = bind_params(&mut stmt, params);
    if let Err(e) = bind_res {
        eprintln!("[sqlite] 参数绑定出错: {}", e);
        return Value::Null;
    }
    let col_names: Vec<String> = stmt
        .column_names()
        .iter()
        .map(|s| s.to_string())
        .collect();
    // 用 raw_query：参数已通过 raw_bind_parameter 绑定（位置 ? 或命名 :name）
    let mut rows = stmt.raw_query();
    let mut out = Vec::new();
    loop {
        match rows.next() {
            Ok(Some(row)) => {
                let mut m = HashMap::new();
                let mut ok = true;
                for (i, name) in col_names.iter().enumerate() {
                    match row.get_ref(i).and_then(|r| row_to_value(r)) {
                        Ok(v) => {
                            m.insert(name.clone(), v);
                        }
                        Err(e) => {
                            eprintln!("[sqlite] 行读取出错: {}", e);
                            ok = false;
                            break;
                        }
                    }
                }
                if ok {
                    out.push(Value::new_dict(m));
                }
            }
            Ok(None) => break,
            Err(e) => {
                eprintln!("[sqlite] 行读取出错: {}", e);
                return Value::Null;
            }
        }
    }
    Value::new_list(out)
}

/// 绑定参数：list → ? 位置；dict → 命名。不匹配 → Err。
fn bind_params(
    stmt: &mut rusqlite::Statement,
    params: Option<&Value>,
) -> Result<(), String> {
    match params {
        None => Ok(()),
        Some(Value::List(l)) => {
            let items = l.lock().unwrap().clone();
            let expected = stmt.parameter_count();
            if items.len() != expected {
                return Err(format!(
                    "参数数量不匹配：SQL 需要 {} 个占位符，给了 {} 个",
                    expected,
                    items.len()
                ));
            }
            for (i, v) in items.iter().enumerate() {
                stmt.raw_bind_parameter(i + 1, to_rusqlite(v))
                    .map_err(|e| e.to_string())?;
            }
            Ok(())
        }
        Some(Value::Dict(d)) => {
            let d = d.lock().unwrap();
            for (k, v) in d.iter() {
                // rusqlite 命名绑定需要完整前缀（:name / $name / @name / ?name）
                let name = if k.starts_with(':') || k.starts_with('$') || k.starts_with('@') || k.starts_with('?') {
                    k.clone()
                } else {
                    format!(":{}", k)
                };
                stmt.raw_bind_parameter(name.as_str(), to_rusqlite(v))
                    .map_err(|e| e.to_string())?;
            }
            Ok(())
        }
        Some(_) => Err("params 需要 list 或 dict".to_string()),
    }
}

/// Value → rusqlite 参数值
fn to_rusqlite(v: &Value) -> rusqlite::types::Value {
    match v {
        Value::Null => rusqlite::types::Value::Null,
        Value::Int(i) => rusqlite::types::Value::Integer(*i),
        Value::Float(f) => rusqlite::types::Value::Real(*f),
        Value::Str(s) => rusqlite::types::Value::Text(s.clone()),
        Value::Bytes(b) => rusqlite::types::Value::Blob(b.clone()),
        Value::Bool(b) => rusqlite::types::Value::Integer(if *b { 1 } else { 0 }),
        _ => rusqlite::types::Value::Text(v.to_string()),
    }
}

/// SQLite 行值 → Value
fn row_to_value(v: rusqlite::types::ValueRef) -> Result<Value, rusqlite::Error> {
    match v {
        rusqlite::types::ValueRef::Null => Ok(Value::Null),
        rusqlite::types::ValueRef::Integer(i) => Ok(Value::Int(i)),
        rusqlite::types::ValueRef::Real(f) => Ok(Value::Float(f)),
        rusqlite::types::ValueRef::Text(t) => {
            Ok(Value::Str(String::from_utf8_lossy(t).to_string()))
        }
        rusqlite::types::ValueRef::Blob(b) => Ok(Value::Bytes(b.to_vec())),
    }
}

/// sqlite_close(db) → bool
pub fn sqlite_close(db: i64) -> bool {
    dbs().lock().unwrap().remove(&db).is_some()
}

/// sqlite_escape(s) → str：SQL 字符串字面量转义（' → ''）
pub fn sqlite_escape(s: &str) -> String {
    s.replace('\'', "''")
}

/// sqlite_last_insert_rowid(db) → int：最近一次 INSERT 的自增 rowid
pub fn sqlite_last_insert_rowid(db: i64) -> i64 {
    let map = dbs().lock().unwrap();
    match map.get(&db) {
        Some(c) => c.last_insert_rowid(),
        None => -1,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_crud() {
        let db = sqlite_open(":memory:");
        let Value::Int(id) = db else {
            panic!("open failed");
        };
        // 建表 + 插入
        assert_eq!(
            sqlite_exec(id, "CREATE TABLE t(id INTEGER PRIMARY KEY, name TEXT, score REAL, data BLOB, flag INTEGER)", None),
            Value::Int(0)
        );
        sqlite_exec(
            id,
            "INSERT INTO t(name, score, data, flag) VALUES ('alice', 90.5, x'0102', 1), ('bob', 88.0, x'FF', 0)",
            None,
        );
        // 查询全部
        let Value::List(rows) = sqlite_query(
            id,
            "SELECT id, name, score, data, flag FROM t ORDER BY id",
            None,
        ) else {
            panic!("query failed");
        };
        let rows = rows.lock().unwrap();
        assert_eq!(rows.len(), 2);
        let r0 = match &rows[0] {
            Value::Dict(d) => d.lock().unwrap(),
            _ => panic!("row 应为 dict"),
        };
        assert_eq!(r0.get("name").unwrap().to_string(), "alice");
        assert_eq!(r0.get("score").unwrap().to_string(), "90.5");
        assert!(matches!(r0.get("data").unwrap(), Value::Bytes(b) if b == &vec![1, 2]));
        drop(r0);
        // 参数绑定（位置）
        let Value::List(rows) = sqlite_query(id, "SELECT name FROM t WHERE score >= ?", Some(&Value::new_list(vec![Value::Int(89)])))
        else {
            panic!();
        };
        let rows = rows.lock().unwrap();
        assert_eq!(rows.len(), 1);
        assert_eq!(rows[0].to_string(), "{name: alice}");
        drop(rows);
        // 命名绑定
        let mut pm = HashMap::new();
        pm.insert("n".to_string(), Value::Str("bob".to_string()));
        let Value::List(rows) = sqlite_query(
            id,
            "SELECT name FROM t WHERE name = :n",
            Some(&Value::new_dict(pm)),
        )
        else {
            panic!();
        };
        assert_eq!(rows.lock().unwrap().len(), 1);
        // last_insert_rowid
        assert_eq!(sqlite_last_insert_rowid(id), 2);
        assert!(sqlite_close(id));
    }

    #[test]
    fn test_escape() {
        assert_eq!(sqlite_escape("it's"), "it''s");
    }
}
