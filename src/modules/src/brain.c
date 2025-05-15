/* C Standard Library */
#include <stdbool.h>
#include <math.h>

/* ChibiOS Library */
#include <ch.h>
#include <hal.h>

/* Modules Library */
#include "main.h"
#include "modules/include/brain.h"
#include "modules/include/telemetry.h"
#include "modules/include/camera.h"
#include "modules/include/motor.h"
#include "modules/include/a_star.h"
#include "modules/include/distance.h"
#include "modules/include/inertial.h"
#include "modules/include/ledstates.h"


static State state = READING;

State get_state() {
    return state;
}

static THD_WORKING_AREA(waFSM, FSM_STACK_SIZE);
static THD_FUNCTION(FSM, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    Graph* graph = (Graph*) malloc(sizeof(Graph));
    a_star_init(graph);

    Path* path = (Path*) malloc(sizeof(Path));
    int8_t path_step = 0;
    uint8_t start = 0, end = 0;

    messagebus_topic_t* color_topic = messagebus_find_topic_blocking(&bus, "/color");
    color_msg_t color_values;

    while (true) {
        messagebus_topic_wait(color_topic, &color_values, sizeof(color_msg_t));
        switch (state) {
        case READING:
            leds_reading();
            path_step = 0;
            start = ReceiveStartFromComputer();
            end = ReceiveDestinationFromComputer();
            uint16_t init_yaw = ReceiveYawFromComputer();
            set_init_yaw((float) init_yaw/RAD2DEG);

            if (a_star_find_path(graph, path, start, end)) {
                SendNodeToComputer(start);
                float target_heading = (float) get_heading(graph, path->path[0],
                                                path->path[1]) * M_PI_4;
                rotate_absolute(target_heading);
                state = MISSION;
            }
            break;
    
        case RECALCULATING_PATH: 
            a_star_set_edge_freeness(graph, path->path[path_step], path->path[path_step + 1], false);

            start = path->path[path_step];
            path_step = 0;
            
            if (a_star_find_path(graph, path, start, end)) {
                state = MISSION;
                path_step--;
            } else {
                state = READING;
            }
            break;
    
        case MISSION: 
            leds_mission();
            switch (color_values.color) {
                case RED_COLOR:
                    state = STOP;
                    stop_motors();
                    break;

                case BLUE_COLOR:
                    state = INTERSECTION;
                    ++path_step;
                    if (path_step == path->path_len - 1) {
                        advance();
                        state = DONE;
                        stop_motors();
                        SendNodeToComputer(path->path[path->path_len - 1]);
                    } else {
                        advance();
                        float target_heading = (float) get_heading(graph, path->path[path_step],
                                                path->path[path_step+1]) * M_PI_4;
                        rotate_absolute(target_heading);
                        SendNodeToComputer(path->path[path_step]);
                    }
                    break;

                case WHITE_COLOR:
                    rotate_relative(get_last_error_direction() ? -ROT_CORRECTION : ROT_CORRECTION);
                    break;
                
                default: break;
            }
            break;

        case INTERSECTION:
            leds_intersection();
            switch (color_values.color) {
                case BLACK_COLOR: {
                    state = MISSION;
                    break;
                }
                default: break;
            }
            break;

        case DONE:
            leds_done();
            stop_motors();
            break;

        case STOP:
            leds_stop();
            switch (color_values.color) {
                case GREEN_COLOR:
                    state = MISSION;
                    break;

                default: break;
            }
            break;

        default: break;
        }
    }
}


static THD_WORKING_AREA(waReset, RST_STACK_SIZE);
static THD_FUNCTION(Reset, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    while (true){
        chBSemWait(&reset_sem);
        stop_motors();
        state = READING;
    }
}

static THD_WORKING_AREA(waDetectObstacle, OBS_STACK_SIZE);
static THD_FUNCTION(DetectObstacle, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;
    
    messagebus_topic_t* dist_topic = messagebus_find_topic_blocking(&bus, "/distance");
    tof_msg_t tof_dist;

    systime_t time;

    while (true) {
        time = chVTGetSystemTime();

        messagebus_topic_wait(dist_topic, &tof_dist, sizeof(tof_msg_t));

        if (tof_dist.dist_mm < OBSTACLE_THRESHOLD_MM && state == MISSION) {
            state = CHANGING_PATH;
            leds_obstacle();
            rotate_relative(M_PI);
            state = RECALCULATING_PATH;
        }

        chThdSleepUntilWindowed(time, time + MS2ST(TOF_THD_PERIOD_MS));
    }
}

void brain_init(void) {
    chThdCreateStatic(waFSM, sizeof(waFSM), NORMALPRIO, FSM, NULL);
    chThdCreateStatic(waReset, sizeof(waReset), NORMALPRIO, Reset, NULL);
    chThdCreateStatic(waDetectObstacle, sizeof(waDetectObstacle), NORMALPRIO, DetectObstacle, NULL);
}
