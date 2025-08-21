
#include <algorithm>
#include <array>
#include <cstdint>
#include <span>

namespace crypto {
namespace AES {

using key128_t = std::span<uint8_t const, 16>;
using key192_t = std::span<uint8_t const, 24>;
using key256_t = std::span<uint8_t const, 32>;

using state_t = std::array<uint32_t, 4>;

template <typename KEY_T> constexpr static int round_count() {
    if constexpr (std::is_same_v<KEY_T, key128_t>)
        return 10;
    else if constexpr (std::is_same_v<KEY_T, key192_t>)
        return 12;
    else if constexpr (std::is_same_v<KEY_T, key256_t>)
        return 14;
    else
        static_assert(std::is_same_v<KEY_T, key128_t>, "invalid key type");
    return 0;
}

[[gnu::hot, gnu::always_inline]]
static constexpr uint32_t GF_MULTIPLY_SIMDx2(uint32_t value) {
    const uint32_t mask = 0x80808080U;
    const auto extended = ((value & mask) >> 7) * 0x1b;
    const auto result = ((value & ~mask) << 1) ^ extended;
    return result;
}

namespace Encryption {
static state_t &row_shift(state_t &state) {
    for (auto i = 1U; i < state.size(); ++i) {
        state[i] = (state[i] << (8 * i)) | (state[i] >> (32 - (8 * i)));
    }
    return state;
}

static state_t &mix_columns(state_t &x1) {
    state_t x2;
    uint32_t all_x1 = 0;
    for (int i = 0; i < x1.size(); ++i) {
        all_x1 ^= x1[i];
        x2[i] = GF_MULTIPLY_SIMDx2(x1[i]);
    }

    // [0]*0x2 [1]*0x3 [2]*0x1 [3]*0x1
    x1[0] = all_x1 ^ x1[0] ^ x2[0] ^ x2[1];
    x1[1] = all_x1 ^ x1[1] ^ x2[1] ^ x2[2];
    x1[2] = all_x1 ^ x1[2] ^ x2[2] ^ x2[3];
    x1[3] = all_x1 ^ x1[3] ^ x2[3] ^ x2[0];
    return x1;
}

static constexpr std::array<uint8_t, 256> sbox = {
    0x63, 0x7c, 0x77, 0x7b, 0xf2, 0x6b, 0x6f, 0xc5, 0x30, 0x01, 0x67, 0x2b, 0xfe, 0xd7, 0xab, 0x76,
    0xca, 0x82, 0xc9, 0x7d, 0xfa, 0x59, 0x47, 0xf0, 0xad, 0xd4, 0xa2, 0xaf, 0x9c, 0xa4, 0x72, 0xc0,
    0xb7, 0xfd, 0x93, 0x26, 0x36, 0x3f, 0xf7, 0xcc, 0x34, 0xa5, 0xe5, 0xf1, 0x71, 0xd8, 0x31, 0x15,
    0x04, 0xc7, 0x23, 0xc3, 0x18, 0x96, 0x05, 0x9a, 0x07, 0x12, 0x80, 0xe2, 0xeb, 0x27, 0xb2, 0x75,
    0x09, 0x83, 0x2c, 0x1a, 0x1b, 0x6e, 0x5a, 0xa0, 0x52, 0x3b, 0xd6, 0xb3, 0x29, 0xe3, 0x2f, 0x84,
    0x53, 0xd1, 0x00, 0xed, 0x20, 0xfc, 0xb1, 0x5b, 0x6a, 0xcb, 0xbe, 0x39, 0x4a, 0x4c, 0x58, 0xcf,
    0xd0, 0xef, 0xaa, 0xfb, 0x43, 0x4d, 0x33, 0x85, 0x45, 0xf9, 0x02, 0x7f, 0x50, 0x3c, 0x9f, 0xa8,
    0x51, 0xa3, 0x40, 0x8f, 0x92, 0x9d, 0x38, 0xf5, 0xbc, 0xb6, 0xda, 0x21, 0x10, 0xff, 0xf3, 0xd2,
    0xcd, 0x0c, 0x13, 0xec, 0x5f, 0x97, 0x44, 0x17, 0xc4, 0xa7, 0x7e, 0x3d, 0x64, 0x5d, 0x19, 0x73,
    0x60, 0x81, 0x4f, 0xdc, 0x22, 0x2a, 0x90, 0x88, 0x46, 0xee, 0xb8, 0x14, 0xde, 0x5e, 0x0b, 0xdb,
    0xe0, 0x32, 0x3a, 0x0a, 0x49, 0x06, 0x24, 0x5c, 0xc2, 0xd3, 0xac, 0x62, 0x91, 0x95, 0xe4, 0x79,
    0xe7, 0xc8, 0x37, 0x6d, 0x8d, 0xd5, 0x4e, 0xa9, 0x6c, 0x56, 0xf4, 0xea, 0x65, 0x7a, 0xae, 0x08,
    0xba, 0x78, 0x25, 0x2e, 0x1c, 0xa6, 0xb4, 0xc6, 0xe8, 0xdd, 0x74, 0x1f, 0x4b, 0xbd, 0x8b, 0x8a,
    0x70, 0x3e, 0xb5, 0x66, 0x48, 0x03, 0xf6, 0x0e, 0x61, 0x35, 0x57, 0xb9, 0x86, 0xc1, 0x1d, 0x9e,
    0xe1, 0xf8, 0x98, 0x11, 0x69, 0xd9, 0x8e, 0x94, 0x9b, 0x1e, 0x87, 0xe9, 0xce, 0x55, 0x28, 0xdf,
    0x8c, 0xa1, 0x89, 0x0d, 0xbf, 0xe6, 0x42, 0x68, 0x41, 0x99, 0x2d, 0x0f, 0xb0, 0x54, 0xbb, 0x16};

static uint32_t substitute_word(uint32_t word) {
    uint32_t result = 0;
    for (ssize_t i = 24; i >= 0; i -= 8) {
        result <<= 8;
        result |= sbox[uint8_t(word >> i)];
    }
    return result;
}

static state_t &substitute_bytes(state_t &state) {
    for (auto &word : state) {
        word = substitute_word(word);
    }
    return state;
}

} // namespace Encryption

namespace Decryption {
static state_t &row_shift(state_t &state) {
    for (auto i = 1U; i < state.size(); ++i) {
        state[i] = (state[i] >> (8 * i)) | (state[i] << (32 - 8 * i));
    }
    return state;
}

static state_t &mix_columns(state_t &x1) {
    state_t x2;
    state_t x4;
    state_t x6;
    uint32_t all_x9 = 0;
    for (int i = 0; i < x1.size(); ++i) {
        all_x9 ^= x1[i];
        x2[i] = GF_MULTIPLY_SIMDx2(x1[i]);
        x4[i] = GF_MULTIPLY_SIMDx2(x2[i]);
        x6[i] = x2[i] ^ x4[i];
        all_x9 ^= GF_MULTIPLY_SIMDx2(x4[i]);
    }

    // [0]*0xE [1]*0xB [2]*0xD [3]*0x9
    x1[0] = all_x9 ^ x1[0] ^ x6[0] ^ x2[1] ^ x4[2];
    x1[1] = all_x9 ^ x1[1] ^ x6[1] ^ x2[2] ^ x4[3];
    x1[2] = all_x9 ^ x1[2] ^ x6[2] ^ x2[3] ^ x4[0];
    x1[3] = all_x9 ^ x1[3] ^ x6[3] ^ x2[0] ^ x4[1];
    return x1;
}

static constexpr std::array<uint8_t, 256> sbox{
    0x52, 0x09, 0x6a, 0xd5, 0x30, 0x36, 0xa5, 0x38, 0xbf, 0x40, 0xa3, 0x9e, 0x81, 0xf3, 0xd7, 0xfb,
    0x7c, 0xe3, 0x39, 0x82, 0x9b, 0x2f, 0xff, 0x87, 0x34, 0x8e, 0x43, 0x44, 0xc4, 0xde, 0xe9, 0xcb,
    0x54, 0x7b, 0x94, 0x32, 0xa6, 0xc2, 0x23, 0x3d, 0xee, 0x4c, 0x95, 0x0b, 0x42, 0xfa, 0xc3, 0x4e,
    0x08, 0x2e, 0xa1, 0x66, 0x28, 0xd9, 0x24, 0xb2, 0x76, 0x5b, 0xa2, 0x49, 0x6d, 0x8b, 0xd1, 0x25,
    0x72, 0xf8, 0xf6, 0x64, 0x86, 0x68, 0x98, 0x16, 0xd4, 0xa4, 0x5c, 0xcc, 0x5d, 0x65, 0xb6, 0x92,
    0x6c, 0x70, 0x48, 0x50, 0xfd, 0xed, 0xb9, 0xda, 0x5e, 0x15, 0x46, 0x57, 0xa7, 0x8d, 0x9d, 0x84,
    0x90, 0xd8, 0xab, 0x00, 0x8c, 0xbc, 0xd3, 0x0a, 0xf7, 0xe4, 0x58, 0x05, 0xb8, 0xb3, 0x45, 0x06,
    0xd0, 0x2c, 0x1e, 0x8f, 0xca, 0x3f, 0x0f, 0x02, 0xc1, 0xaf, 0xbd, 0x03, 0x01, 0x13, 0x8a, 0x6b,
    0x3a, 0x91, 0x11, 0x41, 0x4f, 0x67, 0xdc, 0xea, 0x97, 0xf2, 0xcf, 0xce, 0xf0, 0xb4, 0xe6, 0x73,
    0x96, 0xac, 0x74, 0x22, 0xe7, 0xad, 0x35, 0x85, 0xe2, 0xf9, 0x37, 0xe8, 0x1c, 0x75, 0xdf, 0x6e,
    0x47, 0xf1, 0x1a, 0x71, 0x1d, 0x29, 0xc5, 0x89, 0x6f, 0xb7, 0x62, 0x0e, 0xaa, 0x18, 0xbe, 0x1b,
    0xfc, 0x56, 0x3e, 0x4b, 0xc6, 0xd2, 0x79, 0x20, 0x9a, 0xdb, 0xc0, 0xfe, 0x78, 0xcd, 0x5a, 0xf4,
    0x1f, 0xdd, 0xa8, 0x33, 0x88, 0x07, 0xc7, 0x31, 0xb1, 0x12, 0x10, 0x59, 0x27, 0x80, 0xec, 0x5f,
    0x60, 0x51, 0x7f, 0xa9, 0x19, 0xb5, 0x4a, 0x0d, 0x2d, 0xe5, 0x7a, 0x9f, 0x93, 0xc9, 0x9c, 0xef,
    0xa0, 0xe0, 0x3b, 0x4d, 0xae, 0x2a, 0xf5, 0xb0, 0xc8, 0xeb, 0xbb, 0x3c, 0x83, 0x53, 0x99, 0x61,
    0x17, 0x2b, 0x04, 0x7e, 0xba, 0x77, 0xd6, 0x26, 0xe1, 0x69, 0x14, 0x63, 0x55, 0x21, 0x0c, 0x7d};

static uint32_t substitute_word(uint32_t word) {
    uint32_t result = 0;
    for (ssize_t i = 24; i >= 0; i -= 8) {
        result <<= 8;
        result |= sbox[uint8_t(word >> i)];
    }
    return result;
}

static state_t &substitute_bytes(state_t &state) {
    for (auto &word : state) {
        word = substitute_word(word);
    }
    return state;
}

} // namespace Decryption

namespace Common {
static state_t &add_round_key(state_t &state, const state_t &key) {
    for (auto i = 0U; i < state.size(); ++i) {
        state[i] ^= key[i];
    }
    return state;
}

static state_t &transpose(state_t &state) {
    uint32_t s0 = state[0];
    uint32_t s1 = state[1];

    constexpr uint32_t low_mask = 0xFF00FF00;
    constexpr uint32_t high_mask = 0xFF00FF;

    uint32_t r_0189 = (s0 & low_mask) | ((s1 & low_mask) >> 8);
    uint32_t r_45CD = ((s0 & high_mask) << 8) | (s1 & high_mask);

    uint32_t s2 = state[2];
    uint32_t s3 = state[3];
    uint32_t r_23AB = (s2 & low_mask) | ((s3 & low_mask) >> 8);
    uint32_t r_67EF = ((s2 & high_mask) << 8) | (s3 & high_mask);

    constexpr uint32_t top = 0xFFFF0000;
    constexpr uint32_t bottom = 0xFFFF;

    state[0] = ((r_0189 & top)) | ((r_23AB & top) >> 16);
    state[1] = ((r_45CD & top)) | ((r_67EF & top) >> 16);
    state[2] = ((r_0189 & bottom) << 16) | ((r_23AB & bottom));
    state[3] = ((r_45CD & bottom) << 16) | ((r_67EF & bottom));
    return state;
}

static state_t &state_to_bytes(state_t &state) {
    uint32_t s0 = state[0];
    uint32_t s1 = state[1];

    constexpr uint32_t low_mask = 0xFF00FF00;
    constexpr uint32_t high_mask = 0xFF00FF;

    uint32_t r_1098 = ((s0 & low_mask) >> 8) | (s1 & low_mask);
    uint32_t r_54DC = (s0 & high_mask) | ((s1 & high_mask) << 8);

    uint32_t s2 = state[2];
    uint32_t s3 = state[3];
    uint32_t r_32BA = ((s2 & low_mask) >> 8) | ((s3 & low_mask));
    uint32_t r_76FE = ((s2 & high_mask)) | ((s3 & high_mask) << 8);

    constexpr uint32_t top = 0xFFFF0000;
    constexpr uint32_t bottom = 0xFFFF;

    state[0] = ((r_1098 & top) >> 16) | ((r_32BA & top));
    state[1] = ((r_54DC & top) >> 16) | ((r_76FE & top));
    state[2] = ((r_1098 & bottom)) | ((r_32BA & bottom) << 16);
    state[3] = ((r_54DC & bottom)) | ((r_76FE & bottom) << 16);
    return state;
}

static state_t &bytes_to_state(state_t &state) {
    uint32_t s0 = state[0];
    uint32_t s1 = state[1];

    constexpr uint32_t low_mask = 0xFF00FF;
    constexpr uint32_t high_mask = ~low_mask;

    // bC b8 b4 b0

    uint32_t r_8901 = ((s0 & low_mask) << 8) | (s1 & low_mask);
    uint32_t r_CD45 = (s0 & high_mask) | ((s1 & high_mask) >> 8);

    uint32_t s2 = state[2];
    uint32_t s3 = state[3];
    uint32_t r_AB23 = ((s2 & low_mask) << 8) | ((s3 & low_mask));
    uint32_t r_EF67 = ((s2 & high_mask)) | ((s3 & high_mask) >> 8);

    constexpr uint32_t top = 0xFFFF0000;
    constexpr uint32_t bottom = ~top;

    state[0] = ((r_8901 & bottom) << 16) | ((r_AB23 & bottom));
    state[1] = ((r_CD45 & bottom) << 16) | ((r_EF67 & bottom));
    state[2] = ((r_8901 & top)) | ((r_AB23 & top) >> 16);
    state[3] = ((r_CD45 & top)) | ((r_EF67 & top) >> 16);
    return state;
}

template <typename key_t> static auto expand_key(const key_t &key) {
    constexpr auto ROUNDS = round_count<key_t>() + 1;
    using expanded_keys_t = std::array<state_t, ROUNDS>;
    expanded_keys_t expanded{};
    auto &linear_view = *reinterpret_cast<std::array<uint32_t, ROUNDS * sizeof(uint32_t)> *>(&expanded);
    constexpr auto N = key_t::extent / sizeof(uint32_t);

    for (auto i = 0U; i < key.size(); ++i) {
        linear_view[i / sizeof(uint32_t)] <<= 8;
        linear_view[i / sizeof(uint32_t)] |= key[i];
    }

    auto round_key = 0x01000000;
    for (auto i = N; i < linear_view.size(); ++i) {
        auto tmp = linear_view[i - 1];
        if ((i % N) == 0) {
            tmp = Encryption::substitute_word((tmp >> 24) | (tmp << 8));
            tmp ^= round_key;
            round_key = GF_MULTIPLY_SIMDx2(round_key);
        } else if ((N > 6) && ((i % N) == 4)) {
            tmp = Encryption::substitute_word(tmp);
        }
        linear_view[i] = linear_view[i - N] ^ tmp;
    }

    for (auto &key : expanded) {
        transpose(key);
    }
    return expanded;
}
} // namespace Common

template <typename expanded_keys_t> static void encrypt(state_t &data, const expanded_keys_t &keys) {
    Common::bytes_to_state(data);
    Common::add_round_key(data, keys.front());
    for (auto i = 1U; i < keys.size() - 1; ++i) {
        Encryption::substitute_bytes(data);
        Encryption::row_shift(data);
        Encryption::mix_columns(data);
        Common::add_round_key(data, keys[i]);
    }
    Encryption::substitute_bytes(data);
    Encryption::row_shift(data);
    Common::add_round_key(data, keys.back());
    Common::state_to_bytes(data);
}

template <typename expanded_keys_t> static void decrypt(state_t &data, const expanded_keys_t &keys) {
    Common::bytes_to_state(data);
    Common::add_round_key(data, keys.back());
    Decryption::row_shift(data);
    Decryption::substitute_bytes(data);
    for (auto i = keys.size() - 2; i > 0; --i) {
        Common::add_round_key(data, keys[i]);
        Decryption::mix_columns(data);
        Decryption::row_shift(data);
        Decryption::substitute_bytes(data);
    }
    Common::add_round_key(data, keys.front());
    Common::state_to_bytes(data);
}

} // namespace AES
} // namespace crypto
