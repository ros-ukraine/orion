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

#ifndef ORION_PROTOCOL_ORION_MINOR_H
#define ORION_PROTOCOL_ORION_MINOR_H

#include <stdint.h>
#include <stdbool.h>

#include "main.h"
#include "carmen_hardware/protocol.h"
#include "orion_protocol/orion_header.h"
#include <string.h>

namespace orion
{

class Minor
{
public:
  Minor() {}

  bool receiveCommand(uint8_t * buffer, size_t buffer_size, uint32_t timeout, size_t &size_received)
  {
      flag = !flag;
      if (flag)
      {
          carmen_hardware::SetCommandsCommand command;
          command.header.common.sequence_id = 10001;
          command.left_cmd = 200;
          command.right_cmd = 200;
          memcpy(buffer, &command, sizeof(command));
          size_received = sizeof(command);
      }
      else
      {
          carmen_hardware::SetPIDCommand command;
          command.header.common.sequence_id = 10002;
          command.left_p = 5;
          command.right_p = 9;
          memcpy(buffer, &command, sizeof(command));
          size_received = sizeof(command);
      }
      return true;
  }

  void sendResult(const uint8_t * buffer, const size_t size)
  {
      orion::ResultHeader * header = (orion::ResultHeader*)buffer;
      if (carmen_hardware::MessageType::SetCommands == (carmen_hardware::MessageType)header->common.message_id)
      {
          HAL_GPIO_TogglePin(LD1_GPIO_Port, LD1_Pin);
      }

      if (carmen_hardware::MessageType::SetPID == (carmen_hardware::MessageType)header->common.message_id)
      {
          HAL_GPIO_TogglePin(LD2_GPIO_Port, LD2_Pin);
      }
  }

private:
  bool flag = false;
};

}  // namespace orion

#endif  // ORION_PROTOCOL_ORION_MAJOR_H
