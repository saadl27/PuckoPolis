#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include "i2c_bus.h"
#include <main.h>

#include "modules/include/telemetry.h"
#include "modules/include/distance.h"

#define DIST_OFFSET_MM 50


int main(void) {
    halInit();
    chSysInit();
    mpu_init();

	// /* start peripherals */
	// i2c_start();

	telemetry_init();
	tof_init();
    /* Infinite loop. */
    while (1) {
        uint16_t dist_mm = tof_get_dist_mm();
        /*  CONSIDER OFFSET + CALIB FACTOR IN FORMULA BELOW
            MEASURES SEEM TO BE A BIT OFF
        */
		epuck_printf("dist = %d [mm]\n", dist_mm - DIST_OFFSET_MM);
        chThdSleepMilliseconds(100);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}