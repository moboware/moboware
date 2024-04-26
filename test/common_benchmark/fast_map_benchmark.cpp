#include "benchmark/benchmark.h"
#include "common/logger.hpp"
#include "common/unordered_fast_map.hpp"
#include <map>
#include <unordered_map>

using namespace moboware;

struct MapStruct {
  MapStruct(std::string &&str, double d, std::uint64_t l)
      : m_String(std::move(str))
      , m_Double(d)
      , m_Long(l)
  {
  }

  // todo get/set
  std::string m_String;
  double m_Double{};
  std::uint64_t m_Long{};
};

using FastMap_t = common::unordered_fast_map<std::string, MapStruct>;
using Map_t = std::map<std::string, MapStruct>;

// static void BM_FastMapTryEmplace(benchmark::State &state)
//{
//   FastMap_t  fastMap;
//   std::uint64_t i{};
//   for (auto _ : state) {
//     fastMap.try_emplace(std::move(std::to_string(++i)), std::move(i));
//   }
// }
//
// BENCHMARK(BM_FastMapTryEmplace);

static void BM_FastMapInsert(benchmark::State &state)
{
  FastMap_t fastMap;
  std::uint64_t i{};
  for (auto _ : state) {
    fastMap.insert(std::make_pair(std::to_string(++i), MapStruct{std::to_string(i), double(i), i}));
  }
}

BENCHMARK(BM_FastMapInsert);

// static void BM_FastMapTryEmplaceWithReserve(benchmark::State &state)
//{
//   FastMap_t  fastMap{10'000};
//   std::uint64_t i{};
//   for (auto _ : state) {
//     fastMap.try_emplace(std::to_string(++i), i);
//   }
// }

// BENCHMARK(BM_FastMapTryEmplaceWithReserve);
static void BM_NormalMapInsert(benchmark::State &state)
{
  Map_t map;
  std::uint64_t i{};
  for (auto _ : state) {
    map.insert(std::make_pair(std::to_string(++i), MapStruct{std::to_string(i), double(i), i}));
  }
}

BENCHMARK(BM_NormalMapInsert);

static void BM_NormalMapTryEmplace(benchmark::State &state)
{
  Map_t map;
  std::uint64_t i{};
  for (auto _ : state) {
    map.try_emplace(std::to_string(++i), MapStruct{std::to_string(i), double(i), i});
  }
}

BENCHMARK(BM_NormalMapTryEmplace);

static void BM_UnorderedMapTryEmplace(benchmark::State &state)
{
  Map_t map;
  std::uint64_t i{};
  for (auto _ : state) {
    map.try_emplace(std::to_string(++i), MapStruct{std::to_string(i), double(i), i});
  }
}

BENCHMARK(BM_UnorderedMapTryEmplace);