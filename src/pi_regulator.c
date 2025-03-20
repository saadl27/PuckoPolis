#include <ch.h>
#include <hal.h>
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>

#include "pi_regulator.h"
#include "main.h"
#include "process_image.h"

#define NSTEP_ONE_TURN  1000
#define WHEEL_PERIMETER 13.0 // [cm]
#define CM_TO_STEPS     NSTEP_ONE_TURN / WHEEL_PERIMETER
#define TARGET          10 // [cm]

#define MAX_CMD         10.0

static float int_error = 0;

typedef struct {
    float Kp;
    float Ki;
    float Kd;
} PID;

static PID pid = {5.0, 0.2, 0};

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;

    while(1){
        time = chVTGetSystemTime();

        /*
		*	To complete
		*/
        float error = -(TARGET - get_distance_cm());
        int_error += error;
        chprintf((BaseSequentialStream*) &SDU1, "DISTANCE: %f, ERROR: %f\n", get_distance_cm(), error);

        float Up = pid.Kp * error;
        float Ui = pid.Ki * int_error;
        if (Up + Ui > MAX_CMD)  Ui = MAX_CMD - Up;
        if (Up + Ui < -MAX_CMD) Ui = -MAX_CMD + Up;

        float command = Up + Ui;
        speed = (int16_t)(CM_TO_STEPS * command);
        chprintf((BaseSequentialStream*) &SDU1, "COMMAND: %f, SPEED: %d\n", command, speed);
        chprintf((BaseSequentialStream*) &SDU1, "Up = %f, Ui = %f\n", Up, Ui);
        
        //applies the speed from the PI regulator
		right_motor_set_speed(speed);
		left_motor_set_speed(speed);

        //100Hz
        chThdSleepUntilWindowed(time, time + MS2ST(100));
    }
}

void pi_regulator_start(void){
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}