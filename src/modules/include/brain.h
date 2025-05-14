#ifndef __BRAIN_H__
#define __BRAIN_H__

/* Constants */
#include "modules/include/constants.h"


//FSM states
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


#endif /* __BRAIN_H__ */