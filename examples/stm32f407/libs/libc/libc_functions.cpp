#include <cstddef>
#include <cstdint>

extern "C" {
void *memcpy(void *dst, const void *src, size_t length) {
    auto dst_addr = reinterpret_cast<size_t>(dst);
    auto src_addr = reinterpret_cast<size_t>(src);
    auto const end_addr = dst_addr + length;

    if (((dst_addr % 4) == (src_addr % 4)) && (length >= 8)) {
        while ((dst_addr % 4) != 0) {
            *reinterpret_cast<uint8_t *>(dst_addr) = *reinterpret_cast<uint8_t const *>(src_addr);
            src_addr += 1;
            dst_addr += 1;
        }
        const size_t end_fast_copy_addr = end_addr - (12 * sizeof(size_t));
        while (dst_addr <= end_fast_copy_addr) [[likely]] {
            // load 4 words from src and then store them to dst, while incrementing the addresses
            __asm__ volatile("LDMIA %1!, {r4-r7}\n"
                             "STMIA %0!, {r4-r7}\n"
                             "LDMIA %1!, {r4-r7}\n"
                             "STMIA %0!, {r4-r7}\n"
                             "LDMIA %1!, {r4-r7}\n"
                             "STMIA %0!, {r4-r7}\n"
                             : "+r"(dst_addr), "+r"(src_addr)
                             :
                             : "r4", "r5", "r6", "r7");
        }

        while (dst_addr <= (end_addr - sizeof(size_t))) [[likely]] {
            *reinterpret_cast<size_t *>(dst_addr) = *reinterpret_cast<size_t const *>(src_addr);
            src_addr += sizeof(size_t);
            dst_addr += sizeof(size_t);
        }
    }
    while (dst_addr != end_addr) {
        *reinterpret_cast<uint8_t *>(dst_addr) = *reinterpret_cast<uint8_t const *>(src_addr);
        src_addr += 1;
        dst_addr += 1;
    }
    return dst;
}

void *memset(void *dst, int value, size_t length) {
    auto dst_addr = reinterpret_cast<size_t>(dst);
    auto const end_addr = dst_addr + length;

    if (length >= 16) {
        while ((dst_addr % 4) != 0) {
            *reinterpret_cast<uint8_t *>(dst_addr) = uint8_t(value);
            dst_addr += 1;
        }

        const size_t value_x4 = 0x01010101 * uint8_t(value);
        const size_t end_fast_set_addr = (end_addr - (4 * sizeof(size_t)));
        if (dst_addr <= end_fast_set_addr) {
            do {
                // store 4 copies at a time, while incrementing the addresses
                __asm__ volatile("STRD %1, %1, [%0], +8\n"
                                 "STRD %1, %1, [%0], +8"
                                 : "+r"(dst_addr)
                                 : "r"(value_x4));
            } while (dst_addr <= end_fast_set_addr);
        }

        while (dst_addr <= (end_addr - sizeof(size_t))) {
            *reinterpret_cast<size_t *>(dst_addr) = value_x4;
            dst_addr += sizeof(size_t);
        }
    }
    while (dst_addr != end_addr) {
        *(uint8_t *)(dst_addr) = value;
        dst_addr += 1;
    }
    return dst;
}
}