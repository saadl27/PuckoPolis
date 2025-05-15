#ifndef CAMERA_H
#define CAMERA_H

/* Constants */
#include "modules/include/constants.h"

/* List of detection color */
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

typedef struct {
	uint16_t position;
} line_msg_t;

void camera_init(void);

#endif