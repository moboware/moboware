#include "common/logger.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <chrono>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace boost::multiprecision;
constexpr auto numberOfDecimals{18};
using Price_t = number<cpp_dec_float<numberOfDecimals>>;

template <> struct fmt::formatter<Price_t> : fmt::ostream_formatter {
};

int main(int argc, char *argv[])
{
  Logger::GetInstance().SetLevel(Logger::LogLevel::Debug);

  ::testing::InitGoogleMock(&argc, argv);
  ::testing::FLAGS_gtest_death_test_style = "fast";
  return RUN_ALL_TESTS();
}

TEST(DecimalTest, MultiplyTest)
{

  _log_info(LOG_DETAILS, "Numeric Price_t digits:{}", std::numeric_limits<Price_t>::digits);
  _log_info(LOG_DETAILS, "Numeric double digits:{}", std::numeric_limits<double>::digits10);

  _log_info(LOG_DETAILS, "Max Price_t:{}", std::numeric_limits<Price_t>::max());
  _log_info(LOG_DETAILS, "Max double:{}", std::numeric_limits<double>::max());

  _log_info(LOG_DETAILS, "Min Price_t:{}", std::numeric_limits<Price_t>::min());
  _log_info(LOG_DETAILS, "Min double :{}", std::numeric_limits<double>::min());

  _log_info(LOG_DETAILS, "Max Price_t digits:{}", std::numeric_limits<Price_t>::max_digits10);
  _log_info(LOG_DETAILS, "Max double digits:{}", std::numeric_limits<double>::max_digits10);

  const std::string value{"1000000000000.000000000000000001"};
  const std::string value2{"2000000000000.000000000000000002"};
  const std::string value4{"4000000000000.000000000000000004"};

  Price_t decimal{value};

  _log_info(LOG_DETAILS, "Decimal:{}", decimal);

  EXPECT_EQ(decimal.convert_to<std::string>(), value);
  EXPECT_EQ(decimal += decimal, Price_t{value2});
  _log_info(LOG_DETAILS, "Value  :{}", value);
  _log_info(LOG_DETAILS, "Decimal:{}", decimal);

  EXPECT_EQ(decimal *= 2.0, Price_t{value4});

  _log_info(LOG_DETAILS, "Decimal:{}", decimal);

  EXPECT_EQ(decimal /= decimal, 1);
  _log_info(LOG_DETAILS, "Decimal:{}", decimal);
}
