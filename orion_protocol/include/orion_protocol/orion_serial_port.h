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

#ifndef ORION_PROTOCOL_ORION_SERIAL_PORT_H
#define ORION_PROTOCOL_ORION_SERIAL_PORT_H

#include <stdint.h>
#include <cstdlib>
#include "orion_protocol/orion_communication.h"

namespace orion
{

class SerialPort: public Communication
{
public:
  void connect(const char* port_name, uint32_t baud = 9600);
  void disconnect();

  SerialPort();
  virtual size_t receiveAvailableBuffer(uint8_t *buffer, uint32_t size) = 0;
  virtual size_t receiveBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout) = 0;
  virtual bool hasAvailableBuffer() = 0;
  virtual bool sendBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout) = 0;
  virtual ~SerialPort() = default;

private:
  void setInterfaceAttributes(uint32_t speed);

  int file_descriptor_ = -1;
};

}  // orion

#endif  // ORION_PROTOCOL_ORION_SERIAL_PORT_H
