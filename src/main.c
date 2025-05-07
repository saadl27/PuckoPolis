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


    // messagebus_topic_t* dist_topic = messagebus_find_topic_blocking(&bus, "/distance");
    // tof_msg_t dist;

    messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_processed");

    systime_t time;

    /* Infinite loop. */
    while (1) {
        time = chVTGetSystemTime();

        imu_data_t imu_values = {0};

        messagebus_topic_wait(imu_topic, &imu_values, sizeof(imu_data_t));

        epuck_printf("%f,\t%f,\t%f\n%f,\t%f,\t%f\n\n",
        imu_values.acc[0], imu_values.acc[1], imu_values.acc[2],
        imu_values.ang_vel[0], imu_values.ang_vel[1], imu_values.ang_vel[2]);

        chThdSleepUntilWindowed(time, time + LOOP_PERIOD_MS);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}