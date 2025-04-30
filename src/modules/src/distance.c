#include "modules/include/distance.h"
#include "modules/include/telemetry.h"

#include "sensors/VL53L0X/VL53L0X.h"

void tof_init(void) {
    // tof_sensor->I2cDevAddr = VL53L0X_ADDR;
    // // VL53L0X_start();
    // VL53L0X_Error status = VL53L0X_init(tof_sensor);
    // epuck_printf("tof status = %d\n", status);
    // epuck_printf("tof sensor addr = %d\n", tof_sensor->I2cDevAddr);
    VL53L0X_start();
}

uint16_t tof_get_dist_mm(void) {
    return VL53L0X_get_dist_mm();
}