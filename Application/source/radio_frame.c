#include "radio_frame.h"
#include <string.h>


void BuildRadioFrame(
    RadioFrame_t  *frame,
    char          *data,
    uint32_t       len,
    uint8_t        sequenceId)
{
    frame->length     = len;
    frame->sequenceId = sequenceId;

    /* TODO: Replace memcpy */
    memcpy(frame->data, data, RADIO_FRAME_PAYLOAD_SIZE);
}
