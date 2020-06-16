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
    .backward_compatible = 0
  };
};

struct HandshakeResult
{
  orion::ResultHeader header =
  {
    .message_id = 2,
    .version = 1,
    .backward_compatible = 0,
    .error_code = 0
  };
};

#pragma pack(pop)

class MockTransport: public orion::Transport
{
public:
  MOCK_METHOD5(sendAndReceivePacket, size_t(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout,
    uint8_t *output_buffer, uint32_t output_size));
};

TEST(TestSuite, timeoutExpiredException)
{
  MockTransport mock_transport;
  orion::Major major(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 3;
  uint32_t retry_timeout = orion::Major::Timeout::Second;

  EXPECT_CALL(mock_transport, sendAndReceivePacket(NotNull(), Gt(0), Eq(retry_timeout), NotNull(), Gt(0))
    ).Times(retry_count);

  try
  {
    major.invoke(command, &result, retry_timeout, retry_count);
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
  orion::Major major(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result, reply_result;

  uint8_t retry_count = 5;
  uint32_t retry_timeout = orion::Major::Timeout::Second * 10;

  EXPECT_CALL(mock_transport, sendAndReceivePacket(NotNull(), Gt(0), Eq(retry_timeout), NotNull(), Gt(0))
    ).WillOnce(Invoke(
            [=](const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout, uint8_t *output_buffer,
              uint32_t output_size)
            {
              size_t size = sizeof(HandshakeResult);
              std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
              return size;
            }));

  major.invoke(command, &result, retry_timeout, retry_count);
}

TEST(TestSuite, incompatibleVersion)
{
  MockTransport mock_transport;
  orion::Major major(&mock_transport);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 2;
  uint32_t retry_timeout = orion::Major::Timeout::Second * 4;

  EXPECT_CALL(mock_transport, sendAndReceivePacket(NotNull(), Gt(0), Eq(retry_timeout), NotNull(), Gt(0))
    ).WillOnce(Invoke(
            [=](const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout, uint8_t *output_buffer,
              uint32_t output_size)
            {
              size_t size = sizeof(HandshakeResult);
              HandshakeResult reply_result;
              reply_result.header.version = 2;
              reply_result.header.backward_compatible = 0;

              std::memcpy(output_buffer, reinterpret_cast<const uint8_t*>(&reply_result), size);
              return size;
            }));

  try
  {
    major.invoke(command, &result, retry_timeout, retry_count);
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

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
