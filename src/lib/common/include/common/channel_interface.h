#pragma once
#include "common/session.h"
#include <memory>
#include <string>

namespace moboware::common {
class ChannelInterface : public std::enable_shared_from_this<ChannelInterface>
{
public:
  ChannelInterface() = default;
  ChannelInterface(const ChannelInterface&) = default;
  ChannelInterface(ChannelInterface&&) = default;
  ChannelInterface& operator=(const ChannelInterface&) = default;
  ChannelInterface& operator=(ChannelInterface&&) = default;
  virtual ~ChannelInterface() = default;

  virtual void SendData(const uint64_t tag, const std::string& payload) = 0;
};
}