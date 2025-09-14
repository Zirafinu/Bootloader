#ifndef CRYPTO_CRYPTO_H
#define CRYPTO_CRYPTO_H

#include "aes.h"
#include <functional>
#include <iterator>

namespace crypto {

/**
 * Implements AES with CBC as a single iterator
 */
template <typename BaseIterator> class decrypt_iterator {
  public:
    using base_t = BaseIterator;
    using block_t = AES::state_t;
    using expanded_key_t = decltype(AES::Common::expand_key<>(std::declval<AES::key128_t>()));

    /// implementations for compliance with std::input_iterator
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = block_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit decrypt_iterator(base_t input, const block_t &iv, expanded_key_t const *keys)
        : read_iterator{input}, previous_cipher_text{iv}, expanded_keys{keys} {}

    static decrypt_iterator make_sentinel(base_t input) {
        return decrypt_iterator{input, block_t{}, nullptr};
    }

    decrypt_iterator(const decrypt_iterator &other) = default;

    decrypt_iterator(decrypt_iterator &&other) = default;

    ~decrypt_iterator() = default;

    decrypt_iterator &operator=(const decrypt_iterator &other) = default;

    decrypt_iterator &operator=(decrypt_iterator &&other) = default;

    friend bool operator==(decrypt_iterator const &lhs, decrypt_iterator const &rhs) {
        return lhs.read_iterator == rhs.read_iterator;
    }

    friend bool operator!=(decrypt_iterator const &lhs, decrypt_iterator const &rhs) {
        return lhs.read_iterator != rhs.read_iterator;
    }

    template <typename T> friend bool operator<(decrypt_iterator const &lhs, T const &rhs) {
        return lhs.read_iterator < rhs;
    }

    value_type operator*() const {
        auto plain_text = *read_iterator;
        AES::decrypt(plain_text, *expanded_keys);
        std::transform(previous_cipher_text.begin(), previous_cipher_text.end(),
                       // xor previous cipher text with current plain text
                       plain_text.begin(), plain_text.begin(), std::bit_xor{});
        return plain_text;
    }

    decrypt_iterator operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    decrypt_iterator &operator++() {
        previous_cipher_text = *read_iterator;
        ++read_iterator;
        return *this;
    }

  private:
    base_t read_iterator;
    value_type previous_cipher_text;
    expanded_key_t const *expanded_keys;

    static_assert(std::is_same_v<std::remove_cvref_t<decltype(*std::declval<base_t>())>, block_t>,
                  "base iterator type mismatch");
};

template <typename BaseIterator> class read_as_uint8_t {
  public:
    using base_t = BaseIterator;

    /// implementations for compliance with std::input_iterator
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = uint8_t;
    using pointer = value_type *;
    using reference = value_type &;

    explicit read_as_uint8_t(base_t input, ssize_t in_element_index = -1)
        : read_iterator{input}, in_element_index{in_element_index} {}

    read_as_uint8_t(const read_as_uint8_t &other) = default;

    read_as_uint8_t(read_as_uint8_t &&other) = default;

    ~read_as_uint8_t() = default;

    read_as_uint8_t &operator=(const read_as_uint8_t &other) = default;

    read_as_uint8_t &operator=(read_as_uint8_t &&other) = default;

    value_type operator*() const {
        if (in_element_index < 0) {
            cached_value = *read_iterator;
            in_element_index = 0;
        }
        return reinterpret_cast<uint8_t const *>(&cached_value)[in_element_index];
    }

    read_as_uint8_t operator++(int) {
        auto tmp = *this;
        ++(*this);
        return tmp;
    }

    read_as_uint8_t &operator++() {
        in_element_index = std::max<ssize_t>(in_element_index, 0) + 1;
        if (in_element_index >= static_cast<ssize_t>(sizeof(cached_value))) {
            ++read_iterator;
            in_element_index = -1;
        }
        return *this;
    }

    friend bool operator==(read_as_uint8_t const &lhs, read_as_uint8_t const &rhs) {
        ssize_t merged_index = lhs.in_element_index ^ rhs.in_element_index;
        return lhs.read_iterator == rhs.read_iterator && (merged_index == 0 || merged_index == -1);
    }

    friend bool operator!=(read_as_uint8_t const &lhs, read_as_uint8_t const &rhs) { return !(lhs == rhs); }

    template <typename T> friend bool operator<(read_as_uint8_t const &lhs, T const &rhs) {
        return lhs.read_iterator < rhs;
    }

  private:
    base_t read_iterator;
    mutable ssize_t in_element_index;
    mutable std::remove_cvref_t<decltype(*std::declval<base_t>())> cached_value;
};
} // namespace crypto
#endif
