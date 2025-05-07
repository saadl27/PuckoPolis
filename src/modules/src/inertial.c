/* C Standard Library */
#include <stdbool.h>
#include <stdlib.h>

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

#define IMU_THD_PERIOD_MS 100

/* Kalman calibration (all measured from empirical data) 
   & Kalman filter variables */
#define IMU_PROC_NOISE 0.05f
#define IMU_MEAS_NOISE 0.5f
#define IMU_INIT_ERR 100.0f

typedef struct {
    float est;
    float err;
    bool init;
} kalman_axis_t;

static kalman_axis_t acc_filter[3] = {0};
static kalman_axis_t gyro_filter[3] = {0};

static float kalman_update(kalman_axis_t* kf, float measurement) {
    if (!kf->init) {
        kf->est = measurement;
        kf->err = IMU_INIT_ERR;
        kf->init = true;
        return measurement;
    }

    kf->err += IMU_PROC_NOISE;
    float gain = kf->err / (kf->err + IMU_MEAS_NOISE);
    kf->est += gain * (measurement - kf->est);
    kf->err = (1.0f - gain) * kf->err;
    return kf->est;
}


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

        // imu_msg_t imu_values = {0};
        // messagebus_topic_wait(imu_sub, &imu_values, sizeof(imu_msg_t));

        // imu_data_t data = {
        //     .acc = { imu_values.acceleration[0], imu_values.acceleration[1], imu_values.acceleration[2] },
        //     .ang_vel = { imu_values.gyro_rate[0], imu_values.gyro_rate[1], imu_values.gyro_rate[2] }
        // };

        imu_msg_t raw = {0};
        messagebus_topic_wait(imu_sub, &raw, sizeof(imu_msg_t));

        imu_data_t unfiltered = {
            .acc = { raw.acceleration[0], raw.acceleration[1], raw.acceleration[2] },
            .ang_vel = { raw.gyro_rate[0], raw.gyro_rate[1], raw.gyro_rate[2] }
        };
        
        epuck_printf("Unfiltered:\n%f,\t%f,\t%f\n%f,\t%f,\t%f\n\n",
        unfiltered.acc[0], unfiltered.acc[1], unfiltered.acc[2],
        unfiltered.ang_vel[0], unfiltered.ang_vel[1], unfiltered.ang_vel[2]);

        imu_data_t filtered;
        for (int i = 0; i < NB_AXIS; ++i) {
            filtered.acc[i]     = kalman_update(&acc_filter[i], raw.acceleration[i]);
            filtered.ang_vel[i] = kalman_update(&gyro_filter[i], raw.gyro_rate[i]);
        }

        messagebus_topic_publish(imu_pub, &filtered, sizeof(imu_data_t));
        chThdSleepUntilWindowed(time, time + IMU_THD_PERIOD_MS);
    }
}

void imu_init(void) {
    imu_start();
    calibrate_acc();
	calibrate_gyro();

    chThdCreateStatic(waIMUThd, sizeof(waIMUThd), NORMALPRIO, IMUThd, NULL);
}
