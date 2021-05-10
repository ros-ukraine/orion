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

#include <stdlib.h>
#include "orion_protocol/orion_memory.h"
#include "orion_protocol/orion_assert.h"

orion_memory_error_t orion_memory_allocate(size_t size, void ** pointer)
{
    ORION_ASSERT(size > 0);

    *pointer = malloc(size);

    if (NULL == *pointer)
    {
        return (ORION_MEM_ERROR_COULD_NOT_ALLOCATE_MEMORY);
    }

    return (ORION_MEM_ERROR_OK); 
}

orion_memory_error_t orion_memory_free(void * pointer)
{
    free(pointer);
    return (ORION_MEM_ERROR_OK);
}
