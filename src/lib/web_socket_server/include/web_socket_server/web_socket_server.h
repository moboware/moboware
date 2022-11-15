#pragma once

#include "common/service.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_session.h"
#include "web_socket_server/web_socket_session_callback.h"
#include <map>

namespace moboware::web_socket {

/**
 * @brief Web socket server
 */
class WebSocketServer
  : public WebSocketSessionCallback
  , public std::enable_shared_from_this<WebSocketServer>
{
public:
  explicit WebSocketServer(const std::shared_ptr<moboware::common::Service>& service);
  WebSocketServer(const WebSocketServer&);
  WebSocketServer(WebSocketServer&&);
  WebSocketServer& operator=(const WebSocketServer&);
  WebSocketServer& operator=(WebSocketServer&&);
  ~WebSocketServer() = default;

  [[nodiscard]] auto Start(const std::string address, const short port) -> bool;
  [[nodiscard]] auto SendWebSocketData(const boost::asio::const_buffer& sendBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) -> bool;

  using WebSocketDataReceivedFn = std::function<void(const boost::beast::flat_buffer& sendBuffer, const boost::asio::ip::tcp::endpoint& endpoint)>;
  void SetWebSocketDataReceived(const WebSocketDataReceivedFn& fn);

private:
  void OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) final;
  void OnSessionClosed() final;

  void Accept();
  auto CheckClosedSessions() -> std::size_t;

  const std::shared_ptr<moboware::common::Service> m_Service;

  boost::asio::ip::tcp::acceptor m_Acceptor;

  using endpointPair_t = std::pair<boost::asio::ip::address, boost::asio::ip::port_type>;
  using Sessions_t = std::map<endpointPair_t, std::shared_ptr<moboware::web_socket::WebSocketSession>>;
  Sessions_t m_Sessions;

  WebSocketDataReceivedFn m_WebSocketDataReceivedFn;
};
} // namespace moboware
