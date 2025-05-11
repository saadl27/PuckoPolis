#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

void telemetry_init(void);

void epuck_printf(const char *fmt, ...);
void SendUint8ToComputer(uint8_t* data, uint16_t size);
void SendNodeToComputer(uint8_t node);
uint8_t ReceiveDestinationFromComputer(void);
uint8_t ReceiveStartFromComputer(void);

#endif /* TELEMETRY_H */