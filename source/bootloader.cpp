#include "bootloader.h"
#include <cstddef>
#include <cstdint>

namespace {

/**
 * @return true if the application is should be replaced.
 */
bool launch_application() noexcept {
    if (skip_application::skip_is_requested()) {
        skip_application::skip_reset_request();
        return false;
    }

    if (skip_application::update_is_requested()) {
        skip_application::update_reset_request();
        return true;
    }

    if (!bootloader::application_is_valid()) {
        return true;
    }

    bootloader::jump_to_application();
}

/**
 * @return true if the application was replaced
 */
bool update_application() noexcept {
    if (!bootloader::application_update_is_valid()) {
        return false;
    }

    return bootloader::copy_update_to_application();
}
} // namespace

extern "C" int main() noexcept {
    const bool is_update_pending = launch_application();
    if (is_update_pending) {
        if (update_application()) {
            launch_application();
            // failed to launch after updateing
            return 2;
        }
        return 1;
    }
    // application requested skipping
    return 0;
}
