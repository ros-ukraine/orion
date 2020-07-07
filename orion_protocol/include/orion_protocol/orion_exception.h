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

#ifndef ORION_PROTOCOL_ORION_EXCEPTION_H
#define ORION_PROTOCOL_ORION_EXCEPTION_H

#include <utility>
#include <string>
#include <cstdio>

namespace orion
{

namespace exception
{

template <class ExceptionType, typename ...Args>
void raise(const char* format, Args && ...args)
{
    auto size = std::snprintf(nullptr, 0, format, std::forward<Args>(args)...);
    std::string message(size + 1, '\0');
    std::snprintf(&message[0], size + 1, format, std::forward<Args>(args)...);
    throw ExceptionType(message);
}

template <class ExceptionType, typename ...Args>
void raiseIf(bool condition, const char* format, Args && ...args)
{
  if (condition)
  {
    raise<ExceptionType>(format, std::forward<Args>(args)...);
  }
}

}  // namespace exception

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_EXCEPTION_H
