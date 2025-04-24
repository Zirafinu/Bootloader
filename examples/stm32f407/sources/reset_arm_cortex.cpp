#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

#include <crc.h>
#include <flash_layout.h>
#include <gzip.h>

#include <stm32f4xx_hal.h>
#include <stm32f4xx_hal_flash.h>
#include <stm32f4xx_hal_flash_ex.h>
#include <stm32f4xx_ll_pwr.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_system.h>

extern "C" {
void initialize_HAL_GetTick() {
    constexpr size_t TIMER_BASE_CLOCK_KHZ = 12000;
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;
    TIM2->PSC = TIMER_BASE_CLOCK_KHZ - 1; // 1 kHz
    TIM2->ARR = ~0UL;
    TIM2->CR1 = TIM_CR1_CEN;
}
uint32_t HAL_GetTick() { return TIM2->CNT; }

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

static bool erase_application() noexcept {
    FLASH_EraseInitTypeDef EraseInit;
    EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInit.Banks = 0xFFffFFff;
    EraseInit.Sector = flash_layout::application_begin_page;
    EraseInit.NbSectors = flash_layout::application_end_page - flash_layout::application_begin_page;
    EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t SectorError = 0;
    return HAL_OK == HAL_FLASHEx_Erase(&EraseInit, &SectorError);
}

static uint8_t const *read_update_flash_it = nullptr;
static uint8_t *write_application_it = nullptr;
static size_t write_application_buffer_used = 0;
alignas(4) static std::array<uint8_t, 1024> write_application_buffer{};

static bool can_read_more() {
    return read_update_flash_it < reinterpret_cast<uint8_t const *>(flash_layout::update_end);
}
static uint8_t read_update_flash() { return *read_update_flash_it++; }

static bool flush_write_buffer() {
    Flash_Lock lock{};
    for (size_t i = 0; i < write_application_buffer_used &&
                       reinterpret_cast<size_t>(write_application_it) < flash_layout::application_end;
         i += 4) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, reinterpret_cast<size_t>(write_application_it),
                              *reinterpret_cast<uint32_t *>(&write_application_buffer[i])) != HAL_OK) {
            write_application_buffer_used -= i;
            std::memcpy(write_application_buffer.data(), write_application_buffer.data() + i,
                        write_application_buffer_used);
            return false;
        }
        write_application_it += 4;
    }

    write_application_buffer_used = 0;
    return true;
}
static void write_application(uint8_t value) {
    write_application_buffer[write_application_buffer_used] = value;
    ++write_application_buffer_used;
    if (write_application_buffer_used == write_application_buffer.size()) {
        flush_write_buffer();
    }
}
static uint8_t read_application(std::size_t distance) {
    if (distance > write_application_buffer_used) {
        return *(write_application_it - (distance - write_application_buffer_used));
    }
    return write_application_buffer[write_application_buffer_used - distance];
}
} // namespace

namespace bootloader {
bool application_is_valid() noexcept {
    return *reinterpret_cast<uint32_t *>(flash_layout::application_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::application_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::application_end - 4));
}

bool application_backup_is_valid() noexcept {
    return *reinterpret_cast<uint32_t *>(flash_layout::update_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::update_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::update_end - 4));
}

bool copy_backup_to_application() noexcept {
    initialize_HAL_GetTick();

    if (!erase_application())
        return false;
    read_update_flash_it = reinterpret_cast<uint8_t const *>(&flash_layout::update_begin);
    write_application_it =
        reinterpret_cast<uint8_t *>(const_cast<size_t *>(&flash_layout::application_begin));
    gzip::Inflate inflator{can_read_more, read_update_flash, write_application, read_application};
    if (!inflator.decode()) {
        return false;
    }

    return flush_write_buffer();
}

void jump_to_application() noexcept { application_entry_function(); }
} // namespace bootloader
