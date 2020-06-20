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
#include <stdexcept>
#include <cstring>
#include "orion_protocol/orion_transport.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_major.h"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::NotNull;
using ::testing::Invoke;
using ::testing::Return;

#pragma pack(push, 1)

struct HandshakeCommand
{
  orion::CommandHeader header =
  {
    .message_id = 2,
    .version = 1,
    .oldest_compatible_version = 1,
    .sequence_id = 0
  };
};

struct HandshakeResult
{
  orion::ResultHeader header =
  {
    .message_id = 2,
    .version = 1,
    .oldest_compatible_version = 1,
    .sequence_id = 0,
    .error_code = 0
  };
};

#pragma pack(pop)

class MockTransport: public orion::Transport
{
public:
  MOCK_METHOD3(sendPacket, bool(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout));
  MOCK_METHOD3(receivePacket, size_t(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout));
  MOCK_METHOD0(hasReceivedPacket, bool());
};

TEST(TestSuite, sendPacketTimeoutExpiredException)
{
  MockTransport mock_transport;
  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 3;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 200;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Eq(retry_timeout))).WillRepeatedly(Return(false));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).Times(0);

  try
  {
    main.invoke(command, &result, retry_timeout, retry_count);
    FAIL() << "Expected std::runtime_error";
  }
  catch(std::runtime_error const & err)
  {
    EXPECT_EQ(err.what(), std::string("Timeout expired but result was not received"));
  }
  catch(...)
  {
    FAIL() << "Expected std::runtime_error";
  }
}

TEST(TestSuite, happyPath)
{
  MockTransport mock_transport;
  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 5;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 400;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Return(true));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))
    ).WillOnce(Invoke(
            [=](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
            {
              size_t size = sizeof(HandshakeResult);
              HandshakeResult reply_result;
              reply_result.header.sequence_id = 1;
              std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
              return size;
            }));

  main.invoke(command, &result, retry_timeout, retry_count);
}

TEST(TestSuite, incompatibleVersion)
{
  MockTransport mock_transport;
  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;
  result.header.version = 1;
  result.header.oldest_compatible_version = 1;

  uint8_t retry_count = 2;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 300;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Return(true));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))
    ).WillOnce(Invoke(
            [=](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
            {
              size_t size = sizeof(HandshakeResult);
              HandshakeResult reply_result;
              reply_result.header.sequence_id = 1;
              reply_result.header.version = 2;
              reply_result.header.oldest_compatible_version = 2;
              std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
              return size;
            }));
  try
  {
    main.invoke(command, &result, retry_timeout, retry_count);
    FAIL() << "Expected std::range_error";
  }
  catch(std::range_error const &err)
  {
    EXPECT_EQ(err.what(), std::string("Received reply version is not compatible with existing one"));
  }
  catch(...)
  {
    FAIL() << "Expected std::range_error";
  }
}

TEST(TestSuite, errorInReply)
{
  MockTransport mock_transport;
  orion::Major main(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 2;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 500;

  EXPECT_CALL(mock_transport, sendPacket(NotNull(), Gt(0), Eq(retry_timeout))).WillOnce(Return(true));
  EXPECT_CALL(mock_transport, hasReceivedPacket()).Times(0);
  EXPECT_CALL(mock_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))
    ).WillOnce(Invoke(
            [=](uint8_t *output_buffer, uint32_t output_size, uint32_t timeout)
            {
              size_t size = sizeof(HandshakeResult);
              HandshakeResult reply_result;
              reply_result.header.sequence_id = 1;
              reply_result.header.error_code = 12;
              std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
              return size;
            }));
  try
  {
    main.invoke(command, &result, retry_timeout, retry_count);
    FAIL() << "Expected std::runtime_error";
  }
  catch(std::runtime_error const & err)
  {
    EXPECT_EQ(err.what(), std::string("Error code: 12 detected in return packet"));
  }
  catch(...)
  {
    FAIL() << "Expected std::runtime_error";
  }
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
