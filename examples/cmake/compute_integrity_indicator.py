import sys
import zlib

with open(sys.argv[1], "rb") as binary_file:
    binary_without_crc = binary_file.read()[:-4]
    crc = zlib.crc32(binary_without_crc, 0)
    with open(sys.argv[2], "wb") as crc_file:
        crc_file.write(crc.to_bytes(length=4, byteorder="little", signed=False))
