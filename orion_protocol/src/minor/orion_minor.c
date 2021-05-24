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

#include "orion_protocol/orion_minor.h"
#include "orion_protocol/orion_assert.h"
#include "orion_protocol/orion_memory.h"
#include <stdint.h>
#include <stdbool.h>

struct orion_minor_struct_t
{
  orion_transport_t * transport_;
};

orion_minor_error_t orion_minor_new(orion_minor_t ** me, orion_transport_t * transport)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT_NOT_NULL(transport);
  orion_memory_error_t status = orion_memory_allocate(sizeof(orion_minor_t), (void**)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_MINOR_ERROR_COULD_NOT_ALLOCATE_MEMORY);
  }
  (*me)->transport_ = transport;
  return (ORION_MINOR_ERROR_NONE);
}

orion_minor_error_t orion_minor_delete(const orion_minor_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_memory_error_t status = orion_memory_free((void*)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_MINOR_ERROR_COULD_NOT_FREE_MEMORY);
  }
  return (ORION_MINOR_ERROR_NONE);
}

ssize_t orion_minor_wait_and_receive_command(const orion_minor_t * me, uint8_t * buffer, size_t buffer_size,
  uint32_t timeout)
{
  // TODO(Andriy): Implement
  ORION_ASSERT(false);
  return (ORION_MINOR_ERROR_UNKNOWN);
}

ssize_t orion_minor_receive_command(const orion_minor_t * me, uint8_t * buffer, size_t buffer_size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(buffer);
    ORION_ASSERT(0 < buffer_size);

    ssize_t result = 0;
    if (orion_transport_has_received_packet(me->transport_))
    {
        size_t received_size = 0;
        orion_transport_error_t status = orion_transport_receive_packet(me->transport_, buffer, buffer_size, 0,
            &received_size);
        if (ORION_TRAN_ERROR_NONE != status)
        {
            result = ORION_MINOR_ERROR_RECEIVING_PACKET;

        }
        else
        {
            result = received_size;
        }
    }
    return (result);
}

orion_minor_error_t orion_minor_send_result(const orion_minor_t * me, uint8_t * buffer, const size_t size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(buffer);
    ORION_ASSERT(0 < size);

    orion_transport_error_t status = orion_transport_send_packet(me->transport_, buffer, size, 0);

    if (ORION_TRAN_ERROR_NONE == status)
    {
        return (ORION_MINOR_ERROR_NONE);
    }
    return (ORION_MINOR_ERROR_SENDING_PACKET);
}
