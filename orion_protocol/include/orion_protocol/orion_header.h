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

#ifndef ORION_PROTOCOL_ORION_HEADER_H
#define ORION_PROTOCOL_ORION_HEADER_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

#pragma pack(push, 1)

typedef struct
{
  uint16_t crc;
} orion_frame_header_t;

typedef struct
{
  uint8_t message_id;
  uint8_t version;
  uint8_t oldest_compatible_version;
  uint16_t sequence_id;
} orion_common_header_t;

typedef struct
{
  orion_frame_header_t frame;
  orion_common_header_t common;
} orion_command_header_t;

typedef struct
{
  orion_frame_header_t frame;
  orion_common_header_t common;
  uint8_t error_code;
} orion_result_header_t;

#pragma pack(pop)

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_HEADER_H
