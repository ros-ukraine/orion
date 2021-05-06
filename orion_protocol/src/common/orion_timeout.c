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

#include <time.h>
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_memory.h"
#include "orion_protocol/orion_assert.h"

#define ORION_CLOCKS_PER_MILLISECOND (CLOCKS_PER_SEC / 1000000)

struct orion_timeout_struct_t
{
    clock_t till_time_;
};


orion_error_t orion_timeout_new(orion_timeout_t ** me, uint32_t timeout)
{
  ORION_ASSERT_NOT_NULL(me);
  
  orion_error_t result = orion_memory_allocate(sizeof(orion_timeout_t), (void**)me);
  if (ORION_ERROR_OK == result)
  {
    clock_t time_now = clock();
    (*me)->till_time_ = (clock_t) (((long int)(time_now) / ORION_CLOCKS_PER_MILLISECOND) + timeout);
  }
  return (result);
}

orion_error_t orion_timeout_delete(const orion_timeout_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_error_t result = orion_memory_free((void*)me);
  return (result);
}

bool orion_timeout_has_time(const orion_timeout_t * me)
{
  ORION_ASSERT_NOT_NULL(me);

  if (orion_timeout_time_left(me) > 0)
  {
    return (true);
  }
  return (false);
}

uint32_t orion_timeout_time_left(const orion_timeout_t * me)
{
  ORION_ASSERT_NOT_NULL(me);

  uint32_t result = 0;
  clock_t time_now = clock();
  long int difference = (long int) me->till_time_ - (long int) time_now;
  if (difference > 0)
  {
    result = difference / ORION_CLOCKS_PER_MILLISECOND;
  }
  return (result);
}
