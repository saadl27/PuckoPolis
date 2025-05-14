#ifndef TELEMETRY_H
#define TELEMETRY_H

/* C Standard Library */
#include <stdint.h>

/* ChibiOS Library */
#include <ch.h>

/* Constants */
#include "modules/include/constants.h"

extern binary_semaphore_t reset_sem;

void telemetry_init(void);

void epuck_printf(const char *fmt, ...);
void SendNodeToComputer(uint8_t node);
uint8_t ReceiveDestinationFromComputer(void);
uint8_t ReceiveStartFromComputer(void);
uint16_t ReceiveYawFromComputer(void);

#endif /* TELEMETRY_H */