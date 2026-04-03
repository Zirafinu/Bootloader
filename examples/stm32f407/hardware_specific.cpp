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
} // namespace

namespace bootloader::hardware_specific {
void initialize_core_for_update() { initialize_HAL_GetTick(); }

void initialize_core_for_application_start() {
    TIM2->CR1 = 0;
    RCC->APB1ENR &= ~RCC_APB1ENR_TIM2EN;
}

bool flush_write_buffer(uint8_t *target, std::array<uint8_t, 1024> const &data, std::size_t valid_data) {
    Flash_Lock lock{};
    for (size_t i = 0; i < valid_data && reinterpret_cast<size_t>(target) < flash_layout::application_end;
         i += 4) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, reinterpret_cast<size_t>(target),
                              *reinterpret_cast<uint32_t const *>(&data[i])) != HAL_OK) {
            return false;
        }
        target += 4;
    }

    return true;
}

bool erase_application() {
    FLASH_EraseInitTypeDef EraseInit;
    EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInit.Banks = 0xFFffFFff;
    EraseInit.Sector = flash_layout::application_begin_page;
    EraseInit.NbSectors = flash_layout::application_end_page - flash_layout::application_begin_page;
    EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t SectorError = 0;
    return HAL_OK == HAL_FLASHEx_Erase(&EraseInit, &SectorError);
}
} // namespace bootloader::hardware_specific
