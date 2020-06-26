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

#ifndef ORION_PROTOCOL_ORION_FRAME_TRANSPORT_H
#define ORION_PROTOCOL_ORION_FRAME_TRANSPORT_H

#include <stdint.h>
#include <list>
#include "orion_protocol/orion_communication.h"
#include "orion_protocol/orion_transport.h"

namespace orion
{

class FrameTransport: public Transport
{
public:
  FrameTransport(Communication *communication):communication_(communication) {};
  virtual bool sendPacket(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout);
  virtual size_t receivePacket(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout);
  virtual bool hasReceivedPacket();
  virtual ~FrameTransport() = default;
private:
  bool hasFrameInQueue();

  Communication *communication_;

  // TODO: Add circular buffer
  std::list<uint8_t> queue_;
};

}  // orion

#endif  // ORION_PROTOCOL_ORION_FRAME_TRANSPORT_H
