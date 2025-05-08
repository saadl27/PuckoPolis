#ifndef __IMU_H__
#define __IMU_H__

#include <stdint.h>
#include "sensors/imu.h"

#define GRAVITY_CONST 9.80665f // m/s^2

typedef struct {
    float acc[NB_AXIS];
    float ang_vel[NB_AXIS];
} imu_data_t;

void imu_init(void);

#endif /* __IMU_H__ */