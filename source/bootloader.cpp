#include "bootloader.h"
#include <cstddef>
#include <cstdint>

namespace {

/**
 * @return true if the application is valid.
 */
bool launch_application() noexcept {
    if (skip_application::skip_is_requested()) {
        skip_application::skip_reset_request();
        return true;
    }
    if (skip_application::update_is_requested()) {
        skip_application::update_reset_request();
        return false;
    }

    if (!bootloader::application_is_valid()) {
        return false;
    }

    bootloader::jump_to_application();
}

bool restore_application_from_backup() noexcept {
    if (!bootloader::application_backup_is_valid()) {
        return false;
    }

    return bootloader::copy_backup_to_application();
}
} // namespace

extern "C" int main() noexcept {
    const bool application_valid = launch_application();
    if (!application_valid) {
        if (restore_application_from_backup()) {
            launch_application();
            // failed to launch after updateing
            return 2;
        }
        return 1;
    }
    // failed to launch, the application is not valid, so is the backup
    return 0;
}
