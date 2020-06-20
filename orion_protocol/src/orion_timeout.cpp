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

#include <chrono>
#include "orion_protocol/orion_timeout.h"

using std::chrono::microseconds;
using std::chrono::steady_clock;
using std::chrono::duration_cast;

namespace orion
{

Timeout::Timeout(uint32_t timeout)
{
  this->till_time_ = steady_clock::now() + microseconds(timeout);
}

bool Timeout::hasTime()
{
  return (0 != this->timeLeft());
}

uint32_t Timeout::timeLeft()
{
  steady_clock::time_point time_now = steady_clock::now();

  if (this->till_time_ > time_now)
  {
    auto result = duration_cast<microseconds>(this->till_time_ - time_now).count();
    return static_cast<uint32_t>(result);
  }
  return 0;
}

}  // orion
