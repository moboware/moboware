#include "benchmark/benchmark.h"
#include "common/logger.hpp"

static void BM_LogStream(benchmark::State &state)
{
  const auto intValue{82345923745U};
  const auto doubleValue{82345923745.9485693458763};
  const auto stringValue{"j hkledhrglkjsdhfkljghkljh lksjhdklfgjhs dg"};

  for (auto _ : state) {
    const auto level{static_cast<LogStream::LEVEL>(state.range(0))};
    LOG(level,
        "Level=" << Logger::GetInstance().GetLevelString() << ", String benchmark " << intValue << "," << doubleValue << ","
                 << stringValue);
  }
}

// Register the function as a benchmark
BENCHMARK(BM_LogStream)
    ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::DEBUG))   //
    ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::INFO))    //
    ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::WARN))    //
    ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::ERROR))   //
    ->Arg(static_cast<std::int64_t>(LogStream::LEVEL::FATAL))
    ->Repetitions(50)
    ->Threads(7);
