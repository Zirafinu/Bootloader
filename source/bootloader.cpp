#include <cstddef>
#include <cstdint>
#include <flash_layout.h>

uint32_t crc(const void *mem_begin, const void *mem_end) noexcept;

bool application_is_valid() noexcept {
  return *reinterpret_cast<uint32_t *>(flash_layout::appl_end - 4) ==
         crc(reinterpret_cast<uint8_t *>(flash_layout::appl_begin),
             reinterpret_cast<uint8_t *>(flash_layout::appl_end - 4));
}

bool application_backup_is_valid() noexcept {
  return *reinterpret_cast<uint32_t *>(flash_layout::appl_backup_end - 4) ==
         crc(reinterpret_cast<uint8_t *>(flash_layout::appl_backup_begin),
             reinterpret_cast<uint8_t *>(flash_layout::appl_backup_end - 4));
}

extern "C" {
bool application_requested_skipping() noexcept;
void reset_application_skipping_request() noexcept;
[[noreturn]]
void jump_to_application() noexcept;
void copy_backup_to_application() noexcept;
}

/**
 * @return true if the application is valid.
 */
bool launch_application() noexcept {
  if (application_requested_skipping()) {
    reset_application_skipping_request();
    return true;
  }

  if (!application_is_valid()) {
    return false;
  }

  jump_to_application();
}

bool restore_application_from_backup() noexcept {
  if (!application_backup_is_valid()) {
    return false;
  }

  copy_backup_to_application();
  return true;
}

int main() noexcept {
  const bool application_valid = launch_application();
  if (!application_valid) {
    if (restore_application_from_backup())
      launch_application();
  }

  // failed to launch, if application is not valid, so is the backup
  for (;;) {
    // wait for communication
    // and have a timeout here, if the application is valid, that is
  }
}
