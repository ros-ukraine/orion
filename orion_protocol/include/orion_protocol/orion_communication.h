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

#ifndef ORION_PROTOCOL_ORION_COMMUNICATION_H
#define ORION_PROTOCOL_ORION_COMMUNICATION_H

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef enum
{
  ORION_COM_ERROR_NONE = 0,
  ORION_COM_ERROR_COULD_NOT_ALLOCATE_MEMORY = -1,
  ORION_COM_ERROR_COULD_NOT_FREE_MEMORY = -2,
  ORION_COM_ERROR_TIMEOUT = -3,
  ORION_COM_ERROR_GETTING_TERMINAL_ATTRIBUTES = -4,
  ORION_COM_ERROR_SETTING_TERMINAL_ATTRIBUTES = -5,
  ORION_COM_ERROR_OPENNING_SERIAL_PORT = -6,
  ORION_COM_ERROR_CLOSING_SERIAL_PORT = -7,
  ORION_COM_ERROR_READING_SERIAL_PORT = -8,
  ORION_COM_ERROR_WRITING_TO_SERIAL_PORT = -9,
  ORION_COM_ERROR_CREATE_SOCKET = -10,
  ORION_COM_ERROR_COULD_NOT_FIND_HOST = -11,
  ORION_COM_ERROR_COULD_NOT_CONNECT_TO_HOST = -12,
  ORION_COM_ERROR_CLOSING_SOCKET = -13,
  ORION_COM_ERROR_READING_SOCKET = -14,
  ORION_COM_ERROR_WRITING_TO_SOCKET = -15,
  ORION_COM_ERROR_UNKNOWN = -16
}
orion_communication_error_t;

struct orion_communication_struct_t;

typedef struct orion_communication_struct_t orion_communication_t;

orion_communication_error_t orion_communication_new(orion_communication_t ** me);
orion_communication_error_t orion_communication_delete(const orion_communication_t * me);

ssize_t orion_communication_receive_available_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size);
ssize_t orion_communication_receive_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size,
  uint32_t timeout);
bool orion_communication_has_available_buffer(const orion_communication_t * me);
orion_communication_error_t orion_communication_send_buffer(const orion_communication_t * me, uint8_t *buffer,
  uint32_t size, uint32_t timeout);

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_COMMUNICATION_H
