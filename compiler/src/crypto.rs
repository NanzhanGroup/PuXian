//! 普贤 (PuXian) crypto 哈希模块（M14，P1 缺口：crypto 哈希）
//!
//! 零依赖标准库实现，与编译模式（C runtime）输出完全一致：
//! - SHA-256：签名校验、缓存 key 归一化（返回 64 位小写 hex）
//! - XXH64：高速数据指纹 / 取模分片 / 布隆过滤器（返回 i64，seed=0）
//!
//! 测试向量（FIPS 180-4 / xxHash 官方）：
//!   sha256("")   = e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855
//!   sha256("abc")= ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad
//!   xxh64("")    = 0xef46db3751d8e999
//!   xxh64("a")   = 0xd24ec4f1a98c6e5b
//!   xxh64("abc") = 0x44bc2cf5ad770999

/// SHA-256 轮常量（FIPS 180-4 §4.2.2）
const K256: [u32; 64] = [
    0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
    0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
    0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
    0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
    0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
    0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
    0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
    0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2,
];

/// 计算 SHA-256，返回 32 字节原始摘要
pub fn sha256(data: &[u8]) -> [u8; 32] {
    let mut h: [u32; 8] = [
        0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a, 0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19,
    ];
    let bit_len = (data.len() as u64).wrapping_mul(8);
    // 填充：0x80 + 若干 0x00，直到 ≡56 (mod 64)，再追加 64 位大端比特长度
    let mut msg = data.to_vec();
    msg.push(0x80);
    while msg.len() % 64 != 56 {
        msg.push(0);
    }
    msg.extend_from_slice(&bit_len.to_be_bytes());

    for chunk in msg.chunks_exact(64) {
        let mut w = [0u32; 64];
        for i in 0..16 {
            w[i] = u32::from_be_bytes([
                chunk[i * 4],
                chunk[i * 4 + 1],
                chunk[i * 4 + 2],
                chunk[i * 4 + 3],
            ]);
        }
        for i in 16..64 {
            let s0 = w[i - 15].rotate_right(7) ^ w[i - 15].rotate_right(18) ^ (w[i - 15] >> 3);
            let s1 = w[i - 2].rotate_right(17) ^ w[i - 2].rotate_right(19) ^ (w[i - 2] >> 10);
            w[i] = w[i - 16]
                .wrapping_add(s0)
                .wrapping_add(w[i - 7])
                .wrapping_add(s1);
        }
        let (mut a, mut b, mut c, mut d) = (h[0], h[1], h[2], h[3]);
        let (mut e, mut f, mut g, mut hh) = (h[4], h[5], h[6], h[7]);
        for i in 0..64 {
            let s1 = e.rotate_right(6) ^ e.rotate_right(11) ^ e.rotate_right(25);
            let ch = (e & f) ^ ((!e) & g);
            let t1 = hh.wrapping_add(s1).wrapping_add(ch).wrapping_add(K256[i]).wrapping_add(w[i]);
            let s0 = a.rotate_right(2) ^ a.rotate_right(13) ^ a.rotate_right(22);
            let maj = (a & b) ^ (a & c) ^ (b & c);
            let t2 = s0.wrapping_add(maj);
            hh = g;
            g = f;
            f = e;
            e = d.wrapping_add(t1);
            d = c;
            c = b;
            b = a;
            a = t1.wrapping_add(t2);
        }
        h[0] = h[0].wrapping_add(a);
        h[1] = h[1].wrapping_add(b);
        h[2] = h[2].wrapping_add(c);
        h[3] = h[3].wrapping_add(d);
        h[4] = h[4].wrapping_add(e);
        h[5] = h[5].wrapping_add(f);
        h[6] = h[6].wrapping_add(g);
        h[7] = h[7].wrapping_add(hh);
    }

    let mut out = [0u8; 32];
    for i in 0..8 {
        out[i * 4..i * 4 + 4].copy_from_slice(&h[i].to_be_bytes());
    }
    out
}

/// 计算 SHA-256，返回 64 字符小写 hex 字符串
pub fn sha256_hex(data: &[u8]) -> String {
    const HEX: &[u8; 16] = b"0123456789abcdef";
    let d = sha256(data);
    let mut s = String::with_capacity(64);
    for b in d {
        s.push(HEX[(b >> 4) as usize] as char);
        s.push(HEX[(b & 0x0f) as usize] as char);
    }
    s
}

// ---------------- XXH64（xxHash, seed=0） ----------------

const P64_1: u64 = 0x9E37_79B1_85EB_CA87;
const P64_2: u64 = 0xC2B2_AE3D_27D4_EB4F;
const P64_3: u64 = 0x1656_67B1_9E37_79F9;
const P64_4: u64 = 0x85EB_CA77_C2B2_AE63;
const P64_5: u64 = 0x27D4_EB2F_1656_67C5;

#[inline]
fn xxh64_round(acc: u64, input: u64) -> u64 {
    acc.wrapping_add(input.wrapping_mul(P64_2)).rotate_left(31).wrapping_mul(P64_1)
}

#[inline]
fn xxh64_merge(acc: u64, val: u64) -> u64 {
    let acc = acc ^ xxh64_round(0, val);
    acc.wrapping_mul(P64_1).wrapping_add(P64_4)
}

fn rd64(data: &[u8], p: usize) -> u64 {
    u64::from_le_bytes(data[p..p + 8].try_into().unwrap())
}

fn rd32(data: &[u8], p: usize) -> u64 {
    u32::from_le_bytes(data[p..p + 4].try_into().unwrap()) as u64
}

/// 计算 XXH64（seed=0），返回 u64 摘要（语言层按 i64 展示）
pub fn xxh64(data: &[u8]) -> u64 {
    let len = data.len() as u64;
    let mut p = 0usize;
    let mut h: u64;

    if data.len() >= 32 {
        let mut v1 = P64_1.wrapping_add(P64_2);
        let mut v2 = P64_2;
        let mut v3 = 0u64;
        let mut v4 = 0u64.wrapping_sub(P64_1);
        while p + 32 <= data.len() {
            v1 = xxh64_round(v1, rd64(data, p));
            v2 = xxh64_round(v2, rd64(data, p + 8));
            v3 = xxh64_round(v3, rd64(data, p + 16));
            v4 = xxh64_round(v4, rd64(data, p + 24));
            p += 32;
        }
        h = v1.rotate_left(1)
            .wrapping_add(v2.rotate_left(7))
            .wrapping_add(v3.rotate_left(12))
            .wrapping_add(v4.rotate_left(18));
        h = xxh64_merge(h, v1);
        h = xxh64_merge(h, v2);
        h = xxh64_merge(h, v3);
        h = xxh64_merge(h, v4);
    } else {
        h = P64_5;
    }
    h = h.wrapping_add(len);

    while p + 8 <= data.len() {
        let k = xxh64_round(0, rd64(data, p));
        h ^= k;
        h = h.rotate_left(27).wrapping_mul(P64_1).wrapping_add(P64_4);
        p += 8;
    }
    if p + 4 <= data.len() {
        let k = rd32(data, p);
        h ^= k.wrapping_mul(P64_1);
        h = h.rotate_left(23).wrapping_mul(P64_2).wrapping_add(P64_3);
        p += 4;
    }
    while p < data.len() {
        h ^= (data[p] as u64).wrapping_mul(P64_5);
        h = h.rotate_left(11).wrapping_mul(P64_1);
        p += 1;
    }

    h ^= h >> 33;
    h = h.wrapping_mul(P64_2);
    h ^= h >> 29;
    h = h.wrapping_mul(P64_3);
    h ^= h >> 32;
    h
}

#[cfg(test)]
mod tests {
    use super::*;

    #[test]
    fn sha256_known_vectors() {
        assert_eq!(
            sha256_hex(b""),
            "e3b0c44298fc1c149afbf4c8996fb92427ae41e4649b934ca495991b7852b855"
        );
        assert_eq!(
            sha256_hex(b"abc"),
            "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad"
        );
        assert_eq!(
            sha256_hex(b"hello"),
            "2cf24dba5fb0a30e26e83b2ac5b9e29e1b161e5c1fa7425e73043362938b9824"
        );
        // 跨块边界：>64 字节（FIPS 官方向量：sha256("a"*1000)）
        let a1000 = "a".repeat(1000);
        assert_eq!(
            sha256_hex(a1000.as_bytes()),
            "41edece42d63e8d9bf515a9ba6932e1c20cbc9f5a5d134645adb5db1b9737ea3"
        );
    }

    #[test]
    fn xxh64_known_vectors() {
        assert_eq!(xxh64(b""), 0xef46_db37_51d8_e999);
        assert_eq!(xxh64(b"a"), 0xd24e_c4f1_a98c_6e5b);
        assert_eq!(xxh64(b"abc"), 0x44bc_2cf5_ad77_0999);
        // 跨块边界：>32 字节（走 4 路并行分支，由 python 权威实现互验）
        assert_eq!(xxh64(b"012345678901234567890123456789012345678901234567890123456789"), 0x66ea_70f4_211f_2a4f);
        assert_eq!(xxh64(&[b'x'; 64]), 0xa971_61d5_402f_516e);
        assert_eq!(xxh64(&[b'y'; 100]), 0xcd30_0b25_f4b8_a0bf);
    }

    #[test]
    fn sha256_xxh64_deterministic() {
        let a = b"the quick brown fox jumps over the lazy dog";
        assert_eq!(sha256_hex(a).len(), 64);
        assert_eq!(xxh64(a), xxh64(a));
        // 单字节差异应产生不同摘要
        let b2 = b"the quick brown fox jumps over the lazy doG";
        assert_ne!(xxh64(a), xxh64(b2));
    }
}
