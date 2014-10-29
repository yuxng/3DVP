#!/usr/bin/env python
import os
import sys
from operator import itemgetter
import gmailsend as gmail

ifile = "acflist.txt"
odir = "acfout"

expdescription="I'm running ACF experiments.\n"

# write hadoop input file
f = open(ifile, 'w')
for s in range(1, 126):
    f.write(str(s)+"\n")
f.close()

resultdir = "/workplace/local/wongun/yuxiang/SLM/ACF/cache/"
cmd = "rm " + resultdir + "*"
os.system(cmd)
resultdir = "/workplace/local/wongun/yuxiang/SLM/ACF/data/"
cmd = "rm " + resultdir + "*"
os.system(cmd)

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
-mapper \"python /net/skyserver30/workplace/local/wongun/yuxiang/SLM/ACF/hadoop/mapper.py\" \
-input \"local/"+ifile+"\" \
-output \"local/temp/\""
os.system(hadoopcmd)

# do your own batch processing after collecting all of the result files - ie. detection decoding

# I suggest to generate the final results and send the summary to your email
# title = "Detection experiment {0} is done".format(1)
# contents = "You can find the result at some directory. The testing accuracy for the training set is XXX.\n"+expdescription
# recipient = "yuxiang@umich.edu"

# gmail.sendemail(title, contents, recipient)
