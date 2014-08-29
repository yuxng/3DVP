#!/usr/bin/env python
import os
import sys

slm_bin = '/net/skyserver10/workplace/yxiang/SLM/ACF/mccrun.sh'
# slm_bin = '/net/skyserver10/workplace/yxiang/SLM/ACF/fakerun.sh'
resultdir = '/workplace/hadoop_cache/slm/data/'
odir = "/user/yxiang/yuxiang/acfout/"

for line in sys.stdin:
    # one processing
    line = line.strip()
    cmd = slm_bin+' '+line

    if(not os.path.exists(resultdir)):
        os.makedirs(resultdir)
    os.system(cmd)

    # upload the result files to the hdfs server
    resfile = 'car_'+line+'_final.mat'
    hadoopcmd = "hadoop fs -put "+resultdir+resfile+" "+odir+resfile
    os.system(hadoopcmd)
    # clean up the temporary files in the local server
    cmd = "rm "+resfile
    os.system(hadoopcmd)

    # upload the result files to the hdfs server
    resfile = 'car_'+line+'_test.mat'
    hadoopcmd = "hadoop fs -put "+resultdir+resfile+" "+odir+resfile
    os.system(hadoopcmd)
    # clean up the temporary files in the local server
    cmd = "rm "+resfile
    os.system(hadoopcmd)
