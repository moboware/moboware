#include "common/logger.hpp"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char *argv[])
{
  Logger::GetInstance().SetLevel(Logger::LogLevel::Debug);

  ::testing::InitGoogleMock(&argc, argv);
  ::testing::FLAGS_gtest_death_test_style = "fast";
  return RUN_ALL_TESTS();
}
