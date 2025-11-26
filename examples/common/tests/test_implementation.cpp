#include <bootloader/version_info.h>

#include <cstdio>

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
 * @param is_application_memory_valid indicates whether the application memory can be trusted
 * @return true if the update memory holds a valid replacement for the application
 */
bool application_update_is_valid(bool is_application_memory_valid) noexcept;

/**
 * @brief replaces the current application with the update version.
 * it's only called if the \ref application_update_is_valid() returned true.
 * @return true if the application was replaced successfully
 */
bool copy_update_to_application() noexcept;
} // namespace bootloader

int main() {
    if (bootloader::application_is_valid()) {
        std::printf("Error : An empty application memory is reported to be valid\n");
        return 1;
    }

    if (!bootloader::application_update_is_valid(false)) {
        std::printf("Error : Update is invalid for an invalid application\n");
        return 2;
    }

    if (!bootloader::copy_update_to_application()) {
        std::printf("Error : Performing the update failed\n");
        return 3;
    }

    if (!bootloader::application_is_valid()) {
        std::printf("Error : The application is not valid after the update\n");
        return 4;
    }

    if (!bootloader::application_update_is_valid(true)) {
        std::printf("Error : The update is invalid for a valid application of the same version\n");
        return 5;
    }

    return 0;
}
