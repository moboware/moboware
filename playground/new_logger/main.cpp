#include "./logger.hpp"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace moboware::common;

int main(int argc, char **argv)
{
  _Logger.SetLogFile("./new_logger.log");

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

      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
  }};

  std::vector<std::jthread> producers;
  for (int i = 0; i < 5; i++) {
    producers.emplace_back(std::jthread(producerThreadFunction));
  }

  std::this_thread::sleep_for(std::chrono::seconds(10));

  return 0;
}