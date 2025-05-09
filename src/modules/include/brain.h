#ifndef __BRAIN_H__
#define __BRAIN_H__


enum State {
    READING,
    MISSION, 
    INTERMEDIATE,
    SUCCESS
};

void brain_init(void);
bool get_state(void);

#endif /* __BRAIN_H__ */