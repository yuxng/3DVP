#!/usr/bin/env python
import os
import sys

detections = {}

for input_line in sys.stdin:
    output = input_line.strip()
    tokens = output.split(' ')

    if(tokens[0] != "DET"):
        continue

    name = tokens[1]

    frame = int(tokens[2])
    x1 = float(tokens[3])
    y1 = float(tokens[4])
    x2 = float(tokens[5])
    y2 = float(tokens[6])
    comp = int(tokens[7])
    score = float(tokens[8])
    # dummy for now
    objecttype = 0

    if name not in detections:
        detections[name] = {}

    if frame not in detections[name]:
        detections[name][frame] = []

    onedet = [x1, y1, x2, y2, comp, score, objecttype]
    detections[name][frame].append(onedet)

for name in detections:
    for frame in detections[name]:
        print name, "{0:08d}".format(frame), len(detections[name][frame])
        for det in detections[name][frame]:
            for data in det:
                print data,
            print ''
