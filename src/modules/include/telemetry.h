#ifndef TELEMETRY_H
#define TELEMETRY_H

#include <stdint.h>

void telemetry_init(void);

void epuck_printf(const char *fmt, ...);
void SendUint8ToComputer(uint8_t* data, uint16_t size);

#endif /* TELEMETRY_H */