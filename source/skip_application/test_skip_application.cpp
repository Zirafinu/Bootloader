#include <cstdint>
#include <skip_application.h>

extern "C" {
uint32_t application_skip_request = 0;
}

int main(int argc, char **args) {
    using namespace skip_application;
    int result = 0;
    skip_request();
    result |= skip_is_requested() ? 0 : 1;
    skip_reset_request();
    result |= skip_is_requested() ? 2 : 0;

    update_request();
    result |= update_is_requested() ? 0 : 4;
    update_reset_request();
    result |= update_is_requested() ? 8 : 0;
    return result;
}
