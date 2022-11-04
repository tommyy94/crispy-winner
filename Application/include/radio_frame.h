#ifndef RADIO_FRAME_H
#define RADIO_FRAME_H

#include "wifi_con.h"


#define RADIO_FRAME_HEADER_SIZE   (6)
#define RADIO_FRAME_PAYLOAD_SIZE  (WIFI_M2M_BUFFER_SIZE - RADIO_FRAME_HEADER_SIZE)


typedef struct
{
    /* Header */
    uint32_t  length;
    uint8_t   sequenceId;

    /* Payload */
    char      data[RADIO_FRAME_PAYLOAD_SIZE];
} RadioFrame_t;


#endif /* RADIO_FRAME_H */
