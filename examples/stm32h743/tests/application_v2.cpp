#include <bootloader/version_info.h>

extern "C" __attribute__((used)) //
const version::info version_info_struct{0x01'01'00002, 0x000000001, 0 /* unix time least 4 bytes */};

extern "C" void terminate_program(int rc);
int main() {
    terminate_program(5);
    return 6;
}
