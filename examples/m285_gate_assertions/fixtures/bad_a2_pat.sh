#!/usr/bin/env bash
if tail -60 /tmp/g.log | grep -q '虚构的判据串甲：乙丙丁'; then echo green; fi
