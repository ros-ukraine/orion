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
	uint16_t result = 0xFFFF;
	for (uint32_t i = 0; i < length; i++)
	{
		result ^= (uint16_t)data[i];
    for (uint8_t j = 0; j < 8; ++j)
    {
      if (result & 1)
      {
        result = (result >> 1) ^ 0xA001;
      }
      else
      {
        result = (result >> 1);
      }
    }
	}
	return result;
}

}  // orion
