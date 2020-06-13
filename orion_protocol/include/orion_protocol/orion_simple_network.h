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

#ifndef ORION_PROTOCOL_ORION_SIMPLE_NETWORK_H
#define ORION_PROTOCOL_ORION_SIMPLE_NETWORK_H

#include "orion_protocol/orion_data_link_layer.h"
#include "orion_protocol/orion_network_layer.h"

namespace orion
{

class SimpleNetwork: public NetworkLayer
{
public:
  SimpleNetwork(DataLinkLayer *data_link_layer) : data_link_layer_(DataLinkLayer *data_link_layer) {};
  virtual ~SimpleNetwork() = default;

  virtual size_t sendAndReceivePacket(const uint8_t *input_buffer, uint32_t input_size, uint32_t timeout,
    uint8_t *output_buffer, uint32_t output_size);

private:
  DataLinkLayer *data_link_layer_;

  const uinit32_t BUFFER_SIZE = 500;
  uint8_t command_buffer_[BUFFER_SIZE];

  uint32_t sequence_id_ = 0;
}

}  // orion

#endif  // ORION_PROTOCOL_ORION_SIMPLE_NETWORK_H
