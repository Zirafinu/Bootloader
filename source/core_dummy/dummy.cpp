
#include <cstdint>
#include <cstddef>

extern "C"
{
  bool application_is_valid() noexcept
  {
    return true;
  }

  bool application_backup_is_valid() noexcept
  {
    return true;
  }

  uint32_t application_skip_request;
  constexpr uint32_t application_skip_request_true = 0xC1AEE345UL;
  bool application_requested_skipping() noexcept
  {
    return application_skip_request == application_skip_request_true;
  }
  void reset_application_skipping_request() noexcept
  {
    application_skip_request = ~application_skip_request_true;
  }

  static void erase_application() noexcept {}

  static bool can_read_more() { return false; }
  static uint8_t read_appl_backup() { return 0; }
  static void write_application(uint8_t value) { return; }
  static uint8_t read_application(std::size_t distance) { return 0; }

  void copy_backup_to_application() noexcept {}
}
