#ifndef PMC_DRIVER_H
#define PMC_DRIVER_H

#include <stdint.h>

void PMC_PeripheralClockEnable(const uint32_t pid);
void PMC_PeripheralClockDisable(const uint32_t pid);

#endif /* PMC_DRIVER_H */
