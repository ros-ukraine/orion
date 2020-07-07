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
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include <ros/assert.h>
#include "orion_protocol/orion_serial_port.h"
#include "orion_protocol/orion_exception.h"
#include "orion_protocol/orion_timeout.h"

namespace orion
{

SerialPort::SerialPort()
{
}

void SerialPort::setInterfaceAttributes(uint32_t speed)
{
    termios tty;

    exception::raiseIf<std::runtime_error>(
      (tcgetattr(this->file_descriptor_, &tty) < 0),
      "Error getting terminal attributes: %s", strerror(errno));

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD); // ignore modem controls
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8; // 8-bit characters
    tty.c_cflag &= ~PARENB; // no parity bit
    tty.c_cflag &= ~CSTOPB; // only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS; // no hardware flowcontrol

    // setup for non-canonical mode
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    // fetch bytes as they become available
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 100;

    exception::raiseIf<std::runtime_error>(
      (0 != tcsetattr(this->file_descriptor_, TCSANOW, &tty)),
      "Error setting terminal attributes: %s", strerror(errno));
}

void SerialPort::connect(const char* port_name, uint32_t baud)
{
  ROS_ASSERT(-1 == this->file_descriptor_);
  this->file_descriptor_ = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);

  exception::raiseIf<std::runtime_error>(
    (this->file_descriptor_ < 0),
    "Error opening %s: %s", port_name, strerror(errno));

  this->setInterfaceAttributes(baud);
}

void SerialPort::disconnect()
{
  if (-1 != this->file_descriptor_)
  {
    exception::raiseIf<std::runtime_error>(
      (0 != close(this->file_descriptor_)),
      "Error closing serial port: %s", strerror(errno));

    this->file_descriptor_ = -1;
  }
}

size_t SerialPort::receiveAvailableBuffer(uint8_t *buffer, uint32_t size)
{
  ROS_ASSERT(-1 != this->file_descriptor_);

  fcntl(this->file_descriptor_, F_SETFL, FNDELAY);
  int read_size = read(this->file_descriptor_, buffer, size);

  fcntl(this->file_descriptor_, F_SETFL, 0);

  exception::raiseIf<std::runtime_error>(
    (read_size < 0),
    "Error reading serial port: %s", strerror(errno));

  return read_size;
}

size_t SerialPort::receiveBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
{
  ROS_ASSERT(-1 != this->file_descriptor_);

  fd_set set;
  timeval duration;
  int status = -1;
  size_t read_size = 0;

  FD_ZERO(&set);
  FD_SET(this->file_descriptor_, &set);

  duration.tv_sec = 0;
  duration.tv_usec = timeout;

  status = select(this->file_descriptor_ + 1, &set, NULL, NULL, &duration);

  exception::raiseIf<std::runtime_error>(
    (-1 == status),
    "Error reading serial port: %s", strerror(errno));

  if (0 != status)
  {
    read_size = this->receiveAvailableBuffer(buffer, size);
  }
  return read_size;
}

bool SerialPort::hasAvailableBuffer()
{
  ROS_ASSERT(-1 != this->file_descriptor_);

  int bytes_available;
  ioctl(this->file_descriptor_, FIONREAD, &bytes_available);
  if (bytes_available > 0)
  {
    return true;
  }
  return false;

}

bool SerialPort::sendBuffer(uint8_t *buffer, uint32_t size, uint32_t timeout)
{
  // Current implementation is select based as PySerial implementation
  // Consider using tcdrain to send packs of bytes without blocking for long in case of bad throughput

  ROS_ASSERT(-1 != this->file_descriptor_);

  Timeout duration(timeout);
  size_t bytes_to_send = size;
  int select_status = -1;
  uint32_t position = 0;

  fd_set set;
  timeval interval;

  while ((bytes_to_send > 0) && duration.hasTime())
  {
    ssize_t write_result = write(this->file_descriptor_, buffer + position, bytes_to_send);
    exception::raiseIf<std::runtime_error>(
      (-1 == write_result),
      "Error writing to serial port: %s", strerror(errno));

    bytes_to_send -= write_result;
    position += write_result;

    if ((bytes_to_send > 0) && duration.hasTime())
    {
      FD_ZERO(&set);
      FD_SET(this->file_descriptor_, &set);

      interval.tv_sec = 0;
      interval.tv_usec = duration.timeLeft();

      select_status = select(this->file_descriptor_ + 1, NULL, &set, NULL, &interval);
      exception::raiseIf<std::runtime_error>(
        (-1 == select_status),
        "Error writing to serial port: %s", strerror(errno));
    }
  }

  if (0 == bytes_to_send)
  {
    return true;
  }
  return false;
}

}  // namespace orion
