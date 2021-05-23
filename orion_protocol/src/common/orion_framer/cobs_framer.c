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

#include "orion_protocol/orion_framer.h"

#define StartBlock() (code_ptr = output++, index = 1)
#define FinishBlock() (*code_ptr = index)

static size_t encode(const uint8_t *input, size_t length, uint8_t *output);
static size_t decode(const uint8_t *input, size_t length, uint8_t *output);

// TODO(Andriy): fix parameter description
/**
 *
 * @param data[in] 			pointer on data that should be encoded. At the end should be reserved memory for header
 * @param len[in]			len of data that should be encoded plus reserved size.
 * @param packet[out]		pointer on output buffer that will contain complete frame.
 * @param avail_len[in]		Available len in out buffer.
 * @return Resulting length of encoded data.
 */
orion_framer_error_t orion_framer_encode_packet(const uint8_t* data, size_t length, uint8_t* packet,
  size_t buffer_length, size_t * encoded_size)
{
  *encoded_size = 0;
  size_t result = 0;

  // Start 0
  packet[(*encoded_size)++] = ORION_FRAMER_FRAME_DELIMETER;

  result = encode(data, length, &packet[*encoded_size]);
  if (result < 1)
  {
    result = 0;
  }
  else
  {
    *encoded_size += result;

    // End 0
    packet[(*encoded_size)++] = ORION_FRAMER_FRAME_DELIMETER;
  }

  return (ORION_FRM_ERROR_NONE);
}

// TODO(Andriy): fix parameter description
/**
 *
 * @param packet[in]		Pointer on data that should be decoded.
 * @param len[in]			Length on data
 * @param data[out]			Pointer on output buffer where decoded data will be placed.
 * @param avail_len[out]	Available size of data buffer
 * @return Length of decoded data
 */
orion_framer_error_t orion_framer_decode_packet(const uint8_t* packet, size_t length, uint8_t* data, 
    size_t buffer_length, size_t * decoded_size)
{
  *decoded_size = decode(&packet[1], length, data);
  if (*decoded_size < 1)
  {
    return (ORION_FRM_ERROR_DECODING_FAILED);
  }
  (*decoded_size)--;  // TODO(Andriy): Understand why ?
  return (ORION_FRM_ERROR_NONE);
}

size_t encode(const uint8_t *input, size_t length, uint8_t *output)
{
  const uint8_t *start = output;
  const uint8_t *end = input + length;
  uint8_t index;
  uint8_t *code_ptr;

  StartBlock();
  while (input < end)
  {
    if (0xFF != index)
    {
      uint8_t symbol = *input++;
      if (ORION_FRAMER_FRAME_DELIMETER != symbol)
      {
        *output++ = symbol;
        index++;
        continue;
      }
    }
    FinishBlock();
    StartBlock();
  }
  FinishBlock();
  return output - start;
}

/*
 * decode - decodes "length" bytes of data at
 * the location pointed to by "input", writing the
 * output to the location pointed to by "output".
 *
 * Returns the length of the decoded data
 * (which is guaranteed to be <= length).
 */
size_t decode(const uint8_t *input, size_t length, uint8_t *output)
{
  const uint8_t *start = output;
  const uint8_t *end = input + length;
  uint8_t index = 0xFF;
  uint8_t inverse_index = 0;

  while (input < end)
  {
    if (0 != inverse_index)
    {
      *output++ = *input++;
    }
    else
    {
      if (0xFF != index)
      {
        *output++ = ORION_FRAMER_FRAME_DELIMETER;
      }
      index = *input++;
      inverse_index = index;
      if (0 == index)
      {
        // Source length exceeded limits of 255 symbols
        break;
      }
    }
    inverse_index--;
  }
  return output - start;
}
