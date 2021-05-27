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
#include <fcntl.h>
#include <termios.h>
#include <unistd.h>
#include "orion_protocol/orion_assert.h"
#include "orion_protocol/orion_serial_port.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_memory.h"

struct orion_communication_struct_t
{
    int file_descriptor_;
};

static orion_communication_error_t set_interface_attributes(const orion_communication_t * me, uint32_t speed);

orion_communication_error_t orion_communication_new(orion_communication_t ** me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_memory_error_t status = orion_memory_allocate(sizeof(orion_communication_t), (void**)me);
  if (ORION_MEM_ERROR_NONE != status)
  {
      return (ORION_COM_ERROR_COULD_NOT_ALLOCATE_MEMORY);
  }
  (*me)->file_descriptor_ = -1;
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

orion_communication_error_t orion_communication_connect(orion_communication_t * me, const char* port_name, 
    const uint32_t baud)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(port_name);
    ORION_ASSERT(-1 == me->file_descriptor_);

    me->file_descriptor_ = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);

    if (me->file_descriptor_ < 0) 
    {
        return (ORION_COM_ERROR_OPENNING_SERIAL_PORT);
    }

    orion_communication_error_t result = set_interface_attributes(me, baud);
    return (result);
}

orion_communication_error_t orion_communication_disconnect(orion_communication_t * me)
{
    ORION_ASSERT_NOT_NULL(me);

    orion_communication_error_t result = ORION_COM_ERROR_NONE;
    if (-1 != me->file_descriptor_) 
    {
        if (0 != close(me->file_descriptor_)) 
        {
            return (ORION_COM_ERROR_CLOSING_SERIAL_PORT);
        }
    }
    me->file_descriptor_ = -1;
    return (ORION_COM_ERROR_NONE);
}

ssize_t orion_communication_receive_available_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

    ssize_t result = ORION_COM_ERROR_UNKNOWN;

    fcntl(me->file_descriptor_, F_SETFL, FNDELAY);
    result = read(me->file_descriptor_, buffer, size);

    fcntl(me->file_descriptor_, F_SETFL, 0);

    if (result < 0)
    {
        result = ORION_COM_ERROR_READING_SERIAL_PORT;
    }

    return (result);
}

ssize_t orion_communication_receive_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size,
  uint32_t timeout)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

    fd_set set;
    struct timeval duration;
    int status = -1;
    ssize_t result = ORION_COM_ERROR_UNKNOWN;

    FD_ZERO(&set);
    FD_SET(me->file_descriptor_, &set);

    duration.tv_sec = 0;
    duration.tv_usec = timeout;

    status = select(me->file_descriptor_ + 1, &set, NULL, NULL, &duration);

    if (-1 == status)
    {
        result = ORION_COM_ERROR_READING_SERIAL_PORT;
    }
    else if (0 != status)
    {
        result = orion_communication_receive_available_buffer(me, buffer, size);
    }
    return (result);
}

bool orion_communication_has_available_buffer(const orion_communication_t * me)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

    int bytes_available;
    ioctl(me->file_descriptor_, FIONREAD, &bytes_available);
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
    ORION_ASSERT(-1 != me->file_descriptor_);

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
        ssize_t write_result = write(me->file_descriptor_, buffer + position, bytes_to_send);
        if (-1 == write_result)
        {
            result = ORION_COM_ERROR_WRITING_TO_SERIAL_PORT;
            break;
        }

        bytes_to_send -= write_result;
        position += write_result;

        if ((bytes_to_send > 0) && orion_timeout_has_time(&duration))
        {
            FD_ZERO(&set);
            FD_SET(me->file_descriptor_, &set);

            interval.tv_sec = 0;
            interval.tv_usec = orion_timeout_time_left(&duration);

            select_status = select(me->file_descriptor_ + 1, NULL, &set, NULL, &interval);
            if (-1 == select_status)
            {
                result = ORION_COM_ERROR_WRITING_TO_SERIAL_PORT;
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

orion_communication_error_t set_interface_attributes(const orion_communication_t *object, uint32_t speed)
{
    struct termios tty;

    ORION_ASSERT_NOT_NULL(object);

    if (tcgetattr(object->file_descriptor_, &tty) < 0) {
        return (ORION_COM_ERROR_GETTING_TERMINAL_ATTRIBUTES);
    }

    cfsetospeed(&tty, (speed_t)speed);
    cfsetispeed(&tty, (speed_t)speed);

    tty.c_cflag |= (CLOCAL | CREAD);  // ignore modem controls
    tty.c_cflag &= ~CSIZE;
    tty.c_cflag |= CS8;  // 8-bit characters
    tty.c_cflag &= ~PARENB;  // no parity bit
    tty.c_cflag &= ~CSTOPB;  // only need 1 stop bit
    tty.c_cflag &= ~CRTSCTS;  // no hardware flowcontrol

    // setup for non-canonical mode
    tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
    tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
    tty.c_oflag &= ~OPOST;

    // fetch bytes as they become available
    tty.c_cc[VMIN] = 1;
    tty.c_cc[VTIME] = 100;

    if (0 != tcsetattr(object->file_descriptor_, TCSANOW, &tty)) 
    {
        return (ORION_COM_ERROR_SETTING_TERMINAL_ATTRIBUTES);
    }

    return (ORION_COM_ERROR_NONE);
}
