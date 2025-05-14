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
#include "modules/include/distance.h"
#include "modules/include/inertial.h"
#include "modules/include/ledstates.h"
#include "main.h"

#define FSM_THD_LOOP_MS         100
#define FSM_STACK_SIZE          8192
#define RST_STACK_SIZE          256
#define OBS_STACK_SIZE          1024

#define OBSTACLE_THRESHOLD_MM ((uint16_t) 10)

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

    systime_t time;

    while (true) {
        time = chVTGetSystemTime();
        //epuck_printf("[state] %d\n", state);

        messagebus_topic_wait(color_topic, &color_values, sizeof(color_msg_t));

        // switch (state) {
        // case IDLE: break;
        // case READING:
        //     start = ReceiveStartFromComputer();
        //     end = ReceiveDestinationFromComputer();

        //     if (a_star_find_path(graph, path, start, end)) {
        //         state = MISSION;
        //         set_init_yaw(get_heading(graph, path->path[0], path->path[1]) * M_PI_4);
        //         SendNodeToComputer(start);
        //         test_path(graph, path, start, end);
        //     }

        // case RECALCULATING_PATH:
        //     a_star_set_edge_freeness(graph, path->path[path_step], path->path[path_step + 1], false);
        //     // test_path(graph, path, start, end);

        //     start = path->path[path_step];
        //     path_step = 0;

        //     epuck_printf("[recalculating path] start = %u, path step = %u, end = %u\n", start, path_step, end);
            
        //     if (a_star_find_path(graph, path, start, end)) {
        //         state = MISSION;
        //         epuck_printf("[path] start = %u, end = %u, path len = %u, path cost = %u, path = \n",
        //         path->start, path->end, path->path_len, path->path_cost);
        //         for (int i = 0; i < 15; ++i) {
        //             epuck_printf("%u ", path->path[i]);
        //         }
        //         epuck_printf("\n");
        //         path_step--;
        //         // SendNodeToComputer(start);
        //     } else {
        //         epuck_printf("No new path found\n");
        //         state = READING;
        //     }

        // case MISSION:
        //     switch (color_values.color) {
        //         case RED_COLOR:
        //             // epuck_printf("color = red\n");
        //             // state = STOP;
        //             // stop_motors();
        //             break;

        //         case GREEN_COLOR:
        //             // epuck_printf("color = green\n");
        //             break;

        //         case BLUE_COLOR:
        //             //epuck_printf("color = blue\n");
        //             state = INTERMEDIATE;
        //             ++path_step;
        //             epuck_printf("=============================================\nPATH STEP = %d | PATH LEN = %d\n", path_step, path->path_len);
        //             if (path_step == path->path_len - 1) {
        //                 translate();
        //                 state = DONE;
        //                 stop_motors();
        //                 SendNodeToComputer(path->path[path->path_len - 1]);
        //             } else {
        //                 float target_heading = (float) get_heading(graph, path->path[path_step],
        //                                         path->path[path_step+1]) * M_PI_4;
        //                 epuck_printf("[brain] heading = %f\nbefore loop\n", target_heading);
        //                 correct_heading(target_heading);
        //                 epuck_printf("[brain] AFTER loop\n");
        //                 // state = MISSION;
        //                 SendNodeToComputer(path->path[path_step]);
        //             }
        //             break;

        //         case BLACK_COLOR:
        //             //epuck_printf("color = black\n");
        //             break;
        //     }

        // case INTERMEDIATE:
        //     switch (color_values.color) {
        //         case BLACK_COLOR: {
        //             state = MISSION;
        //             break;
        //         }
        //         default: break;
        //     }

        // case DONE: stop_motors();
        // case STOP:
        //     switch (color_values.color) {
        //         case RED_COLOR:
        //             break;

        //         case GREEN_COLOR:
        //             state = MISSION;
        //             break;

        //         case BLUE_COLOR:
        //             state = INTERMEDIATE;
        //             ++path_step;
        //             epuck_printf("=============================================\nPATH STEP = %d | PATH LEN = %d\n", path_step, path->path_len);
        //             if (path_step == path->path_len - 1) {
        //                 state = DONE;
        //                 stop_motors();
        //                 SendNodeToComputer(path->path[path->path_len - 1]);
        //             } else {
        //                 float target_heading = (float) get_heading(graph, path->path[path_step],
        //                                         path->path[path_step+1]) * M_PI_4;
        //                 epuck_printf("[brain] heading = %f\nbefore loop\n", target_heading);
        //                 correct_heading(target_heading);
        //                 epuck_printf("[brain] AFTER loop\n");
        //                 SendNodeToComputer(path->path[path_step]);
        //             }
        //             break;

        //         case BLACK_COLOR:
        //             state = MISSION;
        //             break;
        //     }
        // }

        if (state == READING) {
            leds_reading();
            path_step = 0;
            path_step = 0;
            start = ReceiveStartFromComputer();
            end = ReceiveDestinationFromComputer();
            set_init_yaw(ReceiveYawFromComputer());

            if (a_star_find_path(graph, path, start, end)) {
                SendNodeToComputer(start);
                float target_heading = (float) get_heading(graph, path->path[0],
                                                path->path[1]) * M_PI_4;
                correct_heading(target_heading);
                // test_path(graph, path, start, end);
                state = MISSION;
            }
        }
        if (state == RECALCULATING_PATH) {
            a_star_set_edge_freeness(graph, path->path[path_step], path->path[path_step + 1], false);
            // test_path(graph, path, start, end);

            start = path->path[path_step];
            path_step = 0;

            //epuck_printf("[recalculating path] start = %u, path step = %u, end = %u\n", start, path_step, end);
            
            if (a_star_find_path(graph, path, start, end)) {
                state = MISSION;
                //epuck_printf("[path] start = %u, end = %u, path len = %u, path cost = %u, path = \n",
                //path->start, path->end, path->path_len, path->path_cost);
                for (int i = 0; i < 15; ++i) {
                    //epuck_printf("%u ", path->path[i]);
                }
                //epuck_printf("\n");
                path_step--;
                // SendNodeToComputer(start);
            } else {
                //epuck_printf("No new path found\n");
                state = READING;
            }
        }

        if (state == MISSION) {
            leds_mission();
            switch (color_values.color) {
                case RED_COLOR:
                    // epuck_printf("color = red\n");
                    // state = STOP;
                    // stop_motors();
                    break;

                case GREEN_COLOR:
                    // epuck_printf("color = green\n");
                    break;

                case BLUE_COLOR:
                    //epuck_printf("color = blue\n");
                    state = INTERMEDIATE;
                    ++path_step;
                    //epuck_printf("=============================================\nPATH STEP = %d | PATH LEN = %d\n", path_step, path->path_len);
                    if (path_step == path->path_len - 1) {
                        translate();
                        state = DONE;
                        stop_motors();
                        SendNodeToComputer(path->path[path->path_len - 1]);
                    } else {
                        float target_heading = (float) get_heading(graph, path->path[path_step],
                                                path->path[path_step+1]) * M_PI_4;
                        //epuck_printf("[brain] heading = %f\nbefore loop\n", target_heading);
                        translate();
                        correct_heading(target_heading);
                        //epuck_printf("[brain] AFTER loop\n");
                        // state = MISSION;
                        SendNodeToComputer(path->path[path_step]);
                    }
                    break;

                case BLACK_COLOR:
                    //epuck_printf("color = black\n");
                    break;
            }
        }

        if (state == INTERMEDIATE) {
            leds_intersection();
            switch (color_values.color) {
                case BLACK_COLOR: {
                    state = MISSION;
                    break;
                }
                default: break;
            }
        }

        if (state == DONE) {
            leds_done();
            stop_motors();
        }

        if (state == STOP) {
            leds_stop();
            switch (color_values.color) {
                case RED_COLOR:
                    break;

                case GREEN_COLOR:
                    state = MISSION;
                    break;

                case BLUE_COLOR:
                    state = INTERMEDIATE;
                    ++path_step;
                    //epuck_printf("=============================================\nPATH STEP = %d | PATH LEN = %d\n", path_step, path->path_len);
                    if (path_step == path->path_len - 1) {
                        state = DONE;
                        stop_motors();
                        SendNodeToComputer(path->path[path->path_len - 1]);
                    } else {
                        float target_heading = (float) get_heading(graph, path->path[path_step],
                                                path->path[path_step+1]) * M_PI_4;
                        //epuck_printf("[brain] heading = %f\nbefore loop\n", target_heading);
                        correct_heading(target_heading);
                        //epuck_printf("[brain] AFTER loop\n");
                        SendNodeToComputer(path->path[path_step]);
                    }
                    break;

                case BLACK_COLOR:
                    state = MISSION;
                    break;
            }
        }
        chThdSleepUntilWindowed(time, time + MS2ST(FSM_THD_LOOP_MS));
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
        // epuck_printf("distance = %u [mm]\n", tof_dist.dist_mm);

        if (tof_dist.dist_mm < OBSTACLE_THRESHOLD_MM && state == MISSION) {
            state = IDLE;
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