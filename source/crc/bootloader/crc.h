#ifndef BOOTLOADER_CRC_CRC_H_
#define BOOTLOADER_CRC_CRC_H_

#include <cstdint>

namespace crc {
uint32_t compute(const void *mem_begin, const void *mem_end) noexcept;
}

#endif
