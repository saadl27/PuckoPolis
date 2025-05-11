#ifndef __INERTIAL_H__
#define __INERTIAL_H__

#include <stdint.h>
#include "sensors/imu.h"

#define GRAVITY_CONST 9.80665f // m/s^2
#define IMU_STATE_SIZE 2       // [theta, bgz]

#define RAD2DEG 57.2957795131f

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


#endif /* __INERTIAL_H__ */
