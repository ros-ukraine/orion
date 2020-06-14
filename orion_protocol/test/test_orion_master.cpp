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
#include "orion_protocol/orion_network_layer.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_master.h"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::NotNull;

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

class MockNetworkLayer: public orion::NetworkLayer
{
public:
  MOCK_METHOD5(sendAndReceivePacket, size_t(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout,
    uint8_t *output_buffer, uint32_t output_size));
};

TEST(TestSuite, timeoutExpiredException)
{
  MockNetworkLayer mock_network_layer;
  orion::Master master(&mock_network_layer);

  HandshakeCommand command;
  HandshakeResult result;

  uint8_t retry_count = 3;
  uint32_t retry_timeout = orion::Master::Timeout::Second;

  EXPECT_CALL(mock_network_layer, sendAndReceivePacket(NotNull(), Gt(0), Eq(retry_timeout), NotNull(), Gt(0))
    ).Times(retry_count);

  try
  {
    master.invoke(command, &result, retry_timeout, retry_count);
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
