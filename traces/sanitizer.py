#!/usr/bin/env python3


# A small trace sanitizer for sanitizing traces
# wraps around all addresses exceeding logical SSD size
# and also removes entries with addr+length over SSD size
traceFile = "./systor17-1-short.trace"
outputFile = "./systor17-1-short-hex.trace"
logicalCapacity = 1073741824
pages = {}
with open(traceFile) as f:
    with open(outputFile, 'w') as w:
        for l in f.readlines():
            l = l.split(',')
            addr = int(l[1])
            l[1] = hex(addr % logicalCapacity)[2:]
            length = int(l[2])
            if addr >= logicalCapacity or addr + length >= logicalCapacity:
                print('exceeded range')
            else:
                w.write(','.join(l))
