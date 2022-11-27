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

/* Status codes */
#define TWI_SUCCESS     (1u << 0)
#define TWI_TIMEOUT     (1u << 1)
#define TWI_ARB_LOST    (1u << 2)
#define TWI_OVERRUN     (1u << 3)
#define TWI_UNDERRUN    (1u << 4)


typedef struct
{
    uint8_t     *pBuf;
    uint32_t     len;
    uint32_t     flags;
} TWI_Msg;

typedef struct
{
    Twihs       *pInst;
    uint32_t     addr;
    TWI_Msg      msgArr[TWI_MSG_LIMIT];
} TWI_Adapter;


void TWI0_Init(void);
bool TWI_Xfer(TWI_Adapter *pAdap, const uint32_t count);

#endif /* TWI_H */
