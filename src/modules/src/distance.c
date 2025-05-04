/* C Standard Library */
#include <stdbool.h>

/* ChibiOS Library */
#include "ch.h"

/* Modules Library */
#include "modules/include/distance.h"
#include "modules/include/telemetry.h"

/* e-puck2 main processor Library */
#include "sensors/VL53L0X/VL53L0X.h"

#define TOF_INIT_TIME_MS 300

/* Kalman calibration (all measured from empirical data) 
   & Kalman filter variables */
#define DIST_OFFSET_MM 50
#define PROC_NOISE 0.3f  // process noise (variance)
#define MEAS_NOISE 8.52f // measurement noise (variance)

static float kalman_gain = 0.0f;
static float estimated_dist = 0.0f;
static float estimation_err = 100.0f; // high initial uncertainty
static bool filter_initialized = false;


void tof_init(void) {
    // tof_sensor->I2cDevAddr = VL53L0X_ADDR;
    // // VL53L0X_start();
    // VL53L0X_Error status = VL53L0X_init(tof_sensor);
    // epuck_printf("tof status = %d\n", status);
    // epuck_printf("tof sensor addr = %d\n", tof_sensor->I2cDevAddr);
    VL53L0X_start();
    chThdSleepMilliseconds(TOF_INIT_TIME_MS);
}

int16_t tof_get_dist_mm(void) {
    return VL53L0X_get_dist_mm() - DIST_OFFSET_MM;
}

uint16_t tof_get_filtered_dist_mm(void) {
    float measurement = (float) tof_get_dist_mm();

    if (!filter_initialized) {
        estimated_dist = measurement;
        filter_initialized = true;
        return (uint16_t)(estimated_dist > 0 ? estimated_dist : 0);
    }

    estimation_err += PROC_NOISE;

    kalman_gain = estimation_err / (estimation_err + MEAS_NOISE);
    estimated_dist += kalman_gain * (measurement - estimated_dist);
    estimation_err = (1 - kalman_gain) * estimation_err;

    epuck_printf("est dist = %f\n", estimated_dist);
    return (uint16_t) (estimated_dist > 0 ? estimated_dist : 0);
}