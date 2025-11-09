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

#include <stm32h7xx_hal.h>
#include <stm32h7xx_hal_flash.h>
#include <stm32h7xx_hal_flash_ex.h>

extern "C" __attribute__((used)) //
const version::info version_info_struct{0x01'01'00001, 0x000000001, 0x000000000};

extern "C" __attribute__((used)) //
const uint8_t secrets[16]{};

extern "C" {
void initialize_HAL_GetTick() {
    constexpr size_t TIMER_BASE_CLOCK_KHZ = 64000;
    RCC->APB1LENR |= RCC_APB1LENR_TIM2EN;
    (void)RCC->APB1LENR;
    TIM2->CR1 = 0;
    TIM2->PSC = TIMER_BASE_CLOCK_KHZ - 1; // 1 kHz
    TIM2->ARR = ~0UL;
    TIM2->CNT = ~0UL;
    TIM2->CR1 = TIM_CR1_CEN;
}
uint32_t HAL_GetTick() { return TIM2->CNT; }

void terminate_program(int rc);
void assert_failed(uint8_t *file, uint32_t line) {
    (void)file;
    (void)line;
#ifdef STARTUP_WITH_SEMIHOSTING
    printf("Assert_Failed %s:%u", file, unsigned(line));
#endif
    terminate_program(134);
}

// the applications reset handler address
using entry_function_t = void (*)() noexcept;
extern const entry_function_t application_entry_function __attribute__((noreturn));
}

namespace {
class Flash_Lock {
  public:
    Flash_Lock() { HAL_FLASH_Unlock(); }
    ~Flash_Lock() { HAL_FLASH_Lock(); }
};

consteval size_t effected_Flash_Banks(size_t begin, size_t end) {
    if (FLASH_BANK1_BASE <= begin && begin <= FLASH_BANK2_BASE && FLASH_BANK1_BASE <= end &&
        end <= FLASH_BANK2_BASE) {
        return FLASH_BANK_1;
    }
    if (FLASH_BANK2_BASE <= begin && FLASH_BANK2_BASE <= end) {
        return FLASH_BANK_2;
    }
    return FLASH_BANK_BOTH;
}

bool erase_application() noexcept {
    constexpr size_t banks =
        effected_Flash_Banks(flash_layout::application_begin, flash_layout::application_end);
    FLASH_EraseInitTypeDef EraseInit;
    uint32_t SectorError = 0;

    Flash_Lock lock{};
    if (banks == FLASH_BANK_BOTH) {
        constexpr size_t sectors_in_first_bank = FLASH_SECTOR_TOTAL - flash_layout::application_begin_page;

        EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInit.Banks = FLASH_BANK_1;
        EraseInit.Sector = flash_layout::application_begin_page;
        EraseInit.NbSectors = sectors_in_first_bank;
        EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_OK != HAL_FLASHEx_Erase(&EraseInit, &SectorError)) {
            return false;
        }

        EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInit.Banks = FLASH_BANK_2;
        EraseInit.Sector = 0;
        EraseInit.NbSectors = flash_layout::application_end_page - FLASH_SECTOR_TOTAL;
        EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_OK != HAL_FLASHEx_Erase(&EraseInit, &SectorError)) {
            return false;
        }
    } else {
        EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
        EraseInit.Banks = banks;
        EraseInit.Sector = flash_layout::application_begin_page;
        EraseInit.NbSectors = flash_layout::application_end_page - flash_layout::application_begin_page;
        EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
        if (HAL_OK != HAL_FLASHEx_Erase(&EraseInit, &SectorError)) {
            return false;
        }
    }
    return true;
}
} // namespace

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

bool flush_write_buffer() {
    Flash_Lock lock{};
    for (size_t i = 0; i < write_application_buffer_used &&
                       reinterpret_cast<size_t>(write_application_it) < flash_layout::application_end;
         i += FLASH_NB_32BITWORD_IN_FLASHWORD) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, reinterpret_cast<size_t>(write_application_it),
                              *reinterpret_cast<uint32_t *>(&write_application_buffer[i])) != HAL_OK) {
            write_application_buffer_used -= i;
            std::memcpy(write_application_buffer.data(), write_application_buffer.data() + i,
                        write_application_buffer_used);
            return false;
        }
        write_application_it += FLASH_NB_32BITWORD_IN_FLASHWORD;
    }

    write_application_buffer_used = 0;
    return true;
}

void write_application(uint8_t value) {
    write_application_buffer[write_application_buffer_used] = value;
    ++write_application_buffer_used;
    if (write_application_buffer_used == write_application_buffer.size()) {
        flush_write_buffer();
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

    // check if the version info of the application is valid
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

    // permit only upgrade
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
    initialize_HAL_GetTick();

    if (!erase_application())
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

    return flush_write_buffer();
}

void jump_to_application() noexcept {
    // clear maybe initialized peripherals
    TIM2->CR1 = 0;
    RCC->APB1LENR &= ~RCC_APB1LENR_TIM2EN;
    application_entry_function();
}

} // namespace bootloader
