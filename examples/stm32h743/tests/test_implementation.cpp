#include "../bootloader_impl.cpp"
#include <bootloader/version_info.h>

extern "C" uint32_t *estack;

namespace application {

void application_reset() { terminate_program(4); }

__attribute__((used)) //
const std::array<void *, 2>
    application_isr_vector{&estack, reinterpret_cast<void *>(&application_reset)};

__attribute__((used)) //
const version::info version_info_struct_app{0x01'01'00001, 0x000000001, 0 /* unix time least 4 bytes */};
} // namespace application

int main() {

    if (!bootloader::application_is_valid()) {
        return 1;
    }

    if (!bootloader::application_update_is_valid(false)) {
        return 2;
    }

    if (!bootloader::application_update_is_valid(true)) {
        return 7;
    }

    if (!bootloader::copy_update_to_application()) {
        return 3;
    }

    return 0;
}
