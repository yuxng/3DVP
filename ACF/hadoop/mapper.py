#!/usr/bin/env python
import os
import sys

slm_bin = './mccrun.sh'
for line in sys.stdin:
    os.chdir('/net/skyserver30/workplace/local/wongun/yuxiang/SLM/ACF/')
    # one processing
    line = line.strip()
    cmd = slm_bin+' '+line
    os.system(cmd)
