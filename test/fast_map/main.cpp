#include "common/logger.hpp"
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

template <> struct fmt::formatter<UnorderedFastMap_t> : fmt::ostream_formatter {
};

template <> struct fmt::formatter<TestStruct> : fmt::ostream_formatter {
};

int main(const int argc, const char **argv)
{
  Logger::GetInstance().SetLevel(Logger::LogLevel::Info);

  UnorderedFastMap_t fastMap{};
  const auto i987{fastMap.find("987")};
  if (fastMap.end() != i987) {
    _log_info(LOG_DETAILS, "found");
  }

  for (int i = 0; i < 74; i++) {
    TestStruct testStruct{std::to_string(i), 9088.9756, i + 12083};
    const auto iter{fastMap.insert(std::make_pair(std::to_string(i), testStruct))};
    // const auto iter{fastMap.try_emplace(std::to_string(i), std::to_string(i), 9088.9756, i + 12083)};
    if (iter.second) {
    }
  }

  _log_info(LOG_DETAILS, "Fast map:P{}", fastMap);

  const auto key{"9"};
  const auto findIter1{fastMap.find(key)};
  if (findIter1 != fastMap.end()) {
    _log_info(LOG_DETAILS, "Found value:{}", findIter1.second());
  } else {
    _log_info(LOG_DETAILS, "key value {} not found", key);
  }

  const auto eraseIter1{fastMap.erase("5")};
  if (eraseIter1 != fastMap.end()) {
    _log_info(LOG_DETAILS, "key {} value:{}", eraseIter1.first(), eraseIter1.second());
    const auto eraseIter2{fastMap.erase(eraseIter1)};
    if (eraseIter2 != fastMap.end()) {
      _log_info(LOG_DETAILS, "key {}, value:{}", eraseIter2.first(), eraseIter2.second());
    }
  }

  _log_info(LOG_DETAILS, "Bye... fast map:{}", fastMap);
  fastMap.clear();
  return 0;
}