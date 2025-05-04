#include <ch.h>
#include <hal.h>
#include <chprintf.h>

#include "main.h"
#include "telemetry.h"
#include "camera.h"

static THD_WORKING_AREA(waColorDecision, 4096);
static THD_FUNCTION(ColorDecision, arg) {

    chRegSetThreadName(__FUNCTION__);
    (void)arg;

    messagebus_topic_t color_topic;
    color_msg_t color_values;

    MUTEX_DECL(color_topic_lock);
    CONDVAR_DECL(color_topic_condvar);
    messagebus_topic_init(&color_topic, &color_topic_lock, &color_topic_condvar, &color_values, sizeof(color_values));
    messagebus_advertise_topic(&bus, &color_topic, "/color");

    color_topic = messagebus_find_topic_blocking(&bus, "/color");

    while (1) {
        messagebus_topic_wait(messagebus_topic_t *topic, void *buf, size_t buf_len);
    }

}


void brain_init(void)
{   


}