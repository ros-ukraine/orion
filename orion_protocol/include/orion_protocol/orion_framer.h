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

#ifndef ORION_PROTOCOL_ORION_FRAMER_H
#define ORION_PROTOCOL_ORION_FRAMER_H

#include <stdint.h>
#include <stddef.h>

namespace orion
{

//  uint8_t data[8] = {1, 2, 3, 4, 0, 0, 0, 0};
//  uint8_t buffer[256] = {0};
//  auto s = framer.EncodePacket(data, sizeof(data), buffer, sizeof(buffer));
//  uint8_t dataD[128] = {0};
//  framer.DecodePacket(&buffer[1], s-2, dataD, sizeof(dataD));

class Framer
{

	struct Header {
		uint32_t crc;
	} formated;

	size_t stuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
	size_t unStuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
	uint16_t crc16Calc(const uint8_t *data, size_t len);
	uint16_t crc16Update(uint16_t crc, const uint8_t a);

public:
	Framer() {};
	virtual ~Framer() {};

	size_t encodePacket(const uint8_t* data, size_t len, uint8_t* packet, size_t avail_len);
	size_t decodePacket(const uint8_t* packet, size_t len, uint8_t* data, size_t avail_len);

	static const uint8_t FRAME_DELIMETER = 0;
};

}  // orion

#endif  // ORION_PROTOCOL_ORION_FRAMER_H
