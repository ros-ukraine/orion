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

#ifndef ORION_PROTOCOL_ORION_TIMEOUT_H
#define ORION_PROTOCOL_ORION_TIMEOUT_H

#include <stdint.h>
#include <stdbool.h>
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    ORION_TOT_ERROR_OK = 0
} orion_timeout_error_t;

typedef struct
{
    clock_t till_time_;
} orion_timeout_t;

/*
  @timeout - time in microseconds
*/
orion_timeout_error_t orion_timeout_init(orion_timeout_t * me, uint32_t timeout);

bool orion_timeout_has_time(const orion_timeout_t * me);
uint32_t orion_timeout_time_left(const orion_timeout_t * me);

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_TIMEOUT_H
