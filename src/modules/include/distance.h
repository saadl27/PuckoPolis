#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdint.h>

void tof_init(void);
int16_t tof_get_dist_mm(void);

// testing distance filters
uint16_t tof_get_filtered_dist_mm(void);

#endif /* DISTANCE_H */