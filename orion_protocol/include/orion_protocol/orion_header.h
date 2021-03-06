/**
* Copyright 2020 ROS Ukraine
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

namespace orion
{

#pragma pack(push, 1)

struct FrameHeader
{
  uint16_t crc;
};

struct CommonHeader
{
  uint8_t message_id;
  uint8_t version;
  uint8_t oldest_compatible_version;
  uint16_t sequence_id;
};

struct CommandHeader
{
  FrameHeader frame;
  CommonHeader common;
};

struct ResultHeader
{
  FrameHeader frame;
  CommonHeader common;
  uint8_t error_code;
};

#pragma pack(pop)

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_HEADER_H
