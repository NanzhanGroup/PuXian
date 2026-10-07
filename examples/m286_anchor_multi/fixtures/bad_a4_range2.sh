#!/usr/bin/env bash
sed -i '/^BEGIN_TWO/,/^END_TWO/ s|ANCHOR_MULTI_Y|REPLACED|' examples/m286_anchor_multi/fixtures/a4_target2.txt
