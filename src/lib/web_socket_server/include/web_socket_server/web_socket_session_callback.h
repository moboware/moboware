#pragma once

#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>

namespace moboware::web_socket {

class WebSocketSessionCallback
{
public:
  WebSocketSessionCallback() = default;
  virtual ~WebSocketSessionCallback() = default;
  WebSocketSessionCallback(const WebSocketSessionCallback&) = default;
  WebSocketSessionCallback(WebSocketSessionCallback&&) = default;
  WebSocketSessionCallback& operator=(const WebSocketSessionCallback&) = default;
  WebSocketSessionCallback& operator=(WebSocketSessionCallback&&) = default;

  // called when data is received from the web socket.
  virtual void OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) = 0;

  // called when the session is closed and the session can be cleaned up.
  virtual void OnSessionClosed() = 0;
};
} // namespace moboware