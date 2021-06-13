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

#include <stdint.h>
#include <stdbool.h>
#include "orion_protocol/orion_communication.h"
#include "orion_protocol/orion_buffered_io.h"
#include "orion_protocol/orion_assert.h"

struct orion_communication_struct_t
{
    uint8_t dummy_;
};


orion_communication_error_t orion_communication_new(orion_communication_t ** me)
{
    // No allocation of memory is needed
    ORION_ASSERT_NOT_NULL(me);
    me = (orion_communication_t**)(0x12345678);
    return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_delete(const orion_communication_t * me)
{
    return (ORION_COM_ERROR_NONE);
}

ssize_t orion_communication_receive_available_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(buffer);
    ORION_ASSERT(0 < size);

    ssize_t result = orion_buffered_io_read(buffer, size);
    return (result);
}

ssize_t orion_communication_receive_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size,
  uint32_t timeout)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(buffer);
    ORION_ASSERT(0 < size);

    // TODO(Andriy): Add logic to wait for timeout in case if no data is available
    ssize_t result = orion_communication_receive_available_buffer(me, buffer, size);
    return (result);
}

bool orion_communication_has_available_buffer(const orion_communication_t * me)
{
    ORION_ASSERT_NOT_NULL(me);
    bool result = true;
    if (orion_buffered_io_read_empty())
    {
        result = false;
    }
    return (result);
}

orion_communication_error_t orion_communication_send_buffer(const orion_communication_t * me, uint8_t *buffer, 
  uint32_t size, uint32_t timeout)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(buffer);
    ORION_ASSERT(0 < size);
    bool result = orion_buffered_io_write(buffer, size);
    if (result)
    {
        return (ORION_COM_ERROR_NONE);
    }
    return (ORION_COM_ERROR_UNKNOWN);
}
