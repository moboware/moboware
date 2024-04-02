#include "benchmark/benchmark.h"
#include "common/log_stream.h"

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./common_benchmark.log"};
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(moboware::common::NewLogStream::LEVEL::DEBUG);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}
