#ifndef DISTANCE_H
#define DISTANCE_H

#include <stdint.h>

void tof_init(void);
uint16_t tof_get_dist_mm(void);

// testing distance filters
void tof_filter_init(void);
void tof_set_kalman_params(float p_noise, float m_noise, float est_err);
uint16_t tof_get_filtered_dist_mm(void);

#endif /* DISTANCE_H */