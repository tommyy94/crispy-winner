#ifndef TWI_H
#define TWI_H
#include <same70.h>
#include <stdint.h>
#include <stdbool.h>


#define TWI_QUEUE_SIZE  (1ul)
#define TWI_MSG_LIMIT   (2ul)

#define TWI_WRITE       (0x00ul)
#define TWI_READ        (0x01ul)
#define TWI_SR          (0x02ul)


typedef struct
{
    uint8_t     *pucBuf;
    uint32_t     ulLen;
    uint32_t     ulFlags;
} TWI_Msg;

typedef struct
{
    Twihs       *pxInst;
    uint32_t     ulAddr;
    TWI_Msg      pxMsg[TWI_MSG_LIMIT];
} TWI_Adapter;


void TWI0_vInit(void);
bool TWI_Xfer(TWI_Adapter *pxAdap, const uint32_t ulCount);

#endif /* TWI_H */
