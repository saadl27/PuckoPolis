#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <stdbool.h>

#include "modules/include/brain.h"
#include "modules/include/telemetry.h"
#include "modules/include/camera.h"
#include "modules/include/motor.h"

static bool moving = true;

bool get_moving() {
    return moving;
}

static THD_WORKING_AREA(waColorDecision, 4096);
static THD_FUNCTION(ColorDecision, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t* color_topic = messagebus_find_topic_blocking(&bus, "/color");
    color_msg_t color_values;

    while (1) {
        messagebus_topic_wait(color_topic, &color_values, sizeof(color_msg_t));
        
        switch (color_values.color) {
            case RED_COLOR: 
                moving = true;
                break;

            case GREEN_COLOR: 
                moving = true; 
                break;

            case BLUE_COLOR: 
                moving = false;
                //rotate_cw(); 
                break;

            case BLACK_COLOR: 
                moving = true; 
                break;
        }
    }
}

void brain_init(void) {
    chThdCreateStatic(waColorDecision, sizeof(waColorDecision), NORMALPRIO, ColorDecision, NULL);
}