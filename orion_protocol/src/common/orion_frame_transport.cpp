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

#include "orion_protocol/orion_assert.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_framer.h"
#include "orion_protocol/orion_crc.h"
#include "orion_protocol/orion_timeout.hpp"
#include "orion_protocol/orion_frame_transport.hpp"
#include "orion_protocol/orion_circular_buffer.h"

namespace orion
{

const size_t FrameTransport::BUFFER_SIZE;

FrameTransport::FrameTransport(Communication *communication, Framer *framer):framer_(framer),
  communication_(communication)
{
  orion_circular_buffer_init(&this->circular_queue_, this->queue_buffer_, QUEUE_BUFFER_SIZE);
}

bool FrameTransport::sendPacket(uint8_t *input_buffer, uint32_t input_size, uint32_t timeout)
{
  ORION_ASSERT(input_size >= sizeof(FrameHeader));
  Timeout duration(timeout);

  FrameHeader *frame_header = reinterpret_cast<FrameHeader*>(input_buffer);
  frame_header->crc = CRC::calculateCRC16(input_buffer + sizeof(FrameHeader), input_size - sizeof(FrameHeader));
  size_t packet_size = this->framer_->encodePacket(input_buffer, input_size, this->buffer_, BUFFER_SIZE);
  orion_communication_error_t status = this->communication_->sendBuffer(this->buffer_, packet_size, 
    duration.timeLeft());
  return (ORION_COM_ERROR_OK == status);
}

size_t FrameTransport::receivePacket(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
{
  Timeout duration(timeout);
  size_t result = 0;
  bool decode = this->hasReceivedPacket();
  if (false == decode)
  {
    size_t size = 0;
    orion_communication_error_t status_received = this->communication_->receiveBuffer(this->buffer_, BUFFER_SIZE, 
      (duration.timeLeft() * 3) / 4, &size);
    if ((ORION_COM_ERROR_OK == status_received) && (size > 0))
    {
      orion_circular_buffer_add(&this->circular_queue_, this->buffer_, size);

      if (this->hasFrameInQueue())
      {
        decode = true;
      }
    }
  }

  if (decode)
  {
    uint32_t size = 0;
    bool status = orion_circular_buffer_dequeue_word(&this->circular_queue_, Framer::FRAME_DELIMETER, this->buffer_,
      BUFFER_SIZE, &size);
    ORION_ASSERT(status);
    result = this->framer_->decodePacket(this->buffer_, size, output_buffer, output_size);
    if (result < sizeof(FrameHeader))
    {
      result = 0;
    }
    else
    {
      FrameHeader *frame_header = reinterpret_cast<FrameHeader*>(output_buffer);
      if (CRC::calculateCRC16(output_buffer + sizeof(FrameHeader),
        result - sizeof(FrameHeader)) != frame_header->crc)
      {
        result = 0;
      }
    }
  }
  return result;
}

bool FrameTransport::hasFrameInQueue()
{
  bool result = orion_circular_buffer_has_word(&this->circular_queue_, Framer::FRAME_DELIMETER);
  return (result);
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
    size_t received_size = 0;
    orion_communication_error_t status = this->communication_->receiveAvailableBuffer(this->buffer_, BUFFER_SIZE, 
      &received_size);
    if (ORION_COM_ERROR_OK == status)
    {
      orion_circular_buffer_add(&this->circular_queue_, this->buffer_, received_size);
      if (this->hasFrameInQueue())
      {
        result = true;
      }
    }
  }
  return (result);
}

}  // namespace orion
