#pragma once

#include "common/logger.hpp"
#include "common/service.h"
#include "common/timer.h"
#include "common/types.hpp"
#include "socket/socket_session_base.hpp"
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>

namespace moboware::web_socket {
/**
 * @brief shared class for web socket client or server implementation
 */
template <typename TSessionCallback> class WebSocketClientServer {
public:
  virtual ~WebSocketClientServer() = default;
  WebSocketClientServer(const WebSocketClientServer &) = delete;
  WebSocketClientServer(WebSocketClientServer &&) = delete;
  WebSocketClientServer &operator=(const WebSocketClientServer &) = delete;
  WebSocketClientServer &operator=(WebSocketClientServer &&) = delete;

protected:
  explicit WebSocketClientServer(const common::ServicePtr &service, TSessionCallback &sessionCallback)
      : m_Service{service}
      , m_Strand(boost::asio::make_strand(service->GetIoService()))
      , m_SslContext{boost::asio::ssl::context::tlsv12}
      , m_PingTimer{service}
      , m_SessionCallback(sessionCallback)
  {
  }

  [[nodiscard]] virtual bool Start(const std::string &address, const std::uint16_t port) = 0;
  [[nodiscard]] virtual bool SendPingRequest() = 0;

  void StartPingTimer(const std::chrono::milliseconds &pingTime)
  {
    m_PingTimer.Start(
        [&](common::Timer &timer) {
          SendPingRequest();

          timer.Restart();
        },
        pingTime);
  }

  const common::ServicePtr m_Service;
  boost::asio::strand<boost::asio::io_context::executor_type> m_Strand;
  boost::asio::ssl::context m_SslContext;
  common::Timer m_PingTimer;
  TSessionCallback &m_SessionCallback{};
};
}   // namespace moboware::web_socket