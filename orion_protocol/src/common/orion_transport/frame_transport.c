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

#include "orion_protocol/orion_assert.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_framer.h"
#include "orion_protocol/orion_crc.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_transport.h"
#include "orion_protocol/orion_circular_buffer.h"
#include "orion_protocol/orion_memory.h"

#define ORION_FRAME_TRANSPORT_BUFFER_SIZE (512)
#define ORION_FRAME_TRANSPORT_QUEUE_BUFFER_SIZE (1024)

struct orion_transport_struct_t
{
  orion_communication_t * communication_;
  uint8_t buffer_[ORION_FRAME_TRANSPORT_BUFFER_SIZE];
  uint8_t queue_buffer_[ORION_FRAME_TRANSPORT_QUEUE_BUFFER_SIZE];
  orion_circular_buffer_t circular_queue_;
};

static bool orion_transport_has_frame_in_queue(const orion_transport_t * me);

orion_transport_error_t orion_transport_new(orion_transport_t ** me, orion_communication_t * communication)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT_NOT_NULL(communication);
  orion_memory_error_t status = orion_memory_allocate(sizeof(orion_transport_t), (void**)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_TRAN_ERROR_COULD_NOT_ALLOCATE_MEMORY);
  }
  (*me)->communication_ = communication;
  orion_circular_buffer_init(&((*me)->circular_queue_), (*me)->queue_buffer_, ORION_FRAME_TRANSPORT_QUEUE_BUFFER_SIZE);
  return (ORION_TRAN_ERROR_NONE);
}

orion_transport_error_t orion_transport_delete(const orion_transport_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_memory_error_t status = orion_memory_free((void*)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_TRAN_ERROR_COULD_NOT_FREE_MEMORY);
  }
  return (ORION_TRAN_ERROR_NONE);
}

orion_transport_error_t orion_transport_send_packet(orion_transport_t * me, uint8_t *input_buffer,
    uint32_t input_size, uint32_t timeout)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT(input_size >= sizeof(orion_frame_header_t));

  orion_timeout_t duration;
  orion_timeout_init(&duration, timeout);

  orion_frame_header_t *frame_header = (orion_frame_header_t*)input_buffer;
  frame_header->crc = orion_crc_calculate_crc16(input_buffer + sizeof(orion_frame_header_t),
    input_size - sizeof(orion_frame_header_t));
  ssize_t packet_size = orion_framer_encode_packet(input_buffer, input_size, me->buffer_, 
    ORION_FRAME_TRANSPORT_BUFFER_SIZE);
  if (packet_size >= 0)
  {
    orion_communication_error_t send_status = orion_communication_send_buffer(me->communication_, me->buffer_, 
      packet_size, orion_timeout_time_left(&duration));
    if (ORION_COM_ERROR_NONE == send_status)
    {
      return (ORION_TRAN_ERROR_NONE);
    }
    return (ORION_TRAN_ERROR_FAILED_TO_SEND_PACKET);
  }
  return (ORION_TRAN_ERROR_FAILED_TO_ENCODE_PACKET);
}

ssize_t orion_transport_receive_packet(orion_transport_t * me, uint8_t * output_buffer, uint32_t output_size,
  uint32_t timeout)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_timeout_t duration;
  orion_timeout_init(&duration, timeout);

  ssize_t result = ORION_TRAN_ERROR_UNKNOWN;
  bool decode = orion_transport_has_received_packet(me);
  if (false == decode)
  {
    ssize_t size = orion_communication_receive_buffer(me->communication_, me->buffer_, 
      ORION_FRAME_TRANSPORT_BUFFER_SIZE, orion_timeout_time_left(&duration) * 3 / 4); 
    if (size > 0)
    {
      orion_circular_buffer_add(&(me->circular_queue_), me->buffer_, size);

      if (orion_transport_has_frame_in_queue(me))
      {
        decode = true;
      }
    }
  }

  if (decode)
  {
    uint32_t size = 0;
    bool status = orion_circular_buffer_dequeue_word(&(me->circular_queue_), ORION_FRAMER_FRAME_DELIMETER, me->buffer_,
      ORION_FRAME_TRANSPORT_BUFFER_SIZE, &size);
    ORION_ASSERT(status);
    result = orion_framer_decode_packet(me->buffer_, size, output_buffer, output_size);
    if (result < 0)
    {
      result = ORION_TRAN_ERROR_FAILED_TO_DECODE_PACKET;
    }
    else if (result < sizeof(orion_frame_header_t))
    {
      result = ORION_TRAN_ERROR_FAILED_TO_RECEIVE_FULL_PACKET;
    }
    else
    {
      orion_frame_header_t *frame_header = (orion_frame_header_t*)output_buffer;
      if (orion_crc_calculate_crc16(output_buffer + sizeof(orion_frame_header_t), 
        result - sizeof(orion_frame_header_t)) != frame_header->crc)
      {
        result = ORION_TRAN_ERROR_CRC_CHECK_FAILED;
      }
    }
  }
  return (result);
}

bool orion_transport_has_received_packet(orion_transport_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  bool result = false;

  if (orion_transport_has_frame_in_queue(me))
  {
    result = true;
  }
  else if (orion_communication_has_available_buffer(me->communication_))
  {
    ssize_t received_size = orion_communication_receive_available_buffer(me->communication_, me->buffer_,
      ORION_FRAME_TRANSPORT_BUFFER_SIZE);
    if (received_size >= 0)
    {
      orion_circular_buffer_add(&(me->circular_queue_), me->buffer_, received_size);
      if (orion_transport_has_frame_in_queue(me))
      {
        result = true;
      }
    }
  }
  return (result);
}

bool orion_transport_has_frame_in_queue(const orion_transport_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  bool result = orion_circular_buffer_has_word(&(me->circular_queue_), ORION_FRAMER_FRAME_DELIMETER);
  return (result);
}
