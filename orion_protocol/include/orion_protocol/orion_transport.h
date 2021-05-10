/**
* Copyright 2021 ROS Ukraine
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"),
* to deal in the Software without restriction, including without limitation
* the rights to use, copy, modify, merge, publish, distribute, sublicense,
* and/or sell copies of the Software, and to permit persons to whom
* the Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included
* in all copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
* ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
* OTHER DEALINGS IN THE SOFTWARE.
*
*/

#ifndef ORION_PROTOCOL_ORION_TRANSPORT_H
#define ORION_PROTOCOL_ORION_TRANSPORT_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include "orion_protocol/orion_communication.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ORION_TRAN_ERROR_OK = 0,
    ORION_TRAN_ERROR_COULD_NOT_ALLOCATE_MEMORY,
    ORION_TRAN_ERROR_COULD_NOT_FREE_MEMORY,
    ORION_TRAN_ERROR_TIMEOUT,
    ORION_TRAN_ERROR_FAILED_TO_ENCODE_PACKET,
    ORION_TRAN_ERROR_FAILED_TO_SEND_PACKET,
    ORION_TRAN_ERROR_FAILED_TO_DECODE_PACKET,
    ORION_TRAN_ERROR_FAILED_TO_RECEIVE_FULL_PACKET,
    ORION_TRAN_ERROR_CRC_CHECK_FAILED
} orion_transport_error_t;

struct orion_transport_struct_t;

typedef struct orion_transport_struct_t orion_transport_t;

orion_transport_error_t orion_transport_new(orion_transport_t ** me, orion_communication_t * communication);
orion_transport_error_t orion_transport_delete(const orion_transport_t * me);

orion_transport_error_t orion_transport_send_packet(orion_transport_t * me, uint8_t *input_buffer,
    uint32_t input_size, uint32_t timeout);
orion_transport_error_t orion_transport_receive_packet(orion_transport_t * me, uint8_t *output_buffer,
    uint32_t output_size, uint32_t timeout, size_t * received_size);
bool orion_transport_has_received_packet(orion_transport_t * me);

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_TRANSPORT_H
