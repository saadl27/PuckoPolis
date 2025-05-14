#ifndef __INERTIAL_H__
#define __INERTIAL_H__

/* Constants */
#include "modules/include/constants.h"

/* e-puck2 main processor Library */
#include "sensors/imu.h"


typedef struct {
    float yaw_rad;
} yaw_msg_t;

typedef struct {
    float acc[NB_AXIS];      // ax, ay, az
    float ang_vel[NB_AXIS];  // gx, gy, gz
} imu_data_t;

/* 
    The following struct name is inherited from an earlier version of the algorithm which was
    an Extended Kalman Filter to estimate full position and orientation in a 2D world map.
    It was then shortened to a small 2 state vector and a 2x2 covariance matrix to accurately
    estimate yaw and angular velocity along the z-axis (parallel to gravity).
    We switched to this more lightweight version of the algorithm because it is very accelerometer
    and gyroscope alone to accurately estimate position, roll and pitch. yaw is much easier in a 2D
    plane.
 */
typedef struct {
    float x[IMU_STATE_SIZE];             // state vector
    float P[IMU_STATE_SIZE][IMU_STATE_SIZE]; // covariance matrix
} ekf_state_t; // EKF state x and covariance P

void imu_init(void);
void set_init_yaw(float yaw);


#endif /* __INERTIAL_H__ */
