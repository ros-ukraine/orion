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

#include "orion_protocol/orion_framer.h"

namespace orion
{

const uint8_t Framer::FRAME_DELIMETER;

#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

/**
 *
 * @param data[in] 			pointer on data that should be encoded. At the end should be reserved memory for header
 * @param len[in]			len of data that should be encoded plus reserved size.
 * @param packet[out]		pointer on output buffer that will contain complete frame.
 * @param avail_len[in]		Available len in out buffer.
 * @return Resulting length of encoded data.
 */
size_t Framer::encodePacket(const uint8_t* data, size_t len, uint8_t* packet, size_t avail_len) {
	size_t send_len = 0;
	size_t ret = 0;
	Header *header = nullptr;

	header = (Header*)&data[len - sizeof(*header)];

	header->crc = 0;
	header->crc = crc16Calc(data, len - sizeof(*header));

	/// Start 0
	packet[send_len++] = FRAME_DELIMETER;

	ret = stuffData(data, len, &packet[send_len]);
	if (ret < 1) {
		return 0;
	}
	send_len += ret;

	// End 0
	packet[send_len++] = FRAME_DELIMETER;

	return send_len;
}

/**
 *
 * @param packet[in]		Pointer on data that should be decoded.
 * @param len[in]			Length on data
 * @param data[out]			Pointer on output buffer where decoded data will be placed.
 * @param avail_len[out]	Available size of data buffer
 * @return Length of decoded data
 */
size_t Framer::decodePacket(const uint8_t* packet, size_t len, uint8_t* data, size_t avail_len) {
	Header *header = nullptr;

	size_t ret = unStuffData(&packet[1], len, data);
	if (ret < 1) {
		return 0;
	}

	header = (Header*)&data[ret - sizeof(*header)];

	uint16_t crc = crc16Calc(data, ret - sizeof(*header));

	if (crc != header->crc) {
		return 0;
	}

  ret -= sizeof(*header);
  ret--; // TODO: Understand why ?
	return ret;
}

uint16_t Framer::crc16Calc(const uint8_t *data, size_t len) {
	uint16_t crc = 0xFFFF;
	for (uint32_t i = 0; i < len; i++) {
		crc = crc16Update(crc, data[i]);
	}
	return crc;
}

uint16_t Framer::crc16Update(uint16_t crc, const uint8_t a) {
	int i;

	crc ^= (uint16_t)a;
	for (i = 0; i < 8; ++i) {
		if (crc & 1)
			crc = (crc >> 1) ^ 0xA001;
		else
			crc = (crc >> 1);
	}

	return crc;
}

size_t Framer::stuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
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
size_t Framer::unStuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
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
