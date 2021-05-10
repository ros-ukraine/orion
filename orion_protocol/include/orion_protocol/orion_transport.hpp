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

#ifndef ORION_PROTOCOL_ORION_TRANSPORT_HPP
#define ORION_PROTOCOL_ORION_TRANSPORT_HPP

#include <stdint.h>
#include <cstdlib>
#include "orion_protocol/orion_transport.h"
#include "orion_protocol/orion_communication.hpp"
#include "orion_protocol/orion_assert.h"

namespace orion
{

class Transport
{
public:
  Transport(Communication * communication)
  {
    ORION_ASSERT_NOT_NULL(communication);
    orion_transport_new(&object_, communication->getObject());
  }

  virtual ~Transport()
  {
    orion_transport_delete(object_);
  }

  virtual orion_transport_error_t sendPacket(uint8_t *input_buffer, uint32_t input_size, uint32_t timeout)
  {
    return (orion_transport_send_packet(object_, input_buffer, input_size, timeout));
  }

  virtual orion_transport_error_t receivePacket(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout,
    size_t * received_size)
  {
    return (orion_transport_receive_packet(object_, output_buffer, output_size, timeout, received_size));
  }

  virtual bool hasReceivedPacket()
  {
    return (orion_transport_has_received_packet(object_));
  }

  orion_transport_t* getObject()
  {
    return object_;
  }

private:
  orion_transport_t * object_;  
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_TRANSPORT_HPP
