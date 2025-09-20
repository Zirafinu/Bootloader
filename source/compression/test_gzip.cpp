#include "bootloader/gzip.h"
#include <cstring>

namespace {
#include "test_gzip_compressed.h"
#include "test_gzip_uncompressed.h"

const std::array<uint8_t, 11> in_memory = {0x2b, 0x49, 0x2d, 0x2e, 0xd1, 0x51, 0x28, 0x01, 0x92, 0x5c, 0x00};
std::array<char, 11> out_memory;
const uint8_t *in_memory_end;
const uint8_t *in_memory_read;
char *out_memory_write;

bool can_read_in() { return in_memory_read < in_memory_end; }
uint8_t read_in() { return *in_memory_read++; }
void write_out(uint8_t data) { *out_memory_write++ = data; }
uint8_t read_old_out(std::size_t offset) { return *(out_memory_write - offset); }
} // namespace

int main() {
    { // small test buffer with only fixed data
        auto const h = gzip::Header::parse(in_memory.begin(), in_memory.end());
        in_memory_end = in_memory.end();
        in_memory_read = in_memory.data() + h.length;
        out_memory_write = out_memory.data();
        gzip::Inflate small{can_read_in, read_in, write_out, read_old_out};
        small.decode();
        if (in_memory.end() != in_memory_read)
            return 1;
        if (out_memory.end() != out_memory_write)
            return 2;
        if (std::strcmp(out_memory.data(), "test, test\n") != 0)
            return 3;
    }

    { // a compressed binary image
        std::array<char, std::size(bootloader_bin)> uncompressed;
        out_memory_write = uncompressed.data();

        auto const h = gzip::Header::parse(std::begin(bootloader_bin_gz), std::end(bootloader_bin_gz));
        in_memory_end = std::end(bootloader_bin_gz);
        in_memory_read = bootloader_bin_gz + h.length;
        gzip::Inflate big{can_read_in, read_in, write_out, read_old_out};
        big.decode();
        if (in_memory_read != std::end(bootloader_bin_gz))
            return 4;
        if (out_memory_write != uncompressed.end())
            return 5;
        if (std::memcmp(bootloader_bin, uncompressed.data(), uncompressed.size()) != 0)
            return 6;
    }
    return 0;
}
