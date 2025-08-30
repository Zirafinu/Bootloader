#include <array>
#include <crypto.h>

int main(int, char **) {
    const uint8_t initial_key[16] = "0123456789ABCDE";
    const auto keys = crypto::AES::Common::expand_key<crypto::AES::key128_t>(initial_key);
    const crypto::AES::state_t iv = {1, 2, 3, 4};
    const std::array<crypto::AES::state_t, 2> cipher_text = {crypto::AES::state_t{1, 2, 3, 4}, {5, 6, 7, 8}};

    // using cipher_text_read_iterator = decltype(cipher_text.begin());
    auto start = crypto::decrypt_iterator{cipher_text.begin(), iv, keys};
    const auto end = decltype(start)::make_sentinel(cipher_text.end());

    std::array<crypto::AES::state_t, 2> plain_text;

    const auto out_end = std::copy(start, end, plain_text.begin());
    if (out_end != plain_text.end()) {
        return 1;
    }

    // reference data is obtained from openssl by running this script:
    // $> echo -n 00 01 00 00 00 02 00 00 00 03 00 00 00 04 00 00 00 | xxd -r - test.bin
    // $> echo -n 10 05 00 00 00 06 00 00 00 07 00 00 00 08 00 00 00 | xxd -r - test.bin
    // $> openssl enc -d -K "30313233343536373839414243444500" -iv "01000000020000000300000004000000" -in
    //  test.bin -out test.bin.aes_cbc -p -aes-128-cbc -nosalt -nopad
    // $> hexdump -e '4/4 "0x%08X, " /0 "\n"' test.bin.aes_cbc
    const std::array<crypto::AES::state_t, 2> expected_plain_text = {
        crypto::AES::state_t{0xCF3D7DD3, 0x21B984FB, 0x5C22D251, 0xD11B2EF0},
        {0xC1825060, 0xC22D40ED, 0xB155DDA5, 0x04609B83}
    };
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

    if (checked != 32) {
        return 4;
    }

    return 0;
}
