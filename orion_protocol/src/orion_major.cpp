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

#include "orion_protocol/orion_major.h"

namespace orion
{

bool Major::processPacket(const CommandHeader *command_header, const ResultHeader *result_header, Timeout &timeout,
  size_t &size_received)
{
  size_received = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
  ResultHeader *received_header;
  bool same_sequence_id = false;
  do
  {
    if (size_received >= sizeof(ResultHeader))
    {
      received_header = reinterpret_cast<ResultHeader*>(this->result_buffer_);
      if (result_header->sequence_id == received_header->sequence_id)
      {
        same_sequence_id = true;
      }
    }
    if (!same_sequence_id && this->transport_->hasReceivedPacket() && timeout.hasTime())
    {
      size_received = this->transport_->receivePacket(this->result_buffer_, BUFFER_SIZE, timeout.timeLeft());
    }
  }
  while ((false == same_sequence_id) && timeout.hasTime());

  return same_sequence_id;
}

void Major::validateResult(const CommandHeader *command_header, const ResultHeader *result_header, size_t size_received)
{
  ResultHeader *received_header;
  if (size_received < sizeof(ResultHeader))
  {
    char message[200];
    std::snprintf(message, sizeof(message), "Received size %zu bytes is less than result header %zu bytes",
      size_received, sizeof(ResultHeader));
    throw std::range_error(message);
  }
  received_header = reinterpret_cast<ResultHeader*>(this->result_buffer_);
  if (result_header->sequence_id != received_header->sequence_id)
  {
    throw std::range_error("Could not receive packet with the same sequence_id");
  }
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
  if ((result_header->version > received_header->version) ||
    (result_header->version < result_header->oldest_compatible_version))
  {
    throw std::range_error("Received reply version is not compatible with existing one");
  }
}

}  // orion
