#include <decimal/decimal>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdio.h>
#include <stdlib.h>

TEST(StdDecimalTest, DoubleToDecimal128Test)
{
  std::decimal::decimal128 dec128(123136.34535235001);

  EXPECT_EQ(dec128, std::decimal::make_decimal128(12313634535235001ll, 11)) << std::decimal::decimal128_to_double(dec128);
}