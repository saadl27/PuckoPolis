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

	// /* start peripherals */
	// i2c_start();

	telemetry_init();
	tof_init();
    init_imu();


    // messagebus_topic_t* dist_topic = messagebus_find_topic_blocking(&bus, "/distance");
    // tof_msg_t dist;

    messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu");

    systime_t time;

    /* Infinite loop. */
    while (1) {
        time = chVTGetSystemTime();

        imu_msg_t imu_values;

        messagebus_topic_wait(imu_topic, &imu_values, sizeof(imu_msg_t));

        epuck_printf("%f,\t%f,\t%f\n%f,\t%f,\t%f\n\n",
        imu_values.acceleration[0], imu_values.acceleration[1], imu_values.acceleration[2],
        imu_values.gyro_rate[0], imu_values.gyro_rate[1], imu_values.gyro_rate[2]);

        // imu_data_t data = imu_read();

        // epuck_printf("%f,\t%f,\t%f\n%f,\t%f,\t%f\n%f,\t%f,\t%f\n%f\n", data.acc[0], data.acc[1], data.acc[2],
                                                                //  data.ang_vel[0], data.ang_vel[1], data.ang_vel[2],
                                                                //  data.mag[0], data.mag[1], data.mag[2],
                                                                //  data.temperature);

        // int16_t unfiltered = tof_get_dist_mm();
        // messagebus_topic_wait(dist_topic, &dist, sizeof(tof_msg_t));

        // epuck_printf("dist = %4d [mm] \t, filt = %4d [mm]\n", unfiltered, dist.dist_mm);
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