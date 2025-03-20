#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <usbcfg.h>
#include <camera/po8030.h>

#include "process_image.h"
#include "main.h"

#define EXTRACT_RED(buf, element)   (*((buf) + 2*(element)) & 0b11111000) >> 3
#define EXTRACT_GREEN(buf, element) (*((buf) + 2*(element)) & 0b00000111) + ((*((buf) + 2*(element) + 1) & 0b11100000) >> 5)
#define EXTRACT_BLUE(buf, element)  (*((buf) + 2*(element) + 1) & 0b00011111)

#define CALIB_CONSTANT 10 * 150 // cm * px
#define EPSILON 0

static float distance_cm = 0;

//semaphore
static BSEMAPHORE_DECL(image_ready_sem, TRUE);

static THD_WORKING_AREA(waCaptureImage, 256);
static THD_FUNCTION(CaptureImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	//Takes pixels 0 to IMAGE_BUFFER_SIZE of the lines USED_LINE and USED_LINE + 1 (minimum 2 lines because reasons)
	po8030_advanced_config(FORMAT_RGB565, 0, USED_LINE, IMAGE_BUFFER_SIZE, 2, SUBSAMPLING_X1, SUBSAMPLING_X1);
	dcmi_enable_double_buffering();
	dcmi_set_capture_mode(CAPTURE_ONE_SHOT);
	dcmi_prepare();

    while(1){
        //starts a capture
		// systime_t time = chVTGetSystemTime();
		dcmi_capture_start();
		//waits for the capture to be done
		wait_image_ready();
		// time = chVTGetSystemTime() - time;
		//signals an image has been captured
		chBSemSignal(&image_ready_sem);
		// chprintf((BaseSequentialStream *) &SDU1, "time = %d\n", time);
		// chThdSleepMilliseconds(12);
    }
}


static THD_WORKING_AREA(waProcessImage, 1024);
static THD_FUNCTION(ProcessImage, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

	uint8_t *img_buff_ptr;
	uint8_t image[IMAGE_BUFFER_SIZE] = {0};

    while(1){
    	//waits until an image has been captured
        chBSemWait(&image_ready_sem);
		//gets the pointer to the array filled with the last image in RGB565    
		img_buff_ptr = dcmi_get_last_image_ptr();
		float mean = 0;

		/*
		*	To complete
		*/

		for (uint16_t i = 0; i < IMAGE_BUFFER_SIZE; ++i) {
			image[i] = (*(img_buff_ptr + 2*i) & 0b11111000) >> 3;
			mean += (float) image[i];
		}
		mean /= IMAGE_BUFFER_SIZE;
		chprintf((BaseSequentialStream*) &SDU1, "mean = %f\n", mean);

		float eps = mean / 4;

		uint8_t minmin = 0;
		uint8_t maxmax = 0;

		bool found_min = false;

		for (size_t i = 0; i < IMAGE_BUFFER_SIZE; ++i) {
			if (!found_min) {
				if (image[i] < mean - eps) {
					minmin = i;
					found_min = true;
				}
			} if (found_min) {
				if (image[i] > mean + eps) {
					maxmax = i;
					break;
				}
			}
		}

		float dist = (minmin == maxmax ? 0 : (float) 1.0 / (maxmax - minmin));
		chprintf((BaseSequentialStream*) &SDU1, "min = %d, max = %d\n", minmin, maxmax);
		chprintf((BaseSequentialStream*) &SDU1, "image[min] = %d, image[max] = %d\n",
				image[minmin], image[maxmax]);
		chprintf((BaseSequentialStream*) &SDU1, "distance = %f\n", dist);

		SendUint8ToComputer(image, IMAGE_BUFFER_SIZE);
    }
}

float get_distance_cm(void){
	return distance_cm;
}

void process_image_start(void){
	chThdCreateStatic(waProcessImage, sizeof(waProcessImage), NORMALPRIO, ProcessImage, NULL);
	chThdCreateStatic(waCaptureImage, sizeof(waCaptureImage), NORMALPRIO, CaptureImage, NULL);
}