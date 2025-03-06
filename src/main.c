#include <stm32f4xx.h>
#include <system_clock_config.h>
#include "gpio.h"
#include "main.h"
#include "timer.h"
#include "motor.h"
#include "selector.h"

#define PI                  3.1415926536f
//TO ADJUST IF NECESSARY. NOT ALL THE E-PUCK2 HAVE EXACTLY THE SAME WHEEL DISTANCE
#define WHEEL_DISTANCE      5.35f    //cm
#define PERIMETER_EPUCK     (PI * WHEEL_DISTANCE)

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

    // Enable GPIOD and GPIOE peripheral clock
    RCC->AHB1ENR    |= RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIODEN;

    // Configure PD14 as alternate function (AF2) for TIM4_CH3
    GPIOD->AFR[1]   = (GPIOD->AFR[1] & ~(0b1111 << 24)) | (2 << 24);

    // enable TIM4 clock
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;

    gpio_config_output_af_pushpull(GPIOD, 14);

    timer4_start(1.0f);


    while (1) {
        for (float i = 0.0f; i < 1.0f; i += 0.01f) {
            TIM4->CCR3 = COUNTER_MAX_TIM4 * i;
            delay(SystemCoreClock / 160);
        }
        
    }
}

