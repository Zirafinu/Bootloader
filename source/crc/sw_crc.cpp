#include "bootloader/crc.h"

namespace crc {

static constexpr uint32_t crc32_gzip_tab[16] = {
    0x00000000, 0x04C11DB7, 0x09823B6E, 0x0D4326D9, //
    0x130476DC, 0x17C56B6B, 0x1A864DB2, 0x1E475005, //
    0x2608EDB8, 0x22C9F00F, 0x2F8AD6D6, 0x2B4BCB61, //
    0x350C9B64, 0x31CD86D3, 0x3C8EA00A, 0x384FBDBD, //
};

[[gnu::optimize(3), gnu::hot]]
static inline uint32_t reverse(uint32_t value) {
    constexpr uint32_t mask[5] = {0xFFffUL, 0xFF00ffUL, 0xF0F0f0fUL, 0x33333333UL, 0x55555555UL};
    constexpr std::size_t shifts[5] = {16, 8, 4, 2, 1};
    for (std::size_t i = 0; i < 5; ++i) {
        value = ((value & mask[i]) << shifts[i]) | ((value >> shifts[i]) & mask[i]);
    }
    return value;
}

uint32_t compute(const void *mem_begin, const void *mem_end) noexcept {
    const uint8_t *p = reinterpret_cast<const uint8_t *>(mem_begin);
    uint32_t crc = ~0U;
    while (p != mem_end) {
        const uint32_t byte = reverse(uint32_t(*p));
        ++p;
        crc ^= byte;
        crc = (crc << 4) ^ crc32_gzip_tab[(crc >> 28) & 0xF];
        crc = (crc << 4) ^ crc32_gzip_tab[(crc >> 28) & 0xF];
    }

    return reverse(~crc);
}
} // namespace crc
