#include <stdbool.h>

#include "modules/include/distance.h"
#include "modules/include/telemetry.h"

#include "sensors/VL53L0X/VL53L0X.h"

// Kalman filter variables
static float kalman_gain = 0.0f;
static float estimated_dist = 0.0f;
static float estimation_err = 0.0f;
static float process_noise = 2.0f; // process noise (variance)
static float measurement_noise = 10.0f; // measurement noise (variance)
static bool filter_initialized = false;


void tof_init(void) {
    // tof_sensor->I2cDevAddr = VL53L0X_ADDR;
    // // VL53L0X_start();
    // VL53L0X_Error status = VL53L0X_init(tof_sensor);
    // epuck_printf("tof status = %d\n", status);
    // epuck_printf("tof sensor addr = %d\n", tof_sensor->I2cDevAddr);
    VL53L0X_start();
    tof_filter_init();
}

void tof_filter_init(void) {
    estimated_dist = 0;
    estimation_err = 100.0f; // high initial uncertainty
    filter_initialized = false;
}

void tof_set_kalman_params(float p_noise, float m_noise, float est_err) {
    process_noise = p_noise;
    measurement_noise = m_noise;
    estimation_err = est_err;
}

uint16_t tof_get_dist_mm(void) {
    return VL53L0X_get_dist_mm();
}

uint16_t tof_get_filtered_dist_mm(void) {
    uint16_t raw_dist = tof_get_dist_mm();
    float measurement = (float) raw_dist;

    if (!filter_initialized) {
        estimated_dist = measurement;
        filter_initialized = true;
        return (uint16_t) estimated_dist;
    }

    estimation_err += process_noise;

    kalman_gain = estimation_err / (estimation_err + measurement_noise);
    estimated_dist += kalman_gain * (measurement - estimated_dist);
    estimation_err = (1 - kalman_gain) * estimation_err;

    return (uint16_t) estimated_dist;
}