#ifndef __BRAIN_H__
#define __BRAIN_H__


typedef enum {
    READING = 0,
    MISSION, 
    INTERMEDIATE,
    STOP,
    DONE
} State;

void brain_init(void);
State get_state(void);

#endif /* __BRAIN_H__ */