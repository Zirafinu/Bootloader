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

int bootloader_main() noexcept;
// static object initialization function pointers (not required)
// extern void (*__init_array_start[])();
// extern void (*__init_array_end[])();
// the addresses to constant data
extern size_t data_start;
extern size_t data_end;
extern size_t data_load_start;
// the addresses to zero out
extern size_t bss_start;
extern size_t bss_end;
// the addresses to fast_text
extern size_t fast_text_start;
extern size_t fast_text_end;
extern size_t fast_text_load_start;
// the addresses to fast_data
extern size_t fast_data_start;
extern size_t fast_data_end;
extern size_t fast_data_load_start;

void fast_memcpy(void *dst, const void *src, size_t length) {
    auto dst_addr = reinterpret_cast<size_t>(dst);
    auto src_addr = reinterpret_cast<size_t>(src);
    auto const end_addr = dst_addr + length;

    if ((dst_addr % 4) == (src_addr % 4) && length >= 16) {
        while ((dst_addr % 4) != 0) {
            *reinterpret_cast<uint8_t *>(dst_addr) = *reinterpret_cast<uint8_t const *>(src_addr);
            src_addr += 1;
            dst_addr += 1;
        }
        const size_t end_fast_copy_addr = end_addr - (4 * sizeof(size_t));
        while (dst_addr <= end_fast_copy_addr) [[likely]] {
            // load 4 words from src and then store them to dst, while incrementing the addresses
            __asm__ volatile("LDMIA %1!, {r4-r7}\n"
                             "STMIA %0!, {r4-r7}"
                             : "+r"(dst_addr), "+r"(src_addr)
                             :
                             : "r4", "r5", "r6", "r7");
        }

        while (dst_addr <= (end_addr - sizeof(size_t))) {
            *reinterpret_cast<size_t *>(dst_addr) = *reinterpret_cast<size_t const *>(src_addr);
            src_addr += sizeof(size_t);
            dst_addr += sizeof(size_t);
        }
    }
    while (dst_addr != end_addr) {
        *reinterpret_cast<uint8_t *>(dst_addr) = *reinterpret_cast<uint8_t const *>(src_addr);
        src_addr += 1;
        dst_addr += 1;
    }
}

void fast_memset(void *dst, uint8_t value, size_t length) {
    auto dst_addr = reinterpret_cast<size_t>(dst);
    auto const end_addr = dst_addr + length;

    if (length >= 16) {
        while ((dst_addr % 4) != 0) {
            *reinterpret_cast<uint8_t *>(dst_addr) = value;
            dst_addr += 1;
        }

        const size_t value_x4 = 0x01010101 * value;
        const size_t end_fast_set_addr = (end_addr - (4 * sizeof(size_t)));
        if (dst_addr <= end_fast_set_addr) {
            do {
                // store 4 copies at a time, while incrementing the addresses
                __asm__ volatile("STRD %1, %1, [%0], +8\n"
                                 "STRD %1, %1, [%0], +8"
                                 : "+r"(dst_addr)
                                 : "r"(value_x4));
            } while (dst_addr <= end_fast_set_addr);
        }

        while (dst_addr <= (end_addr - sizeof(size_t))) {
            *reinterpret_cast<size_t *>(dst_addr) = value_x4;
            dst_addr += sizeof(size_t);
        }
    }
    while (dst_addr != end_addr) {
        *reinterpret_cast<uint8_t *>(dst_addr) = value;
        dst_addr += 1;
    }
}

__attribute__((used)) void init_mem() noexcept {
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 96, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_16);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_16);
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_3);

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_3) {
    }
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    // initialize the crc accelerator and set up a dma, to feed the application
    // into it

    // initialize loaded memory sections
    fast_memcpy(&data_start, &data_load_start, size_t(&data_end) - size_t(&data_start));
    fast_memset(&bss_start, 0, size_t(&bss_end) - size_t(&bss_start));

    // initialize fast ram sections
    fast_memcpy(&fast_data_start, &fast_data_load_start, size_t(&fast_data_end) - size_t(&fast_data_start));
    fast_memcpy(&fast_text_start, &fast_text_load_start, size_t(&fast_text_end) - size_t(&fast_text_start));

    // initialization of static objects (not required)
    // __libc_init_array();
    // or :
    // for (auto *current = __init_array_start; current != __init_array_end; ++current) {
    //     (*current)();
    // }
    bootloader_main();
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
