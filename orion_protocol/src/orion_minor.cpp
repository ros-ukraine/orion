/*
 * orion_minor.cpp
 *
 *  Created on: 16 лип. 2020 р.
 *      Author: Bogdan
 */

#include "orion_protocol/orion_minor.h"
#include <stdexcept>

namespace orion {

#pragma pack(push, 1)

struct HandshakeCommand
{
  orion::CommandHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 }
  };
};

struct HandshakeResult
{
  orion::ResultHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 },
    .error_code = 0
  };
};

#pragma pack(pop)

void Minor::receiveCommand(void)
{
	HandshakeCommand command;
	uint8_t retry_count = 2;
	uint32_t retry_timeout = 500;

    CommandHeader *command_header = reinterpret_cast<CommandHeader*>(&command);

    bool received = false;
    size_t size_received = 0;
    while ((retry_count > 0) && (false == received))
    {
      Timeout timeout(retry_timeout);
      command_header->common.sequence_id = ++(this->sequence_id_);
      {
    	  	 received = processPacket(command_header, timeout, size_received);
      }
      retry_count--;
    }
    if (false == received)
    {
      throw std::runtime_error("Timeout expired but result was not received");
    }

}

bool Minor::processPacket(const CommandHeader *command_header, Timeout &timeout, size_t &size_received)
{
	  size_received = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
	  do
	  {
	    if (size_received >= sizeof(ResultHeader))
	    {
	    	command_header = reinterpret_cast<CommandHeader*>(this->result_buffer_);
	    }
	    if (this->transport_->hasReceivedPacket() && timeout.hasTime())
	    {
	      size_received = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
	    }
	  }
	  while (timeout.hasTime());

	  return true;
}



} /* namespace orion */
