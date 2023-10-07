#include "benchmark/benchmark.h"
#include "common/log_stream.h"
#include <chrono>
#include <deque>
#include <map>
#include <unordered_map>
#include <vector>

// using namespace moboware;
const std::size_t MaxMapSize{100'000UL};

int main(int argc, char **argv)
{
  std::filesystem::path logFilePath{"./smart_search_benchmark.log"};
  std::filesystem::remove(logFilePath);

  LogStream::GetInstance().SetLogFile(logFilePath);
  LogStream::GetInstance().SetLevel(LogStream::LEVEL::DEBUG);

  benchmark::Initialize(&argc, argv);
  benchmark::RunSpecifiedBenchmarks();
  benchmark::Shutdown();
  return 0;
}

class SmartMap {
public:
  SmartMap() = default;

  SmartMap(const SmartMap &) = default;
  SmartMap(SmartMap &&) = default;

  SmartMap &operator=(const SmartMap &rhs) = default;
  SmartMap &operator=(SmartMap &&) = default;

  void insert(const std::string &&key, const std::string &&value)
  {
    m_KeyVector.push_back(key);
    m_ValueVector.push_back(value);
  }

  const std::string *find(const std::string &searchKey)
  {
    if (m_KeyVector.size() != m_ValueVector.size()) {
      throw std::runtime_error("key/value vector out of sync");
    }

    std::size_t i{};

    for (const auto &key : m_KeyVector) {
      if (key == searchKey) {
        return &m_ValueVector[i];
      } else {
        i++;
      }
    }
    return {};
  }

  void erase(const std::string &searchKey)
  {
    if (m_KeyVector.size() != m_ValueVector.size()) {
      throw std::runtime_error("key/value vector out of sync");
    }

    std::size_t i{};
    {
    }

    ValueVector_t::iterator valueIter{};
    for (auto &&keyIter = m_KeyVector.begin(), valueIter = m_ValueVector.begin(); keyIter != m_KeyVector.end();
         ++keyIter, ++valueIter) {
      if (*keyIter == searchKey) {
        m_KeyVector.erase(keyIter);
        m_ValueVector.erase(valueIter);
        return;
      } else {
        i++;
      }
    }
    return;
  }

private:
  using KeyVector_t = std::vector<std::string>;
  KeyVector_t m_KeyVector;

  using ValueVector_t = std::vector<std::string>;
  ValueVector_t m_ValueVector;
};

static void BM_Insert(benchmark::State &state)
{

  SmartMap sm;
  for (auto _ : state) {
    sm.insert("123678451", "djfhvgwo934587t62");
  }
}

BENCHMARK(BM_Insert);

static void BM_Find(benchmark::State &state)
{
  SmartMap sm;
  for (int i = 0; i < MaxMapSize; i++) {
    sm.insert(std::to_string(std::rand()), "123456789098765432101234567890987654321");
  }
  std::srand(MaxMapSize - 1);
  for (auto _ : state) {

    sm.find(std::to_string(std::rand()));
  }
}

BENCHMARK(BM_Find);

static void BM_FindMap(benchmark::State &state)
{
  std::map<std::string, std::string> m;

  for (int i = 0; i < MaxMapSize; i++) {
    m.emplace(std::to_string(std::rand()), "123456789098765432101234567890987654321");
  }

  std::srand(MaxMapSize - 1);
  for (auto _ : state) {

    m.find(std::to_string(std::rand()));
  }
}

BENCHMARK(BM_FindMap);

static void BM_FindUnorderedMap(benchmark::State &state)
{
  std::unordered_map<std::string, std::string> m;

  for (int i = 0; i < MaxMapSize; i++) {
    m.emplace(std::to_string(std::rand()), "123456789098765432101234567890987654321");
  }

  std::srand(MaxMapSize - 1);
  for (auto _ : state) {

    m.find(std::to_string(std::rand()));
  }
}

BENCHMARK(BM_FindUnorderedMap);

static void BM_EraseSmartMap(benchmark::State &state)
{
  std::unordered_map<std::string, std::string> m;

  for (int i = 0; i < MaxMapSize; i++) {
    m.emplace(std::to_string(std::rand()), "123456789098765432101234567890987654321");
  }

  std::srand(MaxMapSize - 1);
  for (auto _ : state) {
    m.erase(std::to_string(std::rand()));
  }
}

BENCHMARK(BM_EraseSmartMap);
