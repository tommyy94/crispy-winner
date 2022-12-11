/** @file */

#ifndef OV5640_I2C_LAYER_H
#define OV5640_I2C_LAYER_H
#include <stdint.h>

int32_t OV5640_ReadReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len);
int32_t OV5640_WriteReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len);
int32_t OV5640_I2C_Stub(void);

#endif /* OV5640_I2C_LAYER_H */
