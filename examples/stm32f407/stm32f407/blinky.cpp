#include <cstdint>

#include <version_info.h>

#include <stm32f407xx.h>

int main() {
    // enable GPIO bank D
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIODEN;
    // the LEDs are connected to pins D12-D15
    GPIOD->MODER = GPIO_MODER_MODE12_0 | GPIO_MODER_MODE13_0 | GPIO_MODER_MODE14_0 | GPIO_MODER_MODE15_0;

    std::size_t leds = 0;

    for (;;) {
        ++leds;
        uint32_t const on_set = leds & 0xF;
        uint32_t const off_set = ((~leds) & 0xF) << 16;
        GPIOD->BSRR = (on_set | off_set) << 12;

        // might be enough delay to see them blink
        for (int i = 0; i < 1000; ++i) {
            __NOP();
        }
    }

    return 0;
}

__attribute__((used))
const version_info::version_info version_info_struct{0x01'01'00001, 0x000000000, 0x000000000};
