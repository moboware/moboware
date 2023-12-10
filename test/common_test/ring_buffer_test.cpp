#include "common/log_stream.h"
#include "common/ring_buffer.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware;

TEST(RingBufferTest, constructTest)
{
  const auto bufferSize{100ul};
  using MyRingBuffer_t = common::RingBuffer<std::uint8_t, bufferSize>;
  MyRingBuffer_t ringBuffer;

  EXPECT_EQ(0ul, ringBuffer.GetWriteBufferSize());
  EXPECT_EQ(0ul, ringBuffer.GetReadBufferSize());
  EXPECT_EQ(bufferSize, ringBuffer.GetFreeWriteBufferSize());

  // write bytes
  const auto stringLength{30ul};
  const std::uint8_t buffer[stringLength]{"@@@@@@@@@@"
                                          "@@@@@@@@@@"
                                          "@@@@@@@@@"};

  EXPECT_EQ(stringLength, ringBuffer.WriteBuffer(buffer, stringLength));

  EXPECT_EQ(stringLength, ringBuffer.GetWriteBufferSize());
  EXPECT_EQ(bufferSize - stringLength, ringBuffer.GetFreeWriteBufferSize());

  // write more bytes
  EXPECT_EQ(stringLength, ringBuffer.WriteBuffer(buffer, stringLength));
  EXPECT_EQ(bufferSize - (2 * stringLength), ringBuffer.GetFreeWriteBufferSize());

  // write more bytes
  EXPECT_EQ(stringLength, ringBuffer.WriteBuffer(buffer, stringLength));
  EXPECT_EQ(bufferSize - (3 * stringLength), ringBuffer.GetFreeWriteBufferSize());

  // write no more bytes, buffer is full
  EXPECT_EQ(0ul, ringBuffer.WriteBuffer(buffer, stringLength));
  EXPECT_EQ(bufferSize - ringBuffer.GetFreeWriteBufferSize(), ringBuffer.GetReadBufferSize());
  //
  //
  // read bytes
  EXPECT_TRUE(ringBuffer.ReadBuffer([&](const std::uint8_t *readBuffer, const std::size_t bytesAvailable) {
    EXPECT_EQ(ringBuffer.GetReadBufferSize(), bytesAvailable);
    return 20ul;   // process 20 bytes that will be flushed
  }));

  EXPECT_EQ(bufferSize - ringBuffer.GetFreeWriteBufferSize() - 20ul, ringBuffer.GetReadBufferSize());

  //
  EXPECT_TRUE(ringBuffer.ReadBuffer([&](const std::uint8_t *readBuffer, const std::size_t bytesAvailable) {
    EXPECT_EQ(ringBuffer.GetReadBufferSize(), bytesAvailable);
    return 10ul;   // process 10 bytes that will be flushed
  }));
  EXPECT_EQ(bufferSize - ringBuffer.GetFreeWriteBufferSize() - 30ul, ringBuffer.GetReadBufferSize());

  // read all bytes
  EXPECT_TRUE(ringBuffer.ReadBuffer([&](const std::uint8_t *readBuffer, const std::size_t bytesAvailable) {
    EXPECT_EQ(ringBuffer.GetReadBufferSize(), bytesAvailable);
    return bytesAvailable;   // process 10 bytes that will be flushed
  }));
  // available write buffer size is max, nothing is written
  // read buffer size is zero
  EXPECT_EQ(0ul, ringBuffer.GetWriteBufferSize());
  EXPECT_EQ(0ul, ringBuffer.GetReadBufferSize());
  EXPECT_EQ(bufferSize, ringBuffer.GetFreeWriteBufferSize());
}