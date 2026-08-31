//! 普贤 (PuXian) 作用域环境
//! 链式作用域：子环境可读父环境，赋值就近

use std::sync::{Arc, Mutex};
use std::collections::HashMap;

use crate::value::Value;

pub type EnvRef = Arc<Mutex<Env>>;

#[derive(Debug)]
pub struct Env {
    vars: HashMap<String, Value>,
    parent: Option<EnvRef>,
}

impl Env {
    pub fn new(parent: Option<EnvRef>) -> EnvRef {
        Arc::new(Mutex::new(Env {
            vars: HashMap::new(),
            parent,
        }))
    }

    pub fn define(&mut self, name: &str, value: Value) {
        self.vars.insert(name.to_string(), value);
    }

    pub fn get(&self, name: &str) -> Option<Value> {
        if let Some(v) = self.vars.get(name) {
            Some(v.clone())
        } else if let Some(p) = &self.parent {
            p.lock().unwrap().get(name)
        } else {
            None
        }
    }

    /// 就近赋值：找到第一个包含该名字的作用域并写入；找不到返回 false
    pub fn set(&mut self, name: &str, value: Value) -> bool {
        if self.vars.contains_key(name) {
            self.vars.insert(name.to_string(), value);
            true
        } else if let Some(p) = &self.parent {
            p.lock().unwrap().set(name, value)
        } else {
            false
        }
    }

    /// M25 GC：枚举本环境全部变量（追踪式 GC 遍历闭包环境边用）
    pub fn iter_vars(&self) -> impl Iterator<Item = (&String, &Value)> {
        self.vars.iter()
    }

    /// M25 GC：清空本环境全部变量（断闭包循环用）
    pub fn clear_vars(&mut self) {
        self.vars.clear();
    }
}
