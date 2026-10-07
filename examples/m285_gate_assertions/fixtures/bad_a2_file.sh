#!/usr/bin/env bash
grep -q '虚构的锚点注释：甲乙丙' runtime/runtime.h || { bad "锚点缺失"; exit 2; }
