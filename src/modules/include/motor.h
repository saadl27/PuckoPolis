#ifndef MOTOR_H
#define MOTOR_H

#include "modules/include/constants.h"

void motor_init(void);
void correct_heading(float target);
void rotate_relative(float relative_angle);
void advance(void);
void stop_motors(void);

#endif 