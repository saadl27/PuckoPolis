#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>
#include <ch.h>
#include <hal.h>

extern binary_semaphore_t reset_sem;

void telemetry_init(void);

void epuck_printf(const char *fmt, ...);
void SendUint8ToComputer(uint8_t* data, uint16_t size);
void SendNodeToComputer(uint8_t node);
uint8_t ReceiveDestinationFromComputer(void);
uint8_t ReceiveStartFromComputer(void);

#endif /* TELEMETRY_H */