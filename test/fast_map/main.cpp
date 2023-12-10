#include "common/log_stream.h"
#include "common/unordered_fast_map.hpp"
#include <iostream>

using namespace moboware::common;

/// @brief
struct TestStruct {
  std::string m_string;
  double m_Double{};
  long m_Long{};

  bool operator==(const TestStruct &rhs) const
  {
    return m_string == rhs.m_string and   //
           m_Double == rhs.m_Double and   //
           m_Long == rhs.m_Long;
  }
};

std::ostream &operator<<(std::ostream &os, const TestStruct &rhs)
{
  return os << "{" << rhs.m_string << "," << rhs.m_Double << "," << rhs.m_Long << "}";
}

using UnorderedFastMap_t = unordered_fast_map<std::string, TestStruct>;

std::ostream &operator<<(std::ostream &os, const UnorderedFastMap_t &rhs)
{
  os << "{";

  for (const auto &&kv : rhs) {
    os << "{" << kv.first << "=" << kv.second << "},";
  }

  return os << "}";
}

int main(const int argc, const char **argv)
{
  LogStream::GetInstance().SetLevel(LogStream::LEVEL::INFO);

  UnorderedFastMap_t fastMap{};
  const auto i987{fastMap.find("987")};
  if (fastMap.end() != i987) {
    LOG_INFO("found");
  }

  for (int i = 0; i < 74; i++) {
    TestStruct testStruct{std::to_string(i), 9088.9756, i + 12083};
    const auto iter{fastMap.insert(std::make_pair(std::to_string(i), testStruct))};
    // const auto iter{fastMap.try_emplace(std::to_string(i), std::to_string(i), 9088.9756, i + 12083)};
    if (iter.second) {
    }
  }

  LOG_INFO("Fast map:" << fastMap);

  const auto key{"9"};
  const auto findIter1{fastMap.find(key)};
  if (findIter1 != fastMap.end()) {
    LOG_INFO("Found value:" << findIter1.second());
  } else {
    LOG_INFO("value " << key << " not found ");
  }

  const auto eraseIter1{fastMap.erase("5")};
  if (eraseIter1 != fastMap.end()) {
    LOG_INFO("key " << eraseIter1.first() << " value:" << eraseIter1.second());
    const auto eraseIter2{fastMap.erase(eraseIter1)};
    if (eraseIter2 != fastMap.end()) {
      LOG_INFO("key " << eraseIter2.first() << " value:" << eraseIter2.second());
    }
  }

  LOG_INFO("Bye... fast map:" << fastMap);
  fastMap.clear();
  return 0;
}