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

#include <gtest/gtest.h>
#include <gmock/gmock.h>
#include "gmock-global/gmock-global.h"
#include "orion_protocol/orion_transport.hpp"
#include "orion_protocol/orion_communication.hpp"
#include "orion_protocol/orion_header.hpp"
#include "orion_protocol/orion_major.hpp"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::_;
using ::testing::NotNull;
using ::testing::Invoke;
using ::testing::Return;

#pragma pack(push, 1)

struct HandshakeCommand
{
  orion::CommandHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 }
  };
};

struct HandshakeResult
{
  orion::ResultHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 },
    .error_code = 0
  };
};

#pragma pack(pop)

MOCK_GLOBAL_FUNC1(orion_communication_new, orion_communication_error_t(orion_communication_t ** me));
MOCK_GLOBAL_FUNC1(orion_communication_delete, orion_communication_error_t(const orion_communication_t * me));

class MockCommunication: public orion::Communication
{
public:
  MOCK_METHOD2(receiveAvailableBuffer, ssize_t(uint8_t *buffer, uint32_t size));
  MOCK_METHOD3(receiveBuffer, ssize_t(uint8_t *buffer, uint32_t size, uint32_t timeout));
  MOCK_METHOD0(hasAvailableBuffer, bool());
  MOCK_METHOD3(sendBuffer, orion_communication_error_t(uint8_t *buffer, uint32_t size, uint32_t timeout));
};

MOCK_GLOBAL_FUNC2(orion_transport_new, orion_transport_error_t(orion_transport_t ** me,
  orion_communication_t * communication));
MOCK_GLOBAL_FUNC1(orion_transport_delete, orion_transport_error_t(const orion_transport_t * me));

class MockTransport: public orion::Transport
{
public:
  explicit MockTransport(orion::Communication * communication) : orion::Transport(communication) {}

  MOCK_METHOD3(sendPacket, orion_transport_error_t(uint8_t *input_buffer, uint32_t input_size, uint32_t timeout));
  MOCK_METHOD3(receivePacket, ssize_t(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout));
  MOCK_METHOD0(hasReceivedPacket, bool());
};

TEST(TestSuite, sendPacketTimeoutExpiredException)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  EXPECT_GLOBAL_CALL(orion_transport_new, orion_transport_new(_, _)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_transport_delete, orion_transport_delete(_)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  MockTransport mock_transport(&mock_communication);

  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 3;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 200;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Eq(retry_timeout))).WillRepeatedly(Return(
    ORION_TRAN_ERROR_TIMEOUT));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).Times(0);

  orion_major_error_t status = main.invoke(command, &result, retry_timeout, retry_count);
  EXPECT_EQ(ORION_MAJOR_ERROR_TIMEOUT, status);
}

TEST(TestSuite, happyPath)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  EXPECT_GLOBAL_CALL(orion_transport_new, orion_transport_new(_, _)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_transport_delete, orion_transport_delete(_)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  MockTransport mock_transport(&mock_communication);

  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 5;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 400;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  auto mock_receive_packet = [](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
    {
      size_t size = sizeof(HandshakeResult);
      HandshakeResult reply_result;
      reply_result.header.common.sequence_id = 1;
      std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
      return size;
    };

  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Invoke(mock_receive_packet));

  main.invoke(command, &result, retry_timeout, retry_count);
}

TEST(TestSuite, incompatibleVersion)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  EXPECT_GLOBAL_CALL(orion_transport_new, orion_transport_new(_, _)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_transport_delete, orion_transport_delete(_)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  MockTransport mock_transport(&mock_communication);

  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;
  result.header.common.version = 1;
  result.header.common.oldest_compatible_version = 1;

  uint8_t retry_count = 2;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 300;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  auto mock_receive_packet = [](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
    {
      size_t size = sizeof(HandshakeResult);
      HandshakeResult reply_result;
      reply_result.header.common.sequence_id = 1;
      reply_result.header.common.version = 2;
      reply_result.header.common.oldest_compatible_version = 2;
      std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
      return size;
    };
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Invoke(mock_receive_packet));
  orion_major_error_t status = main.invoke(command, &result, retry_timeout, retry_count);
  EXPECT_EQ(ORION_MAJOR_ERROR_NOT_COMPATIBLE_PACKET_VERSION, status);
}

TEST(TestSuite, errorInReply)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  EXPECT_GLOBAL_CALL(orion_transport_new, orion_transport_new(_, _)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_GLOBAL_CALL(orion_transport_delete, orion_transport_delete(_)).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  MockTransport mock_transport(&mock_communication);

  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 2;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 500;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Eq(retry_timeout))).WillOnce(Return(ORION_TRAN_ERROR_NONE));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  auto mock_receive_packet = [](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
    {
      size_t size = sizeof(HandshakeResult);
      HandshakeResult reply_result;
      reply_result.header.common.sequence_id = 1;
      reply_result.header.error_code = 12;
      std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
      return size;
    };
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Invoke(mock_receive_packet));
  orion_major_error_t status = main.invoke(command, &result, retry_timeout, retry_count);
  EXPECT_EQ(ORION_MAJOR_ERROR_APPLICATION_ERROR_RECEIVED, status);

  // TODO(Andriy): How to pass error code?
  // EXPECT_EQ(12, result.header.error_code);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
