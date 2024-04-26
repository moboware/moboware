#include "benchmark/benchmark.h"
#include "common/logger.hpp"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <chrono>

// using namespace moboware;
using namespace boost::multiprecision;

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./decimal_benchmark.log"};
  std::filesystem::remove(logFilePath);

  Logger::GetInstance().SetLogFile(logFilePath);
  Logger::GetInstance().SetLevel(Logger::LogLevel::Debug);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

static void BM_MultiplyDecimal(benchmark::State &state)
{
  cpp_dec_float_50 decimal{};
  for (auto _ : state) {
    // o2 = std::move(o1);
  }
}

BENCHMARK(BM_MultiplyDecimal);