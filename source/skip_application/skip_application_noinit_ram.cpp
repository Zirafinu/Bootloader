#include "bootloader/skip_application.h"
#include <cstdint>
namespace {
constexpr uint32_t application_skip_request_true = 0xC1AE'E345UL;
constexpr uint32_t application_update_request_true = 0xC1AE'543EUL;
} // namespace

extern "C" uint32_t application_skip_request;
namespace skip_application {
bool skip_is_requested() noexcept { return application_skip_request == application_skip_request_true; }
void skip_reset_request() noexcept { application_skip_request = ~application_skip_request_true; }
void skip_request() noexcept { application_skip_request = application_skip_request_true; }

bool update_is_requested() noexcept { return application_skip_request == application_update_request_true; }
void update_reset_request() noexcept { application_skip_request = ~application_update_request_true; }
void update_request() noexcept { application_skip_request = application_update_request_true; }
} // namespace skip_application
