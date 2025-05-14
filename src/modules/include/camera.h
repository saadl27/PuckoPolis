#ifndef CAMERA_H
#define CAMERA_H

#include "modules/include/constants.h"

//List of detection color
typedef enum {
	RED_COLOR,
	GREEN_COLOR,
	BLUE_COLOR,
	BLACK_COLOR,
	WHITE_COLOR
} color_detection_t;


typedef struct {
	color_detection_t color;
} color_msg_t;


uint16_t get_line_position(void);

void camera_init(void);

#endif