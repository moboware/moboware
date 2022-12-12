#include "benchmark/benchmark.h"
#include "common/log_stream.h"

int main(int argc, char** argv)
{
  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

static void BM_LogStream(benchmark::State& state)
{
  std::filesystem::path logFilePath{ "./logstream_benchmark.log" };
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(LogStream::DEBUG);

  const auto intValue{ 82345923745U };
  const auto doubleValue{ 82345923745.9485693458763 };
  const auto stringValue{ "j hkledhrglkjsdhfkljghkljh lksjhdklfgjhs dg" };

  for (auto _ : state) {
    LOG_DEBUG("Debug string benchmark " << intValue << "," << doubleValue << "," << stringValue);
    LOG_INFO("Info string benchmark " << intValue << "," << doubleValue << "," << stringValue);
    LOG_FATAL("Fatal string benchmark " << intValue << "," << doubleValue << "," << stringValue);
    LOG_ERROR("Error string benchmark " << intValue << "," << doubleValue << "," << stringValue);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_LogStream);
