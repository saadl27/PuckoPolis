#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <motors.h>

#include "main.h"
#include "pi_regulator.h"
#include "camera.h"
#include "telemetry.h"

int main(void)
{
    halInit();
    chSysInit();
    mpu_init();

	telemetry_init();
	camera_init();

	//inits the motors
	motors_init();

	//enable motors by default. Can be chnaged from plotImage Python code
	set_enabled_motors(true);

	//stars the threads for the pi regulator and the processing of the image
	pi_regulator_start();
	process_image_start();

    /* Infinite loop: used here for handling control commands sent from plotImage Python code */
    while (1) {
		// read control commands
        volatile uint8_t ctrl_cmd = chSequentialStreamGet((BaseSequentialStream *) &SD3);
		switch (ctrl_cmd) {
			case 'R':
			case 'r':
				select_color_detection(RED_COLOR);
				break;
			case 'G':
			case 'g':
				select_color_detection(GREEN_COLOR);
				break;
			case 'B':
			case 'b':
				select_color_detection(BLUE_COLOR);
				break;
			case 'M':
			case 'm':
				toogle_enabled_motors();
				break;
		}
    	//waits 0.2 second
		chThdSleepMilliseconds(200);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}
