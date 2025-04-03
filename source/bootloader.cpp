#include <cstddef>
#include <cstdint>
#include "bootloader.h"

/**
 * @return true if the application is valid.
 */
bool launch_application() noexcept
{
  if (application_requested_skipping())
  {
    reset_application_skipping_request();
    return true;
  }

  if (!application_is_valid())
  {
    return false;
  }

  jump_to_application();
}

bool restore_application_from_backup() noexcept
{
  if (!application_backup_is_valid())
  {
    return false;
  }

  copy_backup_to_application();
  return true;
}

int main() noexcept
{
  const bool application_valid = launch_application();
  if (!application_valid)
  {
    if (restore_application_from_backup())
      launch_application();
  }

  // failed to launch, the application is not valid, so is the backup
  for (;;)
  {
    // wait for communication
    // and have a timeout here, if the application is valid, that is
  }
}
