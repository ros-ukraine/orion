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

#ifndef ORION_PROTOCOL_ORION_MINOR_HPP
#define ORION_PROTOCOL_ORION_MINOR_HPP

#include <stdint.h>
#include <stdbool.h>
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_minor.h"
#include "orion_protocol/orion_transport.hpp"

namespace orion
{

class Minor
{
public:
  Minor(Transport *transport)
  {
    ORION_ASSERT_NOT_NULL(transport);
    orion_minor_new(&object_, transport->getObject());
  }

  virtual ~Minor()
  {
    orion_minor_delete(object_);
  }

  virtual ssize_t waitAndReceiveCommand(uint8_t * buffer, size_t buffer_size, uint32_t timeout)
  {
    return (orion_minor_wait_and_receive_command(object_, buffer, buffer_size, timeout));
  }

  virtual ssize_t receiveCommand(uint8_t * buffer, size_t buffer_size)
  {
    return (orion_minor_receive_command(object_, buffer, buffer_size));
  }

  virtual orion_minor_error_t sendResult(uint8_t * buffer, const size_t size)
  {
    return (orion_minor_send_result(object_, buffer, size));
  }

  orion_minor_t* getObject()
  {
    return object_;
  }

private:
  orion_minor_t * object_;  
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_MINOR_HPP
