#include "common/logger.hpp"
#include "shared_memory/consumer.hpp"
#include "shared_memory/publisher.hpp"
#include <chrono>
#include <signal.h>
#include <thread>

bool done{false};

void SignalInt(int sig)
{                // can be called asynchronously
  done = true;   // set flag
}

int main(const int argc, const char **argv)
{
  signal(SIGINT, SignalInt);
  Logger::GetInstance().SetLevel(Logger::LogLevel::Info);
  const std::string memoryName{"SharedMemoryTest"};
  if (argc == 2) {
    Logger::GetInstance().SetLogFile("shared_memory_publisher.log");

    _log_info(LOG_DETAILS, "Shared memory publisher {}", memoryName);

    SharedMemoryPublisher smp(memoryName);
    std::uint64_t n{};
    while (!done) {
      _log_info(LOG_DETAILS, "Start writing...");
      for (std::uint64_t i = 0; i < 25'000; i++) {
        const auto msg{std::to_string(++n)};
        smp.Write({msg.data(), msg.size()});
      }
      // notify all consumers
      smp.Notify();

      _log_info(LOG_DETAILS, "Ready...{}", n);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } else {
    Logger::GetInstance().SetLogFile("shared_memory_consumer.log");

    _log_info(LOG_DETAILS, "Shared memory consumer {}", memoryName);

    SharedMemoryConsumer smc(memoryName);

    char readBuffer[1024];
    do {
      while (not done and smc.Read(readBuffer, sizeof(readBuffer), std::chrono::milliseconds(1000))) {
      }

    } while (!done);
  }
  _log_info(LOG_DETAILS, "Bye...");
  return 0;
}