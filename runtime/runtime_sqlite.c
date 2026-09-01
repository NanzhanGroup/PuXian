// 普贤 (PuXian) C 运行时 — SQLite 绑定（M28）
// 语言层 API（与解释器 sqlite.rs 双模式一致）：
//   sqlite_open(path) -> int|null       打开/创建数据库（":memory:" 内存库）
//   sqlite_exec(db, sql) -> int|null    执行无结果 SQL（DDL/DML），返回受影响行数
//   sqlite_query(db, sql[, params]) -> list[dict]|null  查询（params: list→? / dict→:name）
//   sqlite_close(db) -> bool            关闭连接
//   sqlite_escape(s) -> str             SQL 字符串字面量转义（' → ''）
//   sqlite_last_insert_rowid(db) -> int 最近 INSERT 的自增 rowid
// 类型映射：NULL→null / INTEGER→int / REAL→float / TEXT→str / BLOB→bytes
// 句柄为自增 id → sqlite3* 映射表（互斥锁保护，可跨线程）。
#define _GNU_SOURCE
#include "runtime.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "sqlite3.h"

#define MAX_SQLITE_DBS 128
static sqlite3* g_sqlite_dbs[MAX_SQLITE_DBS];
static int64_t g_sqlite_next_id = 0;
static pthread_mutex_t g_sqlite_mu = PTHREAD_MUTEX_INITIALIZER;

static sqlite3* sqlite_get_db(int64_t id) {
    sqlite3* db = NULL;
    pthread_mutex_lock(&g_sqlite_mu);
    if (id > 0 && id <= MAX_SQLITE_DBS) db = g_sqlite_dbs[id - 1];
    pthread_mutex_unlock(&g_sqlite_mu);
    return db;
}

static int sqlite_alloc_db(sqlite3* db) {
    pthread_mutex_lock(&g_sqlite_mu);
    int slot = -1;
    for (int i = 0; i < MAX_SQLITE_DBS; i++) {
        if (!g_sqlite_dbs[i]) { slot = i; break; }
    }
    if (slot < 0) {
        pthread_mutex_unlock(&g_sqlite_mu);
        sqlite3_close(db);
        return -1;
    }
    g_sqlite_dbs[slot] = db;
    if ((int64_t)(slot + 1) > g_sqlite_next_id) g_sqlite_next_id = slot + 1;
    pthread_mutex_unlock(&g_sqlite_mu);
    return slot + 1;
}

// sqlite_open(path) → int|null
LXValue bi_sqlite_open(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1) px_error("sqlite_open 需要 (path) 参数");
    if (args[0].type != PX_STR) px_error("sqlite_open 的 path 需要字符串");
    sqlite3* db = NULL;
    if (sqlite3_open(args[0].as.obj->as.str.data, &db) != SQLITE_OK) {
        if (db) { eprintf_hint: sqlite3_close(db); }
        return px_null();
    }
    int id = sqlite_alloc_db(db);
    return id > 0 ? px_int(id) : px_null();
}

// 前向声明（定义在下方）
static int sqlite_bind_params(sqlite3_stmt* stmt, LXValue params);

// sqlite_exec(db, sql[, params]) → int 受影响行数 | null
LXValue bi_sqlite_exec(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("sqlite_exec 需要 (db, sql[, params]) 参数");
    if (args[0].type != PX_INT || args[1].type != PX_STR) px_error("sqlite_exec 参数类型错误");
    sqlite3* db = sqlite_get_db(args[0].as.i);
    if (!db) { fprintf(stderr, "[sqlite] 无效句柄 %lld\n", (long long)args[0].as.i); return px_null(); }
    if (nargs == 2) {
        char* err = NULL;
        if (sqlite3_exec(db, args[1].as.obj->as.str.data, NULL, NULL, &err) != SQLITE_OK) {
            fprintf(stderr, "[sqlite] exec 出错: %s\n", err ? err : "unknown");
            if (err) sqlite3_free(err);
            return px_null();
        }
        return px_int(0);
    }
    // 带参数：prepare + 绑定 + step
    sqlite3_stmt* stmt = NULL;
    if (sqlite3_prepare_v2(db, args[1].as.obj->as.str.data, -1, &stmt, NULL) != SQLITE_OK) {
        fprintf(stderr, "[sqlite] prepare 出错: %s\n", sqlite3_errmsg(db));
        return px_null();
    }
    if (!sqlite_bind_params(stmt, args[2])) {
        sqlite3_finalize(stmt);
        return px_null();
    }
    int rc = sqlite3_step(stmt);
    if (rc != SQLITE_DONE && rc != SQLITE_ROW) {
        fprintf(stderr, "[sqlite] exec 出错: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return px_null();
    }
    int affected = sqlite3_changes(db);
    sqlite3_finalize(stmt);
    return px_int(affected);
}

// Value → SQL 绑定值
static void sqlite_bind_value(sqlite3_stmt* stmt, int idx, LXValue v) {
    switch (v.type) {
        case PX_NULL: sqlite3_bind_null(stmt, idx); break;
        case PX_BOOL: sqlite3_bind_int(stmt, idx, v.as.b ? 1 : 0); break;
        case PX_INT:  sqlite3_bind_int64(stmt, idx, v.as.i); break;
        case PX_FLOAT: sqlite3_bind_double(stmt, idx, v.as.f); break;
        case PX_STR:  sqlite3_bind_text(stmt, idx, v.as.obj->as.str.data, v.as.obj->as.str.len, SQLITE_TRANSIENT); break;
        case PX_BYTES: sqlite3_bind_blob(stmt, idx, v.as.obj->as.str.data, v.as.obj->as.str.len, SQLITE_TRANSIENT); break;
        default: {
            char* s = px_to_string(v);
            sqlite3_bind_text(stmt, idx, s, -1, SQLITE_TRANSIENT);
            break;
        }
    }
}

// 绑定参数：list → ? 位置（1..n）；dict → 命名
static int sqlite_bind_params(sqlite3_stmt* stmt, LXValue params) {
    if (params.type == PX_NULL) return 1;
    int pc = sqlite3_bind_parameter_count(stmt);
    if (params.type == PX_LIST) {
        int n = px_len(params);
        if (n != pc) {
            fprintf(stderr, "[sqlite] 参数数量不匹配：SQL 需要 %d 个占位符，给了 %d 个\n", pc, n);
            return 0;
        }
        for (int i = 0; i < n; i++) {
            LXValue item = px_index(params, px_int(i));
            sqlite_bind_value(stmt, i + 1, item);
        }
        return 1;
    } else if (params.type == PX_DICT) {
        LXObject* o = params.as.obj;
        for (int i = 0; i < o->as.dict.len; i++) {
            const char* k = o->as.dict.keys[i];
            // 完整前缀（:name / $name / @name / ?name）；否则自动加 :
            char full[128];
            const char* name = k;
            if (k[0] != ':' && k[0] != '$' && k[0] != '@' && k[0] != '?') {
                snprintf(full, sizeof(full), ":%s", k);
                name = full;
            }
            int idx = sqlite3_bind_parameter_index(stmt, name);
            if (idx == 0) {
                fprintf(stderr, "[sqlite] 无效命名参数: %s\n", name);
                return 0;
            }
            sqlite_bind_value(stmt, idx, o->as.dict.vals[i]);
        }
        return 1;
    }
    fprintf(stderr, "[sqlite] params 需要 list 或 dict\n");
    return 0;
}

// sqlite_query(db, sql[, params]) → list[dict]|null
LXValue bi_sqlite_query(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs < 2 || nargs > 3) px_error("sqlite_query 需要 (db, sql[, params]) 参数");
    if (args[0].type != PX_INT || args[1].type != PX_STR) px_error("sqlite_query 参数类型错误");
    sqlite3* db = sqlite_get_db(args[0].as.i);
    if (!db) { fprintf(stderr, "[sqlite] 无效句柄 %lld\n", (long long)args[0].as.i); return px_null(); }
    LXValue params = (nargs == 3) ? args[2] : px_null();
    sqlite3_stmt* stmt = NULL;
    const char* tail = NULL;
    if (sqlite3_prepare_v2(db, args[1].as.obj->as.str.data, -1, &stmt, &tail) != SQLITE_OK) {
        fprintf(stderr, "[sqlite] prepare 出错: %s\n", sqlite3_errmsg(db));
        return px_null();
    }
    if (!sqlite_bind_params(stmt, params)) {
        sqlite3_finalize(stmt);
        return px_null();
    }
    int ncols = sqlite3_column_count(stmt);
    LXValue out = px_list(8);
    int rc;
    while ((rc = sqlite3_step(stmt)) == SQLITE_ROW) {
        LXValue row = px_dict();
        for (int i = 0; i < ncols; i++) {
            const char* cname = sqlite3_column_name(stmt, i);
            if (!cname) cname = "";
            LXValue v;
            switch (sqlite3_column_type(stmt, i)) {
                case SQLITE_NULL: v = px_null(); break;
                case SQLITE_INTEGER: v = px_int(sqlite3_column_int64(stmt, i)); break;
                case SQLITE_FLOAT: v = px_float(sqlite3_column_double(stmt, i)); break;
                case SQLITE_TEXT: {
                    const unsigned char* t = sqlite3_column_text(stmt, i);
                    int tl = sqlite3_column_bytes(stmt, i);
                    v = px_str_len((const char*)(t ? t : (const unsigned char*)""), tl);
                    break;
                }
                case SQLITE_BLOB: {
                    const void* b = sqlite3_column_blob(stmt, i);
                    int bl = sqlite3_column_bytes(stmt, i);
                    v = px_bytes_len(b ? b : "", bl);
                    break;
                }
                default: v = px_null(); break;
            }
            px_dict_set(row, cname, v);
        }
        px_list_push(out, row);
    }
    if (rc != SQLITE_DONE) {
        fprintf(stderr, "[sqlite] 查询出错: %s\n", sqlite3_errmsg(db));
        sqlite3_finalize(stmt);
        return px_null();
    }
    sqlite3_finalize(stmt);
    return out;
}

// sqlite_close(db) → bool
LXValue bi_sqlite_close(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("sqlite_close 需要 (db) 参数");
    sqlite3* db = NULL;
    pthread_mutex_lock(&g_sqlite_mu);
    if (args[0].as.i > 0 && args[0].as.i <= MAX_SQLITE_DBS && g_sqlite_dbs[args[0].as.i - 1]) {
        db = g_sqlite_dbs[args[0].as.i - 1];
        g_sqlite_dbs[args[0].as.i - 1] = NULL;
    }
    pthread_mutex_unlock(&g_sqlite_mu);
    if (db) { sqlite3_close(db); return px_bool(true); }
    return px_bool(false);
}

// sqlite_escape(s) → str
LXValue bi_sqlite_escape(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_STR) px_error("sqlite_escape 需要 (s) 参数");
    const char* s = args[0].as.obj->as.str.data;
    int n = 0;
    for (const char* p = s; *p; p++) if (*p == '\'') n++;
    char* out = malloc((size_t)strlen(s) + n + 1);
    char* o = out;
    for (const char* p = s; *p; p++) {
        if (*p == '\'') *o++ = '\'';
        *o++ = *p;
    }
    *o = 0;
    LXValue r = px_str(out);
    free(out);
    return r;
}

// sqlite_last_insert_rowid(db) → int
LXValue bi_sqlite_last_insert_rowid(LXValue* args, int nargs, void* ctx) {
    (void)ctx;
    if (nargs != 1 || args[0].type != PX_INT) px_error("sqlite_last_insert_rowid 需要 (db) 参数");
    sqlite3* db = sqlite_get_db(args[0].as.i);
    return px_int(db ? (int64_t)sqlite3_last_insert_rowid(db) : -1);
}
