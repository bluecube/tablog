#!/usr/bin/env python3
""" This is an extremely crude GCode loader, intended only to pre-chew test data
 for tablog, not for any practical use.
 Converts G1 commands to a CSV of x, y, z, e positions """


import sys
import re

def parse_g1(line):
    ret = {}
    for coord in "XYZE":
        match = re.match(r"^G1 .*" + coord + "([\-0-9.]+)", line)
        if match:
            ret[coord] = float(match[1])

    return ret

print("x,y,z,e")
print("f1000(u32),f1000(u32),f1000(u32),f1000(s64)")
with open(sys.argv[1], "r") as fp:
    state = {coord: 0 for coord in "XYZE"}
    for line in fp:
        g1 = parse_g1(line)
        if not len(g1):
            continue

        for coord in "XYZ":
            if coord in g1:
                state[coord] = g1[coord]
        if "E" in g1:
            state["E"] += g1["E"]

        print(",".join(f"{state[coord]:.03f}" for coord in "XYZE"))


