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

#include <ros/assert.h>
#include <algorithm>
#include <iterator>
#include "orion_protocol/orion_framer.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_frame_transport.h"

namespace orion
{

const size_t FrameTransport::BUFFER_SIZE;

bool FrameTransport::sendPacket(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout)
{
  Timeout duration(timeout);
  size_t packet_size = this->framer_.encodePacket(input_buffer, input_size, this->buffer_, BUFFER_SIZE);
  bool result = this->communication_->sendBuffer(this->buffer_, packet_size, duration.timeLeft());
  return result;
}

size_t FrameTransport::receivePacket(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
{
}

bool FrameTransport::hasFrameInQueue()
{
  auto position_forward = std::find(this->queue_.begin(), this->queue_.end(), Framer::FRAME_DELIMETER);
  auto position_backward = std::find(this->queue_.rbegin(), this->queue_.rend(), Framer::FRAME_DELIMETER);
  auto index_first = std::distance(position_forward, this->queue_.begin());
  auto index_last = std::distance(position_backward, this->queue_.rend());

  if (index_last - index_first > 1)
  {
    return true;
  }
  return false;
}

bool FrameTransport::hasReceivedPacket()
{
  bool result = false;

  if (this->hasFrameInQueue())
  {
    result = true;
  }
  else if (this->communication_->hasAvailableBuffer())
  {
    size_t received_size = this->communication_->receiveAvailableBuffer(this->buffer_, BUFFER_SIZE);
    for (size_t i = 0; i < received_size; i++)
    {
      this->queue_.push_back(this->buffer_[i]);
    }
    if (this->hasFrameInQueue())
    {
      result = true;
    }
  }
  return result;
}

}  // orion