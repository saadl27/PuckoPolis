#ifndef TIMER_H
#define TIMER_H

#define TIMER_CLOCK         84000000    // APB1 clock
#define PRESCALER_TIM7      8400        // timer frequency: 10kHz
#define COUNTER_MAX_TIM7    10000       // timer max counter -> 1Hz

#define PRESCALER_TIM4		8400		// frequency: 10k
#define COUNTER_MAX_TIM4	84			// max counter -> 1

void timer7_start(void);
void timer4_start(float duty);


#endif /* TIMER_H */
