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

#include "orion_protocol/orion_circular_buffer.h"
#include <assert.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

static bool find_byte(const orion_circular_buffer_t * p_this, uint32_t start_index, uint8_t value, uint32_t * p_index);

static bool find_word(const orion_circular_buffer_t * p_this, uint32_t * p_start_index, uint32_t * p_end_index, 
    uint8_t delimiter);

void orion_circular_buffer_init(orion_circular_buffer_t * p_this, uint8_t * p_buffer, uint32_t size)
{
    assert(NULL != p_this);
    assert(NULL != p_buffer);
    assert(0 < size);

    p_this->p_buffer = p_buffer;
    p_this->buffer_size = size;
    p_this->head_index = 0;
    p_this->tail_index = 0;
    p_this->is_full = false;
}

bool orion_circular_buffer_is_empty(const orion_circular_buffer_t * p_this)
{
    assert(NULL != p_this);
    bool result = false;
    if (!p_this->is_full)
    {
        result = (p_this->head_index == p_this->tail_index);
    }
    return (result);
}

void orion_circular_buffer_add(orion_circular_buffer_t * p_this, const uint8_t * p_buffer, uint32_t size)
{
    assert(NULL != p_this);
    assert(NULL != p_this->p_buffer);
    assert(0 != p_this->buffer_size);
    assert(NULL != p_buffer);
    assert(0 != size);
    assert(false == p_this->is_full);

    if (p_this->tail_index >= p_this->head_index)
    {
        assert((p_this->buffer_size - (p_this->tail_index - p_this->head_index)) >= size);
        if ((p_this->tail_index + size) < p_this->buffer_size)
        {
            memcpy(&p_this->p_buffer[p_this->tail_index], &p_buffer[0], size);
            p_this->tail_index += size;
        }
        else
        {
            uint32_t remaning_size = p_this->buffer_size - p_this->tail_index;
            memcpy(&p_this->p_buffer[p_this->tail_index], &p_buffer[0], remaning_size);
            memcpy(&p_this->p_buffer[0], &p_buffer[remaning_size], size - remaning_size);
            p_this->tail_index = size - remaning_size;
        }
    }
    else
    {
        assert((p_this->head_index - p_this->tail_index) >= size);
        memcpy(&p_this->p_buffer[p_this->tail_index], &p_buffer[0], size);
        p_this->tail_index += size;
    }
    if (p_this->tail_index == p_this->head_index)
    {
        p_this->is_full = true;
    }
}

uint32_t orion_circular_buffer_dequeue(orion_circular_buffer_t * p_this, uint8_t * p_buffer, uint32_t size)
{
    assert(NULL != p_this);
    assert(NULL != p_this->p_buffer);
    assert(0 != p_this->buffer_size);
    assert(NULL != p_buffer);
    assert(0 != size);

    uint32_t result = 0;

    if ((p_this->head_index == p_this->tail_index) && !p_this->is_full)
    {
        return (result);
    }
    if (p_this->head_index < p_this->tail_index)
    {
        result = p_this->tail_index - p_this->head_index;
        if (result > size)
        {
            result = size;
        }
        memcpy(&p_buffer[0], &p_this->p_buffer[p_this->head_index], result);
        p_this->head_index += result;
    }
    else
    {
        if ((p_this->buffer_size - p_this->head_index) > size)
        {
            result = size;
            memcpy(&p_buffer[0], &p_this->p_buffer[p_this->head_index], result);
            p_this->head_index += result;
        }
        else
        {
            result = p_this->buffer_size - p_this->head_index;
            memcpy(&p_buffer[0], &p_this->p_buffer[p_this->head_index], result);

            if (p_this->tail_index > (size - result))
            {
                memcpy(&p_buffer[result], &p_this->p_buffer[0], size - result);
                p_this->head_index = size - result;
                result = size;
            }
            else
            {
                memcpy(&p_buffer[result], &p_this->p_buffer[0], p_this->tail_index);
                p_this->head_index = p_this->tail_index;
                result += p_this->tail_index;
            }
        }
        p_this->is_full = false;
    }
    return (result);
}

bool find_byte(const orion_circular_buffer_t * p_this, uint32_t start_index, uint8_t value, uint32_t * p_index)
{
    assert(NULL != p_this);
    assert(NULL != p_this->p_buffer);
    //TODO(Andriy): Add asserts for indeces

    bool result = false;
    uint32_t counter = start_index % p_this->buffer_size;

    if ((counter == p_this->tail_index) && p_this->is_full)
    {
        if (p_this->p_buffer[counter] == value)
        {
            result = true;
            *p_index = counter;
        }
        else
        {
            counter = (counter + 1) % p_this->buffer_size;
        }
    }

    while ((counter != p_this->tail_index) && !result)
    {
        if (p_this->p_buffer[counter] == value)
        {
            result = true;
            *p_index = counter;
        }
        else
        {
            counter = (counter + 1) % p_this->buffer_size;
        }
    }
    
    return (result);
}


bool find_word(const orion_circular_buffer_t * p_this, uint32_t * p_start_index, uint32_t * p_end_index, 
    uint8_t delimiter)
{
    assert(NULL != p_this);
    assert(NULL != p_this->p_buffer);
    assert(0 != p_this->buffer_size);

    //TODO(Andriy): Add asserts on indeces

    bool result = false;

    if (orion_circular_buffer_is_empty(p_this))
    {
        return (result);
    }

    bool found = find_byte(p_this, p_this->head_index, delimiter, p_start_index);
    *p_end_index = *p_start_index;

    while (found && (*p_end_index - *p_start_index < 2))
    {
        *p_start_index = *p_end_index;
        found = find_byte(p_this, *p_start_index + 1, delimiter, p_end_index);
    }

    if (found && (*p_end_index - *p_start_index < 2))
    {
        result = true;
    }

    return (result);
}

bool orion_circular_buffer_has_word(orion_circular_buffer_t * p_this, uint8_t delimeter)
{
    uint32_t start_index = 0;
    uint32_t end_index = 0;
    bool result = find_word(p_this, &start_index, &end_index, delimeter);
    return (result);
}

bool orion_circular_buffer_dequeu_word(orion_circular_buffer_t * p_this, uint8_t delimeter, uint8_t * p_buffer, 
    uint32_t size, uint32_t * p_actual_size)
{
    uint32_t start_index = 0;
    uint32_t end_index = 0;
    bool result = find_word(p_this, &start_index, &end_index, delimeter); 
    if (result)
    {
        uint32_t word_size = 0;
        if (start_index < end_index)
        {
            word_size = end_index - start_index + 1;
        }
        else
        {
            word_size = p_this->buffer_size - start_index + end_index + 1;
        }
        assert(size >= word_size);
        
        p_this->head_index = start_index;
        *p_actual_size = orion_circular_buffer_dequeue(p_this, p_buffer, word_size); 
        assert(*p_actual_size == word_size);
    }
    return (result);
}
