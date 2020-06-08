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

#ifndef ORION_PROTOCOL_ORION_MASTER_H
#define ORION_PROTOCOL_ORION_MASTER_H

#include <stdint.h>

namespace orion
{

class Master
{
public:
  Master() {};

  Master(uint32_t retry_timeout, uint8_t retry_count):default_timeout_(retry_timeout),
    default_retry_count_(retry_count) {};

  template<class Command, class Result>
  void invoke(const Command &command, Result *result)
  {
    this->invoke<Command, Result>(command, result, this->default_timeout_, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(const Command &command, Result *result, uint32_t retry_timeout)
  {
    this->invoke<Command, Result>(command, result, retry_timeout, this->default_retry_count_);
  }

  template<class Command, class Result>
  void invoke(const Command &command, Result *result, uint32_t retry_timeout, uint8_t retry_count)
  {
    this->sendAndReceive(reinterpret_cast<const uint8_t*>(&command), sizeof(command), retry_timeout, retry_count,
      reinterpret_cast<uint8_t*>(result), sizeof(*result));
  }

  static const uint32_t ONE_SECOND_TIMEOUT = 1000000;

private:

  void sendAndReceive(const uint8_t *input_buffer, uint32_t input_size, uint32_t retry_timeout, uint8_t retry_count,
    uint8_t *output_buffer, uint32_t output_size);

  uint32_t default_timeout_ = 100000; // microseconds
  uint8_t default_retry_count_ = 1;

};

}  // orion

#endif  // MOON_HARDWARE_ROBOT_HARDWARE_H