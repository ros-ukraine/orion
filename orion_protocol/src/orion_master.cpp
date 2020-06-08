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

#include "orion_protocol/orion_master.h"

namespace orion
{

template<class Command, class Result>
void Master::invoke(const Command &command, Result *result)
{
  this->invoke<Command, Result>(command, result, this->default_timeout, this->default_retry_count);
}

template<class Command, class Result>
void Master::invoke(const Command &command, Result *result, uint32_t retry_timeout)
{
  this->invoke<Command, Result>(command, result, retry_timeout, this->default_retry_count);
}

template<class Command, class Result>
void Master::invoke(const Command &command, Result *result, uint32_t retry_timeout, uint8_t retry_count)
{
  this->sendAndReceive(static_cast<uint8_t*>(&command), sizeof(command), this->default_timeout,
    this->default_retry_count, static_cast<uint8_t*>(&result), sizeof(result));
}

void Master::sendAndReceive(uint8_t *input_buffer, uint32_t input_size, uint32_t retry_timeout, uint8_t retry_count,
  uint8_t *output_buffer, uint32_t output_size)
{
  // TODO: Implement
}

}  // orion