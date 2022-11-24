#include "common/log_stream.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

int main(int argc, char* argv[])
{
  LogStream::GetInstance().SetLevel(LogStream::DEBUG);

  ::testing::InitGoogleMock(&argc, argv);
  ::testing::FLAGS_gtest_death_test_style = "fast";
  return RUN_ALL_TESTS();
}
