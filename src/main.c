#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <motors.h>

#include "main.h"
#include "modules/include/telemetry.h"

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();

	telemetry_init();
    while (true) {
        uint8_t data = ReceiveDestinationFromComputer();
        epuck_printf("oueoueeee dest received :%u\n", data);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
