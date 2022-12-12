#pragma once

#include "common/service.h"
#include "common/timer.h"
#include "web_socket/web_socket_session.h"
#include "web_socket/web_socket_session_callback.h"
#include <boost/beast/websocket.hpp>

namespace moboware::web_socket {

class WebSocketClient
  : public WebSocketSessionCallback
  , public std::enable_shared_from_this<WebSocketClient>
{
public:
  explicit WebSocketClient(const std::shared_ptr<moboware::common::Service>& service);
  WebSocketClient(const WebSocketClient&) = delete;
  WebSocketClient(WebSocketClient&&) = delete;
  WebSocketClient& operator=(const WebSocketClient&) = delete;
  WebSocketClient& operator=(WebSocketClient&&) = delete;
  ~WebSocketClient() = default;

  [[nodiscard]] auto Start(const std::string& address, const short port) -> bool;
  [[nodiscard]] auto SendWebSocketData(const boost::asio::const_buffer& sendBuffer) -> std::size_t;

private:
  void OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) final;

  // called when the session is closed and the session can be cleaned up.
  void OnSessionClosed() final;

  const std::shared_ptr<moboware::common::Service> m_Service;
  std::shared_ptr<moboware::web_socket::WebSocketSession> m_Session;
};
}