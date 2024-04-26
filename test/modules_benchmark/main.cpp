#include "benchmark/benchmark.h"
#include "common/logger.hpp"

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./modules_benchmark.log"};
  std::filesystem::remove(logFilePath);

  Logger::GetInstance().SetLogFile(logFilePath);
  Logger::GetInstance().SetLevel(Logger::LogLevel::Error);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}