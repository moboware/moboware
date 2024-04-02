#include "benchmark/benchmark.h"
#include "common/log_stream.h"
#include <boost/multiprecision/cpp_dec_float.hpp>
#include <chrono>

// using namespace moboware;
using namespace boost::multiprecision;

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./decimal_benchmark.log"};
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(moboware::common::NewLogStream::LEVEL::DEBUG);

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