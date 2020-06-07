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

  Master(uint32_t retry_timeout, uint8_t retry_count):default_timeout(retry_timeout),
    default_retry_count(retry_count) {};

  template<class Command, class Result>  Result invoke(const Command &command);

  template<class Command, class Result>  Result invoke(const Command &command, uint32_t retry_timeout);

  template<class Command, class Result> Result invoke(const Command &command, uint32_t retry_timeout,
    uint8_t retry_count);

private:

  void sendAndReceive(uint8_t *input_buffer, uint32_t input_size, uint32_t retry_timeout, uint8_t retry_count,
    uint8_t *output_buffer, uint32_t output_size);

  uint32_t default_timeout = 100000; // microseconds
  uint8_t default_retry_count = 1;

};

}  // orion

#endif  // MOON_HARDWARE_ROBOT_HARDWARE_H