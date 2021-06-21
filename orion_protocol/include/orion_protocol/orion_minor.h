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

#ifndef ORION_PROTOCOL_ORION_MINOR_H
#define ORION_PROTOCOL_ORION_MINOR_H

#include <stdint.h>
#include <stdbool.h>
#include <sys/types.h>

#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_transport.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
  ORION_MINOR_ERROR_NONE = 0,
  ORION_MINOR_ERROR_COULD_NOT_ALLOCATE_MEMORY = -1,
  ORION_MINOR_ERROR_COULD_NOT_FREE_MEMORY = -2,
  ORION_MINOR_ERROR_TIMEOUT = -3,
  ORION_MINOR_ERROR_RECEIVING_PACKET = -4,
  ORION_MINOR_ERROR_SENDING_PACKET = -5,
  ORION_MINOR_ERROR_UNKNOWN = -6
}
orion_minor_error_t;

struct orion_minor_struct_t;

typedef struct orion_minor_struct_t orion_minor_t;

orion_minor_error_t orion_minor_new(orion_minor_t ** me, orion_transport_t * transport);
orion_minor_error_t orion_minor_delete(const orion_minor_t * me);

ssize_t orion_minor_wait_and_receive_command(const orion_minor_t * me, uint8_t * buffer, size_t buffer_size,
  uint32_t timeout);
ssize_t orion_minor_receive_command(const orion_minor_t * me, uint8_t * buffer, size_t buffer_size);
orion_minor_error_t orion_minor_send_result(const orion_minor_t * me, uint8_t * buffer, const size_t size);

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_MINOR_H
