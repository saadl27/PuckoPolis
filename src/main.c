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
#include "modules/include/inertial.h"

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

	telemetry_init();
	tof_init();
    imu_init();
    epuck_printf("hello\n");


    messagebus_topic_t* dist_topic = messagebus_find_topic_blocking(&bus, "/distance");
    messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_processed");

    tof_msg_t dist = {0};
    imu_data_t imu = {0};

    systime_t time;

    /* Infinite loop. */
    while (1) {
        time = chVTGetSystemTime();

        messagebus_topic_wait(dist_topic, &dist, sizeof(tof_msg_t));
        messagebus_topic_wait(imu_topic, &imu, sizeof(imu_data_t));

        chThdSleepUntilWindowed(time, time + LOOP_PERIOD_MS);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}