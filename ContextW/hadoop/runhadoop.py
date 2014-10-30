#!/usr/bin/env python
import os
import sys
from operator import itemgetter

ifile = "input.txt"
odir = "output"

# prepare data/directory in hdfs
hadoopcmd = "hadoop dfs -rm local/"+ifile
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -rmr local/"+odir
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -mkdir local/"+odir
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -rmr local/temp/"
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -copyFromLocal ./"+ifile+" local/"+ifile
os.system(hadoopcmd)

# run hadoop command
hadoopcmd = "hadoop jar /opt/hadoop/contrib/streaming/hadoop-streaming-1.2.1.jar \
-mapper \"python /net/skyserver30/workplace/local/wongun/yuxiang/SLM/ContextW/hadoop/mapper.py\" \
-input \"local/"+ifile+"\" \
-output \"local/temp/\""
os.system(hadoopcmd)
