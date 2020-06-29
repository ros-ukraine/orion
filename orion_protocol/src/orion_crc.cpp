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

#include "orion_protocol/orion_crc.h"

namespace orion
{

uint16_t CRC::calculateCRC16(const uint8_t *data, size_t length)
{
	uint16_t crc = 0xFFFF;
	for (uint32_t i = 0; i < length; i++)
	{
		crc = CRC::updateCRC16(crc, data[i]);
	}
	return crc;
}

uint16_t CRC::updateCRC16(uint16_t crc, const uint8_t byte)
{
	int i;

	crc ^= (uint16_t)byte;
	for (i = 0; i < 8; ++i)
	{
		if (crc & 1)
		{
			crc = (crc >> 1) ^ 0xA001;
	  }
		else
		{
			crc = (crc >> 1);
	  }
	}

	return crc;
}

}  // orion
