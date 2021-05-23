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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include "orion_protocol/orion_assert.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_memory.h"
#include "orion_protocol/orion_tcp_serial_bridge.h"

struct orion_communication_struct_t
{
  int socket_descriptor_;
};

orion_communication_error_t orion_communication_new(orion_communication_t ** me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_memory_error_t status = orion_memory_allocate(sizeof(orion_communication_t), (void**)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_COM_ERROR_COULD_NOT_ALLOCATE_MEMORY);
  }
  (*me)->socket_descriptor_ = -1;
  return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_delete(const orion_communication_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_memory_error_t status = orion_memory_free((void*)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_COM_ERROR_COULD_NOT_FREE_MEMORY);
  }
  return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_connect(orion_communication_t * me, const char* hostname,
    const uint32_t port)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT_NOT_NULL(hostname);
  ORION_ASSERT(-1 == me->socket_descriptor_);

  me->socket_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);

  if (me->socket_descriptor_ < 0)
  {
    return (ORION_COM_ERROR_CREATE_SOCKET);
  }

  struct hostent *server = gethostbyname(hostname);
  if (NULL == server)
  {
    return (ORION_COM_ERROR_COULD_NOT_FIND_HOST);
  }

  struct sockaddr_in server_address;
  bzero((char*)(&server_address), sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy((char*)(server->h_addr), (char*)(&server_address.sin_addr.s_addr), server->h_length);
  server_address.sin_port = htons(port);

  int status = connect(me->socket_descriptor_, (struct sockaddr*)(&server_address), sizeof(server_address));
  if (status < 0)
  {
    return (ORION_COM_ERROR_COULD_NOT_CONNECT_TO_HOST);
  }
  return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_disconnect(orion_communication_t * me)
{
  ORION_ASSERT_NOT_NULL(me);

  orion_communication_error_t result = ORION_COM_ERROR_NONE;
  if (-1 != me->socket_descriptor_)
  {
      if (0 != close(me->socket_descriptor_))
      {
          return (ORION_COM_ERROR_CLOSING_SOCKET);
      }
  }
  me->socket_descriptor_ = -1;

  return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_receive_available_buffer(const orion_communication_t * me,
  uint8_t * buffer, uint32_t size, size_t * received_size)
{

  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT(-1 != me->socket_descriptor_);

  fcntl(me->socket_descriptor_, F_SETFL, FNDELAY);
  *received_size = read(me->socket_descriptor_, buffer, size);

  fcntl(me->socket_descriptor_, F_SETFL, 0);

  if (*received_size < 0)
  {
      return (ORION_COM_ERROR_READING_SOCKET);
  }

  return (ORION_COM_ERROR_NONE);
}

orion_communication_error_t orion_communication_receive_buffer(const orion_communication_t * me, uint8_t * buffer,
  uint32_t size, uint32_t timeout, size_t * received_size)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT(-1 != me->socket_descriptor_);

  orion_communication_error_t result = ORION_COM_ERROR_NONE;

  fd_set set;
  struct timeval duration;
  int status = -1;
  *received_size = 0;

  FD_ZERO(&set);
  FD_SET(me->socket_descriptor_, &set);

  duration.tv_sec = 0;
  duration.tv_usec = timeout;

  status = select(me->socket_descriptor_ + 1, &set, NULL, NULL, &duration);

  if (-1 == status)
  {
    result = ORION_COM_ERROR_READING_SOCKET;
  }
  else if (0 != status)
  {
    result = orion_communication_receive_available_buffer(me, buffer, size, received_size);
  }
  return (result);
}

bool orion_communication_has_available_buffer(const orion_communication_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT(-1 != me->socket_descriptor_);

  int bytes_available;
  ioctl(me->socket_descriptor_, FIONREAD, &bytes_available);
  if (bytes_available > 0)
  {
    return true;
  }
  return false;
}

orion_communication_error_t orion_communication_send_buffer(const orion_communication_t * me, uint8_t *buffer,
  uint32_t size, uint32_t timeout)
{
  // Current implementation is select based as PySerial implementation
  // Consider using tcdrain to send packs of bytes without blocking for long in case of bad throughput

  ORION_ASSERT_NOT_NULL(me);
  ORION_ASSERT(-1 != me->socket_descriptor_);

  orion_communication_error_t result = ORION_COM_ERROR_NONE;

  orion_timeout_t duration;
  orion_timeout_init(&duration, timeout);
  size_t bytes_to_send = size;
  int select_status = -1;
  uint32_t position = 0;

  fd_set set;
  struct timeval interval;

  while ((bytes_to_send > 0) && orion_timeout_has_time(&duration))
  {
    ssize_t write_result = write(me->socket_descriptor_, buffer + position, bytes_to_send);
    if (-1 == write_result)
    {
        result = ORION_COM_ERROR_WRITING_TO_SOCKET;
        break;
    }

    bytes_to_send -= write_result;
    position += write_result;

    if ((bytes_to_send > 0) && orion_timeout_has_time(&duration))
    {
      FD_ZERO(&set);
      FD_SET(me->socket_descriptor_, &set);

      interval.tv_sec = 0;
      interval.tv_usec = orion_timeout_time_left(&duration);

      select_status = select(me->socket_descriptor_ + 1, NULL, &set, NULL, &interval);
      if (-1 == select_status)
      {
          result = ORION_COM_ERROR_WRITING_TO_SOCKET;
          break;
      }
    }
  }

  if ((ORION_COM_ERROR_NONE == result) && (bytes_to_send > 0))
  {
      result = ORION_COM_ERROR_TIMEOUT;
  }

  return (result);
}
