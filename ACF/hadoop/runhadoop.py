#!/usr/bin/env python
import os
import sys
from operator import itemgetter
import gmailsend as gmail

ifile = "acflist.txt"
odir = "acfout"

expdescription="I'm running XXX experiment with YYY parameters to see something.\n"

# write hadoop input file
f = open(ifile, 'w')
for s in range(1, 21):
    f.write(str(s)+"\n")
f.close()

# prepare data/directory in hdfs
hadoopcmd = "hadoop dfs -rm yuxiang/"+ifile
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -rmr yuxiang/"+odir
os.system(hadoopcmd)
hadoopcmd = "hadoop dfs -copyFromLocal ./"+ifile+" yuxiang/"+ifile
os.system(hadoopcmd)

# run hadoop command
hadoopcmd = "hadoop jar /opt/hadoop/contrib/streaming/hadoop-streaming-1.2.1.jar \
-mapper \"python $PWD/mapper.py\" \
-input \"yuxiang/"+ifile+"\" \
-output \"yuxiang/"+odir+"/\""
os.system(hadoopcmd)

# download result from hdfs
hadoopcmd = "hadoop fs -get yuxiang/"+odir+" results/"
os.system(hadoopcmd)

# do your own batch processing after collecting all of the result files - ie. detection decoding

# I suggest to generate the final results and send the summary to your email
title = "Detection experiment {0} is done".format(1)
contents = "You can find the result at some directory. The testing accuracy for the training set is XXX.\n"+exdescription
recipient = "yuxiang@umich.edu"

gmail.sendemail(title, contents, recipient)
