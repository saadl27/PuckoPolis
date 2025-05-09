#include <ch.h>
#include <hal.h>
#include <math.h>
#include <usbcfg.h>
#include <chprintf.h>
#include <motors.h>

#include "modules/include/motor.h"
#include "main.h"
#include "modules/include/camera.h"
#include "modules/include/brain.h"

static bool rotating = false; 

//simple PI regulator implementation
int16_t pi_regulator(float distance, float goal){

	float error = 0;
	float speed = 0;

	static float sum_error = 0;

	error = distance - goal;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	sum_error += error;

	//we set a maximum and a minimum for the sum to avoid an uncontrolled growth
	if(sum_error > MAX_SUM_ERROR){
		sum_error = MAX_SUM_ERROR;
	}else if(sum_error < -MAX_SUM_ERROR){
		sum_error = -MAX_SUM_ERROR;
	}

	//speed = KP * error + KI * sum_error;
	speed = KP * error;

    return (int16_t)speed;
}

static THD_WORKING_AREA(waPiRegulator, 256);
static THD_FUNCTION(PiRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();
        
		if (!rotating){
			//computes the speed to give to the motors
			//distance_cm is modified by the image processing thread
			//speed = pi_regulator(get_distance_cm(), GOAL_DISTANCE);
			speed = get_moving() ? FWD_SPEED : 0;
			//computes a correction factor to let the robot rotate to be in front of the line
			speed_correction = pi_regulator(get_line_position(), (IMAGE_BUFFER_SIZE/2));

			//if the line is nearly in front of the camera, don't rotate
			if(abs(speed_correction) < ROTATION_THRESHOLD){
				speed_correction = 0;
			}

			right_motor_set_speed(speed - speed_correction);
			left_motor_set_speed(speed + speed_correction);
			
			//100Hz
			chThdSleepUntilWindowed(time, time + MS2ST(10));

		}
    }
}

static void rotate(int16_t left_speed, int16_t right_speed){
	rotating = true; 
	right_motor_set_speed(FWD_SPEED);
	left_motor_set_speed(FWD_SPEED);
	//about 500ms at 168MHz
    for(uint32_t i = 0 ; i < 21000000 ; i++){
        __asm__ volatile ("nop");
    }
	right_motor_set_speed(right_speed);
	left_motor_set_speed(left_speed);
	//about 500ms at 168MHz
    for(uint32_t i = 0 ; i < 18000000 ; i++){
        __asm__ volatile ("nop");
    }
	right_motor_set_speed(FWD_SPEED);
	left_motor_set_speed(FWD_SPEED);
	//about 500ms at 168MHz
	for(uint32_t i = 0 ; i < 21000000 ; i++){
        __asm__ volatile ("nop");
    }
	rotating = false;
}

void rotate_ccw(void){
	rotate(ROT_SPEED, -ROT_SPEED);
}

void rotate_cw(void){
	rotate(-ROT_SPEED, ROT_SPEED);
}

static void pi_regulator_start(void) {
	chThdCreateStatic(waPiRegulator, sizeof(waPiRegulator), NORMALPRIO, PiRegulator, NULL);
}

void motor_init(){
    //inits the motors
	motors_init();

	//stars the threads for the pi regulator and the processing of the image
	pi_regulator_start();

}