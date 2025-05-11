#include <ch.h>
#include <hal.h>
#include <chprintf.h>
#include <stdbool.h>
#include <math.h>

#include "modules/include/brain.h"
#include "modules/include/telemetry.h"
#include "modules/include/camera.h"
#include "modules/include/motor.h"
#include "modules/include/a_star.h"
#include "main.h"

static uint8_t state = READING;

bool get_state() {
    return state;
}

static THD_WORKING_AREA(waFSM, 4096);
static THD_FUNCTION(FSM, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    Graph* graph = (Graph*) malloc(sizeof(Graph));
    a_star_init(graph);

    Path* path = (Path*) malloc(sizeof(Path));
    uint8_t path_step = 0;

    messagebus_topic_t* color_topic = messagebus_find_topic_blocking(&bus, "/color");
    color_msg_t color_values;

    while (state == READING){
        uint8_t start = ReceiveStartFromComputer();
        uint8_t end = ReceiveDestinationFromComputer();

        if (a_star_find_path(graph, path, start, end)){
            state = MISSION;
            SendNodeToComputer(start);
        }
    }

    while (state == MISSION) {
        messagebus_topic_wait(color_topic, &color_values, sizeof(color_msg_t));
        
        switch (color_values.color) {
            case RED_COLOR:
                //epuck_printf("color = red\n");
                state = STOP;
                stop_motors();
                break;

            case GREEN_COLOR: 
                //epuck_printf("color = green\n");
                if (state == STOP){
                    state = MISSION;
                }
                break;

            case BLUE_COLOR: 
                //epuck_printf("color = blue\n");
                state = INTERMEDIATE;
                float target_heading = (get_heading(graph, path->path[path_step], path->path[path_step+1]))*(M_PI/4);
                correct_heading(target_heading);
                state = MISSION; 
                ++path_step;
                SendNodeToComputer(path->path[path_step]);
                if (path_step == path->path_len){
                    state = DONE;
                }
                //rotate_cw(); 
                break;

            case BLACK_COLOR: 
                //epuck_printf("color = black\n");
                break;
        }
    }
}

void brain_init(void) {
    chThdCreateStatic(waFSM, sizeof(waFSM), NORMALPRIO, FSM, NULL);
}