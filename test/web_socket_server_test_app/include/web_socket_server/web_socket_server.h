#pragma once

#include <map>
#include "common/service.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_session.h"

namespace moboware {

/**
 * @brief
 *
 */
class WebSocketServer {
 public:
  explicit WebSocketServer(const std::shared_ptr<moboware::common::Service>& service, const std::shared_ptr<WebSocketCallback>& callback);
  WebSocketServer(const WebSocketServer&);
  WebSocketServer(WebSocketServer&&);
  WebSocketServer& operator=(const WebSocketServer&);
  WebSocketServer& operator=(WebSocketServer&&);
  ~WebSocketServer() = default;

  [[nodiscard]] bool Start(const std::string address, const short port);

 private:
  void Accept();
  void StartSessionTimer();
  std::size_t CheckClosedSessions();

  const std::shared_ptr<moboware::common::Service> m_Service;
  const std::shared_ptr<WebSocketCallback> m_WebSocketChannelCallback;

  boost::asio::ip::tcp::acceptor m_Acceptor;

  using Sessions_t = std::map<std::pair<boost::asio::ip::address, boost::asio::ip::port_type>, std::shared_ptr<moboware::WebSocketSession>>;
  Sessions_t m_Sessions;
  common::Timer m_sessionTimer;
};
}  // namespace moboware
