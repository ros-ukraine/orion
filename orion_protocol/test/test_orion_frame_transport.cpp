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
#include "orion_protocol/orion_crc.h"
#include "orion_protocol/orion_header.h"
#include "orion_protocol/orion_frame_transport.h"
#include "orion_protocol/orion_major.h"

using ::testing::Eq;
using ::testing::Gt;
using ::testing::Le;
using ::testing::NotNull;
using ::testing::_;
using ::testing::Invoke;
using ::testing::Return;
using ::testing::SetArgPointee;
using ::testing::SetArrayArgument;
using ::testing::SaveArg;

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

TEST(TestSuite, sendPacket)
{
  MockCommunication mock_communication;
  MockFramer mock_framer;
  orion::FrameTransport frame_transport(&mock_communication, &mock_framer);

  const size_t BUFFER_SIZE = 20;
  uint8_t packet[BUFFER_SIZE];
  char encoded_packet[] = "Encoded Packet";
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 200;

  EXPECT_CALL(mock_framer, encodePacket(Eq(packet), Eq(BUFFER_SIZE), _, Gt(strlen(encoded_packet) + 1))).WillOnce(
    DoAll(
      SetArrayArgument<2>(encoded_packet, encoded_packet + strlen(encoded_packet) + 1),
      Return(strlen(encoded_packet) + 1)));
  uint8_t *send_buffer;
  EXPECT_CALL(mock_communication, sendBuffer(NotNull(), Gt(0), Le(retry_timeout))).WillOnce(
    DoAll(
      SaveArg<0>(&send_buffer),
      Return(true)));

  frame_transport.sendPacket(packet, BUFFER_SIZE, retry_timeout);

  ASSERT_STREQ(reinterpret_cast<char*>(send_buffer), encoded_packet);
}

TEST(TestSuite, receivePacket)
{
  MockCommunication mock_communication;
  MockFramer mock_framer;
  orion::FrameTransport frame_transport(&mock_communication, &mock_framer);

  const size_t BUFFER_SIZE = 30;
  uint8_t packet[BUFFER_SIZE];
  char decoded_packet[] = "  Decoded Packet";
  size_t decoded_packet_length = strlen(decoded_packet) + 1;
  uint32_t retry_timeout = orion::Major::Interval::Microsecond * 300;

  uint16_t crc = orion::CRC::calculateCRC16(reinterpret_cast<uint8_t*>(decoded_packet + sizeof(orion::FrameHeader)),
    decoded_packet_length - sizeof(orion::FrameHeader));
  orion::FrameHeader *header = reinterpret_cast<orion::FrameHeader*>(decoded_packet);
  header->crc = crc;

  EXPECT_CALL(mock_communication, hasAvailableBuffer()).WillOnce(Return(false));

  char received_message[] = " Received Message";
  uint8_t* receive_buffer = reinterpret_cast<uint8_t*>(received_message);
  size_t data_size = strlen(received_message) + 1;
  receive_buffer[0] = orion::Framer::FRAME_DELIMETER;
  receive_buffer[data_size] = orion::Framer::FRAME_DELIMETER;
  EXPECT_CALL(mock_communication, receiveBuffer(NotNull(), Gt(data_size), Le(retry_timeout))).WillOnce(
    DoAll(
      SetArrayArgument<0>(receive_buffer, receive_buffer + data_size),
      Return(data_size)));

  EXPECT_CALL(mock_framer, decodePacket(NotNull(), Eq(data_size), _, Gt(decoded_packet_length))).WillOnce(
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
