#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include "i2c_bus.h"
#include "main.h"

#include "modules/include/telemetry.h"
#include "modules/include/distance.h"

/* temporary */
#define LOOP_PERIOD_MS 100

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void) {
    halInit();
    chSysInit();
    mpu_init();
    messagebus_init(&bus, &bus_lock, &bus_condvar);

	// /* start peripherals */
	// i2c_start();

	telemetry_init();
	tof_init();


    messagebus_topic_t* dist_topic = messagebus_find_topic_blocking(&bus, "/distance");
    tof_msg_t dist;

    systime_t time;

    /* Infinite loop. */
    while (1) {
        time = chVTGetSystemTime();
        int16_t unfiltered = tof_get_dist_mm();
        messagebus_topic_wait(dist_topic, &dist, sizeof(tof_msg_t));

        epuck_printf("dist = %4d [mm] \t, filt = %4d [mm]\n", unfiltered, dist.dist_mm);
		// epuck_printf("dist = %4d [mm] \t filt = %4d [mm]\n", dist_mm,
        //                                                    filtered_dist_mm);
        // epuck_printf("distance = %d [mm]\n", dist_mm);
        chThdSleepUntilWindowed(time, time + LOOP_PERIOD_MS);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}