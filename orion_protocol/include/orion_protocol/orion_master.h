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

#ifndef ORION_PROTOCOL_ORION_MASTER_H
#define ORION_PROTOCOL_ORION_MASTER_H

#include "orion_protocol/orion_network_layer.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_timeout.h"
#include <ros/assert.h>
#include <stdint.h>
#include <cstring>
#include <stdexcept>
#include <cstdio>

namespace orion
{

class Master
{
public:
  Master(NetworkLayer *network_layer) : network_layer_(network_layer) {};

  Master(NetworkLayer *network_layer, uint32_t retry_timeout, uint8_t retry_count) : network_layer_(network_layer),
    default_timeout_(retry_timeout),
    default_retry_count_(retry_count) {};

  template<class Command, class Result>
  void invoke(const Command &command, Result *result)
  {
    this->invoke<Command, Result>(command, result, this->default_timeout_, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(const Command &command, Result *result, uint32_t retry_timeout)
  {
    this->invoke<Command, Result>(command, result, retry_timeout, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(const Command &command, Result *result, uint32_t retry_timeout, uint8_t retry_count)
  {
    ROS_ASSERT(NULL != this->network_layer_);
    ROS_ASSERT(sizeof(Command) >= sizeof(CommandHeader));
    ROS_ASSERT(sizeof(Result) >= sizeof(ResultHeader));

    const CommandHeader *command_header = reinterpret_cast<const CommandHeader*>(&command);
    ResultHeader *result_header = reinterpret_cast<ResultHeader*>(result);

    bool received = false;
    while ((retry_count > 0) && (false == received))
    {
      size_t size_received = this->network_layer_->sendAndReceivePacket(reinterpret_cast<const uint8_t*>(&command),
        sizeof(command), retry_timeout, this->result_buffer_, BUFFER_SIZE);
      if (size_received >= sizeof(ResultHeader))
      {
        ResultHeader *received_header = reinterpret_cast<ResultHeader*>(this->result_buffer_);
        if (0 != received_header->error_code)
        {
          char message[200];
          std::snprintf(message, sizeof(message), "Error code: %d detected in return packet",
            received_header->error_code);
          throw std::runtime_error(message);
        }
        if (result_header->message_id != command_header->message_id)
        {
          throw std::range_error("Received different message_id from reply packet");
        }
        if ((result_header->version != received_header->version) && (0 == result_header->backward_compatible))
        {
          throw std::range_error("Received reply version is not compatible with existing one");
        }
        received = true;
      }
      retry_count--;
    }
    if (false == received)
    {
      throw std::runtime_error("Timeout expired but result was not received");
    }
    std::memcpy(reinterpret_cast<uint8_t*>(result), this->result_buffer_, sizeof(Result));
  }

  enum Timeout { Microsecond = 1, Millisecond = 1000 * Microsecond, Second = 1000 * Millisecond };

private:

  void sendAndReceive(const uint8_t *input_buffer, uint32_t input_size, uint32_t retry_timeout, uint8_t retry_count,
    uint8_t *output_buffer, uint32_t output_size);

  uint32_t default_timeout_ = 100 * Timeout::Millisecond;
  uint8_t default_retry_count_ = 1;

  NetworkLayer *network_layer_;

  static const uint32_t BUFFER_SIZE = 500;
  uint8_t result_buffer_[BUFFER_SIZE];
};

}  // orion

#endif  // ORION_PROTOCOL_ORION_MASTER_H
