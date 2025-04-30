#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdint.h>

void tof_init(void);
uint16_t tof_get_dist_mm(void);

#endif /* DISTANCE_H */