/** @file */

#include "ov5640_i2c_layer.h"
#include "twi.h"
#include <stdio.h>


#define OV5640_ADDR                     (0x3C)

/* Register map */
#define OV5640_CHIP_ID_HIGH_BYTE_REG    (0x300A) /* Read only */
#define OV5640_CHIP_ID_LOW_BYTE_REG     (0x300B) /* Read only */

/* Bit definitions */
#define OV5640_CHIP_ID_HIGH_BYTE        (0x56)
#define OV5640_CHIP_ID_LOW_BYTE         (0x40)



TWI_Adapter OV5640Adap =
{
    .pInst = TWIHS0,
    .addr  = OV5640_ADDR
};


static void OV5640_AssertReset(void);
static void OV5640_ReleaseReset(void);


/**
 * Verify OV5640 chip.
 *
 * @param   None.
 *
 * @retval  ret   true if chip ok, else false.
 */
bool OV5640_VerifyChip(void)
{
    uint32_t  status;
    uint8_t   reg[2];
    uint8_t   chipIdH;
    uint8_t   chipIdL;
    uint16_t  chipId;
    bool      ret = false;

    reg[0] = OV5640_CHIP_ID_HIGH_BYTE_REG >> 8;
    reg[1] = OV5640_CHIP_ID_HIGH_BYTE_REG & 0xFF;
    OV5640Adap.msgArr[0].pBuf = reg;
    OV5640Adap.msgArr[0].len = 2;
    OV5640Adap.msgArr[0].flags = TWI_WRITE;
    OV5640Adap.msgArr[1].pBuf = &chipIdH;
    OV5640Adap.msgArr[1].len = 1;
    OV5640Adap.msgArr[1].flags = TWI_READ;
    status = TWI_Xfer(&OV5640Adap, 2);

    reg[0] = OV5640_CHIP_ID_LOW_BYTE_REG >> 8;
    reg[1] = OV5640_CHIP_ID_LOW_BYTE_REG & 0xFF;
    OV5640Adap.msgArr[0].pBuf = reg;
    OV5640Adap.msgArr[0].len = 2;
    OV5640Adap.msgArr[0].flags = TWI_WRITE;
    OV5640Adap.msgArr[1].pBuf = &chipIdL;
    OV5640Adap.msgArr[1].len = 1;
    OV5640Adap.msgArr[1].flags = TWI_READ;
    status &= TWI_Xfer(&OV5640Adap, 2);
    if (status)
    {
        /* Should be 0x5640 */
        chipId = (chipIdH << 8) | chipIdL;
        printf("OV5640 chip ID = 0x%04X\r\n", chipId);
        if (chipId == 0x5640)
        {
            ret = true;
        }
    }

    return ret;
}
