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

#ifndef ORION_PROTOCOL_ORION_COBS_FRAMER_H
#define ORION_PROTOCOL_ORION_COBS_FRAMER_H

#include <stdint.h>
#include <stddef.h>
#include "orion_protocol/orion_framer.h"

namespace orion
{

class COBSFramer: public Framer
{
	size_t stuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
	size_t unStuffData(const uint8_t *ptr, size_t length, uint8_t *dst);

public:
	COBSFramer() = default;

	virtual size_t encodePacket(const uint8_t* data, size_t length, uint8_t* packet, size_t buffer_length);
	virtual size_t decodePacket(const uint8_t* packet, size_t length, uint8_t* data, size_t buffer_length);
  virtual ~COBSFramer() = default;

};

}  // orion

#endif  // ORION_PROTOCOL_ORION_COBS_FRAMER_H
