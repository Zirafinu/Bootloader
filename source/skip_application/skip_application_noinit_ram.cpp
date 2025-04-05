#include "skip_application.h"
#include <cstdint>
namespace
{
    constexpr uint32_t application_skip_request_true = 0xC1AEE345UL;
}

extern "C" uint32_t application_skip_request;
namespace skip_application
{
    bool is_requested() noexcept
    {
        return application_skip_request == application_skip_request_true;
    }
    void reset_request() noexcept
    {
        application_skip_request = ~application_skip_request_true;
    }
    void request() noexcept
    {
        application_skip_request = application_skip_request_true;
    }
}
