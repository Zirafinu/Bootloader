#include "../bootloader.h"
#include <array>

int main(int argc, char **args)
{
    const char message[] = "123456789";
    const auto crc_value = crc(message, &message[std::size(message) - 1]);

    constexpr uint32_t crc_value_bzip2 = 0xFC891918;
    return crc_value != crc_value_bzip2;
}
