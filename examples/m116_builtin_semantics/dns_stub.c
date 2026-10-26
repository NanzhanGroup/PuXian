// ============================================================
// dns_stub.c —— M116 §D5 的**本地 DNS 桩 + 通道探针**（M333 · 缺陷 584 收口）
// ------------------------------------------------------------
// 为什么要有这个文件：
//   缺陷 584 = 「探针探的通道 ≠ 被测通道」。D5 要证的是「UDP 响应被截断（TC=1）
//   ⇒ dns_txt 改走 TCP 53 重发」，它的前提是 **UDP 53 与 TCP 53 都可达**，
//   而老门只拿 getaddrinfo（UDP 侧）探网 ⇒ 探针过、被测通道不通 ⇒ 把环境不可用
//   判成产品红（CI run 40308948055 就是这么红的）。
//   ⇒ 本桩把「UDP 截断 + TCP 多记录」这套语义**由门自己造**，零外网、确定性。
//
// 用法：
//   dns_stub serve <port> <hits_log> <normal|tcp-broken|udp-only>
//   dns_stub probe <ip> <port>      # UDP 53 + TCP 53 双通道探针（外网档用）
//   dns_stub expected               # 桩在 TCP 面给出的 3 条 TXT（判据侧真值）
//   dns_stub decoy                  # 桩在 UDP 截断面塞的诱饵（判别子）
//
// 桩的名字语义（QUERY 名 → 行为）：
//   trunc.dnsstub.test   UDP: TC=1 + 1 条诱饵 "decoy"（ANCOUNT=1，故意带答案）
//                        TCP: TC=0 + 3 条长 TXT（每条两段 character-string，
//                             合并后 190 字节；3 条 RDATA 合计 >512 ⇒ 真会截断）
//   plain.dnsstub.test   UDP/TCP: 1 条 "plain-udp"，不置 TC（证明「不是无脑走 TCP」）
//   nx.dnsstub.test      NXDOMAIN（rcode=3）⇒ 客户端应返回空 list 而不是报错
//   empty.dnsstub.test   NOERROR + ANCOUNT=0 ⇒ 同样空 list
//   其它名字              NXDOMAIN
//
// 桩的每一次查询都以 `<udp|tcp> <name>` 追加一行到 hits_log ——
//   「回退真的发生了」由此**直接可观测**（tcp trunc.* 命中 ≥1 且 udp trunc.* 恰 1 次），
//   而不是靠「返回值看着对」推断。
//
// 判据侧的判别子（NC1）：UDP 面给的诱饵必须与 TCP 面给的真值**不同**，否则
//   「有没有回退」在返回值上不可分 ⇒ 判据没有牙。
// ============================================================
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <poll.h>
#include <ctype.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define RBUF 4096
#define QBUF 1024

static const char *g_log = NULL;

static void hit(const char *proto, const char *name) {
    if (!g_log) return;
    FILE *f = fopen(g_log, "a");
    if (!f) return;
    fprintf(f, "%s %s\n", proto, name);
    fclose(f);
}

// 每条 TXT 由两段 character-string 组成（客户端必须按 RFC 合并）
static void rec_body(int n, char *cs1, char *cs2) {
    int i, k = sprintf(cs1, "T%d-A:", n);
    for (i = 0; i < 90; i++) cs1[k++] = 'a';
    cs1[k] = 0;
    k = sprintf(cs2, "T%d-B:", n);
    for (i = 0; i < 90; i++) cs2[k++] = 'b';
    cs2[k] = 0;
}

static int cmd_expected(void) {
    int n;
    for (n = 1; n <= 3; n++) {
        char a[256], b[256];
        rec_body(n, a, b);
        printf("R%d=%s%s\n", n - 1, a, b);
    }
    return 0;
}

// 解析 QUERY 的 QNAME（小写化），返回 question 段结束偏移（QNAME+4）
static int parse_q(const unsigned char *q, int ql, char *name, int namesz, int *qend) {
    int off = 12, n = 0;
    if (ql < 12) return -1;
    name[0] = 0;
    for (;;) {
        int l;
        if (off >= ql) return -1;
        l = q[off];
        if (l == 0) { off++; break; }
        if ((l & 0xC0) == 0xC0) { off += 2; break; }
        if (l > 63 || off + 1 + l > ql) return -1;
        if (n + l + 2 >= namesz) return -1;
        if (n) name[n++] = '.';
        memcpy(name + n, q + off + 1, (size_t)l);
        n += l;
        name[n] = 0;
        off += 1 + l;
    }
    if (off + 4 > ql) return -1;
    for (n = 0; name[n]; n++) name[n] = (char)tolower((unsigned char)name[n]);
    *qend = off + 4;
    return 0;
}

// 追加一条 TXT answer（owner = 指向 offset 12 的压缩指针）
static int txt_answer(unsigned char *r, int pos, const char *s1, const char *s2) {
    int rdlen_at, rd0, l1, l2, rdlen;
    r[pos++] = 0xC0; r[pos++] = 0x0C;                       // owner: ptr->12
    r[pos++] = 0;    r[pos++] = 16;                         // TYPE TXT
    r[pos++] = 0;    r[pos++] = 1;                          // CLASS IN
    r[pos++] = 0; r[pos++] = 0; r[pos++] = 0; r[pos++] = 60; // TTL 60s
    rdlen_at = pos; r[pos++] = 0; r[pos++] = 0;             // RDLENGTH 占位
    rd0 = pos;
    l1 = (int)strlen(s1);
    r[pos++] = (unsigned char)l1;
    memcpy(r + pos, s1, (size_t)l1); pos += l1;
    if (s2 && s2[0]) {
        l2 = (int)strlen(s2);
        r[pos++] = (unsigned char)l2;
        memcpy(r + pos, s2, (size_t)l2); pos += l2;
    }
    rdlen = pos - rd0;
    r[rdlen_at] = (unsigned char)((rdlen >> 8) & 0xFF);
    r[rdlen_at + 1] = (unsigned char)(rdlen & 0xFF);
    return pos;
}

static int build_resp(const unsigned char *q, int qend, const char *name,
                      int tcp, unsigned char *r, int rcap) {
    char a[256], b[256];
    int pos, rcode = 0, an = 0, tc = 0, n;
    if (qend > rcap - 16) return -1;
    memcpy(r, q, (size_t)qend);
    pos = qend;
    if (strcmp(name, "trunc.dnsstub.test") == 0) {
        if (!tcp) {                       // UDP：真截断 + 诱饵
            tc = 1; an = 1;
            pos = txt_answer(r, pos, "decoy", NULL);
        } else {                          // TCP：完整 3 条长 TXT
            an = 3;
            for (n = 1; n <= 3; n++) {
                rec_body(n, a, b);
                pos = txt_answer(r, pos, a, b);
            }
        }
    } else if (strcmp(name, "plain.dnsstub.test") == 0) {
        an = 1;
        pos = txt_answer(r, pos, "plain-udp", NULL);
    } else if (strcmp(name, "empty.dnsstub.test") == 0) {
        an = 0;
    } else {
        rcode = 3;                        // NXDOMAIN
        an = 0;
    }
    r[2] = (unsigned char)(0x81 | (tc ? 0x02 : 0x00));  // QR · RD · TC
    r[3] = (unsigned char)(0x80 | (rcode & 0x0F));      // RA · RCODE
    r[6] = (unsigned char)((an >> 8) & 0xFF);
    r[7] = (unsigned char)(an & 0xFF);
    r[8] = r[9] = r[10] = r[11] = 0;                    // NS/AR = 0
    return pos;
}

static int read_full(int fd, unsigned char *buf, int need) {
    int got = 0;
    while (got < need) {
        struct pollfd p = { fd, POLLIN, 0 };
        int pr = poll(&p, 1, 3000);
        ssize_t n;
        if (pr <= 0) return -1;
        n = recv(fd, buf + got, (size_t)(need - got), 0);
        if (n <= 0) return -1;
        got += (int)n;
    }
    return got;
}

static int cmd_serve(int port, const char *logp, const char *mode) {
    int udp_fd, tcp_fd = -1, idle = 0;
    struct sockaddr_in sa;
    unsigned char q[QBUF], r[RBUF];
    int broken = (strcmp(mode, "tcp-broken") == 0);
    int udp_only = (strcmp(mode, "udp-only") == 0);

    g_log = logp;
    udp_fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (udp_fd < 0) { fprintf(stderr, "STUB_ERR: udp socket: %s\n", strerror(errno)); return 3; }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_addr.s_addr = inet_addr("127.0.0.1");
    sa.sin_port = htons((unsigned short)port);
    if (bind(udp_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        fprintf(stderr, "STUB_ERR: bind udp %d: %s\n", port, strerror(errno));
        return 3;
    }
    if (!udp_only) {
        int opt = 1;
        tcp_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (tcp_fd < 0) { fprintf(stderr, "STUB_ERR: tcp socket: %s\n", strerror(errno)); return 3; }
        setsockopt(tcp_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
        if (bind(tcp_fd, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
            fprintf(stderr, "STUB_ERR: bind tcp %d: %s\n", port, strerror(errno));
            return 3;
        }
        if (listen(tcp_fd, 8) < 0) { fprintf(stderr, "STUB_ERR: listen: %s\n", strerror(errno)); return 3; }
    }
    fprintf(stderr, "STUB_READY port=%d mode=%s udp=%d tcp=%d\n", port, mode, udp_fd, tcp_fd);
    fflush(stderr);

    for (;;) {
        struct pollfd pfds[2];
        int nf = 0, udp_i = -1, tcp_i = -1, pr;
        pfds[nf].fd = udp_fd; pfds[nf].events = POLLIN; udp_i = nf; nf++;
        if (tcp_fd >= 0) { pfds[nf].fd = tcp_fd; pfds[nf].events = POLLIN; tcp_i = nf; nf++; }
        pr = poll(pfds, (nfds_t)nf, 1000);
        if (pr < 0) { if (errno == EINTR) continue; break; }
        if (pr == 0) { if (++idle > 120) break; continue; }
        idle = 0;
        if (udp_i >= 0 && (pfds[udp_i].revents & POLLIN)) {
            struct sockaddr_in cli;
            socklen_t cl = sizeof(cli);
            int ql = (int)recvfrom(udp_fd, q, sizeof(q), 0, (struct sockaddr *)&cli, &cl);
            if (ql > 12) {
                char name[256];
                int qend = 0;
                if (parse_q(q, ql, name, sizeof(name), &qend) == 0) {
                    int rl = build_resp(q, qend, name, 0, r, (int)sizeof(r));
                    if (rl > 0) sendto(udp_fd, r, (size_t)rl, 0, (struct sockaddr *)&cli, cl);
                    hit("udp", name);
                }
            }
        }
        if (tcp_i >= 0 && (pfds[tcp_i].revents & POLLIN)) {
            int cfd = accept(tcp_fd, NULL, NULL);
            if (cfd >= 0) {
                unsigned char lb[2];
                if (broken) {                     // 故意破损：接受后立刻关（TCP 面不可用）
                    hit("tcp-broken", "(conn)");
                    close(cfd);
                } else if (read_full(cfd, lb, 2) == 2) {
                    int ql = (lb[0] << 8) | lb[1];
                    if (ql > 12 && ql < QBUF && read_full(cfd, q, ql) == ql) {
                        char name[256];
                        int qend = 0;
                        if (parse_q(q, ql, name, sizeof(name), &qend) == 0) {
                            int rl = build_resp(q, qend, name, 1, r, (int)sizeof(r));
                            if (rl > 0) {
                                unsigned char out[RBUF + 2];
                                out[0] = (unsigned char)((rl >> 8) & 0xFF);
                                out[1] = (unsigned char)(rl & 0xFF);
                                memcpy(out + 2, r, (size_t)rl);
                                (void)!write(cfd, out, (size_t)rl + 2);
                            }
                            hit("tcp", name);
                        }
                    }
                }
                close(cfd);
            }
        }
    }
    if (tcp_fd >= 0) close(tcp_fd);
    close(udp_fd);
    return 0;
}

// ---- 双通道探针（外网档用；**必须能报不可达**，否则它永远是绿的 = 没有保护）----
static int build_query(const char *name, unsigned char *q) {
    int ql = 12, i;
    unsigned short id = (unsigned short)(getpid() & 0xFFFF);
    q[0] = (unsigned char)(id >> 8); q[1] = (unsigned char)(id & 0xFF);
    q[2] = 0x01; q[3] = 0x00; q[4] = 0; q[5] = 1;
    q[6] = q[7] = q[8] = q[9] = q[10] = q[11] = 0;
    for (i = 0; name[i];) {
        const char *dot = strchr(name + i, '.');
        int l = dot ? (int)(dot - (name + i)) : (int)strlen(name + i);
        q[ql++] = (unsigned char)l;
        memcpy(q + ql, name + i, (size_t)l);
        ql += l;
        i += l;
        if (dot) i++;
    }
    q[ql++] = 0;
    q[ql++] = 0; q[ql++] = 16;   // TXT
    q[ql++] = 0; q[ql++] = 1;    // IN
    return ql;
}

static int probe_udp(const char *ip, int port, char *why, int whysz) {
    unsigned char q[256], rb[1024];
    int ql = build_query("probe.dnsstub.test", q), fd, n;
    struct sockaddr_in sa;
    struct pollfd p;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) { snprintf(why, whysz, "socket:%s", strerror(errno)); return -1; }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &sa.sin_addr) != 1) { close(fd); snprintf(why, whysz, "bad-ip"); return -1; }
    if (sendto(fd, q, (size_t)ql, 0, (struct sockaddr *)&sa, sizeof(sa)) < 0) {
        snprintf(why, whysz, "send:%s", strerror(errno)); close(fd); return -1;
    }
    p.fd = fd; p.events = POLLIN; p.revents = 0;
    n = poll(&p, 1, 3000);
    if (n <= 0) { close(fd); snprintf(why, whysz, "timeout-3s"); return -1; }
    n = (int)recv(fd, rb, sizeof(rb), 0);
    close(fd);
    if (n <= 0) { snprintf(why, whysz, "recv:%s", strerror(errno)); return -1; }
    return 0;
}

static int probe_tcp(const char *ip, int port, char *why, int whysz) {
    unsigned char q[256], lb[2], rb[1024];
    int ql = build_query("probe.dnsstub.test", q), fd, err = 0, n;
    socklen_t el = sizeof(err);
    struct sockaddr_in sa;
    struct pollfd p;
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) { snprintf(why, whysz, "socket:%s", strerror(errno)); return -1; }
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons((unsigned short)port);
    if (inet_pton(AF_INET, ip, &sa.sin_addr) != 1) { close(fd); snprintf(why, whysz, "bad-ip"); return -1; }
    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);
    if (connect(fd, (struct sockaddr *)&sa, sizeof(sa)) < 0 && errno != EINPROGRESS) {
        snprintf(why, whysz, "connect:%s", strerror(errno)); close(fd); return -1;
    }
    p.fd = fd; p.events = POLLOUT; p.revents = 0;
    n = poll(&p, 1, 3000);
    if (n <= 0) { close(fd); snprintf(why, whysz, "connect-timeout-3s"); return -1; }
    if (getsockopt(fd, SOL_SOCKET, SO_ERROR, &err, &el) != 0 || err != 0) {
        snprintf(why, whysz, "connect:%s", err ? strerror(err) : "unknown");
        close(fd);
        return -1;
    }
    // 连上之后真发一次查询并读回长度前缀 —— 「accept 了但不响应」也算不可达
    lb[0] = (unsigned char)((ql >> 8) & 0xFF);
    lb[1] = (unsigned char)(ql & 0xFF);
    if (write(fd, lb, 2) != 2 || write(fd, q, (size_t)ql) != ql) {
        snprintf(why, whysz, "send:%s", strerror(errno)); close(fd); return -1;
    }
    p.events = POLLIN; p.revents = 0;
    n = poll(&p, 1, 3000);
    if (n <= 0) { close(fd); snprintf(why, whysz, "no-response-3s"); return -1; }
    n = (int)read(fd, rb, sizeof(rb));
    close(fd);
    if (n <= 0) { snprintf(why, whysz, "recv:%s", strerror(errno)); return -1; }
    return 0;
}

static int cmd_probe(const char *ip, int port) {
    char uw[128] = "", tw[128] = "", uline[200], tline[200];
    int uok = (probe_udp(ip, port, uw, sizeof(uw)) == 0);
    int tok = (probe_tcp(ip, port, tw, sizeof(tw)) == 0);
    if (uok) snprintf(uline, sizeof(uline), "UDP:%d=ok", port);
    else     snprintf(uline, sizeof(uline), "UDP:%d=down(%s)", port, uw);
    if (tok) snprintf(tline, sizeof(tline), "TCP:%d=ok", port);
    else     snprintf(tline, sizeof(tline), "TCP:%d=down(%s)", port, tw);
    printf("%s %s\n", uline, tline);
    fflush(stdout);
    return (uok && tok) ? 0 : 2;
}

int main(int argc, char **argv) {
    if (argc >= 2 && strcmp(argv[1], "serve") == 0 && argc == 5)
        return cmd_serve(atoi(argv[2]), argv[3], argv[4]);
    if (argc >= 2 && strcmp(argv[1], "probe") == 0 && argc == 4)
        return cmd_probe(argv[2], atoi(argv[3]));
    if (argc == 2 && strcmp(argv[1], "expected") == 0)
        return cmd_expected();
    if (argc == 2 && strcmp(argv[1], "decoy") == 0) { printf("R0=decoy\n"); return 0; }
    fprintf(stderr,
            "用法: dns_stub serve <port> <hits_log> <normal|tcp-broken|udp-only>\n"
            "      dns_stub probe <ip> <port>\n"
            "      dns_stub expected | decoy\n");
    return 2;
}
