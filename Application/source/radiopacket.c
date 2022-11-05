#include "radiopacket.h"
#include <string.h>


void BuildRadioPacket(
    RadioPacket_t  *packet,
    char           *data,
    uint32_t        len,
    uint8_t         sequenceId)
{
    packet->length     = len;
    packet->sequenceId = sequenceId;

    /* TODO: Replace memcpy */
    memcpy(packet->data, data, RADIOPACKET_PAYLOAD_SIZE);
}
