#pragma once

#include "common/logger.hpp"
#include "socket/server_certificates.hpp"
#include "socket/web_socket_client_server.hpp"
#include "socket/web_socket_session.hpp"
#include <boost/asio/dispatch.hpp>
#include <boost/asio/executor.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>
#include <map>

namespace moboware::web_socket {

/**
 * @brief Web socket server, must be created with a shared_ptr.
 */
template <typename TSessionCallback>   //
class WebSocketServer : public WebSocketClientServer<TSessionCallback> {
public:
  explicit WebSocketServer(const std::shared_ptr<moboware::common::Service> &service, TSessionCallback &sessionCallback);
  WebSocketServer(const WebSocketServer &) = delete;
  WebSocketServer(WebSocketServer &&) = delete;
  WebSocketServer &operator=(const WebSocketServer &) = delete;
  WebSocketServer &operator=(WebSocketServer &&) = delete;
  ~WebSocketServer() = default;

  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) final;
  [[nodiscard]] bool SendWebSocketData(const boost::asio::const_buffer &sendBuffer,
                                       const boost::asio::ip::tcp::endpoint &remoteEndPoint);
  void SendToAllClients(const boost::asio::const_buffer &sendBuffer);

  bool HasConnectedClients() const
  {
    return not m_Sessions.empty();
  }

private:
  using WebSocketClientServer_t = WebSocketClientServer<TSessionCallback>;
  using WebSocketSession_t = WebSocketSession<TSessionCallback>;

  void Accept();
  std::size_t CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint);
  void SendPingRequest() final;

  boost::asio::ip::tcp::acceptor m_Acceptor;

  using endpointPair_t = std::pair<boost::asio::ip::address, boost::asio::ip::port_type>;

  using Sessions_t = std::map<endpointPair_t, std::shared_ptr<WebSocketSession_t>>;

  Sessions_t m_Sessions;
};

template <typename TSessionCallback>
WebSocketServer<TSessionCallback>::WebSocketServer(const std::shared_ptr<moboware::common::Service> &service,
                                                   TSessionCallback &sessionCallback)
    : WebSocketClientServer_t(service, sessionCallback)
    , m_Acceptor(WebSocketClientServer_t::m_Strand)
{
  LoadServerCertificate(WebSocketClientServer_t::m_SslContext);
}

template <typename TSessionCallback>   //
bool WebSocketServer<TSessionCallback>::Start(const std::string &address, const std::uint16_t port)
{
  boost::asio::ip::tcp::resolver resolver(WebSocketClientServer_t::m_Strand.get_inner_executor());
  boost::system::error_code ec{};
  const auto resolveResults{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Resolving address failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  // create end point
  const boost::asio::ip::tcp::endpoint &endpoint{resolveResults.begin()->endpoint()};

  // Open the acceptor
  m_Acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Open acceptor failed:{}", ec.what());
    return false;
  }

  // Allow address reuse
  m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) {
    _log_error(LOG_DETAILS, "set_option failed:{}", ec.what());
    return false;
  }

  // Bind to the server address
  m_Acceptor.bind(endpoint, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "bind failed:{}", ec.what());
    return false;
  }

  // Start listening for connections
  m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "start listen failed:{}", ec.what());
    return false;
  }

  // start accepting connections
  Accept();

  _log_info(LOG_DETAILS, "WebSocket server started");

  // start ping timer
  WebSocketClientServer_t::StartPingTimer(std::chrono::milliseconds(3000));

  return true;
}

template <typename TSessionCallback>   //
std::size_t WebSocketServer<TSessionCallback>::CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint)
{
  const auto endpointPair{std::make_pair(remoteEndpoint.address(), remoteEndpoint.port())};
  const auto iter{m_Sessions.find(endpointPair)};
  if (iter != m_Sessions.end()) {

    const auto session = iter->second;
    if (not session->IsOpen()) {
      m_Sessions.erase(iter);
      _log_trace(LOG_DETAILS, "Removed 'closed' session {}:{}", remoteEndpoint.address().to_string(), remoteEndpoint.port());
    }
  }
  return m_Sessions.size();
}

template <typename TSessionCallback>   //
void WebSocketServer<TSessionCallback>::Accept()
{
  const auto acceptorFunc = [&](boost::system::error_code ec, boost::asio::ip::tcp::socket webSocket) {
    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Failed to accept connection:{}", ec.what());
    } else {
      // create session and store in our session list
      const auto endPointKey = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());

      const auto session = std::make_shared<WebSocketSession_t>(WebSocketClientServer_t ::m_Service,
                                                                WebSocketClientServer_t ::m_SslContext,
                                                                WebSocketClientServer_t::m_SessionCallback,
                                                                std::move(webSocket),
                                                                [&](const boost::asio::ip::tcp::endpoint &endpoint) {
                                                                  CheckClosedSessions(endpoint);
                                                                });

      const auto pair{m_Sessions.emplace(endPointKey, session)};

      if (pair.second and session->Accept()) {
        _log_info(LOG_DETAILS,
                  "Connection accepted from {}:{}",
                  session->GetRemoteEndpoint().address().to_string(),
                  session->GetRemoteEndpoint().port());
        // store this new session in the sessions list
      } else {
        m_Sessions.erase(pair.first);
      }
    }
    // wait for the next connection
    WebSocketServer<TSessionCallback>::Accept();
  };

  m_Acceptor.async_accept(WebSocketClientServer_t::m_Strand, boost::beast::bind_front_handler(acceptorFunc));
}

template <typename TSessionCallback>   //
bool WebSocketServer<TSessionCallback>::SendWebSocketData(const boost::asio::const_buffer &sendBuffer,
                                                          const boost::asio::ip::tcp::endpoint &remoteEndPoint)
{
  const auto endPointKey = std::make_pair(remoteEndPoint.address(), remoteEndPoint.port());
  const auto iter = m_Sessions.find(endPointKey);
  if (iter != std::end(m_Sessions)) {

    const auto &session = iter->second;
    return session->SendWebSocketData(sendBuffer);
  }

  _log_error(LOG_DETAILS,
             "No endpoint not found to send data to {}:{}",
             remoteEndPoint.address().to_string(),
             remoteEndPoint.port());
  return false;
}

template <typename TSessionCallback>   //
void WebSocketServer<TSessionCallback>::SendToAllClients(const boost::asio::const_buffer &sendBuffer)
{
  for (const auto &[k, v] : m_Sessions) {
    const auto &session = v;
    if (not session->SendWebSocketData(sendBuffer)) {
      _log_error(LOG_DETAILS, "Failed to send data to client: {}:{}", k.first.to_string(), k.second);
    }
  }
}

// template <typename TSessionCallback>   //
// void WebSocketServer<TSessionCallback>::OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
//{
//   CheckClosedSessions(endpoint);
//
//   // const auto removeSessionFn{[this, endpoint]() {
//   //   CheckClosedSessions(endpoint);
//   // }};
//   //
//   // m_Service->GetIoService().post(removeSessionFn);
//
//   if (m_WebSocketClosedFn) {
//     m_WebSocketClosedFn(endpoint);
//   }
// }

template <typename TSessionCallback>   //
void WebSocketServer<TSessionCallback>::SendPingRequest()
{
  for (const auto &[k, v] : m_Sessions) {
    const auto &session = v;
    session->SendPingRequest();
  }
}

}   // namespace moboware::web_socket