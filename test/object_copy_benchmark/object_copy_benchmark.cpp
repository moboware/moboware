#include "benchmark/benchmark.h"
#include "common/log_stream.h"
#include <chrono>

// using namespace moboware;

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./object_copy_benchmark.log"};
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(moboware::common::NewLogStream::LEVEL::DEBUG);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

class Object {
public:
  Object() = default;

  explicit Object(const std::string &_S1,
                  const std::string &_S2,
                  const double _D1,
                  const double _D2,
                  const std::chrono::system_clock::time_point &_T1,
                  const std::chrono::system_clock::time_point &_T2,
                  const std::uint64_t _UL1,
                  const std::uint64_t _UL2)
      : S1(_S1)
      , S2(_S2)
      , D1(_D1)
      , D2(_D2)
      , T1(_T1)
      , T2(_T2)
      , UL1(_UL1)
      , UL2(_UL2)
  {
  }

  Object(const Object &) = default;
  Object(Object &&) = default;

  Object &operator=(const Object &rhs)
  {
    if (this != &rhs) {
      S1 = rhs.S1;
      S2 = rhs.S2;
      D1 = rhs.D1;
      D2 = rhs.D2;
      T1 = rhs.T1;
      T2 = rhs.T2;
      UL1 = rhs.UL1;
      UL2 = rhs.UL2;
    }

    return *this;
  }

  Object &SmartCopy(const Object &rhs)
  {
    if (this != &rhs) {
      if (S1 != rhs.S1) {
        S1 = rhs.S1;
      }
      if (S1 != rhs.S1) {
        S1 = rhs.S1;
      }
      if (S2 != rhs.S2) {
        S2 = rhs.S2;
      }
      if (D1 != rhs.D1) {
        D1 = rhs.D1;
      }
      if (D2 != rhs.D2) {
        D2 = rhs.D2;
      }
      if (T1 != rhs.T1) {
        T1 = rhs.T1;
      }
      if (T2 != rhs.T2) {
        T2 = rhs.T2;
      }
      if (UL1 != rhs.UL1) {
        UL1 = rhs.UL1;
      }
      if (UL2 != rhs.UL2) {
        UL2 = rhs.UL2;
      }
    }

    return *this;
  }

  Object &operator=(Object &&) = default;

  std::string S1{};
  std::string S2{};
  double D1{};
  double D2{};
  std::chrono::system_clock::time_point T1{};
  std::chrono::system_clock::time_point T2{};
  std::uint64_t UL1{};
  std::uint64_t UL2{};
};

// test the scenario that a copy is made via the copy or = operator, that will copy the full object into the *this
static void BM_FullObjectCopy(benchmark::State &state)
{
  Object o1("12563412785649276539875o2498u56y948u69p4ij gldfk",
            "12563412785649276539875sdfjklngh liodujhglodig loi",
            24234578925.948679867,
            24234578925.34676789867,
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            67896798693675298,
            64586789300675298);
  Object o2;

  for (auto _ : state) {
    o2.operator=(o1);
  }
}

BENCHMARK(BM_FullObjectCopy);

static void BM_SmartCopy(benchmark::State &state)
{
  Object o1("12563412785649276539875o2498u56y948u69p4ij gldfk",
            "12563412785649276539875sdfjklngh liodujhglodig loi",
            24234578925.948679867,
            24234578925.34676789867,
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            67896798693675298,
            64586789300675298);

  Object o2;
  o2.S1 = o1.S1;
  o2.D1 = o1.D1;
  o2.T1 = o2.T1;
  o2.UL1 = o1.UL1;

  for (auto _ : state) {
    o2.SmartCopy(o1);
  }
}

BENCHMARK(BM_SmartCopy);

static void BM_Move(benchmark::State &state)
{
  Object o1("12563412785649276539875o2498u56y948u69p4ij gldfk",
            "12563412785649276539875sdfjklngh liodujhglodig loi",
            24234578925.948679867,
            24234578925.34676789867,
            std::chrono::system_clock::now(),
            std::chrono::system_clock::now(),
            67896798693675298,
            64586789300675298);

  Object o2;

  for (auto _ : state) {
    o2 = std::move(o1);
  }
}

BENCHMARK(BM_Move);