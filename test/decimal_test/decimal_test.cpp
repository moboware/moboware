#include "common/log_stream.h"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace boost::multiprecision;

int main(int argc, char *argv[])
{
  LogStream::GetInstance().SetLevel(LogStream::LEVEL::DEBUG);

  ::testing::InitGoogleMock(&argc, argv);
  ::testing::FLAGS_gtest_death_test_style = "fast";
  return RUN_ALL_TESTS();
}

TEST(DecimalTest, MultiplyTest)
{
  constexpr auto numberOfDecimals{18};

  using Price_t = number<cpp_dec_float<numberOfDecimals>>;

  LOG_INFO("Numeric Price_t digits:" << std::numeric_limits<Price_t>::digits);
  LOG_INFO("Numeric double digits:" << std::numeric_limits<double>::digits10);

  LOG_INFO("Max Price_t:" << std::numeric_limits<Price_t>::max());
  LOG_INFO("Max double :" << std::fixed << std::setprecision(std::numeric_limits<Price_t>::digits10)
                          << std::numeric_limits<double>::max());

  LOG_INFO("Min Price_t:" << std::numeric_limits<Price_t>::min());
  LOG_INFO("Min double :" << std::fixed << std::setprecision(std::numeric_limits<Price_t>::digits10)
                          << std::numeric_limits<double>::min());

  LOG_INFO("Max Price_t digits:" << std::numeric_limits<Price_t>::max_digits10);
  LOG_INFO("Max double digits:" << std::numeric_limits<double>::max_digits10);

  const std::string value{"1000000000000.000000000000000001"};
  const std::string value2{"2000000000000.000000000000000002"};
  const std::string value4{"4000000000000.000000000000000004"};

  Price_t decimal{value};

  LOG_INFO(std::fixed << std::setprecision(std::numeric_limits<Price_t>::digits10) << "Decimal:" << decimal);

  EXPECT_EQ(decimal.convert_to<std::string>(), value);
  EXPECT_EQ(decimal += decimal, Price_t{value2});
  LOG_INFO("Value  :" << value);
  LOG_INFO("Decimal:" << decimal);

  EXPECT_EQ(decimal *= 2.0, Price_t{value4});

  LOG_INFO("Decimal:" << decimal);

  EXPECT_EQ(decimal /= decimal, 1);
  LOG_INFO("Decimal:" << decimal);
}
