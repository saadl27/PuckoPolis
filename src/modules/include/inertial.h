#ifndef __INERTIAL_H__
#define __INERTIAL_H__

#include <stdint.h>
#include "sensors/imu.h"
#include "modules/include/constants.h"


typedef struct {
    float yaw_rad;
} yaw_msg_t;

typedef struct {
    float acc[NB_AXIS];      // ax, ay, az
    float ang_vel[NB_AXIS];  // gx, gy, gz
} imu_data_t;

typedef struct {
    float x[IMU_STATE_SIZE];             // state vector
    float P[IMU_STATE_SIZE][IMU_STATE_SIZE]; // covariance matrix
} ekf_state_t; // EKF state x and covariance P

void imu_init(void);
void set_init_yaw(float yaw);


#endif /* __INERTIAL_H__ */
