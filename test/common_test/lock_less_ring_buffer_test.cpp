#include "common/lock_less_ring_buffer.h"
#include "common/logger.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace moboware;
using namespace moboware::common;

TEST(LockLessRingBufferTest, constructTest)
{
  using ArrayBuffer_t = std::array<std::uint8_t, 512>;
  constexpr std::size_t capacity{3};
  using LockLessRingBuffer_t = LockLessRingBuffer<ArrayBuffer_t, capacity>;

  LockLessRingBuffer_t rb;

  EXPECT_TRUE(rb.Empty());
  EXPECT_EQ(rb.Capacity(), capacity);
  EXPECT_EQ(rb.Size(), 0);

  const auto popFn{[&](const LockLessRingBuffer_t::Buffer_t &element) {

  }};
  // pop on empty queue should return false
  EXPECT_FALSE(rb.Pop(popFn));

  // push first element
  const std::string str{"3089utr47o isdjfglkwdeuio8use9p igksdfhjgklsdfbg "};

  const auto pushFn{
      [&](LockLessRingBuffer_t::Buffer_t &element, const moboware::common::QueueLengthType_t & /* headPosition*/) {   //
        memcpy(element.data(), str.c_str(), str.size());
        return true;
      }};

  EXPECT_TRUE(rb.Push(pushFn));
  EXPECT_FALSE(rb.Empty());
  EXPECT_EQ(rb.Size(), 1);
  //
  EXPECT_TRUE(rb.Push(pushFn));
  EXPECT_FALSE(rb.Empty());
  EXPECT_EQ(rb.Size(), 2);
  //
  EXPECT_TRUE(rb.Push(pushFn));
  EXPECT_FALSE(rb.Empty());
  EXPECT_EQ(rb.Size(), 3);
  // no space left
  EXPECT_FALSE(rb.Push(pushFn));
  EXPECT_FALSE(rb.Empty());
  EXPECT_EQ(rb.Size(), 3);
  // pop 1
  EXPECT_TRUE(rb.Pop(popFn));
  EXPECT_EQ(rb.Size(), 2);
  EXPECT_FALSE(rb.Empty());
}
