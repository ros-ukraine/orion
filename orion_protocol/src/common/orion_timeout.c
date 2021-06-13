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

#define ORION_CLOCKS_PER_MICROSECOND (CLOCKS_PER_SEC / 1000000)
#define ORION_REVERSED_CLOCKS_PER_MICROSECOND (1000000 / CLOCKS_PER_SEC)

static uint32_t convert_ticks_to_microseconds(const long int value);
static long int convert_microseconds_to_ticks(const uint32_t value);

orion_timeout_error_t orion_timeout_init(orion_timeout_t * me, uint32_t timeout)
{
  ORION_ASSERT_NOT_NULL(me);
 
  clock_t time_now = clock();
  me->till_time_ = (clock_t) ((long int)(time_now) + convert_microseconds_to_ticks(timeout));
  return (ORION_TOT_ERROR_NONE);
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
  long int difference = (long int)me->till_time_ - (long int)time_now;
  if (difference > 0)
  {
	  result = convert_ticks_to_microseconds(difference);
  }
  return (result);
}

uint32_t convert_ticks_to_microseconds(const long int value)
{
  if (0 == ORION_REVERSED_CLOCKS_PER_MICROSECOND)
  {
    return ((uint32_t)(value / ORION_CLOCKS_PER_MICROSECOND));
  }
  return ((uint32_t)(value * ORION_REVERSED_CLOCKS_PER_MICROSECOND));
}

long int convert_microseconds_to_ticks(const uint32_t value)
{
  if (0 == ORION_REVERSED_CLOCKS_PER_MICROSECOND)
  {
    return ((long int)value * ORION_CLOCKS_PER_MICROSECOND);
  }
  return ((long int)value / ORION_REVERSED_CLOCKS_PER_MICROSECOND);
}
