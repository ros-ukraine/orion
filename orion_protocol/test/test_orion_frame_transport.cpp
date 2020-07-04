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
#include "orion_protocol/orion_communication.h"
#include "orion_protocol/orion_framer.h"
#include "orion_protocol/orion_frame_transport.h"
#include "orion_protocol/orion_major.h"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::NotNull;
using ::testing::Invoke;
using ::testing::Return;

class MockCommunication: public orion::Communication
{
public:
  MOCK_METHOD2(receiveAvailableBuffer, size_t(uint8_t *buffer, uint32_t size));
  MOCK_METHOD3(receiveBuffer, size_t(uint8_t *buffer, uint32_t size, uint32_t timeout));
  MOCK_METHOD0(hasAvailableBuffer, bool());
  MOCK_METHOD3(sendBuffer, bool(uint8_t *buffer, uint32_t size, uint32_t timeout));
};

class MockFramer: public orion::Framer
{
public:
  MOCK_METHOD4(encodePacket, size_t(const uint8_t* data, size_t length, uint8_t* packet, size_t buffer_length));
  MOCK_METHOD4(decodePacket, size_t(const uint8_t* packet, size_t length, uint8_t* data, size_t buffer_length));
};

TEST(TestSuite, echoResponse)
{
  MockCommunication mock_communication;
  MockFramer mock_framer;
  orion::FrameTransport frame_transport(&mock_communication, &mock_framer);

  const size_t BUFFER_SIZE = 20;
  uint8_t packet[BUFFER_SIZE];
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 200;

  EXPECT_CALL(mock_framer, encodePacket(Eq(packet), Eq(BUFFER_SIZE), NotNull(), Gt(0))).WillOnce(Return(BUFFER_SIZE));
  EXPECT_CALL(mock_communication, sendBuffer(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Return(BUFFER_SIZE));

  frame_transport.sendPacket(packet, BUFFER_SIZE, retry_timeout);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
