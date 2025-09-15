#include <array>
#include <crc.h>

int main(int, char **) {
    const char message[] = "\x00\x28\x00\x10\x1d\x80\x00\x08";
    const auto crc_value = crc::compute(message, &message[std::size(message) - 1]);

    constexpr uint32_t crc_value_bzip2 = 0xB40D23E0;
    return crc_value != crc_value_bzip2;
}
