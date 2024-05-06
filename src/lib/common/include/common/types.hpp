#pragma once
#include "common/service.h"
#include <chrono>

namespace moboware::common {

using SystemTimePoint_t = std::chrono::steady_clock::time_point;
using ServicePtr = std::shared_ptr<Service>;
}   // namespace moboware::common