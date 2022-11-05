#ifndef RADIOPACKET_H
#define RADIOPACKET_H

#include <stdint.h>
#include "wifi_conf.h"


#define RADIOPACKET_HEADER_SIZE   (8)
#define RADIOPACKET_PAYLOAD_SIZE  (WIFI_M2M_BUFFER_SIZE - RADIOPACKET_HEADER_SIZE)


typedef struct
{
    /* Header */
    uint32_t  length;
    uint8_t   sequenceId;
    uint8_t   padding[2];

    /* Payload */
    char      data[RADIOPACKET_PAYLOAD_SIZE];
} RadioPacket_t;


void BuildRadioPacket(
    RadioPacket_t  *frame,
    char           *data,
    uint32_t        len,
    uint8_t         sequenceId);


#endif /* RADIOPACKET_H */
