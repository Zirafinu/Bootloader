#include <array>
#include <crypto.h>

int main(int, char **) {
    const uint8_t initial_key[16] = "0123456789ABCDE";
    const auto keys = crypto::AES::Common::expand_key<crypto::AES::key128_t>(initial_key);
    const crypto::AES::state_t iv = {1, 2, 3, 4};
    const std::array<crypto::AES::state_t, 1> cipher_text = {{1, 2, 3, 4}};

    using cipher_text_read_iterator = decltype(cipher_text.begin());
    auto start = crypto::decrypt_iterator{cipher_text.begin(), iv, keys};
    auto end = crypto::decrypt_iterator{cipher_text.end()};

    std::array<crypto::AES::state_t, 1> plain_text;

    const auto out_end = std::copy(start, end, plain_text.begin());
    if (out_end != plain_text.end()) {

        return 1;
    }

    const std::array<crypto::AES::state_t, 1> expected_plain_text = {
        {0xe3263ce1, 0x6d66fcc7, 0x31aa301c, 0x316cb8de}};
    if (plain_text != expected_plain_text) {
        return 2;
    }

    crypto::read_as_uint8_t is_begin{plain_text.begin()};
    crypto::read_as_uint8_t is_end{plain_text.end()};
    crypto::read_as_uint8_t expected_begin{expected_plain_text.begin()};

    size_t checked = 0;
    while (is_begin != is_end) {
        ++checked;
        if (*expected_begin != *is_begin) {
            return 3;
        }
        ++expected_begin;
        ++is_begin;
    }

    if (checked != 16) {
        return 4;
    }

    return 0;
}
