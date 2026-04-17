#!/bin/python3
import sys
import time

with open(sys.argv[1], "rb") as version_info_struct:
    struct = version_info_struct.read()

if len(struct) != 12:
    raise ValueError("Bad Version info size")

build_time = int(time.clock_gettime(time.CLOCK_REALTIME) / 60)

with open(sys.argv[1], "wb") as version_info_struct:
    version_info_struct.write(struct[0:8])

    version_info_struct.write(
        build_time.to_bytes(length=4, byteorder="little", signed=False)
    )
