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

#ifndef ORION_PROTOCOL_ORION_COMMUNICATION_HPP
#define ORION_PROTOCOL_ORION_COMMUNICATION_HPP

#include <stdint.h>
#include <cstdlib>
#include "orion_protocol/orion_communication.h"

namespace orion
{

//TODO: (Andriy) Design class with error codes support
class Communication
{
public:
  Communication()
  {
    orion_communication_new(&object_);
  }

  virtual ~Communication()
  {
    orion_communication_delete(object_);
  }

  virtual size_t receiveAvailableBuffer(uint8_t *buffer, uint32_t size)
  {
    size_t result = 0;
    orion_communication_receive_available_buffer(object_, buffer, size, &result);
    return (result);
  }

  virtual size_t receiveBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
  {
    size_t result = 0;
    orion_communication_receive_buffer(object_, buffer, size, timeout, &result);
    return (result);
  }

  virtual bool hasAvailableBuffer()
  {
    bool result = orion_communication_has_available_buffer(object_);
    return (result);
  }

  virtual bool sendBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
  {
    orion_error_t result = orion_communication_send_buffer(object_, buffer, size, timeout);
    return (ORION_ERROR_OK == result);
  }

protected:
  orion_communication_t * object_;
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_COMMUNICATION_HPP
