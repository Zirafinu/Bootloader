#ifndef BOOTLOADER_BOOTLOADER_H_
#define BOOTLOADER_BOOTLOADER_H_

#include <bootloader/skip_application.h>
#include <cstdint>

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

#endif
