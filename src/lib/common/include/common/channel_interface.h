#pragma once
#include "common/session.h"
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
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

  virtual void SendWebSocketData(const boost::asio::const_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) = 0;
};
}