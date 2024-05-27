#include "common/logger.hpp"
#include <chrono>

using namespace moboware::common;

int main(int argc, char **argv)
{
  Logger::GetInstance().SetLevel(Logger::LogLevel::Debug);

  LOG_DEBUG("No parameter log line in debug ");
  LOG_DEBUG("one parameter log line in debug {}", __FUNCTION__);
  LOG_DEBUG("one parameter log line in debug {} {} {} {} {} {} {} {}",
            __FUNCTION__,
            __FILE__,
            42.43,
            124.4456,
            389249345678345678,   //
            true,
            0xFff003,
            std::chrono::milliseconds(100));

  LOG_TRACE("one parameter log line in debug {} {} {} {} {} {} {} {}",
            __FUNCTION__,
            __FILE__,
            42.43,
            124.4456,
            389249345678345678,   //
            true,
            0xFff003,
            std::chrono::milliseconds(100));
  LOG_INFO("one parameter log line in debug {} ", 42);
  LOG_ERROR("one parameter log line in debug {} ", __LINE__);
  LOG_FATAL("one parameter log line in debug {} ", __LINE__);

  Logger::GetInstance().SetLogFile("./new_logger.log");

  const auto producerThreadFunction{[&](const std::stop_token &stop_token) {
    std::uint64_t sequence{};
    while (not stop_token.stop_requested()) {
      for (int i = 0; i < 1'000u; i++) {
        LOG_INFO("Waiting in debug {}, {}, {}, {}, {:x}, {}",
                 sequence++,           //
                 124.4456,             //
                 389249345678345678,   //
                 true,
                 0xFff003,
                 std::chrono::milliseconds(100));
      }

      std::this_thread::sleep_for(std::chrono::nanoseconds(1));
    }
  }};

  std::vector<std::jthread> producers;
  for (int i = 0; i < 2; i++) {
    producers.emplace_back(std::jthread(producerThreadFunction));
  }

  std::this_thread::sleep_for(std::chrono::seconds(10));
  return 0;
}