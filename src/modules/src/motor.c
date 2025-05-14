/* C Standard Library */
#include <math.h>

/* ChibiOS Library */
#include <ch.h>
#include <hal.h>

/* Modules Library */
#include "main.h"
#include "modules/include/motor.h"
#include "modules/include/camera.h"
#include "modules/include/brain.h"
#include "modules/include/inertial.h"
#include "modules/include/telemetry.h"

/* e-puck2 main processor Library */
#include <motors.h>


static float prev_error = 0.0f;

//simple PID regulator implementation
int16_t pid_regulator(float distance, float goal){
    float error = distance - goal;
	static float sum_error = 0.0f;
    static float filtered_derivative = 0.0f;

	//disables the PID regulator if the error is to small
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

	float raw_derivative = (error - prev_error) / MOT_DT;

    if (raw_derivative > MAX_D_ERROR)		raw_derivative = MAX_D_ERROR;
    else if (raw_derivative < -MAX_D_ERROR) raw_derivative = -MAX_D_ERROR;

    filtered_derivative = LOW_PASS_FACTOR * filtered_derivative + (1.0f - LOW_PASS_FACTOR) * raw_derivative;
	if (get_state() != MISSION) sum_error = 0.0f;
    float speed = KP * error + KI * sum_error + KD * filtered_derivative;
    prev_error = error;

	// epuck_printf("[PID] line position = %u,\t error = %f,\t sign = %d\n", (uint16_t)distance, error, error > 0 ? 1 : -1);

    return (int16_t) speed;
}

static THD_WORKING_AREA(waPidRegulator, PID_STACK_SIZE);
static THD_FUNCTION(PidRegulator, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    systime_t time;

    int16_t speed = 0;
    int16_t speed_correction = 0;

	messagebus_topic_t* line_topic = messagebus_find_topic_blocking(&bus, "/line");
    line_msg_t line_values;

    while(1){
        //time = chVTGetSystemTime();

        messagebus_topic_wait(line_topic, &line_values, sizeof(line_msg_t));

		if (get_state() == MISSION) {
			//computes the speed to give to the motors
			//distance_cm is modified by the image processing thread
			//speed = pid_regulator(get_distance_cm(), GOAL_DISTANCE);
			speed = FWD_SPEED;
			//computes a correction factor to let the robot rotate to be in front of the line
			speed_correction = pid_regulator(line_values.position, (IMAGE_BUFFER_SIZE/2));

			//if the line is nearly in front of the camera, don't rotate
			if(abs(speed_correction) < ROTATION_THRESHOLD){
				speed_correction = 0;
			}

			right_motor_set_speed(speed - speed_correction);
			left_motor_set_speed(speed + speed_correction);
		
		} 
		//100Hz
		//chThdSleepUntilWindowed(time, time + MS2ST(PID_LOOP_MS));
    }
}

void advance(void) {
	float target_steps_r = (STEPS_ONE_TURN/WHEEL_PERIMETER)*FWD_DISP;
	float target_steps_l = (STEPS_ONE_TURN/WHEEL_PERIMETER)*FWD_DISP;
	
	float current_steps_r = 0;
	float current_steps_l = 0;

	bool right_position_reached = 0;
	bool left_position_reached = 0;

	left_motor_set_pos(0);
	right_motor_set_pos(0);

	left_motor_set_speed(FWD_SPEED);
	right_motor_set_speed(FWD_SPEED);

	while (true) {

		current_steps_r = fabs(right_motor_get_pos());
		current_steps_l = fabs(left_motor_get_pos());

		if (current_steps_r > target_steps_r) {
			right_motor_set_speed(0);
			right_position_reached = true;
		}

		if (current_steps_l > target_steps_l) {
			left_motor_set_speed(0);
			left_position_reached = true;
		}
		if (right_position_reached && left_position_reached) break;
	}	
}

void stop_motors(void){
	right_motor_set_speed(0);
	left_motor_set_speed(0);
}

//implement thread to rotate using filtered gyro yaw (should take desired heading as input and return true when completed)
void rotate_absolute(float target_heading) {
	messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_yaw");
    yaw_msg_t angle;
	float error = 0;

	while (true) {
		messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
		error = angle.yaw_rad - target_heading;

		if (fabsf(error) < ERROR_ANGLE) break;
		while (error >= M_PI) error -= 2.0f * M_PI;
		while (error < -M_PI) error += 2.0f * M_PI;

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
	yaw_msg_t initial_angle;
	messagebus_topic_wait(imu_topic, &initial_angle, sizeof(yaw_msg_t));

	rotate_absolute(relative_angle + initial_angle.yaw_rad);
}

// void rotate_relative(float relative_angle) {
// 	messagebus_topic_t* imu_topic = messagebus_find_topic_blocking(&bus, "/imu_yaw");
//     yaw_msg_t angle;
// 	float error = 0;

// 	messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
// 	float initial_yaw = angle.yaw_rad;

// 	float target_heading = initial_yaw + relative_angle;

// 	while (target_heading >= 2.0f * M_PI)	target_heading -= 2.0f * M_PI;
// 	while (target_heading < 0.0f)			target_heading += 2.0f * M_PI;

// 	while (true) {
// 		messagebus_topic_wait(imu_topic, &angle, sizeof(yaw_msg_t));
// 		error = angle.yaw_rad - target_heading;

// 		while (error >= M_PI) error -= 2.0f * M_PI;
// 		while (error < -M_PI) error += 2.0f * M_PI;

// 		if (fabsf(error) < ERROR_ANGLE) break;

// 		if (error >= 0) {
// 			right_motor_set_speed(-ROT_SPEED);
// 			left_motor_set_speed(ROT_SPEED);
// 		} else {
// 			right_motor_set_speed(ROT_SPEED);
// 			left_motor_set_speed(-ROT_SPEED);
// 		}
// 	}
// 	right_motor_set_speed(FWD_SPEED);
// 	left_motor_set_speed(FWD_SPEED);
// }

static void pid_regulator_start(void) {
	chThdCreateStatic(waPidRegulator, sizeof(waPidRegulator), NORMALPRIO, PidRegulator, NULL);
}

void motor_init(){
	motors_init();
	pid_regulator_start();
}

bool get_last_error_direction(void) {
	return prev_error > 0;
}
