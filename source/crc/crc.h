#include <cstdint>

namespace crc {
uint32_t compute(const void *mem_begin, const void *mem_end) noexcept;
}
