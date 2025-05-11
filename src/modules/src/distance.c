/* C Standard Library */
#include <stdbool.h>

/* ChibiOS Library */
#include "ch.h"

/* Modules Library */
#include "main.h"
#include "modules/include/distance.h"
#include "modules/include/telemetry.h"

/* e-puck2 main processor Library */
#include "sensors/VL53L0X/VL53L0X.h"

#define TOF_INIT_TIME_MS 300
#define TOF_THD_PERIOD_MS 100

/* Kalman calibration (all measured from empirical data) 
   & Kalman filter variables */
#define DIST_OFFSET_MM 50
#define PROC_NOISE 5.0f  // process noise (variance)
#define MEAS_NOISE 8.52f // measurement noise (variance)

static float kalman_gain = 0.0f;
static float estimated_dist = 0.0f;
static float estimation_err = 100.0f; // high initial uncertainty
static bool filter_initialized = false;

static THD_WORKING_AREA(waDistanceThd, 512);
static THD_FUNCTION(DistanceThd, arg)
{
    (void) arg;
    chRegSetThreadName(__FUNCTION__);

    messagebus_topic_t tof_topic;
    tof_msg_t distance;

    MUTEX_DECL(tof_topic_lock);
    CONDVAR_DECL(tof_topic_condvar);

    messagebus_topic_init(&tof_topic, &tof_topic_lock, &tof_topic_condvar, &distance, sizeof(tof_msg_t));
    messagebus_advertise_topic(&bus, &tof_topic, "/distance");

    systime_t time;

    while (true) {
        time = chVTGetSystemTime();
        distance.dist_mm = tof_get_filtered_dist_mm();
        messagebus_topic_publish(&tof_topic, &distance, sizeof(tof_msg_t));
        chThdSleepUntilWindowed(time, time + MS2ST(TOF_THD_PERIOD_MS));
    }
}

void tof_init(void) {
    VL53L0X_start();
    chThdSleepMilliseconds(TOF_INIT_TIME_MS);
    chThdCreateStatic(waDistanceThd, sizeof(waDistanceThd), NORMALPRIO, DistanceThd, NULL);
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

    return (uint16_t) (estimated_dist > 0 ? estimated_dist : 0);
}