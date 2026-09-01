//! M35：HTTP/2 最小服务端（h2c Upgrade + prior knowledge + HPACK + 帧层）
//! 范围：单连接帧循环；SETTINGS/PING/HEADERS/DATA/WINDOW_UPDATE/RST_STREAM/GOAWAY；
//! HPACK 解码（静态表 + 字面量 + Huffman）+ 响应编码（:status 200 + content-type + content-length）
//! 验证：curl --http2（h2c upgrade 到明文端口）；TLS ALPN h2 后续扩展。
use std::io::{Read, Write};
use crate::tls::SConn;

/// RFC 7541 附录 B Huffman 表（code 对齐 MSB, len 位宽）
pub static HPACK_HUFFMAN: [(u32, u8); 257] = [
    (0x1ff8, 13),  // 0
    (0x7fffd8, 23),  // 1
    (0xfffffe2, 28),  // 2
    (0xfffffe3, 28),  // 3
    (0xfffffe4, 28),  // 4
    (0xfffffe5, 28),  // 5
    (0xfffffe6, 28),  // 6
    (0xfffffe7, 28),  // 7
    (0xfffffe8, 28),  // 8
    (0xffffea, 24),  // 9
    (0x3ffffffc, 30),  // 10
    (0xfffffe9, 28),  // 11
    (0xfffffea, 28),  // 12
    (0x3ffffffd, 30),  // 13
    (0xfffffeb, 28),  // 14
    (0xfffffec, 28),  // 15
    (0xfffffed, 28),  // 16
    (0xfffffee, 28),  // 17
    (0xfffffef, 28),  // 18
    (0xffffff0, 28),  // 19
    (0xffffff1, 28),  // 20
    (0xffffff2, 28),  // 21
    (0x3ffffffe, 30),  // 22
    (0xffffff3, 28),  // 23
    (0xffffff4, 28),  // 24
    (0xffffff5, 28),  // 25
    (0xffffff6, 28),  // 26
    (0xffffff7, 28),  // 27
    (0xffffff8, 28),  // 28
    (0xffffff9, 28),  // 29
    (0xffffffa, 28),  // 30
    (0xffffffb, 28),  // 31
    (0x14, 6),  // 32
    (0x3f8, 10),  // 33
    (0x3f9, 10),  // 34
    (0xffa, 12),  // 35
    (0x1ff9, 13),  // 36
    (0x15, 6),  // 37
    (0xf8, 8),  // 38
    (0x7fa, 11),  // 39
    (0x3fa, 10),  // 40
    (0x3fb, 10),  // 41
    (0xf9, 8),  // 42
    (0x7fb, 11),  // 43
    (0xfa, 8),  // 44
    (0x16, 6),  // 45
    (0x17, 6),  // 46
    (0x18, 6),  // 47
    (0x0, 5),  // 48
    (0x1, 5),  // 49
    (0x2, 5),  // 50
    (0x19, 6),  // 51
    (0x1a, 6),  // 52
    (0x1b, 6),  // 53
    (0x1c, 6),  // 54
    (0x1d, 6),  // 55
    (0x1e, 6),  // 56
    (0x1f, 6),  // 57
    (0x5c, 7),  // 58
    (0xfb, 8),  // 59
    (0x7ffc, 15),  // 60
    (0x20, 6),  // 61
    (0xffb, 12),  // 62
    (0x3fc, 10),  // 63
    (0x1ffa, 13),  // 64
    (0x21, 6),  // 65
    (0x5d, 7),  // 66
    (0x5e, 7),  // 67
    (0x5f, 7),  // 68
    (0x60, 7),  // 69
    (0x61, 7),  // 70
    (0x62, 7),  // 71
    (0x63, 7),  // 72
    (0x64, 7),  // 73
    (0x65, 7),  // 74
    (0x66, 7),  // 75
    (0x67, 7),  // 76
    (0x68, 7),  // 77
    (0x69, 7),  // 78
    (0x6a, 7),  // 79
    (0x6b, 7),  // 80
    (0x6c, 7),  // 81
    (0x6d, 7),  // 82
    (0x6e, 7),  // 83
    (0x6f, 7),  // 84
    (0x70, 7),  // 85
    (0x71, 7),  // 86
    (0x72, 7),  // 87
    (0xfc, 8),  // 88
    (0x73, 7),  // 89
    (0xfd, 8),  // 90
    (0x1ffb, 13),  // 91
    (0x7fff0, 19),  // 92
    (0x1ffc, 13),  // 93
    (0x3ffc, 14),  // 94
    (0x22, 6),  // 95
    (0x7ffd, 15),  // 96
    (0x3, 5),  // 97
    (0x23, 6),  // 98
    (0x4, 5),  // 99
    (0x24, 6),  // 100
    (0x5, 5),  // 101
    (0x25, 6),  // 102
    (0x26, 6),  // 103
    (0x27, 6),  // 104
    (0x6, 5),  // 105
    (0x74, 7),  // 106
    (0x75, 7),  // 107
    (0x28, 6),  // 108
    (0x29, 6),  // 109
    (0x2a, 6),  // 110
    (0x7, 5),  // 111
    (0x2b, 6),  // 112
    (0x76, 7),  // 113
    (0x2c, 6),  // 114
    (0x8, 5),  // 115
    (0x9, 5),  // 116
    (0x2d, 6),  // 117
    (0x77, 7),  // 118
    (0x78, 7),  // 119
    (0x79, 7),  // 120
    (0x7a, 7),  // 121
    (0x7b, 7),  // 122
    (0x7ffe, 15),  // 123
    (0x7fc, 11),  // 124
    (0x3ffd, 14),  // 125
    (0x1ffd, 13),  // 126
    (0xffffffc, 28),  // 127
    (0xfffe6, 20),  // 128
    (0x3fffd2, 22),  // 129
    (0xfffe7, 20),  // 130
    (0xfffe8, 20),  // 131
    (0x3fffd3, 22),  // 132
    (0x3fffd4, 22),  // 133
    (0x3fffd5, 22),  // 134
    (0x7fffd9, 23),  // 135
    (0x3fffd6, 22),  // 136
    (0x7fffda, 23),  // 137
    (0x7fffdb, 23),  // 138
    (0x7fffdc, 23),  // 139
    (0x7fffdd, 23),  // 140
    (0x7fffde, 23),  // 141
    (0xffffeb, 24),  // 142
    (0x7fffdf, 23),  // 143
    (0xffffec, 24),  // 144
    (0xffffed, 24),  // 145
    (0x3fffd7, 22),  // 146
    (0x7fffe0, 23),  // 147
    (0xffffee, 24),  // 148
    (0x7fffe1, 23),  // 149
    (0x7fffe2, 23),  // 150
    (0x7fffe3, 23),  // 151
    (0x7fffe4, 23),  // 152
    (0x1fffdc, 21),  // 153
    (0x3fffd8, 22),  // 154
    (0x7fffe5, 23),  // 155
    (0x3fffd9, 22),  // 156
    (0x7fffe6, 23),  // 157
    (0x7fffe7, 23),  // 158
    (0xffffef, 24),  // 159
    (0x3fffda, 22),  // 160
    (0x1fffdd, 21),  // 161
    (0xfffe9, 20),  // 162
    (0x3fffdb, 22),  // 163
    (0x3fffdc, 22),  // 164
    (0x7fffe8, 23),  // 165
    (0x7fffe9, 23),  // 166
    (0x1fffde, 21),  // 167
    (0x7fffea, 23),  // 168
    (0x3fffdd, 22),  // 169
    (0x3fffde, 22),  // 170
    (0xfffff0, 24),  // 171
    (0x1fffdf, 21),  // 172
    (0x3fffdf, 22),  // 173
    (0x7fffeb, 23),  // 174
    (0x7fffec, 23),  // 175
    (0x1fffe0, 21),  // 176
    (0x1fffe1, 21),  // 177
    (0x3fffe0, 22),  // 178
    (0x1fffe2, 21),  // 179
    (0x7fffed, 23),  // 180
    (0x3fffe1, 22),  // 181
    (0x7fffee, 23),  // 182
    (0x7fffef, 23),  // 183
    (0xfffea, 20),  // 184
    (0x3fffe2, 22),  // 185
    (0x3fffe3, 22),  // 186
    (0x3fffe4, 22),  // 187
    (0x7ffff0, 23),  // 188
    (0x3fffe5, 22),  // 189
    (0x3fffe6, 22),  // 190
    (0x7ffff1, 23),  // 191
    (0x3ffffe0, 26),  // 192
    (0x3ffffe1, 26),  // 193
    (0xfffeb, 20),  // 194
    (0x7fff1, 19),  // 195
    (0x3fffe7, 22),  // 196
    (0x7ffff2, 23),  // 197
    (0x3fffe8, 22),  // 198
    (0x1ffffec, 25),  // 199
    (0x3ffffe2, 26),  // 200
    (0x3ffffe3, 26),  // 201
    (0x3ffffe4, 26),  // 202
    (0x7ffffde, 27),  // 203
    (0x7ffffdf, 27),  // 204
    (0x3ffffe5, 26),  // 205
    (0xfffff1, 24),  // 206
    (0x1ffffed, 25),  // 207
    (0x7fff2, 19),  // 208
    (0x1fffe3, 21),  // 209
    (0x3ffffe6, 26),  // 210
    (0x7ffffe0, 27),  // 211
    (0x7ffffe1, 27),  // 212
    (0x3ffffe7, 26),  // 213
    (0x7ffffe2, 27),  // 214
    (0xfffff2, 24),  // 215
    (0x1fffe4, 21),  // 216
    (0x1fffe5, 21),  // 217
    (0x3ffffe8, 26),  // 218
    (0x3ffffe9, 26),  // 219
    (0xffffffd, 28),  // 220
    (0x7ffffe3, 27),  // 221
    (0x7ffffe4, 27),  // 222
    (0x7ffffe5, 27),  // 223
    (0xfffec, 20),  // 224
    (0xfffff3, 24),  // 225
    (0xfffed, 20),  // 226
    (0x1fffe6, 21),  // 227
    (0x3fffe9, 22),  // 228
    (0x1fffe7, 21),  // 229
    (0x1fffe8, 21),  // 230
    (0x7ffff3, 23),  // 231
    (0x3fffea, 22),  // 232
    (0x3fffeb, 22),  // 233
    (0x1ffffee, 25),  // 234
    (0x1ffffef, 25),  // 235
    (0xfffff4, 24),  // 236
    (0xfffff5, 24),  // 237
    (0x3ffffea, 26),  // 238
    (0x7ffff4, 23),  // 239
    (0x3ffffeb, 26),  // 240
    (0x7ffffe6, 27),  // 241
    (0x3ffffec, 26),  // 242
    (0x3ffffed, 26),  // 243
    (0x7ffffe7, 27),  // 244
    (0x7ffffe8, 27),  // 245
    (0x7ffffe9, 27),  // 246
    (0x7ffffea, 27),  // 247
    (0x7ffffeb, 27),  // 248
    (0xffffffe, 28),  // 249
    (0x7ffffec, 27),  // 250
    (0x7ffffed, 27),  // 251
    (0x7ffffee, 27),  // 252
    (0x7ffffef, 27),  // 253
    (0x7fffff0, 27),  // 254
    (0x3ffffee, 26),  // 255
    (0x3fffffff, 30),  // 256
];
/// RFC 7541 附录 A 静态表（name, value；值可为空）
pub static HPACK_STATIC: [(&str, &str); 61] = [
    (":authority", ""),
    (":method", "GET"),
    (":method", "POST"),
    (":path", "/"),
    (":path", "/index.html"),
    (":scheme", "http"),
    (":scheme", "https"),
    (":status", "200"),
    (":status", "204"),
    (":status", "206"),
    (":status", "304"),
    (":status", "400"),
    (":status", "404"),
    (":status", "500"),
    ("accept-charset", ""),
    ("accept-encoding", "gzip, deflate"),
    ("accept-language", ""),
    ("accept-ranges", ""),
    ("accept", ""),
    ("access-control-allow-origin", ""),
    ("age", ""),
    ("allow", ""),
    ("authorization", ""),
    ("cache-control", ""),
    ("content-disposition", ""),
    ("content-encoding", ""),
    ("content-language", ""),
    ("content-length", ""),
    ("content-location", ""),
    ("content-range", ""),
    ("content-type", ""),
    ("cookie", ""),
    ("date", ""),
    ("etag", ""),
    ("expect", ""),
    ("expires", ""),
    ("from", ""),
    ("host", ""),
    ("if-match", ""),
    ("if-modified-since", ""),
    ("if-none-match", ""),
    ("if-range", ""),
    ("if-unmodified-since", ""),
    ("last-modified", ""),
    ("link", ""),
    ("location", ""),
    ("max-forwards", ""),
    ("proxy-authenticate", ""),
    ("proxy-authorization", ""),
    ("range", ""),
    ("referer", ""),
    ("refresh", ""),
    ("retry-after", ""),
    ("server", ""),
    ("set-cookie", ""),
    ("strict-transport-security", ""),
    ("transfer-encoding", ""),
    ("user-agent", ""),
    ("vary", ""),
    ("via", ""),
    ("www-authenticate", ""),
];


// ==================== HPACK 解码 ====================

/// Huffman 解码（RFC 7541 附录 B）：bit 流 → 字节；EOS(256) 提前结束
pub fn hpack_huffman_decode(bits: &[u8]) -> Result<Vec<u8>, String> {
    let mut out = Vec::new();
    let mut code: u32 = 0;
    let mut n: u8 = 0;
    for &byte in bits {
        for i in (0..8).rev() {
            let b = ((byte >> i) & 1) as u32;
            code = (code << 1) | b;
            n += 1;
            // 线性查表（头部小，效率足够）
            let mut found = None;
            for (sym, &(c, l)) in HPACK_HUFFMAN.iter().enumerate() {
                if l == n && c == code {
                    found = Some(sym);
                    break;
                }
            }
            if let Some(sym) = found {
                if sym == 256 {
                    return Ok(out); // EOS
                }
                out.push(sym as u8);
                code = 0;
                n = 0;
            }
        }
    }
    Ok(out)
}

/// 读取 HPACK 字符串（首字节高位 H=Huffman，低 7 位长度；全 1 则扩展）
fn hpack_str(data: &[u8], pos: &mut usize) -> Result<String, String> {
    if *pos >= data.len() {
        return Err("HPACK 字符串越界".into());
    }
    let b = data[*pos];
    let huff = b & 0x80 != 0;
    let mut len = (b & 0x7f) as usize;
    *pos += 1;
    if len == 127 {
        // 扩展长度（7 位组）
        loop {
            if *pos >= data.len() {
                return Err("HPACK 长度越界".into());
            }
            let c = data[*pos];
            *pos += 1;
            len = (len << 7) | (c & 0x7f) as usize;
            if c & 0x80 == 0 {
                break;
            }
        }
    }
    if *pos + len > data.len() {
        return Err("HPACK 字符串越界".into());
    }
    let raw = &data[*pos..*pos + len];
    *pos += len;
    if huff {
        let dec = hpack_huffman_decode(raw)?;
        Ok(String::from_utf8_lossy(&dec).to_string())
    } else {
        Ok(String::from_utf8_lossy(raw).to_string())
    }
}

/// 解码一个 header block → (name, value) 列表
pub fn hpack_decode(data: &[u8]) -> Result<Vec<(String, String)>, String> {
    let mut headers = Vec::new();
    let mut pos = 0usize;
    while pos < data.len() {
        let b = data[pos];
        if b & 0x80 != 0 {
            // 索引引用（静态表）
            let mut idx = (b & 0x7f) as usize;
            pos += 1;
            if idx == 0 {
                // 多字节
                loop {
                    if pos >= data.len() {
                        return Err("HPACK 索引越界".into());
                    }
                    let c = data[pos];
                    pos += 1;
                    idx = (idx << 7) | (c & 0x7f) as usize;
                    if c & 0x80 == 0 {
                        break;
                    }
                }
            }
            if idx >= 1 && idx <= 61 {
                let (n, v) = HPACK_STATIC[idx - 1];
                headers.push((n.to_string(), v.to_string()));
            } else {
                return Err(format!("HPACK 静态表索引越界: {}", idx));
            }
        } else if b & 0x40 != 0 {
            // 字面量增量索引：name 索引或字面量 + value 字面量
            let nidx = (b & 0x3f) as usize;
            pos += 1;
            let name = if nidx == 0 {
                hpack_str(data, &mut pos)?
            } else if nidx <= 61 {
                HPACK_STATIC[nidx - 1].0.to_string()
            } else {
                return Err(format!("HPACK name 索引越界: {}", nidx));
            };
            let value = hpack_str(data, &mut pos)?;
            headers.push((name, value));
        } else if b & 0x20 != 0 {
            // 动态表大小更新（忽略）
            pos += 1;
        } else {
            // 字面量无索引 / 从不索引：name 索引或字面量 + value 字面量
            let nidx = (b & 0x1f) as usize;
            pos += 1;
            let name = if nidx == 0 {
                hpack_str(data, &mut pos)?
            } else if nidx <= 61 {
                HPACK_STATIC[nidx - 1].0.to_string()
            } else {
                return Err(format!("HPACK name 索引越界: {}", nidx));
            };
            let value = hpack_str(data, &mut pos)?;
            headers.push((name, value));
        }
    }
    Ok(headers)
}

/// HPACK 编码字符串（字面量，无 Huffman——服务端发送简单头）
fn hpack_encode_str(s: &str) -> Vec<u8> {
    let b = s.as_bytes();
    let mut out = vec![b.len() as u8];
    out.extend_from_slice(b);
    out
}

/// 响应 HEADERS 编码（:status 200 + content-type + content-length）
pub fn hpack_encode_response(status: u16, content_type: &str, body_len: usize) -> Vec<u8> {
    let mut out = Vec::new();
    // :status（静态表索引 8 = :status 200；若 status != 200 用字面量）
    if status == 200 {
        out.push(0x88);
    } else {
        out.push(0x40 | 8); // 增量索引 + name 索引 8（:status）—— name 索引 8
        out.extend_from_slice(&hpack_encode_str(&status.to_string()));
    }
    // content-type 字面量（静态表索引 31 = content-type）
    out.push(0x40 | 31);
    out.extend_from_slice(&hpack_encode_str(content_type));
    // content-length 字面量（静态表索引 28 = content-length）
    out.push(0x40 | 28);
    out.extend_from_slice(&hpack_encode_str(&body_len.to_string()));
    out
}

// ==================== 帧层 ====================

struct H2Frame {
    ty: u8,
    flags: u8,
    stream: u32,
    payload: Vec<u8>,
}

/// 精确读 n 字节：先消费 pending（请求头缓冲残留），不足再读连接
fn read_exact_from(conn: &mut SConn, pending: &mut Vec<u8>, buf: &mut [u8]) -> Result<(), String> {
    let mut got = 0;
    while got < buf.len() && !pending.is_empty() {
        let take = (buf.len() - got).min(pending.len());
        buf[got..got + take].copy_from_slice(&pending[..take]);
        pending.drain(..take);
        got += take;
    }
    while got < buf.len() {
        match conn.read(&mut buf[got..]) {
            Ok(n) if n > 0 => got += n,
            Ok(_) => return Err("h2 EOF".into()),
            Err(e) if e.kind() == std::io::ErrorKind::WouldBlock
                || e.kind() == std::io::ErrorKind::TimedOut =>
            {
                // 非阻塞/超时：客户端可能在处理 101 响应，短暂重试
                std::thread::sleep(std::time::Duration::from_millis(5));
            }
            Err(e) => return Err(format!("h2 读失败(got={}): {}", got, e)),
        }
    }
    Ok(())
}

fn read_frame(conn: &mut SConn, pending: &mut Vec<u8>) -> Result<H2Frame, String> {
    let mut h = [0u8; 9];
    read_exact_from(conn, pending, &mut h)
        .map_err(|e| format!("h2 读帧头失败: {}", e))?;
    let len = ((h[0] as usize) << 16) | ((h[1] as usize) << 8) | h[2] as usize;
    let ty = h[3];
    let flags = h[4];
    let stream = ((h[5] as u32) << 24) | ((h[6] as u32) << 16) | ((h[7] as u32) << 8) | h[8] as u32;
    if len > 1 << 20 {
        return Err(format!("h2 帧过长: {}", len));
    }
    let mut payload = vec![0u8; len];
    if len > 0 {
        read_exact_from(conn, pending, &mut payload)
            .map_err(|e| format!("h2 读帧体失败: {}", e))?;
    }
    Ok(H2Frame { ty, flags, stream, payload })
}

fn write_frame(conn: &mut SConn, ty: u8, flags: u8, stream: u32, payload: &[u8]) -> Result<(), String> {
    let mut h = [0u8; 9];
    let len = payload.len();
    h[0] = ((len >> 16) & 0xff) as u8;
    h[1] = ((len >> 8) & 0xff) as u8;
    h[2] = (len & 0xff) as u8;
    h[3] = ty;
    h[4] = flags;
    h[5] = ((stream >> 24) & 0xff) as u8;
    h[6] = ((stream >> 16) & 0xff) as u8;
    h[7] = ((stream >> 8) & 0xff) as u8;
    h[8] = (stream & 0xff) as u8;
    conn.write_all(&h)
        .and_then(|_| conn.write_all(payload))
        .and_then(|_| conn.flush())
        .map_err(|e| format!("h2 写帧失败: {}", e))
}

/// 处理 HTTP/2 连接（upgrade=true 为 h2c 已读 HTTP/1.1 请求头；否则 prior knowledge）
/// residual：read_http_conn 读取请求头时已缓冲的字节（可能含 preface 前几字节）
/// 简化：单流 GET/POST 响应（返回简单 HTML，后续可扩展静态文件/脚本）
pub fn h2_serve(conn: &mut SConn, upgrade: bool, residual: &[u8]) -> Result<(), String> {
    if upgrade {
        conn.write_all(
            b"HTTP/1.1 101 Switching Protocols\r\nConnection: Upgrade\r\nUpgrade: h2c\r\n\r\n",
        )
        .map_err(|e| format!("h2c 101 失败: {}", e))?;
        conn.flush().ok();
    }
    // 服务端 SETTINGS（空）
    write_frame(conn, 4, 0, 0, &[])?;
    // 读 client preface（24 字节）：先消费 residual（可能含 preface+SETTINGS+HEADERS 残留）
    let mut pending: Vec<u8> = residual.to_vec();
    let mut preface = vec![0u8; 24];
    read_exact_from(conn, &mut pending, &mut preface)
        .map_err(|e| format!("h2 preface 失败: {}", e))?;
    if &preface[..] != b"PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n" {
        return Err("h2 preface 无效".into());
    }
    // pending 剩余（preface 后）留作帧读取
    loop {
        let f = read_frame(conn, &mut pending)?;
        match f.ty {
            4 => {
                // SETTINGS：无 ACK 标志则回 ACK
                if f.flags & 1 == 0 {
                    write_frame(conn, 4, 1, 0, &[])?;
                }
            }
            6 => {
                // PING：回 ACK（原样 payload）
                write_frame(conn, 6, 1, 0, &f.payload)?;
            }
            1 => {
                // HEADERS：HPACK 解码 → 找 :method/:path → 简单响应
                let hdrs = hpack_decode(&f.payload)?;
                let mut method = "GET";
                let mut path = "/";
                for (k, v) in &hdrs {
                    if k == ":method" {
                        method = v;
                    } else if k == ":path" {
                        path = v;
                    }
                }
                let body = format!(
                    "<h1>PuXian HTTP/2</h1><p>method={} path={}</p>\n",
                    method, path
                );
                let hb = hpack_encode_response(200, "text/html; charset=utf-8", body.len());
                write_frame(conn, 1, 0x4, f.stream, &hb)?; // HEADERS END_HEADERS
                write_frame(conn, 0, 0x1, f.stream, body.as_bytes())?; // DATA END_STREAM
            }
            3 | 8 => {} // RST_STREAM / WINDOW_UPDATE 忽略
            7 => break, // GOAWAY
            _ => {}
        }
    }
    Ok(())
}
