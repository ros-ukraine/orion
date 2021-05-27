/**
* Copyright 2021 ROS Ukraine
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

#include "orion_protocol/orion_major.hpp"

namespace orion
{

ssize_t Major::processPacket(const CommandHeader *command_header, const ResultHeader *result_header, Timeout &timeout)
{
  ssize_t result = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
  ResultHeader *received_header;
  bool received_same_sequence_id = false;
  do
  {
    if (result >= sizeof(ResultHeader))
    {
      received_header = reinterpret_cast<ResultHeader*>(this->result_buffer_);
      if (command_header->common.sequence_id == received_header->common.sequence_id)
      {
        received_same_sequence_id = true;
      }
    }
    if (!received_same_sequence_id && this->transport_->hasReceivedPacket() && timeout.hasTime())
    {
      result = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
    }
  }
  while (!received_same_sequence_id && timeout.hasTime());

  if (0 > result)
  {
    if (!received_same_sequence_id)
    {
      result = ORION_MAJOR_ERROR_COMMUNICATION_ERROR;
    }
    else if (!timeout.hasTime())
    {
      result = ORION_MAJOR_ERROR_TIMEOUT;
    }
  }

  return (result);
}

orion_major_error_t Major::validateResult(const CommandHeader *command_header, const ResultHeader *result_header,
  size_t size_received)
{
  orion_major_error_t result = ORION_MAJOR_ERROR_UNKNOW;

  ResultHeader *received_header;
  if (size_received < sizeof(ResultHeader))
  {
    return (ORION_MAJOR_ERROR_WRONG_PACKET_HEADER_SIZE); 
  }
  received_header = reinterpret_cast<ResultHeader*>(this->result_buffer_);
  if (command_header->common.sequence_id != received_header->common.sequence_id)
  {
    return (ORION_MAJOR_ERROR_NO_PACKET_WITH_THE_SAME_SEQUENCE_ID);
  }
  if (0 != received_header->error_code)
  {
    //TODO: (Andriy) pass error code to return value
    return (ORION_MAJOR_ERROR_APPLICATION_ERROR_RECEIVED);
  }
  if (result_header->common.message_id != command_header->common.message_id)
  {
    return (ORION_MAJOR_ERROR_DIFFERENT_MESSAGE_ID_RECEIVED);
  }
  if ((result_header->common.version > received_header->common.version) ||
    (result_header->common.version < received_header->common.oldest_compatible_version))
  {
    return (ORION_MAJOR_ERROR_NOT_COMPATIBLE_PACKET_VERSION);
  }
  return (ORION_MAJOR_ERROR_NONE);
}

}  // namespace orion
