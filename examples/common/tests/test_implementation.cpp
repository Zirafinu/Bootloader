#include <bootloader/version_info.h>

extern "C" __attribute__((used)) const uint8_t application{0};
extern "C" __attribute__((used)) const uint8_t update{0};
extern "C" __attribute__((used)) const uint32_t bootloader_crc{0};

namespace bootloader {

/**
 * @brief launches the application
 * might have to perform cleanup tasks like deinitializing hardware
 */
[[noreturn]]
void jump_to_application() noexcept;

/**
 * @return true if the application is valid
 */
bool application_is_valid() noexcept;

/**
 * @param is_application_memory_valid indicates whether or not the application memory can be trusted
 * @return true if the update memory holds a valid replacement for the application
 */
bool application_update_is_valid(bool is_application_memory_valid) noexcept;

/**
 * @brief replaces the current application with the update version.
 * it's only called if the \ref application_update_is_valid() returned true.
 * @return true if the application was replaced sucessfully
 */
bool copy_update_to_application() noexcept;
} // namespace bootloader

int main() {
    if (!bootloader::application_is_valid()) {
        return 1;
    }

    if (!bootloader::application_update_is_valid(false)) {
        return 2;
    }

    if (!bootloader::application_update_is_valid(true)) {
        return 3;
    }

    if (!bootloader::copy_update_to_application()) {
        return 4;
    }

    return 5;
}
