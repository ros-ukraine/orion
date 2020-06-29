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

#include "orion_protocol/orion_cobs_framer.h"

namespace orion
{

#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

// TODO: fix parameter description
/**
 *
 * @param data[in] 			pointer on data that should be encoded. At the end should be reserved memory for header
 * @param len[in]			len of data that should be encoded plus reserved size.
 * @param packet[out]		pointer on output buffer that will contain complete frame.
 * @param avail_len[in]		Available len in out buffer.
 * @return Resulting length of encoded data.
 */
size_t COBSFramer::encodePacket(const uint8_t* data, size_t length, uint8_t* packet, size_t buffer_length)
{
	size_t send_len = 0;
	size_t result = 0;

	// Start 0
	packet[send_len++] = Framer::FRAME_DELIMETER;

	result = stuffData(data, length, &packet[send_len]);
	if (result < 1)
	{
		result = 0;
	}
	else
	{
	  send_len += result;

	  // End 0
	  packet[send_len++] = Framer::FRAME_DELIMETER;
	}

	return send_len;
}

// TODO: fix parameter description
/**
 *
 * @param packet[in]		Pointer on data that should be decoded.
 * @param len[in]			Length on data
 * @param data[out]			Pointer on output buffer where decoded data will be placed.
 * @param avail_len[out]	Available size of data buffer
 * @return Length of decoded data
 */
size_t COBSFramer::decodePacket(const uint8_t* packet, size_t length, uint8_t* data, size_t buffer_length)
{
	size_t result = unStuffData(&packet[1], length, data);
	if (result < 1)
	{
		return 0;
	}
  result--; // TODO: Understand why ?
	return result;
}

size_t COBSFramer::stuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
{
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code, *code_ptr; /* Where to insert the leading count */

	StartBlock();
	while (ptr < end) {
		if (code != 0xFF) {
			uint8_t c = *ptr++;
			if (c != 0) {
				*dst++ = c;
				code++;
				continue;
			}
		}
		FinishBlock();
		StartBlock();
	}
	FinishBlock();
	return dst - start;
}

/*
 * UnStuffData decodes "length" bytes of data at
 * the location pointed to by "ptr", writing the
 * output to the location pointed to by "dst".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 */
size_t COBSFramer::unStuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
{
	const uint8_t *start = dst, *end = ptr + length;
	uint8_t code = 0xFF, copy = 0;

	for (; ptr < end; copy--) {
		if (copy != 0) {
			*dst++ = *ptr++;
		} else {
			if (code != 0xFF)
				*dst++ = 0;
			copy = code = *ptr++;
			if (code == 0)
				break; /* Source length too long */
		}
	}
	return dst - start;
}

}  // orion
