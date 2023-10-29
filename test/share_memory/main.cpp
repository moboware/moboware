#include "common/log_stream.h"
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
  LogStream::GetInstance().SetLevel(LogStream::LEVEL::INFO);
  const std::string memoryName{"SharedMemoryTest"};
  if (argc == 2) {
    LogStream::GetInstance().SetLogFile("shared_memory_publisher.log");

    LOG_INFO("Shared memory publisher " << memoryName);

    SharedMemoryPublisher smp(memoryName);
    std::uint64_t n{};
    while (!done) {
      LOG_INFO("Start writing...");
      for (std::uint64_t i = 0; i < 25'000; i++) {
        const auto msg{std::to_string(++n)};
        smp.Write({msg.data(), msg.size()});
      }
      // notify all consumers
      smp.Notify();

      LOG_INFO("Ready..." << n);
      std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
  } else {
    LogStream::GetInstance().SetLogFile("shared_memory_consumer.log");

    LOG_INFO("Shared memory consumer " << memoryName);

    SharedMemoryConsumer smc(memoryName);

    char readBuffer[1024];
    do {
      while (not done and smc.Read(readBuffer, sizeof(readBuffer), std::chrono::milliseconds(1000))) {
      }

    } while (!done);
  }
  LOG_INFO("Bye...");
  return 0;
}