#include <cstdint>

#include <bootloader/version_info.h>

#include <stm32h7xx.h>

int main() {
    // enable GPIO bank A, B and E
    RCC->AHB4ENR |= RCC_AHB4ENR_GPIOBEN | RCC_AHB4ENR_GPIOEEN;
    GPIOB->MODER &= ~(GPIO_MODER_MODER0_Msk | GPIO_MODER_MODER14_Msk);
    GPIOB->MODER |= GPIO_MODER_MODER0_0 | GPIO_MODER_MODER14_0;
    GPIOE->MODER &= ~(GPIO_MODER_MODER1_Msk);
    GPIOE->MODER |= GPIO_MODER_MODER1_0;

    std::size_t leds = 0;

    for (;;) {
        ++leds;
        GPIOB->BSRR = 1 << ((16 * ((leds & 1) == 0)) + 0);
        GPIOE->BSRR = 1 << ((16 * ((leds & 2) == 0)) + 1);
        GPIOB->BSRR = 1 << ((16 * ((leds & 4) == 0)) + 14);

        // might be enough delay to see them blink
        for (int i = 0; i < (240000 / 4); ++i) {
            for (int j = 0; j < 50; ++j) {
                __NOP();
            }
        }
    }

    return 0;
}

extern "C" __attribute__((used)) //
const version::info version_info_struct{0x01'01'00001, 0x000000001, 0x000000000};
