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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include <string>
#include "orion_protocol/orion_tcp_serial_bridge.h"
#include "orion_protocol/orion_cobs_framer.h"
#include "orion_protocol/orion_frame_transport.h"
#include "orion_protocol/orion_major.h"

#pragma pack(push, 1)

struct HandshakeCommand
{
  orion::CommandHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 }
  };
  uint8_t data1;
  int16_t data2;
};

struct HandshakeResult
{
  orion::ResultHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 },
    .error_code = 0
  };
  int16_t data2;
};

#pragma pack(pop)

TEST(TestSuite, sucessfulEchoResponse)
{
  orion::TCPSerialBridge bridge;
  bridge.connect("0.0.0.0", 9190);
  orion::COBSFramer framer;
  orion::FrameTransport frame_transport(&bridge, &framer);
  orion::Major server(&frame_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 3;
  uint32_t retry_timeout = orion::Major::Interval::Second * 3;

  command.data1 = 0;
  command.data2 = -200;

  server.invoke(command, &result, retry_timeout, retry_count);

  ASSERT_EQ(command.data2, result.data2);
}

TEST(TestSuite, echoResponseWithError)
{
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
