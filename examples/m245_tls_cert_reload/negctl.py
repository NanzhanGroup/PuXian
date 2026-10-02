#!/usr/bin/env python3
# M245 门 · 负控打桩：**忠实退回修前** —— 只摘掉 `bi_tls_server` 里的 `g_srv_hs_mu`
#   （1 个 lock + 4 个 unlock），其余（测试钩子 / 注释 / 一切别的代码）**原样保留**。
#
# 为什么不用「整文件回退到 M244」：
#   那样会把**测试钩子**也一起撤掉 ⇒ 「free→parse」窗口不再被放大 ⇒ 竞态退回
#   「靠时序凑巧发作」⇒ 负控就变成不确定的（**M242 缺陷 408 的同族教训**：
#   负控必须在**当前代码**上仍然可判定地判红，否则要换判据而不是换回退范围）。
#
# ⚠️ 锚点设计：修前形态是修后形态的**子串**（摘掉其中一行）⇒ **不能**用「修前形态出现 0 次」
#   当守卫（恒为假）。守卫只用「修后形态恰好出现 1 次」；摘完即 0 ⇒ 重复运行**必然**拒改。
#
# 用法：negctl.py <runtime.c>          # 摘锁（已摘则 rc=3 拒改，不写文件）
#       negctl.py --check <runtime.c>  # 只检查 5 处修后锚点是否唯一（修后状态）
# 退出码：0=成功 3=锚点不符（拒改，不写文件）
import sys

LOCK = ("    pthread_mutex_lock(&g_srv_hs_mu);\n"
        "    pthread_mutex_lock(&g_srv_tls_mu);\n"
        "    int rc1, rc2;")
LOCK_U = ("    pthread_mutex_lock(&g_srv_tls_mu);\n"
          "    int rc1, rc2;")

UN1 = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
       "            pthread_mutex_unlock(&g_srv_hs_mu);\n"
       "            px_error(\"SNI 证书数量超出上限 %d\", PX_MAX_SNI_CERTS);")
UN1_U = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
         "            px_error(\"SNI 证书数量超出上限 %d\", PX_MAX_SNI_CERTS);")

UN2 = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
       "            pthread_mutex_unlock(&g_srv_hs_mu);\n"
       "            px_error(\"tls_server(%s): 证书/私钥解析失败: %s\", hostname, eb);")
UN2_U = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
         "            px_error(\"tls_server(%s): 证书/私钥解析失败: %s\", hostname, eb);")

UN3 = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
       "            pthread_mutex_unlock(&g_srv_hs_mu);\n"
       "            px_error(\"tls_server: 证书/私钥解析失败: %s\", eb);")
UN3_U = ("            pthread_mutex_unlock(&g_srv_tls_mu);\n"
         "            px_error(\"tls_server: 证书/私钥解析失败: %s\", eb);")

UN4 = ("    g_srv_tls_ready = 1;\n"
       "    pthread_mutex_unlock(&g_srv_tls_mu);\n"
       "    pthread_mutex_unlock(&g_srv_hs_mu);\n"
       "    return px_bool(true);")
UN4_U = ("    g_srv_tls_ready = 1;\n"
         "    pthread_mutex_unlock(&g_srv_tls_mu);\n"
         "    return px_bool(true);")

PAIRS = [
    (LOCK, LOCK_U, "register-lock"),
    (UN1, UN1_U, "exit-slot-full"),
    (UN2, UN2_U, "exit-sni-parse"),
    (UN3, UN3_U, "exit-default-parse"),
    (UN4, UN4_U, "exit-normal"),
]


def main():
    args = list(sys.argv[1:])
    check = False
    if args and args[0] == "--check":
        check = True
        args = args[1:]
    if not args:
        print("用法：negctl.py [--check] <runtime.c>", file=sys.stderr)
        return 3
    path = args[0]
    s = open(path, encoding="utf-8").read()

    bad = []
    for fixed, _unfixed, name in PAIRS:
        n = s.count(fixed)
        if n != 1:
            bad.append(f"  ✗ {name}：修后形态命中 {n} 次（必须恰好 1 次）")
    if bad:
        print("❌ 锚点不符，拒改（未写文件）：")
        print("\n".join(bad))
        return 3
    if check:
        print("✅ 5 处修后锚点全部唯一命中")
        return 0
    for fixed, unfixed, _ in PAIRS:
        s = s.replace(fixed, unfixed, 1)
    open(path, "w", encoding="utf-8").write(s)
    # 正证：摘完必须恰好少 4 个 hs_mu unlock、1 个 hs_mu lock
    print("✅ 已摘掉 g_srv_hs_mu（1 lock + 4 unlock）—— 退回 M245 修前")
    return 0


if __name__ == "__main__":
    sys.exit(main())
