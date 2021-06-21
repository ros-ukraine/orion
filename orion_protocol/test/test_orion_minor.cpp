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
#include "orion_protocol/orion_header.hpp"
#include "orion_protocol/orion_major.hpp"
#include "orion_protocol/orion_minor.hpp"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::NotNull;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::DoAll;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::_;

#pragma pack(push, 1)

struct SimpleCommand
{
  orion::CommandHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 }
  };
  uint8_t data = 0;
};

struct SimpleResult
{
  orion::ResultHeader header =
  {
    .frame = { .crc = 0 },
    .common = { .message_id = 2, .version = 1, .oldest_compatible_version = 1, .sequence_id = 0 },
    .error_code = 0
  };
  uint8_t data1 = 0;
  uint8_t data2 = 0;
};

#pragma pack(pop)

MOCK_GLOBAL_FUNC1(orion_communication_new, orion_communication_error_t(orion_communication_t ** me));
MOCK_GLOBAL_FUNC1(orion_communication_delete, orion_communication_error_t(const orion_communication_t * me));
MOCK_GLOBAL_FUNC4(orion_communication_send_buffer, orion_communication_error_t(const orion_communication_t * me,
  uint8_t *buffer, uint32_t size, uint32_t timeout));
// NOLINTNEXTLINE(readability/casting)
MOCK_GLOBAL_FUNC1(orion_communication_has_available_buffer, bool(const orion_communication_t * me));
MOCK_GLOBAL_FUNC4(orion_communication_receive_buffer, ssize_t(const orion_communication_t * me, uint8_t * buffer,
  uint32_t size, uint32_t timeout));
MOCK_GLOBAL_FUNC3(orion_communication_receive_available_buffer, ssize_t(const orion_communication_t * me,
  uint8_t * buffer, uint32_t size));

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
MOCK_GLOBAL_FUNC4(orion_transport_send_packet, orion_transport_error_t(orion_transport_t * me, uint8_t *input_buffer,
  uint32_t input_size, uint32_t timeout));
MOCK_GLOBAL_FUNC4(orion_transport_receive_packet, ssize_t(orion_transport_t * me, uint8_t *output_buffer,
  uint32_t output_size, uint32_t timeout));
// NOLINTNEXTLINE(readability/casting)
MOCK_GLOBAL_FUNC1(orion_transport_has_received_packet, bool(orion_transport_t * me));

class MockTransport: public orion::Transport
{
public:
  explicit MockTransport(orion::Communication * communication) : orion::Transport(communication) {}

  MOCK_METHOD3(sendPacket, orion_transport_error_t(uint8_t *input_buffer, uint32_t input_size, uint32_t timeout));
  MOCK_METHOD3(receivePacket, ssize_t(uint8_t *output_buffer, uint32_t output_size, uint32_t timeout));
  MOCK_METHOD0(hasReceivedPacket, bool());
};

TEST(TestSuite, happyPath)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(DoAll(
    SetArgPointee<0>(reinterpret_cast<orion_communication_struct_t*>(0xBCBCAAAA)),
    Return(ORION_COM_ERROR_NONE)));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  EXPECT_GLOBAL_CALL(orion_transport_new, orion_transport_new(_, _)).WillRepeatedly(DoAll(
    SetArgPointee<0>(reinterpret_cast<orion_transport_struct_t*>(0xDDDDBBBB)),
    Return(ORION_TRAN_ERROR_NONE)));
  EXPECT_GLOBAL_CALL(orion_transport_delete, orion_transport_delete(_)).WillRepeatedly(Return(ORION_TRAN_ERROR_NONE));
  MockTransport mock_transport(&mock_communication);

  MockTransport mock_outbound_transport(&mock_communication);
  MockTransport mock_inbound_transport(&mock_communication);
  orion::Major main_obj(&mock_outbound_transport);
  orion::Minor minor_obj(&mock_inbound_transport);

  const uint8_t SAMPLE_DATA = 203;
  const uint16_t OUTBOUND_BUFFER_SIZE = 200;
  uint8_t outbound_buffer[OUTBOUND_BUFFER_SIZE] = {0};
  uint8_t *p_outbound = NULL;
  uint32_t actual_outbound_size = 0;

  const uint16_t INBOUND_BUFFER_SIZE = 200;
  uint8_t inbound_buffer[INBOUND_BUFFER_SIZE] = {0};
  uint8_t *p_inbound = NULL;
  uint32_t actual_inbound_size = 0;

  SimpleCommand command;
  SimpleResult result;

  uint8_t retry_count = 5;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 400;

  EXPECT_CALL(mock_outbound_transport, sendPacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(
    DoAll(SaveArg<0>(&p_outbound), SaveArg<1>(&actual_outbound_size), Return(ORION_TRAN_ERROR_NONE)));
  EXPECT_CALL(mock_outbound_transport, hasReceivedPacket()).Times(0);

  // NOLINTNEXTLINE(build/c++11)
  auto mock_outbound_receive_packet = [&](uint8_t *output_buffer, uint32_t output_size,
    uint32_t timeout)
    {
      ssize_t size_received = minor_obj.receiveCommand(inbound_buffer, INBOUND_BUFFER_SIZE);

      size_t size = sizeof(SimpleResult);
      SimpleResult reply_result;
      reply_result.header.common.sequence_id = 1;
      if (size_received == sizeof(SimpleCommand))
      {
        SimpleCommand *p_command = reinterpret_cast<SimpleCommand*>(inbound_buffer);
        reply_result.data1 = p_command->data;
      }
      minor_obj.sendResult(reinterpret_cast<uint8_t*>(&reply_result), sizeof(reply_result));

      std::memcpy(output_buffer, p_inbound, actual_inbound_size);
      return actual_inbound_size;
    };

  EXPECT_CALL(mock_outbound_transport, receivePacket(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(
    Invoke(mock_outbound_receive_packet));

  EXPECT_GLOBAL_CALL(orion_transport_send_packet, orion_transport_send_packet(mock_inbound_transport.getObject(),
    NotNull(), Gt(0), Le(retry_timeout))).WillOnce(DoAll(SaveArg<1>(&p_inbound), SaveArg<2>(&actual_inbound_size),
      Return(ORION_TRAN_ERROR_NONE)));
  EXPECT_GLOBAL_CALL(orion_transport_has_received_packet, orion_transport_has_received_packet(
    mock_inbound_transport.getObject())).WillOnce(Return(true));

  // NOLINTNEXTLINE(build/c++11)
  auto mock_inbound_receive_packet = [&](orion_transport_t * me, uint8_t *output_buffer, uint32_t output_size,
    uint32_t timeout)
    {
      std::memcpy(output_buffer, p_outbound, actual_outbound_size);
      return actual_outbound_size;
    };
  EXPECT_GLOBAL_CALL(orion_transport_receive_packet, orion_transport_receive_packet(mock_inbound_transport.getObject(),
    NotNull(), Gt(0), Le(retry_timeout))).WillOnce(Invoke(mock_inbound_receive_packet));

  command.data = SAMPLE_DATA;
  main_obj.invoke(command, &result, retry_timeout, retry_count);
  ASSERT_EQ(actual_outbound_size, sizeof(command));
  ASSERT_EQ(command.data, result.data1);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
