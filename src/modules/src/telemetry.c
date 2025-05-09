#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <ch.h>
#include <hal.h>
#include <memory_protection.h>
#include <usbcfg.h>
#include <chprintf.h>

// In order to be able to use the RGB LEDs and User button
// These funtcions are handled by the ESP32 and the communication with the uC is done via SPI
#include <spi_comm.h>

/*
	C standard lib-like printf helper wrapper around chprintf 
	to avoid having to provide stream
*/

void epuck_printf(const char *fmt, ...) {
    va_list args;
    va_start(args, fmt);
    chvprintf((BaseSequentialStream *)&SDU1, fmt, args);
    va_end(args);
}

void SendUint8ToComputer(uint8_t* data, uint16_t size) 
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"START", 5);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint16_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
}

/* void SendNodeToComputer(uint8_t* data, uint8_t size) 
{
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)"CURRENT_NODE:", 13);
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)&size, sizeof(uint8_t));
	chSequentialStreamWrite((BaseSequentialStream *)&SD3, (uint8_t*)data, size);
} */

void SendNodeToComputer(uint8_t node) {
    chprintf((BaseSequentialStream *)&SD3, "CURRENT_NODE:%u\n", node);
}

uint8_t ReceiveDestinationFromComputer(void) {
    static const char prefix[] = "DEST:";
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
            // Partial match reset: if this char could be the start of prefix, keep it
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
    return (uint8_t)atoi(numbuf);
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
    //starts the serial communication
    serial_start();
    //start the USB communication
    usb_start();

    //starts RGB LEDS and User button managment
	spi_comm_start();
}