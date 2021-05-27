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

#ifndef ORION_PROTOCOL_ORION_FRAMER_H
#define ORION_PROTOCOL_ORION_FRAMER_H

#include <stdint.h>
#include <stddef.h>
#include <stdlib.h>

#ifdef __cplusplus
extern "C" {
#endif

#define ORION_FRAMER_FRAME_DELIMETER (0)

typedef enum {
    ORION_FRM_ERROR_NONE = 0,
    ORION_FRM_ERROR_DECODING_FAILED = -1,
    ORION_FRM_ERROR_UNKNOWN = -2
} orion_framer_error_t;

ssize_t orion_framer_encode_packet(const uint8_t* data, size_t length, uint8_t* packet, size_t buffer_length);
ssize_t orion_framer_decode_packet(const uint8_t* packet, size_t length, uint8_t* data, size_t buffer_length);

#ifdef __cplusplus
}
#endif

#endif  // ORION_PROTOCOL_ORION_FRAMER_H
