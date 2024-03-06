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
    SystemClock_Config();

    // Enable GPIOD peripheral clock
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIODEN;
    
    // LED7 defined in main.h
    gpio_config_output_pushpull(FRONT_LED);

    while (1) {
        delay(SystemCoreClock/16);
        gpio_toggle(FRONT_LED);
    }
}