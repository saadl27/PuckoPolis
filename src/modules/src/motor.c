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
#include "modules/include/inertial.h"
#include "modules/include/telemetry.h"

#define PID_LOOP_MS		10
const float dt = PID_LOOP_MS / 1000.0f;


//simple PID regulator implementation
int16_t pid_regulator(float distance, float goal){
    float error = distance - goal;
	static float sum_error = 0.0f;
    static float prev_error = 0.0f;
    static float filtered_derivative = 0.0f;

	//disables the PI regulator if the error is to small
	//this avoids to always move as we cannot exactly be where we want and 
	//the camera is a bit noisy
	if(fabs(error) < ERROR_THRESHOLD){
		return 0;
	}

	sum_error += error;

	if (sum_error > MAX_SUM_ERROR) {
		sum_error = MAX_SUM_ERROR;
	} else if (sum_error < -MAX_SUM_ERROR) {
		sum_error = -MAX_SUM_ERROR;
	}

	float raw_derivative = (error - prev_error) / dt;

    if (raw_derivative > MAX_D_ERROR)		raw_derivative = MAX_D_ERROR;
    else if (raw_derivative < -MAX_D_ERROR) raw_derivative = -MAX_D_ERROR;

    filtered_derivative = LOW_PASS_FACTOR * filtered_derivative + (1.0f - LOW_PASS_FACTOR) * raw_derivative;
	if (get_state() != MISSION) sum_error = 0.0f;
    float speed = KP * error + KI * sum_error + KD * filtered_derivative;
    prev_error = error;

    return (int16_t) speed;
}

static THD_WORKING_AREA(waPidRegulator, 256);
static THD_FUNCTION(PidRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

    while(1){
        time = chVTGetSystemTime();
        
		if (get_state() == MISSION) {
			//computes the speed to give to the motors
			//distance_cm is modified by the image processing thread
			//speed = pid_regulator(get_distance_cm(), GOAL_DISTANCE);
			speed = FWD_SPEED;
			//computes a correction factor to let the robot rotate to be in front of the line
			speed_correction = pid_regulator(get_line_position(), (IMAGE_BUFFER_SIZE/2));

			//if the line is nearly in front of the camera, don't rotate
			if(abs(speed_correction) < ROTATION_THRESHOLD){
				speed_correction = 0;
			}

			right_motor_set_speed(speed - speed_correction);
			left_motor_set_speed(speed + speed_correction);
		
		} 
		//100Hz
		chThdSleepUntilWindowed(time, time + MS2ST(PID_LOOP_MS));

    }
}

void translate(void) {
	right_motor_set_speed(FWD_SPEED);
	left_motor_set_speed(FWD_SPEED);
	//about 500ms at 168MHz
    for(uint32_t i = 0; i < 21000000; i++){
        __asm__ volatile ("nop");
    }
}

void stop_motors(void){
	right_motor_set_speed(0);
	left_motor_set_speed(0);
}

//implement thread to rotate using filtered gyro yaw (should take desired heading as input and return true when completed)
void correct_heading(float target_heading) {
	// epuck_printf("[motors] before moving straight\n");
	//translate();
	// epuck_printf("[motors] AFTER moving straight\n");
	messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_yaw");
    yaw_msg_t angle;
	float error = 0;

	while (true) {
		messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
		error = angle.yaw_rad - target_heading;

		if (fabsf(error) < ERROR_ANGLE) break;
		while (error >= M_PI) error -= 2.0f * M_PI;
		while (error < -M_PI) error += 2.0f * M_PI;

		epuck_printf("[motors] current = %f, \t target = %f, \t, error = %f\n",
						angle.yaw_rad * RAD2DEG, target_heading * RAD2DEG, error * RAD2DEG);

		if (error >= 0) {
			right_motor_set_speed(-ROT_SPEED);
			left_motor_set_speed(ROT_SPEED);
		} else {
			right_motor_set_speed(ROT_SPEED);
			left_motor_set_speed(-ROT_SPEED);
		}
	}
	right_motor_set_speed(FWD_SPEED);
	left_motor_set_speed(FWD_SPEED);
}

void rotate_relative(float relative_angle) {
	messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_yaw");
    yaw_msg_t angle;
	float error = 0;

	messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
	float initial_yaw = angle.yaw_rad;

	float target_heading = initial_yaw + relative_angle;

	while (target_heading >= 2.0f * M_PI)	target_heading -= 2.0f * M_PI;
	while (target_heading < 0.0f)			target_heading += 2.0f * M_PI;

	while (true) {
		messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
		error = angle.yaw_rad - target_heading;

		while (error >= M_PI) error -= 2.0f * M_PI;
		while (error < -M_PI) error += 2.0f * M_PI;

		epuck_printf("[motors] current = %f, \t target = %f, \t error = %f\n",
						angle.yaw_rad * RAD2DEG, target_heading * RAD2DEG, error * RAD2DEG);

		if (fabsf(error) < ERROR_ANGLE) break;

		if (error >= 0) {
			right_motor_set_speed(-ROT_SPEED);
			left_motor_set_speed(ROT_SPEED);
		} else {
			right_motor_set_speed(ROT_SPEED);
			left_motor_set_speed(-ROT_SPEED);
		}
	}
	right_motor_set_speed(FWD_SPEED);
	left_motor_set_speed(FWD_SPEED);
}

static void rotate(int16_t left_speed, int16_t right_speed){
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
}

void rotate_ccw(void){
	rotate(ROT_SPEED, -ROT_SPEED);
}

void rotate_cw(void){
	rotate(-ROT_SPEED, ROT_SPEED);
}

static void pid_regulator_start(void) {
	chThdCreateStatic(waPidRegulator, sizeof(waPidRegulator), NORMALPRIO, PidRegulator, NULL);
}

void motor_init(){
    //inits the motors
	motors_init();

	//stars the threads for the pi regulator and the processing of the image
	pid_regulator_start();

}