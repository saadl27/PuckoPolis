#ifndef MOTOR_H
#define MOTOR_H

#define ROT_SPEED               250
#define FWD_SPEED               200
#define ROTATION_THRESHOLD		10
#define ERROR_THRESHOLD			0.1f	//[cm] because of the noise of the camera
#define KP						1.0f
#define KI 						0.001f	//must not be zero
#define MAX_SUM_ERROR 			(MOTOR_SPEED_LIMIT/KI)
#define ERROR_ANGLE             1       //degree
#define KP_ROT                  1

void motor_init(void);
void rotate_ccw(void);
void rotate_cw(void);
void correct_heading(uint16_t target);
void stop_motors(void);

#endif 