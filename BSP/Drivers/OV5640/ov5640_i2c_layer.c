/** @file */

#include "ov5640_i2c_layer.h"
#include "twi.h"
#include "ov5640.h"


TWI_Adapter OV5640_Adap =
{
    .pInst = TWIHS0,
};


/**
 * Register read 8-bit value.
 *
 * @param[in]   addr  7-bit slave address.
 *
 * @param[in]   reg   Register to read.
 *
 * @param[out]  val   Pointer where register contents are read.
 *
 * @param[in]   len   Transfer length.
 *
 * @retval      ret   0 if ok.
 */
int32_t OV5640_ReadReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len)
{
    bool ret;
    uint8_t seq[] =
    {
        (reg >> 8) & 0xFF,
        reg & 0xFF
    };

    OV5640_Adap.addr            = addr;
    OV5640_Adap.msgArr[0].pBuf  = seq;
    OV5640_Adap.msgArr[0].len   = 2;
    OV5640_Adap.msgArr[0].flags = TWI_WRITE;
    OV5640_Adap.msgArr[1].pBuf  = val;
    OV5640_Adap.msgArr[1].len   = len;
    OV5640_Adap.msgArr[1].flags = TWI_READ;
    ret = TWI_Xfer(&OV5640_Adap, 2);
    if (ret == true)
    {
        return OV5640_OK;
    }
    else
    {
        return OV5640_ERROR;
    }
}


/**
 * Register write 8-bit value.
 *
 * @param[in]   addr  7-bit slave address.
 *
 * @param[in]   reg   Register to write.
 *
 * @param[in]   val   Value to write.
 *
 * @param[in]   len   Transfer length.
 *
 * @retval      ret   0 if ok.
 */
int32_t OV5640_WriteReg(uint16_t addr, uint16_t reg, uint8_t *val, uint16_t len)
{
    bool ret;
    uint8_t seq[] =
    {
        (reg >> 8) & 0xFF,
        reg & 0xFF,
        *val
    };

    OV5640_Adap.addr            = addr;
    OV5640_Adap.msgArr[0].pBuf  = seq;
    OV5640_Adap.msgArr[0].len   = 2 + len;
    OV5640_Adap.msgArr[0].flags = TWI_WRITE;
    ret = TWI_Xfer(&OV5640_Adap, 1);
    if (ret == true)
    {
        return OV5640_OK;
    }
    else
    {
        return OV5640_ERROR;
    }
}


/**
 * Don't allow this module to initialize/deinitialize
 * I2C bus as it is multi-slave bus. I2C initialization
 * handled elsewhere.
 *
 * @retval      return OV5640_OK
 */
int32_t OV5640_I2C_Stub(void)
{
    return OV5640_OK;
}
