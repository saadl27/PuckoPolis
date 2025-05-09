#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "ch.h"
#include "hal.h"
#include "memory_protection.h"
#include "i2c_bus.h"
#include "main.h"

#include "modules/include/telemetry.h"
#include "modules/include/distance.h"
#include "modules/include/a_star.h"
#include "modules/include/brain.h"
#include "modules/include/motor.h"
#include "modules/include/camera.h"

/* temporary */
#define LOOP_PERIOD_MS 100

messagebus_t bus;
MUTEX_DECL(bus_lock);
CONDVAR_DECL(bus_condvar);

int main(void) {
    halInit();
    chSysInit();
    mpu_init();
    messagebus_init(&bus, &bus_lock, &bus_condvar);

	telemetry_init();
    camera_init();
	motor_init();
    brain_init();

    Graph* graph = (Graph*) malloc(sizeof(Graph));

    a_star_init_graph(graph, 15);

    a_star_add_edge(graph, 1,  5,  120);
    a_star_add_edge(graph, 1,  3,  90);
    a_star_add_edge(graph, 1,  2,  7);
    a_star_add_edge(graph, 2,  6,  60);
    a_star_add_edge(graph, 2,  4,  35);
    a_star_add_edge(graph, 4,  10, 95);
    a_star_add_edge(graph, 4,  9,  120);
    a_star_add_edge(graph, 5,  6,  15);
    a_star_add_edge(graph, 5,  7,  8);
    a_star_add_edge(graph, 7,  8,  30);
    a_star_add_edge(graph, 6,  8,  17);
    a_star_add_edge(graph, 7,  13, 107);
    a_star_add_edge(graph, 8,  9,  76);
    a_star_add_edge(graph, 11, 13, 63);
    a_star_add_edge(graph, 13, 14, 66);
    a_star_add_edge(graph, 14, 15, 36);
    a_star_add_edge(graph, 9,  10, 20);
    a_star_add_edge(graph, 10, 12, 47);
    a_star_add_edge(graph, 3,  12, 90);
    a_star_add_edge(graph, 12, 15, 25);
    a_star_add_edge(graph, 3,  15, 235);
    a_star_add_edge(graph, 11, 14, 15);
    a_star_add_edge(graph, 9,  11, 9);

    epuck_printf("hello world\n");


    /* Infinite loop. */
    while (1) {
        Path* path = (Path*) malloc(sizeof(Path));

        // test_path(graph, path, 1, 3);
        // test_path(graph, path, 4, 10);
        // test_path(graph, path, 12, 15);
        // test_path(graph, path, 7, 5);
        // test_path(graph, path, 14, 15);
        // test_path(graph, path, 1, 15);
        epuck_printf("==========================================\n");
        test_path(graph, path, 1, 6);
        test_path(graph, path, 4, 11);
        test_path(graph, path, 10, 13);
        test_path(graph, path, 3, 13);


        free(path);
        
        chThdSleepMilliseconds(LOOP_PERIOD_MS);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}