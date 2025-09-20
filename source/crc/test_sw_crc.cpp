#include <array>
#include <bootloader/crc.h>

int main(int, char **) {
    const char message1[] = "\x00\x28\x00\x10\x1d\x80\x00\x08";
    const auto value1 = crc::compute(message1, &message1[std::size(message1) - 1]);

    constexpr uint32_t expected1 = 0xB40D23E0;
    if (value1 != expected1)
        return 1;

    const char message2[] = "The quick brown fox jumps over the lazy dog";
    const auto value2 = crc::compute(message2, &message2[std::size(message2) - 1]);

    constexpr uint32_t expected2 = 0x414FA339;
    if (value2 != expected2)
        return 1;
    return 0;
}
