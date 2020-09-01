/*
 * orion_minor.h
 *
 *  Created on: 16 лип. 2020 р.
 *      Author: Bogdan
 */

#ifndef THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_
#define THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_

#include "orion_protocol/orion_transport.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_timeout.h"


namespace orion {

class Minor {
public:
	explicit Minor(Transport *transport) : transport_(transport) {}
	virtual ~Minor() {};

	void receiveCommand(void);
	bool processPacket(const CommandHeader *command_header, Timeout &timeout, size_t &size_received);

	  Transport *transport_;

	  static const uint32_t BUFFER_SIZE = 500;
	  uint8_t result_buffer_[BUFFER_SIZE];

	  uint32_t sequence_id_ = 0;
};

} /* namespace orion */

#endif /* THIRD_PARTY_LIBS_ORION_ORION_MINOR_H_ */
