#include "common/log_stream.h"
#include <chrono>
#include <iomanip>
#include <iostream>
#include <sstream>

using namespace moboware::common;

int main(int argc, char **argv)
{
  LogStream::GetInstance().SetLogFile("./new_logger.log");

  const auto producerThreadFunction{[&](const std::stop_token &stop_token) {
    std::uint64_t sequence{};
    while (not stop_token.stop_requested()) {
      LOG_DEBUG("waoieruiower<<" << sequence++ << "," << 124.4456 << ","    //
                                 << std::dec << 389249345678345678 << ","   //
                                 << std::boolalpha << true << ","           //
                                 << std::hex << 0xFff003 << ","             //
                                 << std::dec << std::chrono::milliseconds(100));
      std::this_thread::sleep_for(std::chrono::nanoseconds(10));
    }
  }};

  std::vector<std::jthread> producers;
  for (int i = 0; i < 1; i++) {
    producers.emplace_back(std::jthread(producerThreadFunction));
  }

  for (std::size_t i = 0; i < 1'000'000u; i++) {
    std::this_thread::sleep_for(std::chrono::nanoseconds(100));
    LOG_INFO("Waiting ......");
  }

  return 0;
}