#pragma once

#include "common/logger.hpp"
#include "socket/web_socket_client_server.hpp"
#include "socket/web_socket_session.hpp"
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace moboware::web_socket {

template <typename TSessionCallback>   //
class WebSocketClient : public WebSocketClientServer<TSessionCallback> {
public:
  explicit WebSocketClient(const moboware::common::ServicePtr &service, TSessionCallback &sessionCallback);
  WebSocketClient(const WebSocketClient &) = delete;
  WebSocketClient(WebSocketClient &&) = delete;
  WebSocketClient &operator=(const WebSocketClient &) = delete;
  WebSocketClient &operator=(WebSocketClient &&) = delete;
  virtual ~WebSocketClient() = default;

  inline void SetTarget(const std::string &target)
  {
    m_Target = target;
  }

  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) final;
  [[nodiscard]] auto SendWebSocketData(const boost::asio::const_buffer &sendBuffer) -> std::size_t;

private:
  bool SendPingRequest() final;
  using WebSocketClientServer_t = WebSocketClientServer<TSessionCallback>;
  using WebSocketSession_t = WebSocketSession<TSessionCallback>;
  std::shared_ptr<WebSocketSession_t> m_Session;
  std::string m_Target{"/"};
};

template <typename TSessionCallback>   //
WebSocketClient<TSessionCallback>::WebSocketClient(const moboware::common::ServicePtr &service, TSessionCallback &sessionCallback)
  : WebSocketClientServer_t(service, sessionCallback)
{
}

template <typename TSessionCallback>   //
auto WebSocketClient<TSessionCallback>::Start(const std::string &address, const std::uint16_t port) -> bool
{
  LOG_TRACE("Connecting web socket client, {}:{}", address, port);

  boost::asio::ip::tcp::socket webSocket(WebSocketClientServer_t::m_Strand.get_inner_executor());
  m_Session = std::make_shared<WebSocketSession_t>(
    WebSocketClientServer_t::m_Service,
    WebSocketClientServer_t::m_SslContext,
    WebSocketClientServer_t::m_SessionCallback,
    std::move(webSocket),
    [&](const boost::asio::ip::tcp::endpoint &endpoint) {
      //
      WebSocketClientServer_t::m_SessionCallback.OnSessionClosed(endpoint);
    },
    m_Target);

  if (m_Session->Connect(address, port)) {
    // start ping timer
    WebSocketClientServer_t::StartPingTimer(std::chrono::milliseconds(3000));
    return true;
  }
  return false;
}

template <typename TSessionCallback>   //
auto WebSocketClient<TSessionCallback>::SendWebSocketData(const boost::asio::const_buffer &sendBuffer) -> std::size_t
{
  return m_Session->SendWebSocketData(sendBuffer);
}

template <typename TSessionCallback>   //
bool WebSocketClient<TSessionCallback>::SendPingRequest()
{
  return m_Session->SendPingRequest();
}

}   // namespace moboware::web_socket