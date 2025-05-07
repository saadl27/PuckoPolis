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

#define IMU_THD_PERIOD_MS 100
#define DELTA_T IMU_THD_PERIOD_MS / 1000.0f

/* Kalman calibration (all measured from empirical data) 
   & Kalman filter variables */

typedef struct {
    float est;
    float err;
    float proc_noise;
    float meas_noise;
    float init_err;
    bool init;
} kalman_axis_t;

// 0.00005f, 0.00005f, 0.0001f, 
static kalman_axis_t acc_filter[3] = {
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.015f, .meas_noise = 0.00038f, .init_err = 0.01f, .init = false }, // acc_x
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.015f, .meas_noise = 0.00038f, .init_err = 0.01f, .init = false }, // acc_y
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.03f, .meas_noise = 0.00086f, .init_err = 0.01f, .init = false }  // acc_z
};

// 0.000001f, 0.0000015f, 0.000001f
static kalman_axis_t gyro_filter[3] = {
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.0001f, .meas_noise = 0.0000042f, .init_err = 0.001f, .init = false },  // gyr_x
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.00015f, .meas_noise = 0.0000062f, .init_err = 0.001f, .init = false }, // gyr_y
    { .est = 0.0f, .err = 0.0f, .proc_noise = 0.0001f, .meas_noise = 0.000005f, .init_err = 0.001f, .init = false }   // gyr_z
};

static pose_data_t current_pose = {
    .position = {0},
    .velocity = {0},
    .orientation = {0}
};

static float kalman_update(kalman_axis_t* kf, float measurement) {
    if (!kf->init) {
        kf->est = measurement;
        kf->err = kf->init_err;
        kf->init = true;
        return measurement;
    }

    kf->err += kf->proc_noise;
    float gain = kf->err / (kf->err + kf->meas_noise);
    kf->est += gain * (measurement - kf->est);
    kf->err = (1 - gain) * kf->err;
    return kf->est;
}

static void update_pose(const imu_data_t* imu_data) {
    for (size_t i = 0; i < NB_AXIS; ++i) {
        current_pose.orientation[i] += imu_data->ang_vel[i] * DELTA_T;

        // normalize angles to [-PI, PI]
        while (current_pose.orientation[i] > M_PI) current_pose.orientation[i] -= 2.0f * M_PI;
        while (current_pose.orientation[i] < -M_PI) current_pose.orientation[i] += 2.0f * M_PI;
    }

    // gravity compensated acceleration
    float acc_world[NB_AXIS];

    float sin_roll = sinf(current_pose.orientation[0]);
    float cos_roll = cosf(current_pose.orientation[0]);
    float sin_pitch = sinf(current_pose.orientation[1]);
    float cos_pitch = cosf(current_pose.orientation[1]);

    // rotate accelerometer readings to world frame
    acc_world[0] = imu_data->acc[0] * cos_pitch + imu_data->acc[2] * sin_pitch;
    acc_world[1] = imu_data->acc[0] * sin_roll * sin_pitch + imu_data->acc[1] * cos_roll
                 - imu_data->acc[2] * sin_roll * cos_pitch;
    acc_world[2] = -imu_data->acc[0] * cos_roll * sin_pitch + imu_data->acc[1] * sin_roll
                 + imu_data->acc[2] * cos_roll * cos_pitch - GRAVITY_CONST;

    for (size_t i = 0; i < NB_AXIS; ++i) {
        current_pose.velocity[i] += acc_world[i] * DELTA_T;
    }

    for (size_t i = 0; i < NB_AXIS; ++i) {
        current_pose.position[i] += current_pose.velocity[i] * DELTA_T;
    }
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

        imu_msg_t raw = {0};
        messagebus_topic_wait(imu_sub, &raw, sizeof(imu_msg_t));
        raw.acceleration[2] += GRAVITY_CONST;

        // epuck_printf("Unfiltered:\t ax = %f\t ay = %f\t az = %f\n", raw.acceleration[0], raw.acceleration[1], raw.acceleration[2]);
        // epuck_printf("Unfiltered:\t gx = %f\t gy = %f\t gz = %f\n", raw.gyro_rate[0], raw.gyro_rate[1], raw.gyro_rate[2]);

        imu_data_t filtered;
        for (size_t i = 0; i < NB_AXIS; ++i) {
            filtered.acc[i]     = kalman_update(&acc_filter[i], raw.acceleration[i]);
            filtered.ang_vel[i] = kalman_update(&gyro_filter[i], raw.gyro_rate[i]);
        }

        // epuck_printf("Filtered:\t ax = %f\t ay = %f\t az = %f\n", filtered.acc[0], filtered.acc[1], filtered.acc[2]);
        // epuck_printf("Filtered:\t gx = %f\t gy = %f\t gz = %f\n", raw.gyro_rate[0], raw.gyro_rate[1], raw.gyro_rate[2]);

        pose_data_t pose = current_pose;

        update_pose(&filtered);

        epuck_printf("Position:\t x = %f, y = %f, z = %f\n", pose.position[0], pose.position[1], pose.position[2]);
        epuck_printf("Velocity:\t vx= %f, vy= %f, vz= %f\n", pose.velocity[0], pose.velocity[1], pose.velocity[2]);
        epuck_printf("Orientat:\t rx= %f, ry= %f, rz= %f\n", pose.orientation[0], pose.orientation[1], pose.orientation[2]);

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
