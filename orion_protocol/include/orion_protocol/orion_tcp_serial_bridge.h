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

#ifndef ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_H
#define ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_H

#include <stdint.h>
#include <cstdlib>
#include "orion_protocol/orion_communication.h"

namespace orion
{

class TCPSerialBridge: public Communication
{
public:
  void connect(const char* hostname, uint32_t port = 8080);
  void disconnect();

  TCPSerialBridge();
  virtual size_t receiveAvailableBuffer(uint8_t *buffer, uint32_t size);
  virtual size_t receiveBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout);
  virtual bool hasAvailableBuffer();
  virtual bool sendBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout);
  virtual ~TCPSerialBridge() = default;

private:
  int socket_descriptor_ = -1;
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_TCP_SERIAL_BRIDGE_H
