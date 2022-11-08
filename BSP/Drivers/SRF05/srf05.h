#ifndef SRF05_H
#define SRF05_H

#include <stdint.h>
#include <stdbool.h>


void      SRF05_Init(void);
bool      SRF05_MeasureDistance(uint32_t *pDistanceCm);

#endif /* SRF05_H */
