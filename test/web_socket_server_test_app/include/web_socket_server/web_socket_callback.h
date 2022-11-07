#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace moboware {

class WebSocketCallback {
 public:
  WebSocketCallback() = default;
  virtual ~WebSocketCallback() = default;
  WebSocketCallback(const WebSocketCallback&) = default;
  WebSocketCallback(WebSocketCallback&&) = default;
  WebSocketCallback& operator=(const WebSocketCallback&) = default;
  WebSocketCallback& operator=(WebSocketCallback&&) = default;

  virtual void OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) = 0;
};
}  // namespace moboware