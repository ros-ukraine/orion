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

#ifndef ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_HPP
#define ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_HPP

#include <stdint.h>
#include <cstdlib>
#include "orion_protocol/orion_communication.hpp"
#include "orion_protocol/orion_tcp_serial_bridge.h"

namespace orion
{

class TCPSerialBridge: public Communication
{
public:
  TCPSerialBridge() : Communication() {}

  orion_communication_error_t connect(const char* hostname, const uint32_t port)
  {
    return (orion_communication_connect(getObject(), hostname, port));
  }

  orion_communication_error_t disconnect()
  {
    return (orion_communication_disconnect(getObject()));
  }
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_HPP
