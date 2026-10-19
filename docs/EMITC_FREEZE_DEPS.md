# 发射冻结门 · 类别 B 的已知盲区：传递依赖（M313 归因留档）

## 症状
改了 registry 里的模块（如 registry/mysql/0.2.0/mysql.px），
import 它的用例（examples/m313_mysql_driver/probe.px）会被判「类别 B · 发射行为回归」：
入口 .px 的 src 指纹没变，但 emit-c 输出变了。

## 为什么
emitc_freeze.sh 的源码指纹只对**入口文件**做 sha256，import 的模块不在指纹里。
于是「依赖模块有意改」与「发射路径回归」在判据上不可区分。

## 本次（M313 · HEAD 1924038）实测
- probe.px out 48e52a19… → 336707b5…（rc 都是 0）
- 隔离法：同编译器 + 同 probe.px，仅把 mysql.px 换回 304db24 ⇒ 差异**完全消失**
- 240 行差异全部来自 mysql.px：字符串前缀 "mysql: " 2 条 + mysql_ping / mysql_select_db
  的槽位与指令数（nslots 44→32 / 56→44）
⇒ 非发射回归；按门文档「有意改发射 ⇒ --freeze + 说明」重定基（482 件，仅 1 行变化）。

## 待办（M314）
1. 源码指纹升级为**传递指纹**：入口 + 递归 import 的 sha 合并后再算；
2. 重定基时输出「差异行 → 所属模块」的归因提示，减少人工排查。
