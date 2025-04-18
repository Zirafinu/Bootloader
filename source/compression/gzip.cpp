#include "gzip.h"
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace gzip {

Header Header::parse(uint8_t const *start, uint8_t const *end) {
    Header result{};
    if (end < (start + 10)) {
        return result;
    }
    if (Magic_Bytes[0] != start[0] || Magic_Bytes[1] != start[1] || Method_Deflate != start[2]) {
        return result;
    }
    result.flags = Flags(start[3]);
    result.timestamp = start[4] | (start[5] << 8) | (start[6] << 16) | (start[7] << 24);
    result.extra_flags = Extra_Flags(start[8]);
    if (int(result.extra_flags) != 0 && int(result.extra_flags) != int(Extra_Flags::Deflate_Fast) &&
        int(result.extra_flags) != int(Extra_Flags::Deflate_Strong)) {
        return result;
    }

    result.os = start[9];
    std::size_t length = 10;
    if (uint8_t(result.flags) & uint8_t(Flags::EXTRA)) {
        result.extra_length = start[length] | (start[length + 1] << 8);
        length += 2;
        result.extra_data = &start[length];
        length += result.extra_length;
    }
    if (uint8_t(result.flags) & uint8_t(Flags::NAME)) {
        result.name = &start[length];
        while (start[length] != 0)
            length++;
        length += 1;
    }
    if (uint8_t(result.flags) & uint8_t(Flags::COMMENT)) {
        result.comment = &start[length];
        while (start[length] != 0)
            length++;
        length += 1;
    }
    if (uint8_t(result.flags) & uint8_t(Flags::HCRC)) {
        length += 2;
    }
    result.length = length;
    return result;
}

template <typename Tree_Count_t = std::array<uint8_t, 16>>
int construct(Tree_Count_t &tree_count, uint16_t *tree_symbols, const uint8_t *length,
              std::size_t symbol_count) noexcept {
    std::remove_cvref_t<decltype(tree_count)> offs; /* offsets in symbol table for each length */

    /* count number of codes of each length */
    std::memset(tree_count.begin(), 0, tree_count.size() * sizeof(tree_count[0]));

    for (std::size_t symbol = 0; symbol < symbol_count; ++symbol) { /* assumes lengths are within bounds */
        ++tree_count[length[symbol]];
    }
    if (tree_count[0] == symbol_count) /* no codes! */
        return 0;                      /* complete, but decode() will fail */

    /* check for an over-subscribed or incomplete set of lengths */
    int left = 1; /* one possible code of zero length */
    for (std::size_t len = 1; len < tree_count.size(); len++) {
        left <<= 1;              /* one more bit, double codes left */
        left -= tree_count[len]; /* deduct count from possible codes */
        if (left < 0) {
            return left; /* over-subscribed--return negative */
        }
    } /* left > 0 means incomplete */

    /* generate offsets into symbol table for each length for sorting */
    offs[1] = 0;
    for (std::size_t len = 1; len < (tree_count.size() - 1); len++) {
        offs[len + 1] = offs[len] + tree_count[len];
    }

    /*
     * put symbols in table sorted by length, by symbol order within each
     * length
     */
    for (std::size_t symbol = 0; symbol < symbol_count; ++symbol) {
        if (length[symbol] != 0) {
            tree_symbols[offs[length[symbol]]++] = symbol;
        }
    }

    /* return zero for complete set, positive for incomplete set */
    return left;
}

auto Inflate::SymbolHuffmanTree::construct_fixed() noexcept -> SymbolHuffmanTree {
    std::array<uint8_t, FIXED_LITERAL_LENGTH_CODES> lengths{};
    std::memset(&lengths[0], 8, 144);
    std::memset(&lengths[144], 9, 256 - 144);
    std::memset(&lengths[256], 7, 280 - 256);
    std::memset(&lengths[280], 8, FIXED_LITERAL_LENGTH_CODES - 280);

    SymbolHuffmanTree result{};
    (void)construct(result.count, result.symbols.data(), lengths.data(), lengths.size());
    return result;
}

auto Inflate::DistanceHuffmanTree::construct_fixed() noexcept -> DistanceHuffmanTree {
    std::array<uint8_t, MAX_DISTANCE_CODES> lengths;
    std::memset(&lengths[0], 5, MAX_DISTANCE_CODES);

    DistanceHuffmanTree result{};
    (void)construct(result.count, result.symbols.data(), lengths.data(), lengths.size());
    return result;
}

bool Inflate::decode() noexcept {
    while (can_read() > 0) {
        const auto bits = read_bits(3);

        bool failed = true;
        switch (static_cast<Block_Type_t>((bits >> 1) & 0x3)) {
        case Block_Type_t::Stored:
            failed = inflate_stored();
            break;
        case Block_Type_t::Fixed:
            failed = inflate_fixed();
            break;
        case Block_Type_t::Dynamic:
            failed = inflate_dynamic();
            break;
        default:
            // bad block type
            break;
        }
        if (failed)
            return false;
        if ((bits & 1) != 0)
            return true;
    }
    return true;
}

uint32_t Inflate::read_bits(std::size_t count) noexcept {
    while (count > mBit_Count) { // fill the bit buffer until enough data is present
        uint8_t byte = read();
        mBit_Buffer |= uint32_t(byte) << mBit_Count;
        mBit_Count += 8;
    }

    mBit_Count -= count;
    if (count >= 32) { // shift would have invalid length
        return mBit_Buffer;
    }

    uint32_t result = mBit_Buffer & ((1U << count) - 1);
    mBit_Buffer >>= count;
    return result;
}

bool Inflate::inflate_stored() noexcept {
    mBit_Buffer = 0;
    mBit_Count = 0;
    const uint16_t block_length = read_bits(16);
    {
        const uint16_t stored_block_length_k1 = read_bits(16);
        if (block_length != uint16_t(~stored_block_length_k1)) { // k1 compliment didn't match
            return true;
        }
    }

    for (auto i = 0U; i < block_length; ++i) {
        write(read());
    }
    return false;
}

bool Inflate::inflate_fixed() noexcept {
    mLiteral_tree = SymbolHuffmanTree::construct_fixed();
    mDistance_tree = DistanceHuffmanTree::construct_fixed();
    return codes();
}

bool Inflate::inflate_dynamic() noexcept {
    int err;                                /* construct() return value */
    uint8_t lengths[MAX_LENGTH_CODE_COUNT]; /* descriptor code lengths */
    static const short order[19] =          /* permutation of code length codes */
        {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};

    /* get number of lengths in each table, check lengths */
    const std::size_t nlen = read_bits(5) + 257;
    const std::size_t ndist = read_bits(5) + 1;
    const std::size_t ncode = read_bits(4) + 4;
    if (nlen > MAX_LITERAL_LENGTH_CODES || ndist > MAX_DISTANCE_CODES)
        return true; /* bad counts */

    /* read code length code lengths (really), missing lengths are zero */
    for (std::size_t index = 0; index < ncode; index++)
        lengths[order[index]] = read_bits(3);
    for (std::size_t index = ncode; index < 19; index++)
        lengths[order[index]] = 0;

    /* build huffman table for code lengths codes (use lencode temporarily) */
    err = construct(mLiteral_tree.count, mLiteral_tree.symbols.data(), lengths, 19);
    if (err != 0) /* require complete code set here */
        return true;

    /* read length/literal and distance code length tables */
    std::size_t index = 0;
    while (index < (nlen + ndist)) {
        int symbol; /* decoded value */
        int len;    /* last length to repeat */

        symbol = decode_symbol();
        if (symbol < 0)
            return true; /* invalid symbol */
        if (symbol < 16) /* length in 0..15 */
            lengths[index++] = symbol;
        else {                  /* repeat instruction */
            len = 0;            /* assume repeating zeros */
            if (symbol == 16) { /* repeat last length 3..6 times */
                if (index == 0)
                    return true;          /* no last length! */
                len = lengths[index - 1]; /* last length */
                symbol = 3 + read_bits(2);
            } else if (symbol == 17) /* repeat zero 3..10 times */
                symbol = 3 + read_bits(3);
            else /* == 18, repeat zero 11..138 times */
                symbol = 11 + read_bits(7);
            if (index + symbol > nlen + ndist)
                return true; /* too many lengths! */
            while (symbol--) /* repeat last or zero symbol times */
                lengths[index++] = len;
        }
    }

    /* check for end-of-block code -- there better be one! */
    if (lengths[256] == 0)
        return true;

    /* build huffman table for literal/length codes */
    err = construct(mLiteral_tree.count, mLiteral_tree.symbols.data(), lengths, nlen);
    if (err && (err < 0 || nlen != mLiteral_tree.count[0] + mLiteral_tree.count[1]))
        return true; /* incomplete code ok only for single length 1 code */

    /* build huffman table for distance codes */
    err = construct(mDistance_tree.count, mDistance_tree.symbols.data(), lengths + nlen, ndist);
    if (err && (err < 0 || ndist != mDistance_tree.count[0] + mDistance_tree.count[1]))
        return true; /* incomplete code ok only for single length 1 code */

    /* decode data until end-of-block code */
    return codes();
}

bool Inflate::codes() noexcept {
    constexpr std::array<uint16_t, 29> length_symbols = {/* Size base for length codes 257..285 */
                                                         3,  4,  5,  6,   7,   8,   9,   10,  11, 13,
                                                         15, 17, 19, 23,  27,  31,  35,  43,  51, 59,
                                                         67, 83, 99, 115, 131, 163, 195, 227, 258};
    constexpr std::array<uint8_t, 29> length_symbol_extra_bits = {/* Extra bits for length codes 257..285 */
                                                                  0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                                                  2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    constexpr std::array<uint16_t, 30> distance_symbols = {
        /* Offset base for distance codes 0..29 */
        1,   2,   3,   4,   5,   7,    9,    13,   17,   25,   33,   49,   65,    97,    129,
        193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097, 6145, 8193, 12289, 16385, 24577};
    constexpr std::array<uint8_t, 30> distance_symbol_extra_bits = {
        /* Extra bits for distance codes 0..29 */
        0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};

    /* decode literals and length/distance pairs */
    while (true) {
        const int32_t symbol = decode_symbol();
        if (symbol < 0)
            return true;    /* invalid symbol */
        if (symbol < 256) { /* literal: symbol is the byte */
            /* write out the literal */
            uint8_t const literal = symbol;
            write(literal);
        } else if (symbol == 256) { /* done with a valid fixed or dynamic block */
            return false;
        } else if (symbol > 256) { /* length */
            /* get and compute length */
            const uint32_t length_index = symbol - 257;
            if (length_index >= length_symbols.size()) { /* invalid length code literal */
                return true;
            }
            const int32_t length =
                length_symbols[length_index] + read_bits(length_symbol_extra_bits[length_index]);

            /* get and check distance */
            const int32_t distance_index = decode_distance();
            if (distance_index < 0)
                return true; /* invalid symbol */
            const uint32_t distance =
                distance_symbols[distance_index] + read_bits(distance_symbol_extra_bits[distance_index]);

            /* copy length bytes from distance bytes back */
            for (auto i = 0; i < length; ++i) {
                write(read_old(distance));
            }
        }
    }

    /* invalid code path */
    return true;
}

int32_t Inflate::decode_symbol() noexcept {
    int code = 0;  /* len bits being decoded */
    int first = 0; /* first code of length len */
    int count = 0; /* number of codes of length len */
    int index = 0; /* index of first code of length len in symbol table */

    for (size_t len = 1; len <= MAX_BITS; ++len) {
        code |= read_bits(1); /* get next bit */
        count = mLiteral_tree.count[len];
        if (code - count < first) /* if length len, return symbol */
            return mLiteral_tree.symbols[index + (code - first)];
        index += count; /* else update for next length */
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -10; /* ran out of codes */
}

int8_t Inflate::decode_distance() noexcept {
    int code = 0;  /* len bits being decoded */
    int first = 0; /* first code of length len */
    int count = 0; /* number of codes of length len */
    int index = 0; /* index of first code of length len in symbol table */

    for (size_t len = 1; len <= MAX_BITS; ++len) {
        code |= read_bits(1); /* get next bit */
        count = mDistance_tree.count[len];
        if (code - count < first) /* if length len, return symbol */
            return mDistance_tree.symbols[index + (code - first)];
        index += count; /* else update for next length */
        first += count;
        first <<= 1;
        code <<= 1;
    }
    return -10; /* ran out of codes */
}
} // namespace gzip
