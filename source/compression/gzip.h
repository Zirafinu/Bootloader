#ifndef COMPRESSION_GZIP_H
#define COMPRESSION_GZIP_H

#include <array>
#include <cstdint>

namespace gzip {
struct Header {
    enum class Flags : uint8_t {
        TEXT = 0x01,    // data could be ascii-text
        HCRC = 0x02,    // header is crc16 protected
        EXTRA = 0x04,   // has extra data
        NAME = 0x08,    // has file name
        COMMENT = 0x10, // has comment field
    };

    static constexpr std::array<uint8_t, 2> Magic_Bytes = {0x1F, 0x8B};
    static constexpr uint8_t Method_Deflate = 0x08;

    enum class Extra_Flags : uint8_t {
        Deflate_Strong = 0x02,
        Deflate_Fast = 0x04,
    };

    static Header parse(uint8_t const *start, uint8_t const *end);

    Flags flags = Flags(0);
    uint32_t timestamp = 0;
    Extra_Flags extra_flags = Extra_Flags(0);
    uint8_t os = 0;
    uint16_t extra_length = 0;
    uint8_t const *extra_data = nullptr;
    uint8_t const *name = nullptr;
    uint8_t const *comment = nullptr;
    uint32_t length = 0;
};

struct Footer {
    uint32_t crc32;         // crc32 over the data
    uint32_t inflated_size; // only valid for less then 4 GiBi
};

inline constexpr auto operator&(Header::Flags lhs, Header::Flags rhs) {
    return static_cast<Header::Flags>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
}
inline constexpr auto operator|(Header::Flags lhs, Header::Flags rhs) {
    return static_cast<Header::Flags>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
}
inline constexpr auto &operator&=(Header::Flags &lhs, Header::Flags rhs) {
    lhs = lhs & rhs;
    return lhs;
}
inline constexpr auto operator|=(Header::Flags &lhs, Header::Flags rhs) {
    lhs = lhs | rhs;
    return lhs;
}

class Inflate {
  public:
    typedef bool (*can_read_t)();
    typedef uint8_t (*read_t)();
    typedef void (*write_t)(uint8_t);
    typedef uint8_t (*read_old_t)(std::size_t);
    Inflate(can_read_t can_read, read_t read, write_t write, read_old_t read_old)
        : can_read(can_read), read(read), write(write), read_old(read_old) {}
    bool decode() noexcept;

  private:
    enum class Block_Type_t : uint8_t {
        Stored = 0,  // the decoder is in a stored block
        Fixed = 1,   // the decoder is in a fixed block
        Dynamic = 2, // the decoder is in a dynamic block
    };

    /**
     * reads \em count bits from the \em mBit_Buffer into the return value.
     * if insufficient bits are in the \em mBit_Buffer \em mInput is read.
     * if \em mInput holds insufficient bits, \em Insufficient_Bits is returned.
     */
    uint32_t read_bits(std::size_t count) noexcept;

    /// true if \em mInput has no more valid data
    bool inflate_stored() noexcept;
    /// true if \em mInput has no more valid data
    bool inflate_fixed() noexcept;
    /// true if \em mInput has no more valid data
    bool inflate_dynamic() noexcept;

    bool codes() noexcept;
    int32_t decode_symbol() noexcept;
    int8_t decode_distance() noexcept;

    static constexpr std::size_t MAX_BITS = 15;
    static constexpr std::size_t MAX_DISTANCE_CODES = 30;
    static constexpr std::size_t MAX_LITERAL_LENGTH_CODES = 286;
    static constexpr std::size_t FIXED_LITERAL_LENGTH_CODES = 288;
    static constexpr std::size_t MAX_LITERAL_LENGTH_CODE_COUNT = FIXED_LITERAL_LENGTH_CODES;
    static constexpr std::size_t MAX_LENGTH_CODE_COUNT = MAX_LITERAL_LENGTH_CODES + MAX_DISTANCE_CODES;

    struct SymbolHuffmanTree {
        std::array<uint8_t, MAX_BITS + 1> count;
        std::array<uint16_t, MAX_LITERAL_LENGTH_CODE_COUNT> symbols;

        static SymbolHuffmanTree construct_fixed() noexcept;
    };
    struct DistanceHuffmanTree {
        std::array<uint8_t, MAX_BITS + 1> count;
        std::array<uint16_t, MAX_DISTANCE_CODES> symbols;

        static DistanceHuffmanTree construct_fixed() noexcept;
    };

    // memory layout
    can_read_t can_read;
    read_t read;
    write_t write;
    read_old_t read_old;
    uint32_t mBit_Buffer = 0;
    uint8_t mBit_Count = 0;
    SymbolHuffmanTree mLiteral_tree;
    DistanceHuffmanTree mDistance_tree;
};

} // namespace gzip

#endif
