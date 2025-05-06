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

    Graph graph = {0};

    a_star_init_graph(&graph, 5);
    a_star_add_edge(&graph, 1, 2, 1);
    a_star_add_edge(&graph, 1, 3, 3);
    a_star_add_edge(&graph, 3, 4, 1);
    a_star_add_edge(&graph, 4, 5, 2);
    a_star_add_edge(&graph, 2, 5, 4);

    /* Infinite loop. */
    while (1) {
        Path path = {0};

        test_path(&graph, &path, 1, 5);
        test_path(&graph, &path, 1, 4);
        
        epuck_printf("Hello world\n");
        chThdSleepMilliseconds(LOOP_PERIOD_MS);
    }
}

#define STACK_CHK_GUARD 0xe2dee396
uintptr_t __stack_chk_guard = STACK_CHK_GUARD;

void __stack_chk_fail(void)
{
    chSysHalt("Stack smashing detected");
}