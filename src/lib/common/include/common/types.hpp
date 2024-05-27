#pragma once

#include "common/service.h"
#include <chrono>
#include <cstdint>

namespace moboware::common {

using SessionTime_t = std::chrono::steady_clock;
using SessionTimePoint_t = SessionTime_t::time_point;

using SystemTime_t = std::chrono::system_clock;
using SystemTimePoint_t = SystemTime_t::time_point;

using ServicePtr = std::shared_ptr<Service>;
}   // namespace moboware::common