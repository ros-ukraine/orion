/*
 * Framer.cpp
 *
 *  Created on: 28 квіт. 2020 р.
 *      Author: Bogdan
 */

#include "Framer.h"

Framer::Framer() {
	// TODO Auto-generated constructor stub

}

Framer::~Framer() {
	// TODO Auto-generated destructor stub
}

size_t Framer::EncodePacket(uint8_t* data, size_t len, uint8_t* packet, size_t avail_len) {
	size_t send_len = 0;
	size_t ret = 0;
	Header *header = nullptr;

	header = (Header*)&data[len - sizeof(*header)];

	header->crc = 0;
	header->crc = crc16_calc(data, len - sizeof(*header));

	/// Start 0
	packet[send_len++] = 0;

	ret = StuffData(data, len, &packet[send_len]);
	if (ret < 1) {
		return 0;
	}
	send_len += ret;

	// End 0
	packet[send_len++] = 0;

	return send_len;
}

size_t Framer::DecodePacket(uint8_t* packet, size_t len, uint8_t* data, size_t avail_len) {
	size_t send_len = 0;
	Header *header = nullptr;

	size_t ret = UnStuffData(packet, len, data);
	if (ret < 1) {
		return 0;
	}

	header = (Header*)&data[ret - sizeof(*header)];

	uint16_t crc = crc16_calc(data, ret - sizeof(*header));

	if (crc != header->crc) {
		return 0;
	}

	return ret;
}

uint16_t Framer::crc16_calc(uint8_t *data, size_t len) {
	uint16_t crc = 0xFFFF;
	for (uint32_t i = 0; i < len; i++) {
		crc = crc16_update(crc, data[i]);
	}
	return crc;
}

uint16_t Framer::crc16_update(uint16_t crc, uint8_t a) {
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

#define StartBlock()	(code_ptr = dst++, code = 1)
#define FinishBlock()	(*code_ptr = code)

size_t Framer::StuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
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
size_t Framer::UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst)
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


