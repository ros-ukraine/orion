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
#include "orion_protocol/orion_cobs_framer.h"

TEST(TestSuite, positiveTestCase)
{
  orion::COBSFramer framer;

  const char test1[100] = "Hello Test 1";
  const char test2[100] = "Different String";

  const size_t BUFFER_SIZE = 500;
  uint8_t buffer[BUFFER_SIZE];
  char result[BUFFER_SIZE];

  size_t packet_size = 0;
  size_t data_size = 0;

  packet_size = framer.encodePacket(reinterpret_cast<const uint8_t*>(test1), strlen(test1) + 1, buffer, BUFFER_SIZE);

  ASSERT_GT(packet_size, 0);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[0]);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[packet_size - 1]);

  data_size = framer.decodePacket(buffer, packet_size, reinterpret_cast<uint8_t*>(result), BUFFER_SIZE);
  ASSERT_EQ(strlen(test1) + 1, data_size);
  ASSERT_STREQ(test1, result);

  packet_size = framer.encodePacket(reinterpret_cast<const uint8_t*>(test2),
    strlen(test2)  + 1, buffer, BUFFER_SIZE);

  ASSERT_GT(packet_size, 0);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[0]);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[packet_size - 1]);

  data_size = framer.decodePacket(buffer, packet_size, reinterpret_cast<uint8_t*>(result), BUFFER_SIZE);
  ASSERT_EQ(strlen(test2) + 1, data_size);
  ASSERT_STREQ(test2, result);

  packet_size = framer.encodePacket(reinterpret_cast<const uint8_t*>(test1), 0, buffer, BUFFER_SIZE);

  ASSERT_GT(packet_size, 0);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[0]);
  ASSERT_EQ(orion::Framer::FRAME_DELIMETER, buffer[packet_size - 1]);

  data_size = framer.decodePacket(buffer, packet_size, reinterpret_cast<uint8_t*>(result), BUFFER_SIZE);
  ASSERT_EQ(0, data_size);
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
