#ifndef __IMU_H__
#define __IMU_H__

#include <stdint.h>
#include "sensors/imu.h"

typedef struct {
    float acc[NB_AXIS];
    float ang_vel[NB_AXIS];
} imu_data_t;

void init_imu(void);
// imu_data_t imu_read(void);

#endif /* __IMU_H__ */