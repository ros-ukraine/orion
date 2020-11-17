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
#include "gmock-global/gmock-global.h"
#include "orion_protocol/orion_circular_buffer.h"
#include <stdexcept>

using ::testing::NotNull;
using ::testing::Gt;
using ::testing::Throw;

// TODO(Andriy): Fix warning: function declared ‘noreturn’ has a ‘return’ statement
MOCK_GLOBAL_FUNC4(__assert_fail, void(const char*, const char*, unsigned int, const char*));

TEST(TestSuite, parametersValidation)
{
  const uint32_t buffer_length = 20;
  uint8_t buffer[buffer_length] = { 0 };
  const uint32_t circular_buffer_length = 20;
  uint8_t circular_buffer[circular_buffer_length] = { 0 };

  orion_circular_buffer_t circular_buff_struct;
  circular_buff_struct.head_index = 0;
  circular_buff_struct.tail_index = 0;

  circular_buff_struct.buffer_size = 10;
  circular_buff_struct.p_buffer = NULL;

  ON_GLOBAL_CALL(__assert_fail, __assert_fail(NotNull(), NotNull(), Gt(0), NotNull())).WillByDefault(Throw(
    std::exception()));
  ASSERT_ANY_THROW(orion_circular_buffer_add(NULL, NULL, 10));
  ASSERT_ANY_THROW(orion_circular_buffer_add(NULL, buffer, buffer_length));
  ASSERT_ANY_THROW(orion_circular_buffer_add(&circular_buff_struct, buffer, buffer_length));

  ASSERT_ANY_THROW(orion_circular_buffer_dequeue(NULL, NULL, 10));
  ASSERT_ANY_THROW(orion_circular_buffer_dequeue(NULL, buffer, buffer_length));
  ASSERT_ANY_THROW(orion_circular_buffer_dequeue(&circular_buff_struct, buffer, buffer_length));

  circular_buff_struct.p_buffer = circular_buffer;
  circular_buff_struct.buffer_size = 0;
  ASSERT_ANY_THROW(orion_circular_buffer_add(&circular_buff_struct, buffer, buffer_length));
  ASSERT_ANY_THROW(orion_circular_buffer_dequeue(&circular_buff_struct, buffer, buffer_length));
}

TEST(TestSuite, happyPath)
{
  uint8_t buffer[] = "Hello from test";
  const uint32_t circular_buffer_length = 2 * sizeof(buffer);
  uint8_t circular_buffer[circular_buffer_length] = { 0 };
  uint8_t output_buffer[circular_buffer_length] = { 0 };
  orion_circular_buffer_t circular_buff_struct;
  uint32_t actual_size = 0;

  ON_GLOBAL_CALL(__assert_fail, __assert_fail(NotNull(), NotNull(), Gt(0), NotNull())).WillByDefault(Throw(
    std::exception()));
  orion_circular_buffer_init(&circular_buff_struct, circular_buffer, circular_buffer_length);
  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));
  orion_circular_buffer_add(&circular_buff_struct, buffer, sizeof(buffer));
  ASSERT_EQ(circular_buff_struct.tail_index - circular_buff_struct.head_index, sizeof(buffer));
  ASSERT_STREQ((char*)circular_buffer, (char*)buffer);
  ASSERT_FALSE(orion_circular_buffer_is_empty(&circular_buff_struct));

  actual_size = orion_circular_buffer_dequeue(&circular_buff_struct, output_buffer, sizeof(buffer) + 2);
  ASSERT_EQ(circular_buff_struct.tail_index, circular_buff_struct.head_index);
  ASSERT_STREQ((char*)output_buffer, (char*)buffer);
  ASSERT_EQ(sizeof(buffer), actual_size);
  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));

  orion_circular_buffer_add(&circular_buff_struct, buffer, sizeof(buffer));
  ASSERT_FALSE(orion_circular_buffer_is_empty(&circular_buff_struct));
  orion_circular_buffer_add(&circular_buff_struct, buffer, sizeof(buffer));
  ASSERT_ANY_THROW(orion_circular_buffer_add(&circular_buff_struct, buffer, 1));
  ASSERT_FALSE(orion_circular_buffer_is_empty(&circular_buff_struct));
  actual_size = orion_circular_buffer_dequeue(&circular_buff_struct, output_buffer, sizeof(buffer) + 2);
  ASSERT_FALSE(orion_circular_buffer_is_empty(&circular_buff_struct));
  ASSERT_EQ(sizeof(buffer) + 2, actual_size);
  actual_size = orion_circular_buffer_dequeue(&circular_buff_struct, output_buffer, sizeof(buffer) - 2);
  ASSERT_EQ(circular_buff_struct.tail_index, circular_buff_struct.head_index);
  ASSERT_EQ(sizeof(buffer) - 2, actual_size);
  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));
}

TEST(TestSuite, wordsParsing)
{
  uint8_t delimiter = static_cast<uint8_t>('z');
  uint8_t buffer[] = "initvectorzPacket1zzHelloz";
  const uint32_t circular_buffer_length = 2 * sizeof(buffer);
  uint8_t circular_buffer[circular_buffer_length] = { 0 };
  uint8_t output_buffer[circular_buffer_length] = { 0 };
  orion_circular_buffer_t circular_buff_struct;
  uint32_t actual_size = 0;

  ON_GLOBAL_CALL(__assert_fail, __assert_fail(NotNull(), NotNull(), Gt(0), NotNull())).WillByDefault(Throw(
    std::exception()));
  orion_circular_buffer_init(&circular_buff_struct, circular_buffer, circular_buffer_length);
  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));
  orion_circular_buffer_add(&circular_buff_struct, buffer, sizeof(buffer) - 1);

  ASSERT_TRUE(orion_circular_buffer_has_word(&circular_buff_struct, delimiter));
  ASSERT_TRUE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  output_buffer[actual_size] = 0;
  ASSERT_STREQ("zPacket1z", reinterpret_cast<char*>(output_buffer));

  ASSERT_TRUE(orion_circular_buffer_has_word(&circular_buff_struct, delimiter));
  ASSERT_TRUE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  output_buffer[actual_size] = 0;
  ASSERT_STREQ("zHelloz", reinterpret_cast<char*>(output_buffer));

  ASSERT_FALSE(orion_circular_buffer_has_word(&circular_buff_struct, delimiter));
  ASSERT_FALSE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  ASSERT_FALSE(orion_circular_buffer_has_word(&circular_buff_struct, delimiter));
  ASSERT_FALSE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));

  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));

  uint8_t overflow_buffer[] = "overflow";
  orion_circular_buffer_add(&circular_buff_struct, overflow_buffer, sizeof(overflow_buffer) - 1);
  orion_circular_buffer_add(&circular_buff_struct, buffer, sizeof(buffer) - 1);

  ASSERT_TRUE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  output_buffer[actual_size] = 0;
  ASSERT_STREQ("zPacket1z", reinterpret_cast<char*>(output_buffer));

  ASSERT_TRUE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  output_buffer[actual_size] = 0;
  ASSERT_STREQ("zHelloz", reinterpret_cast<char*>(output_buffer));

  ASSERT_FALSE(orion_circular_buffer_dequeue_word(&circular_buff_struct, delimiter, output_buffer,
    circular_buffer_length, &actual_size));
  ASSERT_TRUE(orion_circular_buffer_is_empty(&circular_buff_struct));
}

int main(int argc, char **argv)
{
  ::testing::InitGoogleMock(&argc, argv);
  return RUN_ALL_TESTS();
}
