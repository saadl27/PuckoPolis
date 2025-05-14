#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdint.h>
#include "modules/include/constants.h"


typedef struct {
    uint16_t dist_mm;
} tof_msg_t;

void tof_init(void);
int16_t tof_get_dist_mm(void);

// testing distance filters
uint16_t tof_get_filtered_dist_mm(void);

#endif /* DISTANCE_H */