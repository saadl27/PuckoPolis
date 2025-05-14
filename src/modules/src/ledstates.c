#include "modules/include/ledstates.h"
#include "modules/include/constants.h"
#include "leds.h"

void leds_reading(void) {
    clear_leds();
    set_body_led(1);
}

void leds_mission(void) {
    clear_leds();
    set_rgb_led(LED2, 0, 0, BLUE_INTENSITY); // blue
    set_rgb_led(LED8, 0, 0, BLUE_INTENSITY); // blue
}

void leds_intersection(void) {
    clear_leds();
    set_rgb_led(LED2, RED_INTENSITY, 0, 0); // red
    set_rgb_led(LED4, 0, 0, BLUE_INTENSITY); // blue
    set_rgb_led(LED6, RED_INTENSITY, 0, 0); // red
    set_rgb_led(LED8, 0, 0, BLUE_INTENSITY); // blue
}

void leds_stop(void) {
    clear_leds();
    set_rgb_led(LED2, RED_INTENSITY, 0, 0); // red
    set_rgb_led(LED4, RED_INTENSITY, 0, 0); // red
    set_rgb_led(LED6, RED_INTENSITY, 0, 0); // red
    set_rgb_led(LED8, RED_INTENSITY, 0, 0); // red
}

void leds_done(void) {
    clear_leds();
    set_rgb_led(LED2, 0, GREEN_INTENSITY, 0); // green
    set_rgb_led(LED4, 0, GREEN_INTENSITY, 0); // green
    set_rgb_led(LED6, 0, GREEN_INTENSITY, 0); // green
    set_rgb_led(LED8, 0, GREEN_INTENSITY, 0); // green
}

void leds_obstacle(void) {
    clear_leds();
    set_rgb_led(LED2, RED_INTENSITY, 0, BLUE_INTENSITY); // purple
    set_rgb_led(LED4, RED_INTENSITY, 0, BLUE_INTENSITY); // purple
    set_rgb_led(LED6, RED_INTENSITY, 0, BLUE_INTENSITY); // purple
    set_rgb_led(LED8, RED_INTENSITY, 0, BLUE_INTENSITY); // purple
}
