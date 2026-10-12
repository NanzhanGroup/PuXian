#!/usr/bin/env bash
set -uo pipefail
gp_patch runtime/vm.h 'PXOP_ITERLEN' 'PXOP_ITERLEN_X' || exit 2
