#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "brain.h"
#include "main.h"
#include "telemetry.h"
#include "camera.h"

static THD_WORKING_AREA(waColorDecision, 4096);
static THD_FUNCTION(ColorDecision, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t* color_topic = messagebus_find_topic_blocking(&bus, "/color");
    color_msg_t color_values;

    while (1) {
        messagebus_topic_wait(color_topic, &color_values, sizeof(color_msg_t));
        char* color_s = NULL;

        switch (color_values.color) {
            case RED_COLOR: color_s = "red"; break;
            case GREEN_COLOR: color_s = "green"; break;
            case BLUE_COLOR: color_s = "blue"; break;
            case BLACK_COLOR: color_s = "black"; break;
        }

        if (color_s) epuck_printf("Color = %s\n", color_s);
        else epuck_printf("No color\n");
    }
}

void brain_init(void) {
    chThdCreateStatic(waColorDecision, sizeof(waColorDecision), NORMALPRIO, ColorDecision, NULL);
}