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
#define DT IMU_THD_PERIOD_MS / 1000.0f

#define GZ_MEAS_NOISE 0.0000051610f


static ekf_state_t ekf;


void ekf_init(ekf_state_t* ekf) {
    for (int i = 0; i < IMU_STATE_SIZE; i++) {
        ekf->x[i] = 0.0f;
        for (int j = 0; j < IMU_STATE_SIZE; j++) {
            ekf->P[i][j] = (i == j) ? 0.01f : 0.0f;
        }
    }
}

// compute Jacobian F and process noise Q
static void compute_jacobians(float F[IMU_STATE_SIZE][IMU_STATE_SIZE], float Q[IMU_STATE_SIZE][IMU_STATE_SIZE]) {
    /* 
        zero F and Q
        set F = I
     */
    for (int i = 0; i < IMU_STATE_SIZE; i++) {
        for (int j = 0; j < IMU_STATE_SIZE; j++) {
            F[i][j] = (i == j) ? 1.0f : 0.0f;
            Q[i][j] = 0.0f;
        }
    }

    // dtheta/dbgz = -dt
    F[0][1] = -DT;

    // process noise Q: gyro noise + bias walk
    Q[0][0] = GZ_MEAS_NOISE; // variance of gz
    Q[1][1] = 1e-7f;         // variance of gyro bias random walk (models slow drift of bias over time)
}


void ekf_predict(ekf_state_t* ekf, float wz) {
    float bgz = ekf->x[1];
    float theta_dot = wz - bgz;

    // state prediction
    ekf->x[0] += theta_dot * DT;
    // bias remains unchanged (random walk, zero mean gaussian)

    // cov prediction
    float F[IMU_STATE_SIZE][IMU_STATE_SIZE], Q[IMU_STATE_SIZE][IMU_STATE_SIZE];
    compute_jacobians(F, Q);

    float P_tmp[IMU_STATE_SIZE][IMU_STATE_SIZE] = {{0}};

    /*
        P_{n+1} = F * P_n​ * F.T + G * Q_n * G.T
        Q = diag(gyro bias, random walk)
    */
    for (int i = 0; i < IMU_STATE_SIZE; i++)
        for (int k = 0; k < IMU_STATE_SIZE; k++)
            for (int j = 0; j < IMU_STATE_SIZE; j++)
                P_tmp[i][j] += F[i][k] * ekf->P[k][j];

    for (int i = 0; i < IMU_STATE_SIZE; i++) {
        for (int j = 0; j < IMU_STATE_SIZE; j++) {
            float sum = Q[i][j];
            for (int k = 0; k < IMU_STATE_SIZE; k++)
                sum += P_tmp[i][k] * F[j][k];
            ekf->P[i][j] = sum;
        }
    }

    while (ekf->x[0] >= 2.0f * M_PI)    ekf->x[0] -= 2.0f * M_PI;
    while (ekf->x[0] <  0)              ekf->x[0] += 2.0f * M_PI;
}


MUTEX_DECL(imu_pub_lock);
CONDVAR_DECL(imu_pub_condvar);

static THD_WORKING_AREA(waIMUThd, 512);
static THD_FUNCTION(IMUThd, arg)
{
    (void)arg;
    chRegSetThreadName(__FUNCTION__);

    messagebus_topic_t* imu_sub = messagebus_find_topic_blocking(&bus, "/imu");
    messagebus_topic_t* imu_pub = (messagebus_topic_t*)malloc(sizeof(messagebus_topic_t));

    imu_data_t raw;
    yaw_msg_t angle;
    messagebus_topic_init(imu_pub, &imu_pub_lock, &imu_pub_condvar, &angle, sizeof(yaw_msg_t));
    messagebus_advertise_topic(&bus, imu_pub, "/imu_yaw");

    ekf_init(&ekf);
    systime_t time;

    while (true) {
        time = chVTGetSystemTime();

        imu_msg_t in = {0};
        messagebus_topic_wait(imu_sub, &in, sizeof(in));

        raw.acc[0]     = in.acceleration[0];
        raw.acc[1]     = in.acceleration[1];
        raw.acc[2]     = in.acceleration[2];
        raw.ang_vel[0] = in.gyro_rate[0];
        raw.ang_vel[1] = in.gyro_rate[1];
        raw.ang_vel[2] = in.gyro_rate[2];

        ekf_predict(&ekf, raw.ang_vel[2]);
        angle.yaw_rad = ekf.x[0];

        messagebus_topic_publish(imu_pub, &angle, sizeof(angle));

        chThdSleepUntilWindowed(time, time + MS2ST(IMU_THD_PERIOD_MS));
    }
}

void imu_init(void) {
    imu_start();
    calibrate_acc();
    calibrate_gyro();
    chThdCreateStatic(waIMUThd, sizeof(waIMUThd), NORMALPRIO, IMUThd, NULL);
}
