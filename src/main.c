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


int main(void) {
    halInit();
    chSysInit();
    mpu_init();

	// /* start peripherals */
	// i2c_start();

	telemetry_init();
	tof_init();

    uint16_t i = 0;
    float mean = 0;
    float filt_mean = 0;
    /* Infinite loop. */
    while (1) {
        int16_t dist_mm = tof_get_dist_mm();
        uint16_t filtered_dist_mm = tof_get_filtered_dist_mm();
        mean += dist_mm;
        filt_mean += filtered_dist_mm;
        i++;

		epuck_printf("dist = %4d [mm] \t filt = %4d [mm]\n", dist_mm,
                                                           filtered_dist_mm);
        // epuck_printf("distance = %d [mm]\n", dist_mm);
        chThdSleepMilliseconds(100);

        if (i % 100 == 0) {
            mean /= 100;
            filt_mean /= 100;
            // epuck_printf("mean = %f \t filt_mean = %f\n", mean, filt_mean);
            i = 0;
            mean = 0;
            filt_mean = 0;
        }
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}