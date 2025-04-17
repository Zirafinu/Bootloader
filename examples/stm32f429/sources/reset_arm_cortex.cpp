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

void SysTick_Handler(void) { HAL_IncTick(); }

// the applications reset handler address
using entry_function_t = void (*)() noexcept;
extern const entry_function_t application_entry_function __attribute__((noreturn));

int main() noexcept;
// static object initialization function pointers (not required)
// extern void (*__init_array_start[])();
// extern void (*__init_array_end[])();
// the addresses to constant data
extern size_t data_load_start;
extern size_t data_load_end;
extern size_t data_start;
// the addresses to zero out
extern size_t bss_start;
extern size_t bss_end;

__attribute__((used)) void init_mem() noexcept {
    // set core in hsi clock mode, with pll enabled (just push out the
    // registers)
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_1);
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_1) {
    }
    LL_PWR_SetRegulVoltageScaling(LL_PWR_REGU_VOLTAGE_SCALE3);
    LL_PWR_DisableOverDriveMode();
    LL_RCC_HSI_Enable();

    /* Wait till HSI is ready */
    while (LL_RCC_HSI_IsReady() != 1) {
    }
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_HSI);
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_HSI) {
    }

    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 50, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    while (LL_PWR_IsActiveFlag_VOS() == 0) {
    }
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_16);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_16);
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    // initialize the crc accelerator and set up a dma, to feed the application
    // into it

    // initialize loaded memory sections
    std::memcpy(&data_start, &data_load_start, size_t(&data_load_end) - size_t(&data_load_start));
    std::memset(&bss_start, 0, size_t(&bss_end) - size_t(&bss_start));

    // initialization of static objects (not required)
    // __libc_init_array();
    // or :
    // for (auto *current = __init_array_start; current != __init_array_end; ++current) {
    //     (*current)();
    // }
    main();
}
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
    EraseInit.Sector = flash_layout::appl_begin_page;
    EraseInit.NbSectors = flash_layout::appl_end_page - flash_layout::appl_begin_page;
    EraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;

    uint32_t SectorError = 0;
    return HAL_OK == HAL_FLASHEx_Erase(&EraseInit, &SectorError);
}

static uint8_t const *read_backup_it = nullptr;
static uint8_t *write_application_it = nullptr;
static size_t write_application_buffer_used = 0;
alignas(4) static std::array<uint8_t, 1024> write_application_buffer{};

static bool can_read_more() {
    return read_backup_it < reinterpret_cast<uint8_t const *>(flash_layout::appl_backup_end);
}
static uint8_t read_appl_backup() { return *read_backup_it++; }

static bool flush_write_buffer() {
    Flash_Lock lock{};

    for (size_t i = 0; i < write_application_buffer_used &&
                       reinterpret_cast<size_t>(write_application_it) < flash_layout::appl_end;
         i += 4) {
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, reinterpret_cast<size_t>(write_application_it),
                              *reinterpret_cast<uint32_t *>(&write_application_buffer[i])) != HAL_OK) {
            write_application_buffer_used -= i;
            std::memcpy(write_application_buffer.data(), write_application_buffer.data() + i, write_application_buffer_used);
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
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::appl_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::appl_end - 4));
}

bool application_backup_is_valid() noexcept {
    return *reinterpret_cast<uint32_t *>(flash_layout::appl_backup_end - 4) ==
           crc::compute(reinterpret_cast<uint8_t *>(flash_layout::appl_backup_begin),
                        reinterpret_cast<uint8_t *>(flash_layout::appl_backup_end - 4));
}

bool copy_backup_to_application() noexcept {
    if (!erase_application())
        return false;
    read_backup_it = reinterpret_cast<uint8_t const *>(&flash_layout::appl_backup_begin);
    write_application_it = reinterpret_cast<uint8_t *>(const_cast<size_t *>(&flash_layout::appl_begin));
    gzip::Inflate inflator{can_read_more, read_appl_backup, write_application, read_application};
    if (!inflator.decode()) {
        return false;
    }

    return flush_write_buffer();
}

void jump_to_application() noexcept { application_entry_function(); }
} // namespace bootloader
