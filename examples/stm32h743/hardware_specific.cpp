#include <hardware_specific.h>

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#ifdef STARTUP_WITH_SEMIHOSTING
#include <cstdio>
#endif

#include <flash_layout.h>

#include <stm32h7xx_hal.h>
#include <stm32h7xx_hal_flash.h>
#include <stm32h7xx_hal_flash_ex.h>

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
} // namespace

namespace bootloader::hardware_specific {
void initialize_core_for_update() { initialize_HAL_GetTick(); }

void initialize_core_for_application_start() {
    TIM2->CR1 = 0;
    RCC->APB1LENR &= ~RCC_APB1LENR_TIM2EN;
}

bool flush_write_buffer(uint8_t *target, std::array<uint8_t, 1024> const &data, std::size_t valid_data) {
    Flash_Lock lock{};
    for (size_t i = 0; i < valid_data && reinterpret_cast<size_t>(target) < flash_layout::application_end;
         i += FLASH_NB_32BITWORD_IN_FLASHWORD) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_FLASHWORD, reinterpret_cast<size_t>(target),
                              *reinterpret_cast<uint32_t const *>(&data[i])) != HAL_OK) {
            return false;
        }
        target += FLASH_NB_32BITWORD_IN_FLASHWORD;
    }

    return true;
}

bool erase_application() {
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
} // namespace bootloader::hardware_specific
