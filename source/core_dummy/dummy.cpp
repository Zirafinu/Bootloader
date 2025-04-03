
#include <cstdint>
#include <cstddef>
#include <cstdlib>

extern "C"
{
  uint32_t application_skip_request;
  constexpr uint32_t application_skip_request_true = 0xC1AEE345UL;

  bool application_is_valid() noexcept { return true; }
  bool application_backup_is_valid() noexcept { return true; }
  bool application_requested_skipping() noexcept { return application_skip_request == application_skip_request_true; }
  void reset_application_skipping_request() noexcept { application_skip_request = ~application_skip_request_true; }

  void copy_backup_to_application() noexcept {}
  void jump_to_application() noexcept { std::exit(1); }
}
