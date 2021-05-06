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
#include "orion_protocol/orion_error.h"
#include "orion_protocol/orion_serial_port.h"
#include "orion_protocol/orion_timeout.h"
#include "orion_protocol/orion_memory.h"

struct orion_communication_struct_t
{
    int file_descriptor_;
};

struct orion_communication_settings_struct_t
{
    char * port_name;
    uint32_t baud;
};

static orion_error_t set_interface_attributes(const orion_communication_t * me, uint32_t speed);

orion_error_t orion_communication_new(orion_communication_t ** me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_error_t result = orion_memory_allocate(sizeof(orion_communication_t), (void**)me);
  if (ORION_ERROR_OK == result)
  {
    (*me)->file_descriptor_ = -1;
  }
  return (result);
}

orion_error_t orion_communication_delete(const orion_communication_t * me)
{
  ORION_ASSERT_NOT_NULL(me);
  orion_error_t result = orion_memory_free((void*)me);
  return (result);
}

orion_error_t orion_communication_connect(orion_communication_t * me, const char* port_name, const uint32_t baud)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT_NOT_NULL(port_name);
    ORION_ASSERT(-1 == me->file_descriptor_);

    me->file_descriptor_ = open(port_name, O_RDWR | O_NOCTTY | O_SYNC);

    if (me->file_descriptor_ < 0) 
    {
        return (ORION_ERROR_OPENNING_SERIAL_PORT);
    }

    orion_error_t result = set_interface_attributes(me, baud);
    return (result);
}

orion_error_t orion_communication_disconnect(orion_communication_t * me)
{
    ORION_ASSERT_NOT_NULL(me);

    orion_error_t result = ORION_ERROR_OK;
    if (-1 != me->file_descriptor_) 
    {
        if (0 != close(me->file_descriptor_)) 
        {
            return (ORION_ERROR_CLOSING_SERIAL_PORT);
        }
    }
    return (ORION_ERROR_OK);
}

orion_error_t orion_communication_receive_available_buffer(const orion_communication_t * me, uint8_t * buffer, 
  uint32_t size, size_t * received_size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

    fcntl(me->file_descriptor_, F_SETFL, FNDELAY);
    *received_size = read(me->file_descriptor_, buffer, size);

    fcntl(me->file_descriptor_, F_SETFL, 0);

    if (*received_size < 0)
    {
        return (ORION_ERROR_READING_SERIAL_PORT);
    }

    return (ORION_ERROR_OK);
}

orion_error_t orion_communication_receive_buffer(const orion_communication_t * me, uint8_t * buffer, uint32_t size,
  uint32_t timeout, size_t * received_size)
{
    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

    fd_set set;
    struct timeval duration;
    int status = -1;
    orion_error_t result = ORION_ERROR_OK;
    *received_size = 0;

    FD_ZERO(&set);
    FD_SET(me->file_descriptor_, &set);

    duration.tv_sec = 0;
    duration.tv_usec = timeout;

    status = select(me->file_descriptor_ + 1, &set, NULL, NULL, &duration);

    if (-1 == status)
    {
        result = ORION_ERROR_READING_SERIAL_PORT;
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
    ORION_ASSERT(-1 != me->file_descriptor_);

    int bytes_available;
    ioctl(me->file_descriptor_, FIONREAD, &bytes_available);
    if (bytes_available > 0)
    {
        return true;
    }
    return false;
}

orion_error_t orion_communication_send_buffer(const orion_communication_t * me, uint8_t *buffer, uint32_t size, 
  uint32_t timeout)
{
    // Current implementation is select based as PySerial implementation
    // Consider using tcdrain to send packs of bytes without blocking for long in case of bad throughput

    ORION_ASSERT_NOT_NULL(me);
    ORION_ASSERT(-1 != me->file_descriptor_);

//TODO: (Andriy) Implement Timeout functionality and use it here !!!
    // Timeout duration(timeout);
    // size_t bytes_to_send = size;
    // int select_status = -1;
    // uint32_t position = 0;

    // fd_set set;
    // timeval interval;

    // while ((bytes_to_send > 0) && duration.hasTime())
    // {
    // ssize_t write_result = write(this->file_descriptor_, buffer + position, bytes_to_send);
    // exception::raiseIf<std::runtime_error>(
    //     (-1 == write_result),
    //     "Error writing to serial port: %s", strerror(errno));

    // bytes_to_send -= write_result;
    // position += write_result;

    // if ((bytes_to_send > 0) && duration.hasTime())
    // {
    //     FD_ZERO(&set);
    //     FD_SET(this->file_descriptor_, &set);

    //     interval.tv_sec = 0;
    //     interval.tv_usec = duration.timeLeft();

    //     select_status = select(this->file_descriptor_ + 1, NULL, &set, NULL, &interval);
    //     exception::raiseIf<std::runtime_error>(
    //     (-1 == select_status),
    //     "Error writing to serial port: %s", strerror(errno));
    // }
    // }

    // if (0 == bytes_to_send)
    // {
    // return true;
    // }
    return false;
}

orion_error_t set_interface_attributes(const orion_communication_t *object, uint32_t speed)
{
    struct termios tty;

    ORION_ASSERT_NOT_NULL(object);

    if (tcgetattr(object->file_descriptor_, &tty) < 0) {
        return (ORION_ERROR_GETTING_TERMINAL_ATTRIBUTES);
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
        return (ORION_ERROR_SETTING_TERMINAL_ATTRIBUTES);
    }

    return (ORION_ERROR_OK);
}
