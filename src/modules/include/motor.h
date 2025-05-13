#ifndef MOTOR_H
#define MOTOR_H

#define ROT_SPEED               250
#define FWD_SPEED               250
#define ROTATION_THRESHOLD		10
#define ERROR_THRESHOLD			0.1f	//[cm] because of the noise of the camera
#define KP						1.0f
#define KI 						(0.001f * 0)
#define KD                      (0.1f * 0)
#define MAX_D_ERROR             100.0f
#define LOW_PASS_FACTOR         0.8f
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define ERROR_ANGLE             0.05f    // rad (roughly +/- 3 deg)

void motor_init(void);
void rotate_ccw(void);
void rotate_cw(void);
void correct_heading(float target);
void rotate_relative(float relative_angle);
void rotate_relative(float relative_angle);
void stop_motors(void);

#endif 