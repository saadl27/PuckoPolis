#ifndef TELEMETRY_H
#define TELEMETRY_H


void telemetry_init(void);

void SendUint8ToComputer(uint8_t* data, uint16_t size);

#endif