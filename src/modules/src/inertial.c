/* C Standard Library */
#include <stdbool.h>

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

void init_imu(void) {
    // i2c_start();
    // int8_t status = mpu9250_setup(MPU9250_ACC_FULL_RANGE_2G
	// 	                        | MPU9250_GYRO_FULL_RANGE_250DPS
	// 	                        | MPU9250_SAMPLE_RATE_DIV(100));

    // epuck_printf("status = %d\n", status);
    imu_start();
    calibrate_acc();
	calibrate_gyro();
}

// imu_data_t imu_read(void) {
    // imu_msg_t data = {0};
    // int8_t status = mpu9250_read(data.ang_velocity, data.acceleration, &data.temperature,
    //                              data.magnetometer, data.gyro_raw, data.acc_raw,
    //                              data.gyro_offset, data.acc_offset, &data.status);

    // imu_data_t res = {
    //     .acc = { data.acceleration[0], data.acceleration[1], data.acceleration[2] },
    //     .ang_vel = { data.gyro_raw[0], data.gyro_raw[1], data.gyro_raw[2] },
    //     .mag = { data.magnetometer[0], data.magnetometer[1], data.magnetometer[2] },
    //     .temperature = data.temperature
    // };

    // return res;


// }