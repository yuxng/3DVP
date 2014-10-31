#!/usr/bin/env python
import os
import sys

slm_bin = 'sh ./run_testall.sh /net/denali/local/mathworks_r2011a_local/'
for line in sys.stdin:
    os.chdir('/net/skyserver30/workplace/local/wongun/yuxiang/SLM/ContextW/hadoop/')
    # one processing
    line = line.strip()
    cmd = slm_bin+' '+line
    os.system(cmd)
