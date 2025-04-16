
#include <cstddef>
#include <cstdint>
#include <cstdlib>

namespace {
uint32_t application_skip_request = 0;
}

namespace bootloader {
bool application_is_valid() noexcept { return true; }
bool application_backup_is_valid() noexcept { return true; }

void copy_backup_to_application() noexcept {}
void jump_to_application() noexcept { std::exit(1); }
} // namespace bootloader
