#!/bin/python3
import sys

combined_srec = []
input_file_count = len(sys.argv) - 1
if input_file_count < 1:
    raise Exception("no output file was provided")

with open(sys.argv[input_file_count], "w") as combined:
    termination_line = ""
    for i in range(1, input_file_count):
        with open(sys.argv[i], "r") as srec:
            current_srec = srec.readlines()

        if i == 1:
            termination_line = current_srec[-1]

        current_srec = current_srec[:-1]
        if i > 1:
            current_srec = current_srec[1:]
        combined.writelines(current_srec)
    combined.write(termination_line)
