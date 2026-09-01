//! 普贤 (PuXian) RSA（纯标准库实现，M23）
//! 大整数（u32 数组，小端）+ 模幂 + Miller-Rabin 素数生成 + PKCS#1 v1.5
//! 与编译模式 C 端（mbedtls）双模式互通：密钥与密文均以 hex 字符串表示，
//! padding 为标准 PKCS#1 v1.5（type 2 加密 / type 1 签名），两端可互解。

/// 大整数：小端 u32 数组（无前导零；0 表示为空）
pub type Big = Vec<u32>;

pub fn bi_is_zero(a: &Big) -> bool {
    // 0 有两种表示：空 Vec（无前导零规范）或 [0]（divmod/shift 归一化的结果）
    a.is_empty() || (a.len() == 1 && a[0] == 0)
}

pub fn bi_compare(a: &Big, b: &Big) -> std::cmp::Ordering {
    if a.len() != b.len() {
        return a.len().cmp(&b.len());
    }
    for i in (0..a.len()).rev() {
        if a[i] != b[i] {
            return a[i].cmp(&b[i]);
        }
    }
    std::cmp::Ordering::Equal
}

/// 大端字节 → 大数
pub fn bi_from_bytes(b: &[u8]) -> Big {
    let mut out: Big = Vec::new();
    let mut i = b.len();
    while i > 0 {
        let start = i.saturating_sub(4);
        let mut v: u32 = 0;
        for &byte in &b[start..i] {
            v = (v << 8) | byte as u32;
        }
        if v != 0 || !out.is_empty() {
            out.push(v);
        }
        i = start;
    }
    out
}

/// 大数 → 大端字节（去前导零；0 → 空）
pub fn bi_to_bytes(a: &Big) -> Vec<u8> {
    if a.is_empty() {
        return Vec::new();
    }
    let mut out = Vec::new();
    for &limb in a.iter().rev() {
        out.extend_from_slice(&limb.to_be_bytes());
    }
    // 去前导零
    while out.len() > 1 && out[0] == 0 {
        out.remove(0);
    }
    out
}

pub fn bi_from_hex(s: &str) -> Option<Big> {
    let t = s.trim().to_lowercase();
    let t = t.strip_prefix("0x").unwrap_or(&t);
    if t.is_empty() || !t.chars().all(|c| c.is_ascii_hexdigit()) {
        return None;
    }
    // 补偶数字节（奇数长度时首个半字节为高位，隐含前导 0）
    let mut bytes = Vec::new();
    let chars: Vec<char> = t.chars().collect();
    let mut i = if chars.len() % 2 == 1 { 1 } else { 0 };
    if i == 1 {
        let h = chars[0].to_digit(16)?;
        bytes.push(h as u8);
    }
    while i < chars.len() {
        let hi = chars[i].to_digit(16)?;
        let lo = chars[i + 1].to_digit(16)?;
        bytes.push((hi * 16 + lo) as u8);
        i += 2;
    }
    if bytes.is_empty() {
        return Some(Vec::new());
    }
    Some(bi_from_bytes(&bytes))
}

pub fn bi_to_hex(a: &Big) -> String {
    let bytes = bi_to_bytes(a);
    if bytes.is_empty() {
        return "0".to_string();
    }
    let s: String = bytes.iter().map(|b| format!("{:02x}", b)).collect();
    let t = s.trim_start_matches('0').to_string();
    if t.is_empty() {
        "0".to_string()
    } else {
        t
    }
}

pub fn bi_add(a: &Big, b: &Big) -> Big {
    let n = a.len().max(b.len());
    let mut out = Vec::with_capacity(n + 1);
    let mut carry: u64 = 0;
    for i in 0..n {
        let av = *a.get(i).unwrap_or(&0) as u64;
        let bv = *b.get(i).unwrap_or(&0) as u64;
        let s = av + bv + carry;
        out.push((s & 0xFFFF_FFFF) as u32);
        carry = s >> 32;
    }
    if carry > 0 {
        out.push(carry as u32);
    }
    out
}

/// a - b（要求 a >= b）
pub fn bi_sub(a: &Big, b: &Big) -> Big {
    let mut out = Vec::with_capacity(a.len());
    let mut borrow: i64 = 0;
    for i in 0..a.len() {
        let av = a[i] as i64;
        let bv = *b.get(i).unwrap_or(&0) as i64;
        let mut d = av - bv - borrow;
        if d < 0 {
            d += (1i64 << 32);
            borrow = 1;
        } else {
            borrow = 0;
        }
        out.push(d as u32);
    }
    while out.len() > 1 && *out.last().unwrap() == 0 {
        out.pop();
    }
    if out.is_empty() {
        out.push(0);
    }
    out
}

pub fn bi_mul(a: &Big, b: &Big) -> Big {
    if a.is_empty() || b.is_empty() {
        return Vec::new();
    }
    let mut out = vec![0u32; a.len() + b.len()];
    for (i, &av) in a.iter().enumerate() {
        let mut carry: u64 = 0;
        for (j, &bv) in b.iter().enumerate() {
            let cur = out[i + j] as u64 + (av as u64) * (bv as u64) + carry;
            out[i + j] = (cur & 0xFFFF_FFFF) as u32;
            carry = cur >> 32;
        }
        let mut k = i + b.len();
        while carry > 0 {
            let cur = out[k] as u64 + carry;
            out[k] = (cur & 0xFFFF_FFFF) as u32;
            carry = cur >> 32;
            k += 1;
            if k >= out.len() {
                out.push(0);
            }
        }
    }
    while out.len() > 1 && *out.last().unwrap() == 0 {
        out.pop();
    }
    if out.len() == 1 && out[0] == 0 {
        out.clear();
    }
    out
}

/// 左移 bits 位
fn bi_shl(a: &Big, bits: usize) -> Big {
    let limb_shift = bits / 32;
    let bit_shift = (bits % 32) as u32;
    let mut out = vec![0u32; limb_shift];
    if a.is_empty() {
        return out;
    }
    if bit_shift == 0 {
        out.extend_from_slice(a);
        return out;
    }
    let mut carry = 0u32;
    for &limb in a {
        let cur = ((limb as u64) << bit_shift) | (carry as u64);
        out.push((cur & 0xFFFF_FFFF) as u32);
        carry = (cur >> 32) as u32;
    }
    if carry > 0 {
        out.push(carry);
    }
    out
}

/// 右移 bits 位
fn bi_shr(a: &Big, bits: usize) -> Big {
    let limb_shift = bits / 32;
    let bit_shift = (bits % 32) as u32;
    if a.is_empty() || limb_shift >= a.len() {
        return Vec::new();
    }
    let mut out = Vec::with_capacity(a.len() - limb_shift);
    if bit_shift == 0 {
        out.extend_from_slice(&a[limb_shift..]);
    } else {
        for i in 0..(a.len() - limb_shift) {
            let lo = a[limb_shift + i];
            let hi = if limb_shift + i + 1 < a.len() {
                a[limb_shift + i + 1]
            } else {
                0
            };
            out.push((lo >> bit_shift) | (hi << (32 - bit_shift)));
        }
    }
    while out.len() > 1 && *out.last().unwrap() == 0 {
        out.pop();
    }
    if out.is_empty() {
        out.push(0);
    }
    out
}

/// 长除法：a = q*b + r（Knuth 归一化试商法，qhat 最多偏大 2）
pub fn bi_divmod(a: &Big, b: &Big) -> (Big, Big) {
    assert!(!bi_is_zero(b), "除数为零");
    if bi_compare(a, b) == std::cmp::Ordering::Less {
        return (Vec::new(), a.clone());
    }
    // 归一化：b 最高位左移到 ≥ 2^31
    let top_bit = bi_bit_len(b) - 1;
    let shift = 31 - (top_bit % 32);
    let u = bi_shl(a, shift);
    let v = bi_shl(b, shift);
    let mut q = vec![0u32; u.len()];
    let mut r: Big = Vec::new();
    for i in (0..u.len()).rev() {
        r.insert(0, u[i]);
        // Knuth 估商：qhat = (r[n]*B + r[n-1]) / v[n-1]（n = v 的 limb 数），
        // 归一化后最多偏大 2。不可用 r 的最高 2 limb（r 比 v 长时会严重高估）。
        let n = v.len();
        let r_top = if r.len() > n { r[n] } else { 0 };
        let r_next = if r.len() >= n && n >= 1 { r[n - 1] } else { 0 };
        let r_hi = ((r_top as u64) << 32) | r_next as u64;
        let v_top = *v.last().unwrap_or(&1) as u64;
        let mut qhat = if v_top == 0 { u32::MAX as u64 } else { r_hi / v_top };
        if qhat > u32::MAX as u64 {
            qhat = u32::MAX as u64;
        }
        // 校正（Knuth 保证最多 2 次）
        while qhat > 0 {
            let prod = bi_mul(&v, &vec![qhat as u32]);
            if bi_compare(&prod, &r) != std::cmp::Ordering::Greater {
                break;
            }
            qhat -= 1;
        }
        let sub = bi_mul(&v, &vec![qhat as u32]);
        r = bi_sub(&r, &sub);
        q[i] = qhat as u32;
    }
    // 反归一化：r >>= shift
    let r = bi_shr(&r, shift);
    while q.len() > 1 && *q.last().unwrap() == 0 {
        q.pop();
    }
    if q.is_empty() {
        q.push(0);
    }
    let mut rr = r;
    while rr.len() > 1 && *rr.last().unwrap() == 0 {
        rr.pop();
    }
    if rr.is_empty() {
        rr.push(0);
    }
    (q, rr)
}

pub fn bi_mod(a: &Big, m: &Big) -> Big {
    let (_, r) = bi_divmod(a, m);
    r
}

/// 模幂：base^exp mod m（平方-乘）
pub fn bi_modpow(mut base: Big, exp: &Big, m: &Big) -> Big {
    let mut result: Big = vec![1];
    base = bi_mod(&base, m);
    let mut e = exp.clone();
    while !bi_is_zero(&e) {
        if e[0] & 1 == 1 {
            result = bi_mod(&bi_mul(&result, &base), m);
        }
        base = bi_mod(&bi_mul(&base, &base), m);
        // e >>= 1
        let mut carry = 0u32;
        for limb in e.iter_mut().rev() {
            let cur = *limb as u64 | (carry as u64) << 32;
            *limb = (cur >> 1) as u32;
            carry = (cur & 1) as u32;
        }
        while e.len() > 1 && *e.last().unwrap() == 0 {
            e.pop();
        }
        if e.len() == 1 && e[0] == 0 {
            e.clear();
        }
    }
    result
}

pub fn bi_gcd(mut a: Big, mut b: Big) -> Big {
    while !bi_is_zero(&b) {
        let r = bi_mod(&a, &b);
        a = b;
        b = r;
    }
    if a.is_empty() {
        a.push(0);
    }
    a
}

/// 模逆：a^-1 mod m（扩展欧几里得，要求 gcd=1）
/// 标准算法：r 序列精确收敛（r_{i+1} = r_{i-1} - q*r_i，q = floor 保证非负）；
/// t 序列每步 mod m 保持有界（newt = t - q*newt mod m，最终即逆）。
pub fn bi_modinv(a: &Big, m: &Big) -> Big {
    let mut t: Big = Vec::new(); // 0
    let mut newt: Big = vec![1];
    let mut r = m.clone();
    let mut newr = bi_mod(a, m);
    while !bi_is_zero(&newr) {
        let (q, _) = bi_divmod(&r, &newr);
        // newt = (t - q*newt) mod m：先算 q*newt mod m，再做模减（old_t + m - qt 恒非负）
        let qt = bi_mod(&bi_mul(&q, &newt), m);
        let old_t = t.clone();
        t = newt.clone();
        newt = bi_mod(&bi_sub(&bi_add(&old_t, m), &qt), m);
        // r 序列精确：newr = r - q*newr（q = floor(r/newr)，恒 ≥ 0）
        let q_newr = bi_mul(&q, &newr);
        let old_r = newr.clone();
        newr = bi_sub(&r, &q_newr);
        r = old_r;
    }
    if bi_compare(&r, &vec![1]) != std::cmp::Ordering::Equal {
        return Vec::new(); // 无逆
    }
    if t.is_empty() {
        t.push(0);
    }
    t
}

/// 位长
pub fn bi_bit_len(a: &Big) -> usize {
    if a.is_empty() {
        return 0;
    }
    let top = a[a.len() - 1];
    let mut bits = (a.len() - 1) * 32;
    let mut t = top;
    while t > 0 {
        bits += 1;
        t >>= 1;
    }
    bits
}

/// 随机大数（bits 位，最高位为 1）：从 /dev/urandom 读字节（复用句柄）
fn bi_rand(bits: usize) -> Big {
    static U: std::sync::OnceLock<std::fs::File> = std::sync::OnceLock::new();
    let nbytes = (bits + 7) / 8;
    let mut bytes = vec![0u8; nbytes];
    {
        use std::io::Read;
        let f = U.get_or_init(|| std::fs::File::open("/dev/urandom").expect("urandom"));
        let mut h = f.try_clone().unwrap_or_else(|_| std::fs::File::open("/dev/urandom").unwrap());
        let _ = h.read_exact(&mut bytes);
    }
    // 保证 bits 位（最高位置 1，最低位处理）
    let rem = bits % 8;
    if rem > 0 {
        bytes[0] |= 1u8 << (rem - 1);
    } else {
        bytes[0] |= 0x80;
    }
    // 最低位为 1（奇数）
    bytes[nbytes - 1] |= 1;
    bi_from_bytes(&bytes)
}

/// 小素数预筛：能被 3..997 整除则返回 false（拒绝 ~85% 候选，避免昂贵模幂）
pub fn bi_small_filter(n: &Big) -> bool {
    const SMALL: &[u32] = &[
        3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37, 41, 43, 47, 53, 59, 61, 67, 71, 73, 79, 83, 89,
        97, 101, 103, 107, 109, 113, 127, 131, 137, 139, 149, 151, 157, 163, 167, 173, 179, 181,
        191, 193, 197, 199, 211, 223, 227, 229, 233, 239, 241, 251, 257, 263, 269, 271, 277, 281,
        283, 293, 307, 311, 313, 317, 331, 337, 347, 349, 353, 359, 367, 373, 379, 383, 389, 397,
        401, 409, 419, 421, 431, 433, 439, 443, 449, 457, 461, 463, 467, 479, 487, 491, 499, 503,
        509, 521, 523, 541, 547, 557, 563, 569, 571, 577, 587, 593, 599, 601, 607, 613, 617, 619,
        631, 641, 643, 647, 653, 659, 661, 673, 677, 683, 691, 701, 709, 719, 727, 733, 739, 743,
        751, 757, 761, 769, 773, 787, 797, 809, 811, 821, 823, 827, 829, 839, 853, 857, 859, 863,
        877, 881, 883, 887, 907, 911, 919, 929, 937, 941, 947, 953, 967, 971, 977, 983, 991, 997,
    ];
    for &p in SMALL {
        // 用 64 位累加计算 n mod p（避免每次大数除法）
        let mut rem: u64 = 0;
        for &limb in n.iter().rev() {
            rem = ((rem << 32) | limb as u64) % p as u64;
        }
        if rem == 0 {
            return false;
        }
    }
    true
}

/// Miller-Rabin 素性测试（固定 12 基 + 通过后再补 4 随机基；512 位以上概率可忽略）
pub fn bi_miller_rabin(n: &Big) -> bool {
    if bi_is_zero(n) || bi_compare(n, &vec![2]) == std::cmp::Ordering::Less {
        return false;
    }
    if *n.last().unwrap() & 1 == 0 {
        return *n == vec![2];
    }
    // n-1 = d * 2^s
    let one = vec![1u32];
    let two = vec![2u32];
    let mut d = bi_sub(n, &one);
    let mut s = 0usize;
    while d[0] & 1 == 0 {
        s += 1;
        // d >>= 1
        let mut carry = 0u32;
        for limb in d.iter_mut().rev() {
            let cur = *limb as u64 | (carry as u64) << 32;
            *limb = (cur >> 1) as u32;
            carry = (cur & 1) as u32;
        }
        while d.len() > 1 && *d.last().unwrap() == 0 {
            d.pop();
        }
        if d.len() == 1 && d[0] == 0 {
            d.clear();
        }
    }
    let n_minus_1 = bi_sub(n, &one);
    let mut bases: Vec<u64> = vec![2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37];
    // 固定基全过后再补 4 个随机基（防极端伪素数；成本低因候选已极少）
    let mut rnd = bi_rand(128);
    for _ in 0..4 {
        let bv = if rnd.is_empty() { 2 } else { (rnd[0] as u64) % 900 + 2 };
        bases.push(bv);
        rnd = bi_shr(&rnd, 24);
        if rnd.is_empty() {
            rnd = bi_rand(128);
        }
    }
    'outer: for &a in &bases {
        let ab = bi_from_bytes(&a.to_be_bytes());
        if a as u64 >= 2 && bi_compare(&ab, &n_minus_1) == std::cmp::Ordering::Greater {
            continue;
        }
        let mut x = bi_modpow(ab, &d, n);
        if x == vec![1] || x == n_minus_1 {
            continue;
        }
        for _ in 0..s {
            x = bi_modpow(x, &two, n);
            if x == n_minus_1 {
                continue 'outer;
            }
        }
        return false;
    }
    true
}

/// 生成 bits 位素数（小素数预筛 + Miller-Rabin）
pub fn bi_gen_prime(bits: usize) -> Big {
    loop {
        let n = bi_rand(bits);
        if bi_small_filter(&n) && bi_miller_rabin(&n) {
            return n;
        }
    }
}

// ==================== PKCS#1 v1.5 ====================

/// type 2 编码（加密）：00 02 || PS(≥8 非零随机) || 00 || M
fn pkcs1_type2_encode(m: &[u8], k: usize) -> Vec<u8> {
    let mut ps_len = k - m.len() - 3;
    if ps_len < 8 {
        ps_len = 8;
    }
    let mut out = vec![0u8; k];
    out[0] = 0;
    out[1] = 2;
    // 随机非零填充
    let mut i = 2;
    while i < 2 + ps_len {
        let mut b = 0u8;
        if let Ok(mut f) = std::fs::File::open("/dev/urandom") {
            use std::io::Read;
            let mut buf = [0u8; 1];
            let _ = f.read_exact(&mut buf);
            b = buf[0];
        }
        if b != 0 {
            out[i] = b;
            i += 1;
        }
    }
    out[2 + ps_len] = 0;
    out[3 + ps_len..].copy_from_slice(m);
    out
}

/// type 2 解码（解密）：校验 00 02 ... 00 M，返回 M 或 None
fn pkcs1_type2_decode(em: &[u8]) -> Option<Vec<u8>> {
    if em.len() < 11 || em[0] != 0 || em[1] != 2 {
        return None;
    }
    let mut i = 2;
    while i < em.len() && em[i] != 0 {
        if em[i] == 0 {
            break;
        }
        i += 1;
    }
    if i < 10 || i >= em.len() {
        return None; // 至少 8 字节 PS + 分隔 0
    }
    Some(em[i + 1..].to_vec())
}

/// type 1 编码（签名/私钥运算）：00 01 || FF...FF(≥8) || 00 || T
fn pkcs1_type1_encode(t: &[u8], k: usize) -> Vec<u8> {
    let mut ff_len = k - t.len() - 3;
    if ff_len < 8 {
        ff_len = 8;
    }
    let mut out = vec![0u8; k];
    out[0] = 0;
    out[1] = 1;
    for i in 2..2 + ff_len {
        out[i] = 0xFF;
    }
    out[2 + ff_len] = 0;
    out[3 + ff_len..].copy_from_slice(t);
    out
}

/// type 1 解码：返回 T 或 None
fn pkcs1_type1_decode(em: &[u8]) -> Option<Vec<u8>> {
    if em.len() < 11 || em[0] != 0 || em[1] != 1 {
        return None;
    }
    let mut i = 2;
    while i < em.len() && em[i] == 0xFF {
        i += 1;
    }
    if i < 10 || i >= em.len() || em[i] != 0 {
        return None;
    }
    Some(em[i + 1..].to_vec())
}

// ==================== 语言层 API ====================

/// rsa_gen_key(bits) → dict {n, e, d, p, q}（hex 字符串）
pub fn rsa_gen_key(bits: usize) -> Option<Vec<(String, String)>> {
    if bits < 512 {
        return None;
    }
    // 选择 e = 65537
    let e = bi_from_bytes(&[0x01, 0x00, 0x01]);
    // p、q 各 bits/2 位，且 p != q
    let half = bits / 2;
    let (mut p, mut q);
    loop {
        p = bi_gen_prime(half);
        q = bi_gen_prime(half);
        if bi_compare(&p, &q) != std::cmp::Ordering::Equal {
            break;
        }
    }
    let n = bi_mul(&p, &q);
    let p1 = bi_sub(&p, &vec![1]);
    let q1 = bi_sub(&q, &vec![1]);
    let phi = bi_mul(&p1, &q1);
    let d = bi_modinv(&e, &phi);
    if d.is_empty() {
        return None;
    }
    let mut items = Vec::new();
    items.push(("n".to_string(), bi_to_hex(&n)));
    items.push(("e".to_string(), bi_to_hex(&e)));
    items.push(("d".to_string(), bi_to_hex(&d)));
    items.push(("p".to_string(), bi_to_hex(&p)));
    items.push(("q".to_string(), bi_to_hex(&q)));
    Some(items)
}

/// rsa_encrypt(data, n_hex, e_hex) → hex 密文（PKCS#1 v1.5 type 2）
pub fn rsa_encrypt(data: &[u8], n_hex: &str, e_hex: &str) -> Option<String> {
    let n = bi_from_hex(n_hex)?;
    let e = bi_from_hex(e_hex)?;
    if n.is_empty() || e.is_empty() {
        return None;
    }
    let k = bi_to_bytes(&n).len(); // 模长字节
    if data.len() > k - 11 {
        return None;
    }
    let em = pkcs1_type2_encode(data, k);
    let m = bi_from_bytes(&em);
    let c = bi_modpow(m, &e, &n);
    let cb = bi_to_bytes(&c);
    // 填充到 k 字节
    let mut padded = vec![0u8; k];
    let off = k - cb.len();
    padded[off..].copy_from_slice(&cb);
    Some(padded.iter().map(|b| format!("{:02x}", b)).collect())
}

/// rsa_decrypt(ct_hex, n_hex, d_hex) → 明文（PKCS#1 v1.5 type 2 解码）；失败 → None
pub fn rsa_decrypt(ct_hex: &str, n_hex: &str, d_hex: &str) -> Option<Vec<u8>> {
    let n = bi_from_hex(n_hex)?;
    let d = bi_from_hex(d_hex)?;
    let c = bi_from_hex(ct_hex)?;
    if n.is_empty() || d.is_empty() || c.is_empty() {
        return None;
    }
    let m = bi_modpow(c, &d, &n);
    let mb = bi_to_bytes(&m);
    let k = bi_to_bytes(&n).len();
    let mut padded = vec![0u8; k];
    let off = k - mb.len();
    if off < k {
        padded[off..].copy_from_slice(&mb);
    }
    pkcs1_type2_decode(&padded)
}

/// rsa_sign(data, n_hex, d_hex) → hex 签名（PKCS#1 v1.5 type 1，直接签名数据）
pub fn rsa_sign(data: &[u8], n_hex: &str, d_hex: &str) -> Option<String> {
    let n = bi_from_hex(n_hex)?;
    let d = bi_from_hex(d_hex)?;
    if n.is_empty() || d.is_empty() {
        return None;
    }
    let k = bi_to_bytes(&n).len();
    if data.len() > k - 11 {
        return None;
    }
    let em = pkcs1_type1_encode(data, k);
    let m = bi_from_bytes(&em);
    let s = bi_modpow(m, &d, &n);
    let sb = bi_to_bytes(&s);
    let mut padded = vec![0u8; k];
    let off = k - sb.len();
    padded[off..].copy_from_slice(&sb);
    Some(padded.iter().map(|b| format!("{:02x}", b)).collect())
}

/// rsa_verify(data, sig_hex, n_hex, e_hex) → bool
pub fn rsa_verify(data: &[u8], sig_hex: &str, n_hex: &str, e_hex: &str) -> bool {
    let (Some(n), Some(e), Some(s)) = (
        bi_from_hex(n_hex),
        bi_from_hex(e_hex),
        bi_from_hex(sig_hex),
    ) else {
        return false;
    };
    if n.is_empty() || e.is_empty() || s.is_empty() {
        return false;
    }
    let m = bi_modpow(s, &e, &n);
    let mb = bi_to_bytes(&m);
    let k = bi_to_bytes(&n).len();
    let mut padded = vec![0u8; k];
    let off = k - mb.len();
    if off < k {
        padded[off..].copy_from_slice(&mb);
    }
    match pkcs1_type1_decode(&padded) {
        Some(t) => t == data,
        None => false,
    }
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn test_big_arith() {
        let a = bi_from_hex("123456789abcdef").unwrap();
        let b = bi_from_hex("f").unwrap();
        let (q, r) = bi_divmod(&a, &b);
        let prod = bi_add(&bi_mul(&q, &b), &r);
        assert_eq!(prod, a, "a != q*b + r: q={:x?} r={:x?}", q, r);
        assert_eq!(bi_to_hex(&r), "0", "r={}", bi_to_hex(&r));
        // mul/div roundtrip
        let x = bi_from_hex("ffffffffffffffffffffffffffffffff").unwrap();
        let y = bi_from_hex("10001").unwrap();
        let p = bi_mul(&x, &y);
        let (q2, r2) = bi_divmod(&p, &y);
        assert_eq!(q2, x);
        assert_eq!(bi_to_hex(&r2), "0");
    }

    #[test]
    fn test_modpow() {
        let base = bi_from_hex("3").unwrap();
        let exp = bi_from_hex("d").unwrap();
        let m = bi_from_hex("11").unwrap();
        assert_eq!(bi_to_hex(&bi_modpow(base, &exp, &m)), "c"); // 3^13 mod 17 = 12
    }

    #[test]
    fn test_rsa_roundtrip() {
        let k = rsa_gen_key(1024).unwrap();
        let mut n = String::new();
        let mut e = String::new();
        let mut d = String::new();
        for (kk, v) in &k {
            if kk == "n" { n = v.clone(); }
            if kk == "e" { e = v.clone(); }
            if kk == "d" { d = v.clone(); }
        }
        let msg = b"hello rsa";
        let ct = rsa_encrypt(msg, &n, &e).unwrap();
        let pt = rsa_decrypt(&ct, &n, &d).unwrap();
        assert_eq!(pt, msg);
        let sig = rsa_sign(msg, &n, &d).unwrap();
        assert!(rsa_verify(msg, &sig, &n, &e));
        assert!(!rsa_verify(b"tampered", &sig, &n, &e));
    }
}
