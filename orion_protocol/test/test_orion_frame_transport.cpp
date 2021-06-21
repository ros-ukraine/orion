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
#include <string.h>
#include "orion_protocol/orion_communication.hpp"
#include "orion_protocol/orion_framer.h"
#include "orion_protocol/orion_crc.h"
#include "orion_protocol/orion_header.hpp"
#include "orion_protocol/orion_transport.hpp"
#include "orion_protocol/orion_major.hpp"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::NotNull;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArrayArgument;
using ::testing::SaveArg;
using ::testing::SetArgPointee;
using ::testing::DoAll;

MOCK_GLOBAL_FUNC1(orion_communication_new, orion_communication_error_t(orion_communication_t ** me));
MOCK_GLOBAL_FUNC1(orion_communication_delete, orion_communication_error_t(const orion_communication_t * me));
MOCK_GLOBAL_FUNC4(orion_communication_send_buffer, orion_communication_error_t(const orion_communication_t * me,
  uint8_t *buffer, uint32_t size, uint32_t timeout));
// NOLINTNEXTLINE(readability/casting)
MOCK_GLOBAL_FUNC1(orion_communication_has_available_buffer, bool(const orion_communication_t * me));
MOCK_GLOBAL_FUNC4(orion_communication_receive_buffer, ssize_t(const orion_communication_t * me, uint8_t * buffer,
  uint32_t size, uint32_t timeout));

class MockCommunication: public orion::Communication
{
public:
  MOCK_METHOD2(receiveAvailableBuffer, ssize_t(uint8_t *buffer, uint32_t size));
  MOCK_METHOD3(receiveBuffer, ssize_t(uint8_t *buffer, uint32_t size, uint32_t timeout));
  MOCK_METHOD0(hasAvailableBuffer, bool());
  MOCK_METHOD3(sendBuffer, orion_communication_error_t(uint8_t *buffer, uint32_t size, uint32_t timeout));
};

MOCK_GLOBAL_FUNC4(orion_framer_encode_packet, ssize_t(const uint8_t* data, size_t length, uint8_t* packet,
  size_t buffer_length));
MOCK_GLOBAL_FUNC4(orion_framer_decode_packet, ssize_t(const uint8_t* packet, size_t length, uint8_t* data,
  size_t buffer_length));

TEST(TestSuite, sendPacket)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(DoAll(
    SetArgPointee<0>(reinterpret_cast<orion_communication_struct_t*>(0xBCBCAAAA)),
    Return(ORION_COM_ERROR_NONE)));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  orion::Transport frame_transport(&mock_communication);

  const size_t BUFFER_SIZE = 20;
  uint8_t packet[BUFFER_SIZE];
  char encoded_packet[] = "Encoded Packet";
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 200;

  EXPECT_GLOBAL_CALL(orion_framer_encode_packet, orion_framer_encode_packet(Eq(packet), Eq(BUFFER_SIZE), _,
    Gt(strlen(encoded_packet) + 1))).WillOnce(DoAll(
      SetArrayArgument<2>(encoded_packet, encoded_packet + strlen(encoded_packet) + 1),
      Return(strlen(encoded_packet) + 1)));
  uint8_t *send_buffer;
  EXPECT_GLOBAL_CALL(orion_communication_send_buffer, orion_communication_send_buffer(NotNull(), NotNull(), Gt(0),
    Le(retry_timeout))).WillOnce(
    DoAll(
      SaveArg<1>(&send_buffer),
      Return(ORION_COM_ERROR_NONE)));

  frame_transport.sendPacket(packet, BUFFER_SIZE, retry_timeout);

  ASSERT_STREQ(reinterpret_cast<char*>(send_buffer), encoded_packet);
}

TEST(TestSuite, receivePacket)
{
  EXPECT_GLOBAL_CALL(orion_communication_new, orion_communication_new(_)).WillOnce(DoAll(
    SetArgPointee<0>(reinterpret_cast<orion_communication_struct_t*>(0xBCBCAAAA)),
    Return(ORION_COM_ERROR_NONE)));
  EXPECT_GLOBAL_CALL(orion_communication_delete, orion_communication_delete(_)).WillOnce(Return(ORION_COM_ERROR_NONE));
  MockCommunication mock_communication;

  orion::Transport frame_transport(&mock_communication);

  const size_t BUFFER_SIZE = 30;
  uint8_t packet[BUFFER_SIZE];
  char decoded_packet[] = "  Decoded Packet";
  size_t decoded_packet_length = strlen(decoded_packet) + 1;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 300;

  uint16_t crc = orion_crc_calculate_crc16(reinterpret_cast<uint8_t*>(decoded_packet + sizeof(orion::FrameHeader)),
    decoded_packet_length - sizeof(orion::FrameHeader));
  orion::FrameHeader *header = reinterpret_cast<orion::FrameHeader*>(decoded_packet);
  header->crc = crc;

  EXPECT_GLOBAL_CALL(orion_communication_has_available_buffer, orion_communication_has_available_buffer(
    NotNull())).WillOnce(Return(false));

  char received_message[] = " Received Message";
  uint8_t* receive_buffer = reinterpret_cast<uint8_t*>(received_message);
  size_t data_size = strlen(received_message) + 1;
  receive_buffer[0] = ORION_FRAMER_FRAME_DELIMETER;
  receive_buffer[data_size] = ORION_FRAMER_FRAME_DELIMETER;

  EXPECT_GLOBAL_CALL(orion_communication_receive_buffer, orion_communication_receive_buffer(NotNull(), NotNull(),
    Gt(data_size), Le(retry_timeout))).WillOnce(
    DoAll(
      SetArrayArgument<1>(receive_buffer, receive_buffer + data_size),
      Return(data_size)));

  EXPECT_GLOBAL_CALL(orion_framer_decode_packet, orion_framer_decode_packet(NotNull(), Eq(data_size), _,
    Gt(decoded_packet_length))).WillOnce(
    DoAll(
      SetArrayArgument<2>(decoded_packet, decoded_packet + decoded_packet_length),
      Return(decoded_packet_length)));

  size_t packet_size = frame_transport.receivePacket(packet, BUFFER_SIZE, retry_timeout);

  ASSERT_EQ(decoded_packet_length, packet_size);

  ASSERT_STREQ(reinterpret_cast<char*>(packet), decoded_packet);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
