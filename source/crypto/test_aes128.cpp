#include <array>
#include <bootloader/crypto.h>

int main(int, char **) {
    const uint8_t initial_key[16] = {
        0x10, 0x21, 0x4a, 0x61, 0xcb, 0x99, 0xde, 0xae, 0xe5, 0x60, 0x57, 0x5d, 0xf8, 0x77, 0x74, 0x29,
    }; // 10214a61cb99deaee560575df8777429
    const uint8_t initial_vector[] = {
        0xe7, 0xee, 0xf2, 0x00, 0xb8, 0x81, 0x1a, 0x3d, 0xa4, 0x74, 0xf1, 0xa6, 0xba, 0x12, 0x23, 0x10,
    }; // e7eef200b8811a3da474f1a6ba122310
    const uint8_t expected_text[64] = "This is some example text of just the right size with some padd";
    // the cipher text is obtained from this command line :
    /* $> echo -n "This is some example text of just the right size with some padd\0" | openssl enc -K \
     "10214a61cb99deaee560575df8777429" -iv "e7eef200b8811a3da474f1a6ba122310" -in - -out -  -aes-128-cbc \
     -nosalt -nopad | xxd -i
    */
    const uint8_t cipher_text[64] = {
        0x1a, 0x46, 0x08, 0x2b, 0xd0, 0x1d, 0x25, 0xe2, 0x26, 0x73, 0xff, 0x4d, 0x0f, 0x85, 0xa8, 0x19,
        0xc6, 0x82, 0x87, 0x05, 0x06, 0xc5, 0x95, 0x79, 0x3d, 0xa2, 0x2c, 0x2b, 0xf3, 0xc9, 0x4f, 0x81,
        0xdc, 0xfc, 0x9f, 0x6a, 0xab, 0x13, 0xdb, 0x9c, 0xa1, 0x24, 0x29, 0x60, 0x25, 0xf2, 0x28, 0x63,
        0xdf, 0xde, 0x1a, 0xfc, 0x36, 0xe6, 0x05, 0x2e, 0x4c, 0x91, 0x11, 0x51, 0xe6, 0x2c, 0x75, 0x0d,
    }; // the encrypted version

    const auto keys = crypto::AES::Common::expand_key<crypto::AES::key128_t>(initial_key);

    const crypto::AES::state_t iv = *crypto::bytes_to_state_iterator{initial_vector};

    auto start =
        crypto::decrypt_iterator{crypto::bytes_to_state_iterator{std::begin(cipher_text)}, iv, &keys};
    const auto end = decltype(start)::make_sentinel(crypto::bytes_to_state_iterator{std::end(cipher_text)});

    std::array<crypto::AES::state_t, 4> plain_text;

    const auto out_end = std::copy(start, end, plain_text.begin());
    if (out_end != plain_text.end()) {
        return 1;
    }

    // The expected plain text, as little endian integers
    const std::array<crypto::AES::state_t, 4> expected_plain_text = {
        crypto::AES::state_t //
        {0x73696854, 0x20736920, 0x656D6F73, 0x61786520},
        {0x656C706D, 0x78657420, 0x666F2074, 0x73756A20},
        {0x68742074, 0x69722065, 0x20746867, 0x657A6973},
        {0x74697720, 0x6F732068, 0x7020656D, 0x00646461},
    };
    if (plain_text != expected_plain_text) {
        return 2;
    }

    crypto::read_as_uint8_t is_begin{plain_text.begin()};
    crypto::read_as_uint8_t is_end{plain_text.end()};
    const uint8_t *expected_begin = expected_text;

    size_t checked = 0;
    while (is_begin != is_end) {
        ++checked;
        if (*expected_begin != *is_begin) {
            return 3;
        }
        ++expected_begin;
        ++is_begin;
    }

    if (checked != 64) {
        return 4;
    }

    return 0;
}
