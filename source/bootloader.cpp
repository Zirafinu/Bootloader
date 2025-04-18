#include "bootloader.h"
#include <cstddef>
#include <cstdint>

namespace {

/**
 * @return true if the application is valid.
 */
bool launch_application() noexcept {
    if (skip_application::is_requested()) {
        skip_application::reset_request();
        return true;
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

extern "C" int bootloader_main() noexcept {
    const bool application_valid = launch_application();
    if (!application_valid) {
        if (restore_application_from_backup())
            launch_application();
    }

    // failed to launch, the application is not valid, so is the backup
    for (;;) {
        // wait for communication
        // and have a timeout here, if the application is valid, that is
    }
}
