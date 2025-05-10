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

#define IMU_THD_PERIOD_MS 0.100
#define RAD_DEG (180/3.14)

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

static void kalman_update(float measurement, float *yaw_estimate, float *rate_estimate, 
                            float *yaw_uncertainty, float *rate_uncertainty) {

    float gain_yaw = *yaw_uncertainty/(*yaw_uncertainty + IMU_MEAS_NOISE);
    float gain_rate = *rate_uncertainty/(*rate_uncertainty + IMU_MEAS_NOISE);

    *yaw_estimate = *yaw_estimate + gain_yaw*(measurement - *rate_estimate);
    *rate_estimate = *rate_estimate + gain_rate*(measurement - *rate_estimate);

    *yaw_uncertainty = (1-gain_yaw)*(*yaw_uncertainty);
    *rate_uncertainty = (1-gain_rate)*(*rate_uncertainty);
}

static void kalman_predict(float *yaw_estimate, float *rate_estimate, 
                            float *yaw_uncertainty, float *rate_uncertainty) {

    /* integrate angle with correct dt */
    *yaw_estimate += IMU_THD_PERIOD_MS * (*rate_estimate);

    /* inject process noise Q */
    *yaw_uncertainty  += IMU_PROC_NOISE * IMU_THD_PERIOD_MS * IMU_THD_PERIOD_MS;
    *rate_uncertainty += IMU_PROC_NOISE * IMU_THD_PERIOD_MS;
}


static THD_WORKING_AREA(waIMUThd, 512);
static THD_FUNCTION(IMUThd, arg)
{
    (void) arg;
    chRegSetThreadName(__FUNCTION__);

    messagebus_topic_t* imu_sub = messagebus_find_topic_blocking(&bus, "/imu"); // subscriber to reader thd
    //messagebus_topic_t* imu_pub = (messagebus_topic_t*) malloc(sizeof(messagebus_topic_t)); // publishes filtered data
    messagebus_topic_t* odo_pub = (messagebus_topic_t*) malloc(sizeof(messagebus_topic_t));

    //imu_data_t msg = {0};
    odometry_t msg = {0};

    //MUTEX_DECL(imu_pub_lock);
    //CONDVAR_DECL(imu_pub_condvar);
    MUTEX_DECL(odo_pub_lock);
    CONDVAR_DECL(odo_pub_condvar);

    messagebus_topic_init(odo_pub, &odo_pub_lock, &odo_pub_condvar, &msg, sizeof(odometry_t));
    messagebus_advertise_topic(&bus, odo_pub, "/odometry");

    systime_t time;

    float yaw_estimate = 0;
    float yaw_uncertainty = 4;
    float rate_estimate = 0;
    float rate_uncertainty = 1;

    kalman_predict(&yaw_estimate, &rate_estimate, &yaw_uncertainty, &rate_uncertainty);

    while (true) {
        time = chVTGetSystemTime();

        imu_msg_t raw = {0};
        messagebus_topic_wait(imu_sub, &raw, sizeof(imu_msg_t));
        
        imu_data_t unfiltered = {
            .acc = { raw.acceleration[0], raw.acceleration[1], raw.acceleration[2] },
            .ang_vel = { raw.gyro_rate[0], raw.gyro_rate[1], raw.gyro_rate[2]*RAD_DEG}
        };
        
        kalman_update(unfiltered.ang_vel[2],&yaw_estimate, &rate_estimate, &yaw_uncertainty, &rate_uncertainty);

        odometry_t odometry_data;
        
        odometry_data.yaw = ((uint16_t) yaw_estimate) % 360;
        //odometry_data.yaw = yaw_estimate;

        messagebus_topic_publish(odo_pub, &odometry_data, sizeof(odometry_t));

        kalman_predict(&yaw_estimate, &rate_estimate, &yaw_uncertainty, &rate_uncertainty);

        chThdSleepUntilWindowed(time, time + IMU_THD_PERIOD_MS);
    }
}

void imu_init(void) {
    imu_start();
    calibrate_acc();
	calibrate_gyro();

    chThdCreateStatic(waIMUThd, sizeof(waIMUThd), NORMALPRIO, IMUThd, NULL);
}
