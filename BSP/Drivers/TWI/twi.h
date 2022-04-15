#ifndef TWI_H
#define TWI_H
#include <same70.h>
#include <stdint.h>
#include <stdbool.h>


#define TWI_QUEUE_SIZE  (1ul)
#define TWI_MSG_LIMIT   (2ul)

/* Cntrol codes */
#define TWI_WRITE       (1u << 0)
#define TWI_READ        (1u << 1)
#define TWI_SR          (1u << 2)

/* Status codes */
#define TWI_SUCCESS     (1u << 0)
#define TWI_TIMEOUT     (1u << 1)
#define TWI_ARB_LOST    (1u << 2)
#define TWI_OVERRUN     (1u << 3)
#define TWI_UNDERRUN    (1u << 4)


typedef struct
{
    uint8_t     *buf;
    uint32_t     len;
    uint32_t     flags;
} TWI_Msg;

typedef struct
{
    Twihs       *pInst;
    uint32_t     addr;
    TWI_Msg      msgArr[TWI_MSG_LIMIT];
} TWI_Adapter;


void     TWI0_Init(void);
uint32_t TWI_Xfer(TWI_Adapter      *pAdap,
                  const uint32_t    count,
                  const uint32_t    timeoutMs);

#endif /* TWI_H */
