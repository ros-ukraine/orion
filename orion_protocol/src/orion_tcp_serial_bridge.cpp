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

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <ros/assert.h>
#include "orion_protocol/orion_exception.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_tcp_serial_bridge.h"

namespace orion
{

TCPSerialBridge::TCPSerialBridge()
{
}

void TCPSerialBridge::connect(const char* hostname, uint32_t port)
{
  ROS_ASSERT(-1 == this->socket_descriptor_);
  this->socket_descriptor_ = socket(AF_INET, SOCK_STREAM, 0);

  exception::raiseIf<std::runtime_error>(
    (this->socket_descriptor_ < 0),
    "Could not create socket %s", strerror(errno));

  hostent *server = gethostbyname(hostname);
  exception::raiseIf<std::runtime_error>(
    (nullptr == server),
    "There is no host %s", hostname);

  sockaddr_in server_address;
  bzero(reinterpret_cast<char*>(&server_address), sizeof(server_address));
  server_address.sin_family = AF_INET;
  bcopy(reinterpret_cast<char*>(server->h_addr), reinterpret_cast<char*>(&server_address.sin_addr.s_addr),
    server->h_length);
  server_address.sin_port = htons(port);

  int status = ::connect(this->socket_descriptor_, reinterpret_cast<sockaddr*>(&server_address),
    sizeof(server_address));
  exception::raiseIf<std::runtime_error>(
    (status < 0),
    "Could not connect to %s:%d %s", hostname, port, strerror(errno));
}

void TCPSerialBridge::disconnect()
{
  if (-1 != this->socket_descriptor_)
  {
    exception::raiseIf<std::runtime_error>(
      (0 != close(this->socket_descriptor_)),
      "Error closing socket: %s", strerror(errno));

    this->socket_descriptor_ = -1;
  }
}

size_t TCPSerialBridge::receiveAvailableBuffer(uint8_t *buffer, uint32_t size)
{
  ROS_ASSERT(-1 != this->socket_descriptor_);

  fcntl(this->socket_descriptor_, F_SETFL, FNDELAY);
  int read_size = read(this->socket_descriptor_, buffer, size);

  fcntl(this->socket_descriptor_, F_SETFL, 0);

  exception::raiseIf<std::runtime_error>(
    (read_size < 0),
    "Error reading socket: %s", strerror(errno));

  return read_size;
}

size_t TCPSerialBridge::receiveBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
{
  ROS_ASSERT(-1 != this->socket_descriptor_);

  fd_set set;
  timeval duration;
  int status = -1;
  size_t read_size = 0;

  FD_ZERO(&set);
  FD_SET(this->socket_descriptor_, &set);

  duration.tv_sec = 0;
  duration.tv_usec = timeout;

  status = select(this->socket_descriptor_ + 1, &set, NULL, NULL, &duration);

  exception::raiseIf<std::runtime_error>(
    (-1 == status),
    "Error reading socket: %s", strerror(errno));

  if (0 != status)
  {
    read_size = this->receiveAvailableBuffer(buffer, size);
  }
  return read_size;
}

bool TCPSerialBridge::hasAvailableBuffer()
{
  ROS_ASSERT(-1 != this->socket_descriptor_);

  int bytes_available;
  ioctl(this->socket_descriptor_, FIONREAD, &bytes_available);
  if (bytes_available > 0)
  {
    return true;
  }
  return false;
}

bool TCPSerialBridge::sendBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
{
  // Current implementation is select based as PySerial implementation
  // Consider using tcdrain to send packs of bytes without blocking for long in case of bad throughput

  ROS_ASSERT(-1 != this->socket_descriptor_);

  Timeout duration(timeout);
  size_t bytes_to_send = size;
  int select_status = -1;
  uint32_t position = 0;

  fd_set set;
  timeval interval;

  while ((bytes_to_send > 0) && duration.hasTime())
  {
    ssize_t write_result = write(this->socket_descriptor_, buffer + position, bytes_to_send);
    exception::raiseIf<std::runtime_error>(
      (-1 == write_result),
      "Error writing to socket: %s", strerror(errno));

    bytes_to_send -= write_result;
    position += write_result;

    if ((bytes_to_send > 0) && duration.hasTime())
    {
      FD_ZERO(&set);
      FD_SET(this->socket_descriptor_, &set);

      interval.tv_sec = 0;
      interval.tv_usec = duration.timeLeft();

      select_status = select(this->socket_descriptor_ + 1, NULL, &set, NULL, &interval);
      exception::raiseIf<std::runtime_error>(
        (-1 == select_status),
        "Error writing to socket: %s", strerror(errno));
    }
  }

  if (0 == bytes_to_send)
  {
    return true;
  }
  return false;
}

}  // namespace orion
