#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include <main.h>

#include "modules/include/telemetry.h"


int main(void) {
    halInit();
    chSysInit();
    mpu_init();

	telemetry_init();
    /* Infinite loop. */
    while (1) {
    	//waits 1 second
		epuck_printf("Hello world\n");
        chThdSleepMilliseconds(1000);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}