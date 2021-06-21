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

#ifndef ORION_PROTOCOL_ORION_CIRCULAR_BUFFER_H_
#define ORION_PROTOCOL_ORION_CIRCULAR_BUFFER_H_

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct
{
  uint8_t * p_buffer;
  uint32_t buffer_size;
  uint32_t head_index;
  uint32_t tail_index;
  bool is_full;
}
orion_circular_buffer_t;

void orion_circular_buffer_init(orion_circular_buffer_t * p_this, uint8_t * p_buffer, uint32_t size);

bool orion_circular_buffer_is_empty(const orion_circular_buffer_t * p_this);

void orion_circular_buffer_add(orion_circular_buffer_t * p_this, const uint8_t * p_buffer, uint32_t size);

uint32_t orion_circular_buffer_dequeue(orion_circular_buffer_t * p_this, uint8_t * p_buffer, uint32_t size);

bool orion_circular_buffer_has_word(const orion_circular_buffer_t * p_this, uint8_t delimeter);

bool orion_circular_buffer_dequeue_word(orion_circular_buffer_t * p_this, uint8_t delimeter, uint8_t * p_buffer,
    uint32_t size, uint32_t * p_actual_size);

#ifdef __cplusplus
}
#endif

#endif /* ORION_PROTOCOL_ORION_CIRCULAR_BUFFER_H_ */
