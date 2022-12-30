#include "benchmark/benchmark.h"
#include "common/log_stream.h"

int main(int argc, char** argv)
{
  std::filesystem::path logFilePath{ "./logstream_benchmark.log" };
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(LogStream::LEVEL::DEBUG);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

static void BM_LogStream(benchmark::State& state)
{
  const auto intValue{ 82345923745U };
  const auto doubleValue{ 82345923745.9485693458763 };
  const auto stringValue{ "j hkledhrglkjsdhfkljghkljh lksjhdklfgjhs dg" };

  for (auto _ : state) {
    const auto level{ static_cast<LogStream::LEVEL>(state.range(0)) };
    LOG(level, "Level=" << LogStream::GetInstance().GetLevelString() << ", String benchmark " << intValue << "," << doubleValue << "," << stringValue);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_LogStream)
  ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::DEBUG)) //
  ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::INFO))  //
  ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::WARN))  //
  ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::ERROR)) //
  ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::FATAL))
  ->Repetitions(50)
  ->Threads(7);
