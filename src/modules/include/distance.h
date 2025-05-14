#ifndef DISTANCE_H
#define DISTANCE_H

/* C Standard Library */
#include <stdint.h>

/* Constants */
#include "modules/include/constants.h"


typedef struct {
    uint16_t dist_mm;
} tof_msg_t;

void tof_init(void);


#endif /* DISTANCE_H */