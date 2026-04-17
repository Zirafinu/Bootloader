import sys
import zlib
from Cryptodome import Random
from Cryptodome.Cipher import AES

with open(sys.argv[1], "rb") as binary_file:
    binary = binary_file.read()

compressed_binary = zlib.compress(binary, level=9)
with open(sys.argv[3], "rb") as key_file:
    key = key_file.read()
    iv = Random.get_random_bytes(16)
    padding = Random.get_random_bytes(16 - (len(compressed_binary) % 16))
    padded_binary = compressed_binary + (0 if len(padding) == 16 else padding)
    encrypted_binary = AES.new(key, AES.MODE_CBC, iv=iv).encrypt(padded_binary)

with open(sys.argv[2], "rb") as package_file:
    package = package_file.read()

with open(sys.argv[2], "wb") as package_file:
    package_file.write(package)
    package_file.write(iv)
    package_file.write(encrypted_binary)
