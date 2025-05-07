#ifndef __IMU_H__
#define __IMU_H__

#include <stdint.h>
#include "sensors/imu.h"

#define GRAVITY_CONST 9.80665f // m/s^2

typedef struct {
    float acc[NB_AXIS];
    float ang_vel[NB_AXIS];
} imu_data_t;

typedef struct {
    float position[NB_AXIS];     // position in x, y, z
    float velocity[NB_AXIS];     // velocity in x, y, z
    float orientation[NB_AXIS];  // rpy
} pose_data_t;

void imu_init(void);
// imu_data_t imu_read(void);

#endif /* __IMU_H__ */