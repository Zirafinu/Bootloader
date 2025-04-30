
#include <cstddef>
#include <cstdint>
#include <cstdlib>

extern "C" {
uint32_t application_skip_request = 0;
}

namespace bootloader {
bool application_is_valid() noexcept { return true; }
bool application_update_is_valid() noexcept { return true; }

bool copy_update_to_application() noexcept { return true; }
void jump_to_application() noexcept { std::exit(1); }
} // namespace bootloader
