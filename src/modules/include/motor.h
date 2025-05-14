#ifndef MOTOR_H
#define MOTOR_H

#include "modules/include/constants.h"

void motor_init(void);
void correct_heading(float target);
void rotate_relative(float relative_angle);
void advance(void);
void stop_motors(void);
bool get_last_error_direction(void); // 1 if error > 0, 0 otherwise

#endif 