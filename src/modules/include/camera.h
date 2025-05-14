#ifndef CAMERA_H
#define CAMERA_H

// Specify the 2 consecutive lines used for tracking the black line
// The line number starts from 0 and ending to PO8030_MAX_HEIGHT - 1. Consult camera/po8030.h
// But as 2 lines will be used, the value of the first line can be higher than PO8030_MAX_HEIGHT - 2
#define USED_LINE 400   // Must be inside [0..478], according to the above explanations


#define IMAGE_BUFFER_SIZE		640
#define WIDTH_SLOPE				5
#define MIN_LINE_WIDTH			40
#define MAX_DISTANCE 			25.0f
#define PXTOCM					1570.0f //experimental value

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