#ifndef __IMU_H__
#define __IMU_H__

#include <stdint.h>

#define NB_AXIS 2

typedef struct {
    float acceleration[NB_AXIS]; // m/s^2
    float ang_velocity[NB_AXIS]; // rad/s
    float temperature;
    float magnetometer[NB_AXIS]; //uT
    int16_t acc_raw[NB_AXIS]; //raw values
    int16_t gyro_raw[NB_AXIS]; //raw values
    int16_t acc_offset[NB_AXIS]; //raw offsets
    int16_t gyro_offset[NB_AXIS]; //raw offsets
    int16_t acc_filtered[NB_AXIS];
    int16_t gyro_filtered[NB_AXIS];
    uint8_t status;
} imu_msg_t;

typedef struct {
    float acc[NB_AXIS];
    float ang_vel[NB_AXIS];
    float mag[NB_AXIS];
    float temperature;
} imu_data;

void imu_init(void);
imu_data imu_read(void);

#endif /* __IMU_H__ */