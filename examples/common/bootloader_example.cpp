#include "hardware_specific.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <bootloader/crc.h>
#include <bootloader/crypto.h>
#include <bootloader/gzip.h>
#include <bootloader/version_info.h>
#include <flash_layout.h>

extern "C" const uint8_t secrets[16]{};

extern "C" __attribute__((used)) //
const version::info version_info_struct{0x01'01'00001, 0x000000001, 0x000000000};

// the applications reset handler address
using entry_function_t = void (*)() noexcept;
extern "C" const entry_function_t application_entry_function __attribute__((noreturn));

namespace {
using decrypt_iterator = crypto::decrypt_iterator<crypto::AES::state_t const *>;
crypto::read_as_uint8_t<decrypt_iterator> read_iterator{decrypt_iterator{nullptr, {}, nullptr}, 0};
uint8_t *write_application_it = nullptr;
size_t write_application_buffer_used = 0;

alignas(4) std::array<uint8_t, 1024> write_application_buffer{};

bool can_read_more() {
    return read_iterator < reinterpret_cast<crypto::AES::state_t const *>(flash_layout::update_end);
}

uint8_t read_update_flash() {
    const uint8_t result = *read_iterator;
    ++read_iterator;
    return result;
}

void write_application(uint8_t value) {
    write_application_buffer[write_application_buffer_used] = value;
    ++write_application_buffer_used;
    if (write_application_buffer_used == write_application_buffer.size()) {
        (void)bootloader::hardware_specific::flush_write_buffer(
            write_application_it, write_application_buffer, write_application_buffer_used);
        write_application_it += write_application_buffer_used;
        write_application_buffer_used = 0;
    }
}

uint8_t read_application(std::size_t distance) {
    if (distance > write_application_buffer_used) {
        return *(write_application_it - (distance - write_application_buffer_used));
    }
    return write_application_buffer[write_application_buffer_used - distance];
}
} // namespace

namespace bootloader {
bool application_is_valid() noexcept {
    if (*reinterpret_cast<uint32_t *>(flash_layout::application_end - 4) !=
        crc::compute(reinterpret_cast<uint8_t *>(flash_layout::application_begin),
                     reinterpret_cast<uint8_t *>(flash_layout::application_end - 4)))
        return false;

    // check if the product id is correct
    const version::info *application_version_info =
        reinterpret_cast<version::info const *>(flash_layout::application_end - 4 - sizeof(version::info));
    if (application_version_info->product_id != version_info_struct.product_id)
        return false;

    return true;
}

bool application_update_is_valid(bool is_application_memory_valid) noexcept {
    if (*reinterpret_cast<uint32_t *>(flash_layout::update_end - 4) !=
        crc::compute(reinterpret_cast<uint8_t *>(flash_layout::update_begin),
                     reinterpret_cast<uint8_t *>(flash_layout::update_end - 4)))
        return false;

    // check if the version info of the application is valid
    const version::info *update_version_info =
        reinterpret_cast<version::info const *>(flash_layout::update_begin);
    if (update_version_info->product_id != version_info_struct.product_id)
        return false;

    // permit only upgrade and repair(same version)
    if (is_application_memory_valid) {
        const version::info *application_version_info = reinterpret_cast<version::info const *>(
            flash_layout::application_end - 4 - sizeof(version::info));
        if (update_version_info->version < application_version_info->version &&
            application_version_info->version != 0xFFffFFffU)
            return false;
    }

    return true;
}

bool copy_update_to_application() noexcept {
    bootloader::hardware_specific::initialize_core_for_update();

    if (!bootloader::hardware_specific::erase_application())
        return false;
    // initialize round keys
    auto expanded_keys = crypto::AES::Common::expand_key(crypto::AES::key128_t{secrets});
    // encrypted area starts behind the version info
    const auto *encrypted_area =
        reinterpret_cast<crypto::AES::state_t const *>(flash_layout::update_begin + sizeof(version::info));
    // initialize the AES iterator
    read_iterator =
        decltype(read_iterator){decrypt_iterator{encrypted_area + 1, *encrypted_area, &expanded_keys}};

    // initialize the output iterator
    write_application_it =
        reinterpret_cast<uint8_t *>(const_cast<size_t *>(&flash_layout::application_begin));

    gzip::Inflate inflator{can_read_more, read_update_flash, write_application, read_application};
    const bool updated = inflator.decode();
    memset(expanded_keys.front().data(), 0, sizeof(expanded_keys)); // zero the key
    if (!updated) {
        return false;
    }

    return bootloader::hardware_specific::flush_write_buffer(write_application_it, write_application_buffer,
                                                             write_application_buffer_used);
}

void jump_to_application() noexcept {
    // clear maybe initialized peripherals
    bootloader::hardware_specific::initialize_core_for_application_start();

    application_entry_function();
}

} // namespace bootloader
