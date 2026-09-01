//! M37：S3/MinIO 对象存储客户端（AWS Signature V4）
//! 语言层 API（builtin.rs 分派）：
//!   s3_put(endpoint, bucket, key, data, ak, sk) → bool
//!   s3_get(endpoint, bucket, key, ak, sk) → str|null
//!   s3_delete(endpoint, bucket, key, ak, sk) → bool
//!   s3_list(endpoint, bucket, prefix, ak, sk) → list（对象 key）
//! endpoint 形如 "http://127.0.0.1:9000" 或 "https://s3.amazonaws.com"
//! 签名：AWS4-HMAC-SHA256（region us-east-1 / service s3）；HMAC-SHA256 自实现（crypto.sha256）

use crate::crypto::sha256;
use std::collections::HashMap;

fn hmac_sha256(key: &[u8], data: &[u8]) -> [u8; 32] {
    const BLOCK: usize = 64;
    let mut k = [0u8; BLOCK];
    if key.len() > BLOCK {
        k[..32].copy_from_slice(&sha256(key));
    } else {
        k[..key.len()].copy_from_slice(key);
    }
    let mut ipad = [0x36u8; BLOCK];
    let mut opad = [0x5cu8; BLOCK];
    for i in 0..BLOCK {
        ipad[i] ^= k[i];
        opad[i] ^= k[i];
    }
    let mut inner = Vec::with_capacity(BLOCK + data.len());
    inner.extend_from_slice(&ipad);
    inner.extend_from_slice(data);
    let h1 = sha256(&inner);
    let mut outer = Vec::with_capacity(BLOCK + 32);
    outer.extend_from_slice(&opad);
    outer.extend_from_slice(&h1);
    sha256(&outer)
}

fn hex(data: &[u8]) -> String {
    data.iter().map(|b| format!("{:02x}", b)).collect()
}

/// URI 编码（S3 路径段：非安全字符 %XX；保留 /）
fn uri_encode(s: &str, encode_slash: bool) -> String {
    let mut out = String::new();
    for b in s.bytes() {
        let c = b as char;
        if c.is_ascii_alphanumeric() || matches!(c, '-' | '_' | '.' | '~') || (!encode_slash && c == '/') {
            out.push(c);
        } else {
            out.push_str(&format!("%{:02X}", b));
        }
    }
    out
}

/// SigV4 签名 → Authorization 头值
pub fn sigv4_authorization(
    method: &str,
    host: &str,
    path: &str,
    query: &str,
    headers: &HashMap<String, String>,
    payload_hash: &str,
    access_key: &str,
    secret_key: &str,
    region: &str,
    service: &str,
    amz_date: &str,
    date_stamp: &str,
) -> String {
    let canonical_uri = if path.is_empty() { "/" } else { path };
    let canonical_query = query;
    let mut canonical_headers = String::new();
    let mut signed_headers = String::new();
    let mut hs: Vec<(String, String)> = headers
        .iter()
        .map(|(k, v)| (k.to_lowercase(), v.trim().to_string()))
        .collect();
    // host 必参与签名（即使不在请求头）
    if !hs.iter().any(|(k, _)| k == "host") {
        hs.push(("host".to_string(), host.to_string()));
    }
    hs.sort();
    for (k, v) in &hs {
        canonical_headers.push_str(&format!("{}:{}\n", k, v));
        if !signed_headers.is_empty() {
            signed_headers.push(';');
        }
        signed_headers.push_str(k);
    }
    let canonical_request = format!(
        "{}\n{}\n{}\n{}\n{}\n{}",
        method, canonical_uri, canonical_query, canonical_headers, signed_headers, payload_hash
    );
    let scope = format!("{}/{}/{}/aws4_request", date_stamp, region, service);
    let string_to_sign = format!(
        "AWS4-HMAC-SHA256\n{}\n{}\n{}",
        amz_date,
        scope,
        hex(&sha256(canonical_request.as_bytes()))
    );
    let k_date = hmac_sha256(format!("AWS4{}", secret_key).as_bytes(), date_stamp.as_bytes());
    let k_region = hmac_sha256(&k_date, region.as_bytes());
    let k_service = hmac_sha256(&k_region, service.as_bytes());
    let k_signing = hmac_sha256(&k_service, b"aws4_request");
    let signature = hex(&hmac_sha256(&k_signing, string_to_sign.as_bytes()));
    format!(
        "AWS4-HMAC-SHA256 Credential={}/{}, SignedHeaders={}, Signature={}",
        access_key, scope, signed_headers, signature
    )
}

/// 当前 UTC 时间（amz_date + date_stamp）
pub fn amz_dates() -> (String, String) {
    let now = std::time::SystemTime::now()
        .duration_since(std::time::UNIX_EPOCH)
        .map(|d| d.as_secs() as i64)
        .unwrap_or(0);
    let (y, mo, d) = crate::tztime::civil_from_days(now / 86400);
    let h = (now % 86400) / 3600;
    let mi = (now % 3600) / 60;
    let sec = now % 60;
    let ds = format!("{:04}{:02}{:02}", y, mo, d);
    let ad = format!("{}{:02}{:02}T{:02}{:02}{:02}Z", y, mo, d, h, mi, sec);
    (ad, ds)
}

/// 构造 S3 请求（带 SigV4 签名）→ 返回 (url, method, body, headers)
pub fn s3_request(
    endpoint: &str,
    method: &str,
    bucket: &str,
    key: &str,
    query: &str,
    body: &str,
    access_key: &str,
    secret_key: &str,
) -> Result<(String, String, String, HashMap<String, String>), String> {
    let base = endpoint.trim_end_matches('/');
    let path = format!("/{}/{}", bucket, uri_encode(key, false));
    let url = format!("{}{}", base, path);
    let mut headers = HashMap::new();
    let (amz_date, date_stamp) = amz_dates();
    headers.insert("x-amz-date".to_string(), amz_date.clone());
    headers.insert("x-amz-content-sha256".to_string(), payload_hash(body));
    // host 不放入请求头（http_request_full 自动加 Host；签名用 host 变量参与 canonical headers）
    let host = url_host(base);
    headers.insert("Content-Type".to_string(), "application/octet-stream".to_string());
    let auth = sigv4_authorization(
        method, &host, &path, query, &headers, &payload_hash(body), access_key, secret_key,
        "us-east-1", "s3", &amz_date, &date_stamp,
    );
    headers.insert("Authorization".to_string(), auth);
    let full_url = if query.is_empty() { url } else { format!("{}?{}", url, query) };
    Ok((full_url, method.to_string(), body.to_string(), headers))
}

pub fn payload_hash(body: &str) -> String {
    hex(&sha256(body.as_bytes()))
}

fn url_host(endpoint: &str) -> String {
    // http://host:port → host:port（去 scheme 和路径）
    let e = endpoint.split("://").nth(1).unwrap_or(endpoint);
    e.split('/').next().unwrap_or(e).to_string()
}

/// S3 PUT / GET / DELETE / LIST（内部：用 http_request_full 发请求）
pub fn s3_execute(
    method: &str,
    endpoint: &str,
    bucket: &str,
    key: &str,
    query: &str,
    body: &str,
    ak: &str,
    sk: &str,
) -> Result<(u16, String), String> {
    let (url, m, b, headers) = s3_request(endpoint, method, bucket, key, query, body, ak, sk)?;
    // 走普贤 http_request（http/https）
    let d = super::builtin::http_request_full(&url, &m, Some(&b), &headers, &Default::default())
        .map_err(|e| format!("s3 请求失败: {}", e))?;
    let status = match d.get("status") {
        Some(crate::value::Value::Int(s)) => *s as u16,
        _ => 0,
    };
    let resp_body = match d.get("body") {
        Some(crate::value::Value::Str(s)) => s.clone(),
        _ => String::new(),
    };
    Ok((status, resp_body))
}
