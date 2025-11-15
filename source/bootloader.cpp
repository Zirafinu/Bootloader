#include "bootloader/bootloader.h"
#include "bootloader/skip_application.h"

#include <cstddef>
#include <cstdint>

namespace {

enum class Application_State {
    Skip,    /// skip was requested
    Update,  /// update was requested
    Invalid, /// memory check failed
};

/**
 * @return the reason, why the application didn't start.
 */
Application_State launch_application() noexcept {
    if (skip_application::skip_is_requested()) {
        skip_application::skip_reset_request();
        return Application_State::Skip;
    }

    if (skip_application::update_is_requested()) {
        skip_application::update_reset_request();
        return Application_State::Update;
    }

    if (!bootloader::application_is_valid()) {
        return Application_State::Invalid;
    }

    bootloader::jump_to_application();
}

/**
 * @param is_application_memory_valid indicates whether or not the application memory can be trusted
 * @return true if the application was replaced
 */
bool update_application(bool is_application_memory_valid) noexcept {
    if (!bootloader::application_update_is_valid(is_application_memory_valid)) {
        return false;
    }

    return bootloader::copy_update_to_application();
}
} // namespace

extern "C" int main() noexcept {
    const Application_State state = launch_application();
    if (state == Application_State::Invalid || state == Application_State::Update) {
        if (update_application(state == Application_State::Invalid)) {
            launch_application();
            // failed to launch after updateing
            return 2;
        }
        return 1;
    }
    // application requested skipping
    return 0;
}
