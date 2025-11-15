#include <flash_layout.h>
#include <hardware_specific.h>

#include <cstring>

namespace bootloader::hardware_specific {
void initialize_core_for_update() {}

void initialize_core_for_application_start() {}

[[nodiscard]]
bool flush_write_buffer(uint8_t *dst, std::array<uint8_t, 1024> const &data, std::size_t used) {
    for (size_t i = 0; i < used; ++i) {
        dst[i] = data[i];
    }
    return true;
}

[[nodiscard]]
bool erase_application() {
    std::memset(reinterpret_cast<uint8_t *>(flash_layout::application_begin), 0,
                flash_layout::application_end - flash_layout::application_begin);
    return true;
}
} // namespace bootloader::hardware_specific
