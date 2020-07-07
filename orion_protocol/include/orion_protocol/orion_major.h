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

#ifndef ORION_PROTOCOL_ORION_MAJOR_H
#define ORION_PROTOCOL_ORION_MAJOR_H

#include "orion_protocol/orion_transport.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_timeout.h"
#include <ros/assert.h>
#include <stdint.h>
#include <cstring>
#include <stdexcept>
#include <cstdio>

namespace orion
{

class Major
{
public:
  Major(Transport *transport) : transport_(transport) {};

  Major(Transport *transport, uint32_t retry_timeout, uint8_t retry_count) : transport_(transport),
    default_timeout_(retry_timeout),
    default_retry_count_(retry_count) {};

  template<class Command, class Result>
  void invoke(Command command, Result *result)
  {
    this->invoke<Command, Result>(command, result, this->default_timeout_, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(Command command, Result *result, uint32_t retry_timeout)
  {
    this->invoke<Command, Result>(command, result, retry_timeout, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(Command command, Result *result, uint32_t retry_timeout, uint8_t retry_count)
  {
    ROS_ASSERT(NULL != this->transport_);
    ROS_ASSERT(sizeof(Command) >= sizeof(CommandHeader));
    ROS_ASSERT(sizeof(Result) >= sizeof(ResultHeader));

    CommandHeader *command_header = reinterpret_cast<CommandHeader*>(&command);
    ResultHeader *result_header = reinterpret_cast<ResultHeader*>(result);

    bool received = false;
    size_t size_received = 0;
    while ((retry_count > 0) && (false == received))
    {
      Timeout timeout(retry_timeout);
      command_header->common.sequence_id = ++(this->sequence_id_);
      bool send_status = this->transport_->sendPacket(reinterpret_cast<uint8_t*>(&command), sizeof(command),
        retry_timeout);
      if (send_status)
      {
          received = this->processPacket(command_header, result_header, timeout, size_received);
      }
      retry_count--;
    }
    if (false == received)
    {
      throw std::runtime_error("Timeout expired but result was not received");
    }
    this->validateResult(command_header, result_header, size_received);
    std::memcpy(reinterpret_cast<uint8_t*>(result), this->result_buffer_, sizeof(Result));
  }

  enum Interval { Microsecond = 1, Millisecond = 1000 * Microsecond, Second = 1000 * Millisecond };

private:
  bool processPacket(const CommandHeader *command_header, const ResultHeader *result_header, Timeout &timeout,
    size_t &size_received);
  void validateResult(const CommandHeader *command_header, const ResultHeader *result_header, size_t size_received);

  uint32_t default_timeout_ = 100 * Interval::Millisecond;
  uint8_t default_retry_count_ = 1;

  Transport *transport_;

  static const uint32_t BUFFER_SIZE = 500;
  uint8_t result_buffer_[BUFFER_SIZE];

  uint32_t sequence_id_ = 0;
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_MAJOR_H
