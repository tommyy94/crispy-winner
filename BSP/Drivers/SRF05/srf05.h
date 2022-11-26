#ifndef SRF05_H
#define SRF05_H

#include <stdint.h>
#include <stdbool.h>


void      SRF05_Init(void);
bool      SRF05_MeasureDistance(float *pDistanceCm);
void      SRF05_StartMeasuring(void);
void      SRF05_StopMeasuring(void);

#endif /* SRF05_H */
