#include <stm32f4xx.h>
#include <system_clock_config.h>
#include "gpio.h"
#include "main.h"

// Init function required by __libc_init_array
void _init(void) {}

// Simple delay function
void delay(unsigned int n)
{
    while (n--) {
        __asm__ volatile ("nop");
    }
}

int main(void)
{
    // 0 -> S0 = 0, S1 = 0
    // 1 -> S0 = 1, S1 = 0
    // 2 -> S0 = 0, S2 = 1
    // 3 -> S0 = 1

    // selector = S3 S2 S1 S0


    SystemClock_Config();

    // Enable GPIOB, GPIOC & GPIOD peripheral clocks
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN;
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOCEN;
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;
    
    // LEDs defined in main.h
    gpio_config_output_pushpull(LED1);
    gpio_config_output_pushpull(LED3);
    gpio_config_output_pushpull(LED5);
    gpio_config_output_pushpull(LED7);
    gpio_config_output_pushpull(BODY_LED);

    gpio_config_input_pd(SELECTOR0);
    gpio_config_input_pd(SELECTOR1);
    gpio_config_input_pd(SELECTOR2);
    gpio_config_input_pd(SELECTOR3);

    while (1) {
        uint8_t S0 = (uint8_t)gpio_read(SELECTOR0);
        uint8_t S1 = (uint8_t)gpio_read(SELECTOR1);
        uint8_t S2 = (uint8_t)gpio_read(SELECTOR2);
        uint8_t S3 = (uint8_t)gpio_read(SELECTOR3);

        uint8_t selector = (S3 << 3) | (S2 << 2) | (S1 << 1) | S0;

        switch (selector) {
            case 0:
            case 1:
                gpio_toggle(LED1);
                break;
            case 2:
            case 3:
                gpio_toggle(LED3);
                break;
            case 4:
            case 5:
                gpio_toggle(LED5);
                break;
            case 6:
            case 7:
                gpio_toggle(LED7);
                break;
            default:
                gpio_toggle(BODY_LED);
                break;
        }

        // delay(SystemCoreClock/16);
    }
}
