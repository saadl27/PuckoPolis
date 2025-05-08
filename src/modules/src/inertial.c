/* C Standard Library */
#include <stdbool.h>
#include <stdlib.h>
#include <math.h>

/* ChibiOS Library */
#include "ch.h"

/* e-puck2 Library */
#include "i2c_bus.h"

/* Modules Library */
#include "main.h"
#include "modules/include/telemetry.h"
#include "modules/include/inertial.h"

/* e-puck2 main processor Library */
#include "sensors/imu.h"

#define IMU_THD_PERIOD_MS 4


static THD_WORKING_AREA(waIMUThd, 512);
static THD_FUNCTION(IMUThd, arg)
{
    (void) arg;
    chRegSetThreadName(__FUNCTION__);

    messagebus_topic_t* imu_sub = messagebus_find_topic_blocking(&bus, "/imu"); // subscriber to reader thd
    messagebus_topic_t* imu_pub = (messagebus_topic_t*) malloc(sizeof(messagebus_topic_t)); // publishes filtered data

    imu_data_t msg = {0};

    MUTEX_DECL(imu_pub_lock);
    CONDVAR_DECL(imu_pub_condvar);

    messagebus_topic_init(imu_pub, &imu_pub_lock, &imu_pub_condvar, &msg, sizeof(imu_data_t));
    messagebus_advertise_topic(&bus, imu_pub, "/imu_processed");

    systime_t time;

    while (true) {
        time = chVTGetSystemTime();

        imu_msg_t raw = {0};
        messagebus_topic_wait(imu_sub, &raw, sizeof(imu_msg_t));

        imu_data_t data = {
            .acc = { raw.acceleration[0], raw.acceleration[1], raw.acceleration[2] },
            .ang_vel = { raw.gyro_rate[0], raw.gyro_rate[1], raw.gyro_rate[2] }
        };

        epuck_printf("%4x\t%4x\t%4x\n%4x\t%4x\n\n",
                    data.acc[0], data.acc[1], data.acc[2],
                    data.ang_vel[0], data.ang_vel[1], data.ang_vel[2]);

        messagebus_topic_publish(imu_pub, &data, sizeof(imu_data_t));
        chThdSleepUntilWindowed(time, time + IMU_THD_PERIOD_MS);
    }
}

void imu_init(void) {
    imu_start();
    calibrate_acc();
	calibrate_gyro();

    chThdCreateStatic(waIMUThd, sizeof(waIMUThd), NORMALPRIO, IMUThd, NULL);
}
