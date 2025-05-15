/* C Standard Library */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

/* ChibiOS Library */
#include <ch.h>
#include <hal.h>
#include <chprintf.h>

/* e-puck2 main processor Library */
#include <memory_protection.h>
#include <usbcfg.h>
#include <spi_comm.h>

/* Modules Library */
#include "modules/include/telemetry.h"
#include "modules/include/brain.h"


BSEMAPHORE_DECL(reset_sem, TRUE);

/*
	C standard lib-like printf helper wrapper around chprintf 
	to avoid having to provide stream.
    Only used for debugging
*/
void epuck_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    chvprintf((BaseSequentialStream *)&SDU1, fmt, args);
    va_end(args);
}

void SendNodeToComputer(uint8_t node) {
    chprintf((BaseSequentialStream *)&SD3, "CURRENT_NODE:%u\n", node);
}

static uint16_t ReceiveFromComputer(const char* mess) {
    const char* prefix = mess;
    const size_t prefix_len = sizeof(prefix) - 1;
    size_t match_idx = 0;
    char c;

    BaseSequentialStream *bss = (BaseSequentialStream *)&SD3;

    // Sync on the prefix "DEST:"
    while (true) {
        chSequentialStreamRead(bss, (uint8_t *)&c, 1);
        if (c == prefix[match_idx]) {
            match_idx++;
            if (match_idx == prefix_len) {
                // Full prefix matched
                break;
            }
        }
        else {
            // Partial match reset: if this char could be the start of prefix
            match_idx = (c == prefix[0]) ? 1 : 0;
        }
    }

    // Read ASCII digits until newline (or buffer full)
    char numbuf[4];  // enough for 0–255 plus NUL
    size_t num_idx = 0;
    while (true) {
        chSequentialStreamRead(bss, (uint8_t *)&c, 1);
        if (c == '\n' || c == '\r' || num_idx >= sizeof(numbuf) - 1) {
            break;
        }
        if (c >= '0' && c <= '9') {
            numbuf[num_idx++] = c;
        }
        // else ignore any stray characters
    }
    numbuf[num_idx] = '\0';

    // Convert to integer and return
    return (uint16_t)atoi(numbuf);
}

uint16_t ReceiveYawFromComputer(void) {
    const char* str = "YAW";
    return ReceiveFromComputer(str);
}

uint8_t ReceiveDestinationFromComputer(void) {
	const char* str = "DEST:";
	return ReceiveFromComputer(str);
}

uint8_t ReceiveStartFromComputer(void) {
	const char* str = "START:";
	return ReceiveFromComputer(str);
}

static THD_WORKING_AREA(waReceiveReset, RECEIVE_RESET_STACK_SIZE);
static THD_FUNCTION(ReceiveReset, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    const char* prefix = "RESET";
    const size_t prefix_len = sizeof(prefix) - 1;
    size_t match_idx = 0;
    char c;

    BaseSequentialStream *bss = (BaseSequentialStream *)&SD3;

    // Sync on the prefix "DEST:"
    while (true) {
        if (get_state() != READING){
            chSequentialStreamRead(bss, (uint8_t *)&c, 1);
            if (c == prefix[match_idx]) {
                match_idx++;
                if (match_idx == prefix_len) {
                    chBSemSignal(&reset_sem);
                    // Full prefix matched
                }
            }
            else {
                // Partial match reset: if this char could be the start of prefix, keep it
                match_idx = (c == prefix[0]) ? 1 : 0;
            }
        }
        chThdSleepMilliseconds(100);
    }
}

static void serial_start(void)
{
	static SerialConfig ser_cfg = {
	    115200,
	    0,
	    0,
	    0,
	};

	sdStart(&SD3, &ser_cfg); // UART3.
}

void telemetry_init(void)
{
    serial_start();
    usb_start();
    // RGB LEDS and User button management
	spi_comm_start();
    chThdCreateStatic(waReceiveReset, sizeof(waReceiveReset), NORMALPRIO, ReceiveReset, NULL);
}