#include "common/lock_less_ring_buffer.h"
#include "common/log_stream.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware;
using namespace moboware::common;

TEST(LockLessRingBufferTest, constructTest)
{
  using LockLessRingBuffer_t = LockLessRingBuffer<std::uint8_t, 512, 100>;
  LockLessRingBuffer_t rb;

  EXPECT_EQ(rb.Size(), 0);
  const auto popFn{[&](const LockLessRingBuffer_t::ArrayBuffer_t &element) {

  }};
  EXPECT_FALSE(rb.Pop(popFn));
  // push first element
  const std::string str{"3089utr47o isdjfglkwdeuio8use9p igksdfhjgklsdfbg "};

  const auto pushFn{[&](LockLessRingBuffer_t::ArrayBuffer_t &element) {   //
    memcpy(element.data(), str.c_str(), str.size());
    return true;
  }};

  EXPECT_TRUE(rb.Push(pushFn));
  EXPECT_EQ(rb.Size(), 1);
}
