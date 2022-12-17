/** @file */

#ifndef OV5640_I2C_LAYER_H
#define OV5640_I2C_LAYER_H
#include <stdint.h>
#include "ov5640.h"

int32_t OV5640_ReadReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len);
int32_t OV5640_WriteReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len);
int32_t OV5640_I2C_Stub(void);
int32_t OV5640_LoadRegisters(
  OV5640_Object_t   *pObj,
  OV5640_Register_t  pRegTable[],
  uint32_t           size);

#endif /* OV5640_I2C_LAYER_H */
