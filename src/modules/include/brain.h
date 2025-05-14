#ifndef __BRAIN_H__
#define __BRAIN_H__

#include "modules/include/constants.h"


typedef enum {
    IDLE = 0,
    READING,
    MISSION, 
    INTERMEDIATE,
    RECALCULATING_PATH,
    STOP,
    DONE
} State;

void brain_init(void);
State get_state(void);
void reset(void);

#endif /* __BRAIN_H__ */