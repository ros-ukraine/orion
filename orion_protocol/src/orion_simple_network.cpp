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

#include "orion_protocol/orion_simple_network.h"
#include "orion_protocol/orion_header.h"
#include <ros/assert.h>
#include <cstring>

namespace orion
{

size_t SimpleNetwork::sendAndReceivePacket(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout,
    uint8_t *output_buffer, uint32_t output_size)
{
  Timeout timer(timeout);
  PacketHeader *packet_header = reinterpret_cast<PacketHeader*>(this->command_buffer);
  size_t command_buffer_size = sizeof(PacketHeader);

  ROS_ASSERT(command_buffer_size + input_size < BUFFER_SIZE);

  packet_header.sequence_id = ++(this->sequence_id);

  std::memcpy(this->command_buffer + command_buffer_size, input_buffer, input_size);
  command_buffer_size += input_size;

  data_link_layer_.sendFrame(this->command_buffer, command_buffer_size, timer.timeLeft());
  bool received = false;
  size_t output_size = 0;
  while (timer.hasTime() && (false == received))
  {
    output_size = data_link_layer_.receiveFrame(output_buffer, output_size, timer.timeLeft());
    if (output_size >= sizeof(PacketHeader))
    {
      PacketHeader *packet_header = reinterpret_cast<PacketHeader*>(output_size);
      received = (this->sequence_id == packet_header.sequence_id)
    }
  }
  return output_size;
}

void SimpleNetwork::receivePacket(uint8_t *buffer, uint32_t size, uint32_t timeout)
{
}

}
