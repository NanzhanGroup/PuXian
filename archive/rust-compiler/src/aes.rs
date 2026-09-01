//! 普贤 (PuXian) AES 加密模块（M19）
//!
//! 零依赖标准库实现（FIPS-197），与编译模式（mbedtls）输出逐字节一致：
//! - AES-128/192/256（密钥 16/24/32 字节）
//! - CBC 模式 + PKCS7 填充（企微回调加解密、数据落盘加密）
//! - GCM 模式（CTR + GHASH，tag 16 字节，NIST SP 800-38D）
//!
//! 测试向量（NIST / 标准）：
//!   AES-128 ECB: key=000102...0f, pt=00112233445566778899aabbccddeeff
//!                → 69c4e0d86a7b0430d8cdb78070b4c55a
//!   GCM: key=feffe992...cc, iv=cafebabefacedbaddecaf888, pt=d9313225...
//!        → tag 5bc94fbc3221a5db94fae95ae7121a47（NIST 测试用例 3）

/// AES S-box（FIPS-197 §5.1.1）
static SBOX: [u8; 256] = [
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16,
];

/// GF(2^8) 乘法（xtime 累加）
#[inline]
fn gmul(a: u8, b: u8) -> u8 {
    let mut a = a;
    let mut b = b;
    let mut p = 0u8;
    for _ in 0..8 {
        if b & 1 != 0 {
            p ^= a;
        }
        let hi = a & 0x80;
        a <<= 1;
        if hi != 0 {
            a ^= 0x1b;
        }
        b >>= 1;
    }
    p
}

/// 密钥扩展：返回各轮 16 字节轮密钥（Nk*4 轮）
fn key_expansion(key: &[u8]) -> Vec<[u8; 16]> {
    let nk = key.len() / 4; // 4/6/8
    let nr = nk + 6; // 10/12/14
    let total = 4 * (nr + 1);
    let mut w: Vec<[u8; 4]> = Vec::with_capacity(total);
    for i in 0..nk {
        w.push([key[i * 4], key[i * 4 + 1], key[i * 4 + 2], key[i * 4 + 3]]);
    }
    let rcon: [u8; 16] = [0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x1b, 0x36, 0x6c, 0xd8, 0xab, 0x4d, 0x9a, 0x00];
    for i in nk..total {
        let mut t = w[i - 1];
        if i % nk == 0 {
            // RotWord + SubWord + Rcon
            t = [t[1], t[2], t[3], t[0]];
            for j in 0..4 {
                t[j] = SBOX[t[j] as usize];
            }
            t[0] ^= rcon[i / nk - 1];
        } else if nk > 6 && i % nk == 4 {
            for j in 0..4 {
                t[j] = SBOX[t[j] as usize];
            }
        }
        let prev = w[i - nk];
        w.push([prev[0] ^ t[0], prev[1] ^ t[1], prev[2] ^ t[2], prev[3] ^ t[3]]);
    }
    let mut rk = Vec::with_capacity(nr + 1);
    for r in 0..=nr {
        let mut k = [0u8; 16];
        for c in 0..4 {
            k[c * 4..c * 4 + 4].copy_from_slice(&w[r * 4 + c]);
        }
        rk.push(k);
    }
    rk
}

/// 加密单个 16 字节块
fn encrypt_block(rk: &[[u8; 16]], block: &mut [u8; 16]) {
    let nr = rk.len() - 1;
    for c in 0..4 {
        let col = c * 4;
        for r in 0..4 {
            block[col + r] ^= rk[0][col + r];
        }
    }
    for round in 1..=nr {
        // SubBytes
        for b in block.iter_mut() {
            *b = SBOX[*b as usize];
        }
        // ShiftRows
        let mut t = [0u8; 16];
        for r in 0..4 {
            for c in 0..4 {
                t[c * 4 + r] = block[((c + r) % 4) * 4 + r];
            }
        }
        // MixColumns（最后一轮不做）
        if round != nr {
            for c in 0..4 {
                let o = c * 4;
                let (a0, a1, a2, a3) = (t[o], t[o + 1], t[o + 2], t[o + 3]);
                t[o] = gmul(a0, 2) ^ gmul(a1, 3) ^ a2 ^ a3;
                t[o + 1] = a0 ^ gmul(a1, 2) ^ gmul(a2, 3) ^ a3;
                t[o + 2] = a0 ^ a1 ^ gmul(a2, 2) ^ gmul(a3, 3);
                t[o + 3] = gmul(a0, 3) ^ a1 ^ a2 ^ gmul(a3, 2);
            }
        }
        // AddRoundKey
        for c in 0..4 {
            for r in 0..4 {
                t[c * 4 + r] ^= rk[round][c * 4 + r];
            }
        }
        block.copy_from_slice(&t);
    }
}

/// 解密单个 16 字节块
fn decrypt_block(rk: &[[u8; 16]], block: &mut [u8; 16]) {
    let nr = rk.len() - 1;
    let mut s = *block;
    // InvAddRoundKey（最后一轮密钥）
    for c in 0..4 {
        for r in 0..4 {
            s[c * 4 + r] ^= rk[nr][c * 4 + r];
        }
    }
    for round in (0..nr).rev() {
        // InvShiftRows（行 r 右移 r 位）
        let mut t = [0u8; 16];
        for r in 0..4 {
            for c in 0..4 {
                t[c * 4 + r] = s[((c + 4 - r) % 4) * 4 + r];
            }
        }
        // InvSubBytes
        for b in t.iter_mut() {
            *b = inv_sbox(*b);
        }
        // AddRoundKey
        for c in 0..4 {
            for r in 0..4 {
                t[c * 4 + r] ^= rk[round][c * 4 + r];
            }
        }
        // InvMixColumns（第一轮不做）
        if round != 0 {
            for c in 0..4 {
                let o = c * 4;
                let (a0, a1, a2, a3) = (t[o], t[o + 1], t[o + 2], t[o + 3]);
                t[o] = gmul(a0, 14) ^ gmul(a1, 11) ^ gmul(a2, 13) ^ gmul(a3, 9);
                t[o + 1] = gmul(a0, 9) ^ gmul(a1, 14) ^ gmul(a2, 11) ^ gmul(a3, 13);
                t[o + 2] = gmul(a0, 13) ^ gmul(a1, 9) ^ gmul(a2, 14) ^ gmul(a3, 11);
                t[o + 3] = gmul(a0, 11) ^ gmul(a1, 13) ^ gmul(a2, 9) ^ gmul(a3, 14);
            }
        }
        s = t;
    }
    block.copy_from_slice(&s);
}

/// 逆 S-box：查正表生成
fn inv_sbox(b: u8) -> u8 {
    for (i, &v) in SBOX.iter().enumerate() {
        if v == b {
            return i as u8;
        }
    }
    0
}

/// 校验密钥/IV 长度
fn check_key_iv(key: &[u8], iv: &[u8]) -> Result<(), String> {
    if key.len() != 16 && key.len() != 24 && key.len() != 32 {
        return Err(format!("AES 密钥长度须为 16/24/32 字节（128/192/256 位），实际 {}", key.len()));
    }
    if iv.len() != 16 {
        return Err(format!("CBC 模式 IV 必须 16 字节，实际 {}", iv.len()));
    }
    Ok(())
}

/// CBC 加密（PKCS7 填充），返回密文
pub fn cbc_encrypt(data: &[u8], key: &[u8], iv: &[u8]) -> Result<Vec<u8>, String> {
    check_key_iv(key, iv)?;
    let rk = key_expansion(key);
    // PKCS7：填充 1..=16 字节
    let pad = 16 - (data.len() % 16);
    let pad = if pad == 0 { 16 } else { pad };
    let mut buf = data.to_vec();
    buf.extend(std::iter::repeat(pad as u8).take(pad));
    let mut prev: [u8; 16] = iv.try_into().unwrap();
    let mut out = Vec::with_capacity(buf.len());
    for chunk in buf.chunks_exact(16) {
        let mut b: [u8; 16] = chunk.try_into().unwrap();
        for i in 0..16 {
            b[i] ^= prev[i];
        }
        encrypt_block(&rk, &mut b);
        out.extend_from_slice(&b);
        prev = b;
    }
    Ok(out)
}

/// CBC 解密（校验 PKCS7），Ok(None) = padding 非法
pub fn cbc_decrypt(data: &[u8], key: &[u8], iv: &[u8]) -> Result<Option<Vec<u8>>, String> {
    check_key_iv(key, iv)?;
    if data.is_empty() || data.len() % 16 != 0 {
        return Ok(None);
    }
    let rk = key_expansion(key);
    let mut prev: [u8; 16] = iv.try_into().unwrap();
    let mut out = Vec::with_capacity(data.len());
    for chunk in data.chunks_exact(16) {
        let mut b: [u8; 16] = chunk.try_into().unwrap();
        let orig = b;
        decrypt_block(&rk, &mut b);
        for i in 0..16 {
            b[i] ^= prev[i];
        }
        out.extend_from_slice(&b);
        prev = orig;
    }
    // PKCS7 校验
    let pad = *out.last().unwrap() as usize;
    if pad == 0 || pad > 16 {
        return Ok(None);
    }
    let len = out.len();
    if !out[len - pad..].iter().all(|&x| x as usize == pad) {
        return Ok(None);
    }
    out.truncate(len - pad);
    Ok(Some(out))
}

// ---------------- GCM（NIST SP 800-38D） ----------------

/// GHASH 乘法：GF(2^128)，不可约多项式 x^128+x^7+x^2+x+1
fn ghash_mul(x: &[u8; 16], y: &[u8; 16]) -> [u8; 16] {
    // R = 11100001 || 0^120
    let mut z = [0u8; 16];
    let mut v = *y;
    for i in 0..128 {
        if x[i / 8] & (1 << (7 - i % 8)) != 0 {
            for j in 0..16 {
                z[j] ^= v[j];
            }
        }
        let lsb = v[15] & 1;
        for j in (1..16).rev() {
            v[j] = (v[j] >> 1) | (v[j - 1] << 7);
        }
        v[0] >>= 1;
        if lsb != 0 {
            v[0] ^= 0xe1;
        }
    }
    z
}

/// GHASH(H, data)
fn ghash(h: &[u8; 16], data: &[u8]) -> [u8; 16] {
    let mut y = [0u8; 16];
    let mut padded = data.to_vec();
    while padded.len() % 16 != 0 {
        padded.push(0);
    }
    for chunk in padded.chunks_exact(16) {
        for i in 0..16 {
            y[i] ^= chunk[i];
        }
        y = ghash_mul(h, &y);
    }
    y
}

/// GCM CTR 加密核心：从 inc32(icb) 开始计数（GCM 数据块用 inc32(J0)，J0 仅用于 tag）
fn gcm_ctr(rk: &[[u8; 16]], icb: &[u8; 16], data: &[u8]) -> Vec<u8> {
    let mut counter = *icb;
    // inc32：低 32 位 +1
    for i in (12..16).rev() {
        counter[i] = counter[i].wrapping_add(1);
        if counter[i] != 0 {
            break;
        }
    }
    let mut out = Vec::with_capacity(data.len());
    for chunk in data.chunks(16) {
        let mut e = counter;
        encrypt_block(rk, &mut e);
        for (i, &b) in chunk.iter().enumerate() {
            out.push(b ^ e[i]);
        }
        // 计数器 +1（32 位大端）
        for i in (12..16).rev() {
            counter[i] = counter[i].wrapping_add(1);
            if counter[i] != 0 {
                break;
            }
        }
    }
    out
}

/// GCM 公共：计算 H 与 J0
fn gcm_h_j0(rk: &[[u8; 16]], iv: &[u8]) -> ([u8; 16], [u8; 16]) {
    let mut h = [0u8; 16];
    encrypt_block(rk, &mut h);
    let mut j0 = [0u8; 16];
    if iv.len() == 12 {
        j0[..12].copy_from_slice(iv);
        j0[15] = 1;
    } else {
        let mut s = iv.to_vec();
        let rem = s.len() % 16;
        if rem != 0 {
            s.extend(std::iter::repeat(0u8).take(16 - rem));
        }
        s.extend_from_slice(&((iv.len() as u64) * 8).to_be_bytes());
        j0 = ghash(&h, &s);
    }
    (h, j0)
}

/// GCM 加密：返回 (密文, 16 字节 tag)
pub fn gcm_encrypt(data: &[u8], key: &[u8], iv: &[u8]) -> Result<(Vec<u8>, [u8; 16]), String> {
    if key.len() != 16 && key.len() != 24 && key.len() != 32 {
        return Err(format!("AES 密钥长度须为 16/24/32 字节，实际 {}", key.len()));
    }
    if iv.is_empty() {
        return Err("GCM 模式 IV 不能为空".to_string());
    }
    let rk = key_expansion(key);
    let (h, j0) = gcm_h_j0(&rk, iv);
    // 密文 = CTR（从 inc32(J0) 开始）
    let ct = gcm_ctr(&rk, &j0, data);
    // tag = E(K, J0) XOR GHASH(H, 密文 || len(AAD)||len(CT))
    let s = gcm_tag_s(&h, &ct);
    let mut ej0 = j0;
    encrypt_block(&rk, &mut ej0);
    let mut tag = [0u8; 16];
    for i in 0..16 {
        tag[i] = ej0[i] ^ s[i];
    }
    Ok((ct, tag))
}

/// GCM tag 的 GHASH 部分：GHASH(H, C || [0^64][len(C)*8])
fn gcm_tag_s(h: &[u8; 16], ct: &[u8]) -> [u8; 16] {
    let mut sdata = ct.to_vec();
    let mut lens = [0u8; 16];
    lens[8..].copy_from_slice(&((ct.len() as u64) * 8).to_be_bytes());
    sdata.extend_from_slice(&lens);
    ghash(h, &sdata)
}

/// GCM 解密：校验 tag，Ok(None) = tag 不匹配
pub fn gcm_decrypt(ct: &[u8], key: &[u8], iv: &[u8], tag: &[u8]) -> Result<Option<Vec<u8>>, String> {
    if key.len() != 16 && key.len() != 24 && key.len() != 32 {
        return Err(format!("AES 密钥长度须为 16/24/32 字节，实际 {}", key.len()));
    }
    if iv.is_empty() {
        return Err("GCM 模式 IV 不能为空".to_string());
    }
    if tag.len() != 16 {
        return Ok(None);
    }
    let rk = key_expansion(key);
    let (h, j0) = gcm_h_j0(&rk, iv);
    // 明文 = 同一密钥流异或
    let pt = gcm_ctr(&rk, &j0, ct);
    // 用输入密文重新计算 tag 校验
    let s = gcm_tag_s(&h, ct);
    let mut ej0 = j0;
    encrypt_block(&rk, &mut ej0);
    let mut expect = [0u8; 16];
    for i in 0..16 {
        expect[i] = ej0[i] ^ s[i];
    }
    if expect != *tag {
        return Ok(None);
    }
    Ok(Some(pt))
}

#[cfg(test)]
mod tests {
    use super::*;

    fn hex(s: &str) -> Vec<u8> {
        (0..s.len())
            .step_by(2)
            .map(|i| u8::from_str_radix(&s[i..i + 2], 16).unwrap())
            .collect()
    }
    fn to_hex(b: &[u8]) -> String {
        b.iter().map(|x| format!("{:02x}", x)).collect()
    }

    #[test]
    fn test_aes128_ecb_vector() {
        // FIPS-197 附录 B：单块加密
        let key = hex("000102030405060708090a0b0c0d0e0f");
        let rk = key_expansion(&key);
        let mut pt: [u8; 16] = hex("00112233445566778899aabbccddeeff").try_into().unwrap();
        encrypt_block(&rk, &mut pt);
        assert_eq!(to_hex(&pt), "69c4e0d86a7b0430d8cdb78070b4c55a");
    }

    #[test]
    fn test_cbc_roundtrip() {
        let key = b"0123456789abcdef0123456789abcdef"; // 32B AES-256
        let iv = b"1234567890abcdef";
        let data = b"Hello PuXian \xe4\xb8\xad\xe6\x96\x87";
        let ct = cbc_encrypt(data, key, iv).unwrap();
        assert_eq!(ct.len() % 16, 0);
        let pt = cbc_decrypt(&ct, key, iv).unwrap().unwrap();
        assert_eq!(pt, data);
    }

    #[test]
    fn test_cbc_bad_padding_returns_none() {
        let key = b"0123456789abcdef0123456789abcdef";
        let iv = b"1234567890abcdef";
        let ct = cbc_encrypt(b"abc", key, iv).unwrap();
        // 篡改最后一个字节 → padding 非法
        let mut bad = ct.clone();
        let n = bad.len();
        bad[n - 1] ^= 0xff;
        assert!(cbc_decrypt(&bad, key, iv).unwrap().is_none());
        // 长度非 16 倍数
        assert!(cbc_decrypt(b"123", key, iv).unwrap().is_none());
    }

    #[test]
    fn test_gcm_nist_vector() {
        // NIST GCM 测试用例 2（AES-128, iv 12 字节, 无 AAD）
        let key = [0u8; 16];
        let iv = [0u8; 12];
        let pt = [0u8; 16];
        let (ct, tag) = gcm_encrypt(&pt, &key, &iv).unwrap();
        assert_eq!(to_hex(&ct), "0388dace60b6a392f328c2b971b2fe78");
        assert_eq!(to_hex(&tag), "ab6e47d42cec13bdf53a67b21257bddf");
        // 解密回明文
        let back = gcm_decrypt(&ct, &key, &iv, &tag).unwrap().unwrap();
        assert_eq!(back, pt);
        // tag 错误 → None
        let mut bad = tag;
        bad[0] ^= 1;
        assert!(gcm_decrypt(&ct, &key, &iv, &bad).unwrap().is_none());
    }
}
