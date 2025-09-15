#include <cstddef>
#include <cstdint>
#include <cstring>

#include <stm32f4xx_hal.h>
#include <stm32f4xx_ll_pwr.h>
#include <stm32f4xx_ll_rcc.h>
#include <stm32f4xx_ll_system.h>

extern "C" {
#ifdef STARTUP_WITH_SEMIHOSTING
extern void initialise_monitor_handles(void); /* prototype */
constexpr static uint32_t SYS_EXIT_EXTENDED = 0x20;
constexpr static uint32_t angel_SWIreason_ReportException = 0x18;
constexpr static uint32_t ADP_Stopped_ApplicationExit = 0x20026;
#endif

#ifdef BOOT_WITH_INIT_ARRAY
// static object initialization function pointers (not required)
extern void (*__init_array_start[])();
extern void (*__init_array_end[])();
#endif
// the addresses to constant data
extern size_t data_start;
extern size_t data_load_start;
extern size_t data_load_size;
// the addresses to zero out
extern size_t bss_start;
extern size_t bss_size;
// the addresses to fast_text
extern size_t fast_text_start;
extern size_t fast_text_load_start;
extern size_t fast_text_load_size;
// the addresses to fast_data
extern size_t fast_data_start;
extern size_t fast_data_load_start;
extern size_t fast_data_load_size;

__attribute__((used)) void init_mem() noexcept {
    // configure 16MHz / 8 * 96 / 2 = 96 MHz
    LL_RCC_PLL_ConfigDomain_SYS(LL_RCC_PLLSOURCE_HSI, LL_RCC_PLLM_DIV_8, 96, LL_RCC_PLLP_DIV_2);
    LL_RCC_PLL_Enable();
    LL_RCC_SetAHBPrescaler(LL_RCC_SYSCLK_DIV_1);
    LL_RCC_SetAPB1Prescaler(LL_RCC_APB1_DIV_16);
    LL_RCC_SetAPB2Prescaler(LL_RCC_APB2_DIV_16);
    LL_FLASH_SetLatency(LL_FLASH_LATENCY_4);

    /* Wait till PLL is ready */
    while (LL_RCC_PLL_IsReady() != 1) {
    }
    while (LL_FLASH_GetLatency() != LL_FLASH_LATENCY_4) {
    }
    LL_RCC_SetSysClkSource(LL_RCC_SYS_CLKSOURCE_PLL);

    /* Wait till System clock is ready */
    while (LL_RCC_GetSysClkSource() != LL_RCC_SYS_CLKSOURCE_STATUS_PLL) {
    }
    // initialize the crc accelerator and set up a dma, to feed the application
    // into it

    // initialize loaded memory sections
    std::memcpy(&data_start, &data_load_start, size_t(&data_load_size));
    std::memset(&bss_start, 0, size_t(&bss_size));

    // initialize fast ram sections
    std::memcpy(&fast_data_start, &fast_data_load_start, size_t(&fast_data_load_size));
    std::memcpy(&fast_text_start, &fast_text_load_start, size_t(&fast_text_load_size));

#ifdef BOOT_WITH_INIT_ARRAY
    // initialization of static objects (not required)
    //__libc_init_array();
    // or :
    for (auto *current = __init_array_start; current != __init_array_end; ++current) {
        (*current)();
    }
#endif

#ifdef STARTUP_WITH_SEMIHOSTING
    initialise_monitor_handles(); /* prototype */
#endif
}

__attribute__((used)) void terminate_program(int rc) noexcept {
    (void)rc;
#ifdef STARTUP_WITH_SEMIHOSTING
    uint32_t const args[2] = {ADP_Stopped_ApplicationExit, uint32_t(rc)};
    // set semi hosting exit value from asm call
    __asm__ volatile("\n\tmov r1, %1"
                     "\n\tmov r0, %0"
                     "\n\tBKPT 0xAB"
                     : //
                     : "i"(SYS_EXIT_EXTENDED), "r"(args)
                     : "r0", "r1");
#endif
    // no valid actions to take anymore, reboot
    NVIC_SystemReset();

    // Loop until the power goes down
    for (;;) {
    }
}

void NMI_Handler() { __BKPT(0); }
void Hard_fault_Handler() { __BKPT(0); }
void Memeory_managment_fault_Handler() { __BKPT(0); }
void Bus_fault_Handler() { __BKPT(0); }
void Usage_fault_Handler() { __BKPT(0); }
}
