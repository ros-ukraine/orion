/*
 * Framer.h
 *
 *  Created on: 28 квіт. 2020 р.
 *      Author: Bogdan
 */

#pragma once

#include <stdint.h>
#include <stddef.h>

//  uint8_t data[8] = {1, 2, 3, 4, 0, 0, 0, 0};
//  uint8_t buffer[256] = {0};
//  auto s = framer.EncodePacket(data, sizeof(data), buffer, sizeof(buffer));
//  uint8_t dataD[128] = {0};
//  framer.DecodePacket(&buffer[1], s-2, dataD, sizeof(dataD));

class Framer {

	struct Header {
		uint32_t crc;
	} formated;

	size_t StuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
	size_t UnStuffData(const uint8_t *ptr, size_t length, uint8_t *dst);
	uint16_t crc16_calc(uint8_t *data, size_t len);
	uint16_t crc16_update(uint16_t crc, uint8_t a);

public:
	Framer();
	virtual ~Framer();

	size_t EncodePacket(uint8_t* data, size_t len, uint8_t* packet, size_t avail_len);
	size_t DecodePacket(uint8_t* packet, size_t len, uint8_t* data, size_t avail_len);
};

