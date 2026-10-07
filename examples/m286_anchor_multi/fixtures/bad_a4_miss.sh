#!/usr/bin/env bash
sed -i '/^BEGIN_THREE/,/^END_THREE/ s|ANCHOR_MULTI_W|REPLACED|' examples/m286_anchor_multi/fixtures/a4_target3.txt
