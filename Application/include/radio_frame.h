#ifndef RADIO_FRAME_H
#define RADIO_FRAME_H

#include <stdint.h>
#include "wifi_conf.h"


#define RADIO_FRAME_HEADER_SIZE   (8)
#define RADIO_FRAME_PAYLOAD_SIZE  (WIFI_M2M_BUFFER_SIZE - RADIO_FRAME_HEADER_SIZE)


typedef struct
{
    /* Header */
    uint32_t  length;
    uint8_t   sequenceId;
    uint8_t   padding[2];

    /* Payload */
    char      data[RADIO_FRAME_PAYLOAD_SIZE];
} RadioFrame_t;


void BuildRadioFrame(
    RadioFrame_t  *frame,
    char          *data,
    uint32_t       len,
    uint8_t        sequenceId);


#endif /* RADIO_FRAME_H */
