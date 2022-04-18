#ifndef PORT_H
#define PORT_H
#include <same70.h>


#define PORT_GROUP_A    (PIOA)
#define PORT_GROUP_B    (PIOB)
#define PORT_GROUP_C    (PIOC)
#define PORT_GROUP_D    (PIOD)
#define PORT_GROUP_E    (PIOE)


typedef enum
{
    IO_PULLUP,
    IO_PULLDOWN
} IO_PullType;


typedef enum
{
    IO_SENSE_RISE = 0,
    IO_SENSE_FALL,
    IO_SENSE_HIGH,
    IO_SENSE_LOW,
    IO_SENSE_RISE_FALL,
    IO_SENSE_COUNT
} IO_Sense_t;


typedef enum
{
    IO_PERIPH_A = 0,
    IO_PERIPH_B,
    IO_PERIPH_C,
    IO_PERIPH_D,
} IO_PeriphFunc;


#define IO_MASK(pin)     (1u << pin)
#define IOn              (31u)

#define IO_PULL_DISABLE  (0u)
#define IO_PULL_ENABLE   (1u)
#define IO_PULLDOWN      (0u)
#define IO_PULLUP        (1u)


void      IO_Init(void);
void      IO_DisableIRQ(Pio *pio, uint32_t pin);
void      IO_EnableIRQ(Pio *pio, uint32_t pin);
void      IO_ConfigurePull(Pio *pio, uint32_t mask, IO_PullType pull);
IRQn_Type IO_ConfigureIRQ(Pio *pio, IO_Sense_t sense, uint32_t mask);
void      IO_SetPeripheralFunction(Pio *pio,const uint32_t mask, const IO_PeriphFunc xFunc);
void      IO_ConfigureOutput(Pio   *const         pio,
                             const uint32_t       pinMask,
                             const uint32_t       driveMask);
void      IO_ConfigureInput(Pio   *const          pio,
                            const uint32_t        pinMask,
                            const uint32_t        pullMask,
                            const uint32_t        pullDir);
void      IO_SetOutput(Pio *pio, const uint32_t   mask);
void      IO_ClearOutput(Pio *pio, const uint32_t mask);
void      IO_InstallIrqHandler(uint32_t const     pin,
                               void              *pfIsr);

 #endif /* PORT_H */
