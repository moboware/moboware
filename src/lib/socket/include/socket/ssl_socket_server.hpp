#pragma once

#include "socket/server_certificates.hpp"
#include "socket/ssl_socket_client_server.hpp"
#include "socket/ssl_socket_session.hpp"
#include <map>
#include <memory>

namespace moboware::ssl_socket {

/**
 * @brief SSL socket server, must be created with a shared_ptr.
 */
template <typename TSessionCallback>   //
class SslSocketServer : public ssl_socket::SslSocketClientServer<TSessionCallback> {
public:
  explicit SslSocketServer(const std::shared_ptr<moboware::common::Service> &service, TSessionCallback &sessionCallback);
  SslSocketServer(const SslSocketServer &) = delete;
  SslSocketServer(SslSocketServer &&) = delete;
  SslSocketServer &operator=(const SslSocketServer &) = delete;
  SslSocketServer &operator=(SslSocketServer &&) = delete;
  virtual ~SslSocketServer() = default;

  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) override;
  [[nodiscard]] std::size_t SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer,
                                           const boost::asio::ip::tcp::endpoint &remoteEndPoint);
  void SendToAllClients(const std::vector<boost::asio::const_buffer> &sendBuffer);

  [[nodiscard]] inline bool HasConnectedClients() const
  {
    return not m_Sessions.empty();
  }

private:
  void Accept() noexcept;
  /**
   * Cleanup closed sessions and returns the current number of sessions open
   */
  std::size_t CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint) noexcept;

  boost::asio::ip::tcp::acceptor m_Acceptor;

  using endpointPair_t = std::pair<boost::asio::ip::address, boost::asio::ip::port_type>;
  using SslSocketClientServer_t = SslSocketClientServer<TSessionCallback>;
  using SslSocketSession_t = ssl_socket::SslSocketSession<TSessionCallback>;
  using Sessions_t = std::map<endpointPair_t, std::shared_ptr<SslSocketSession_t>>;

  Sessions_t m_Sessions;
};

/**
 * @brief Construct a new Ssl Socket Server< T Session Callback>:: Ssl Socket Server object
 *
 * @tparam TSessionCallback
 * @param service
 * @param sessionCallback
 */
template <typename TSessionCallback>   //
SslSocketServer<TSessionCallback>::SslSocketServer(const std::shared_ptr<moboware::common::Service> &service, TSessionCallback &sessionCallback)
  : SslSocketClientServer_t(service, sessionCallback)
  , m_Acceptor(SslSocketClientServer_t::m_Strand)
{
  ::LoadServerCertificate(SslSocketClientServer_t::m_SslContext);
}

template <typename TSessionCallback>   //
bool SslSocketServer<TSessionCallback>::Start(const std::string &address, const std::uint16_t port)
{
  LOG_INFO("Start server on {}:{}", address, port);
  boost::asio::ip::tcp::resolver resolver(SslSocketClientServer_t::m_Strand.get_inner_executor());
  boost::system::error_code ec{};

  const auto resolveResults{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    LOG_ERROR("Resolving address failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  // create end point
  const boost::asio::ip::tcp::endpoint &endpoint{resolveResults.begin()->endpoint()};

  // Open the acceptor
  m_Acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    LOG_ERROR("Open acceptor failed:{}", ec.what());
    return false;
  }

  // Allow address reuse
  m_Acceptor.set_option(boost::asio::socket_base::reuse_address(true), ec);
  if (ec) {
    LOG_ERROR("set_option failed:{}", ec.what());
    return false;
  }

  // Bind to the server address
  m_Acceptor.bind(endpoint, ec);
  if (ec) {
    LOG_ERROR("bind failed:{}", ec.what());
    return false;
  }

  // Start listening for connections
  m_Acceptor.listen(boost::asio::socket_base::max_listen_connections, ec);
  if (ec) {
    LOG_ERROR("start listen failed:{}", ec.what());
    return false;
  }

  // start accepting connections
  Accept();

  LOG_INFO("SslSocket server started, waiting for clients");

  return true;
}

template <typename TSessionCallback>   //
void SslSocketServer<TSessionCallback>::Accept() noexcept
{
  const auto acceptorFunc{[&](const boost::system::error_code &ec, boost::asio::ip::tcp::socket webSocket) {
    if (ec.failed()) {
      LOG_ERROR("Failed to accept connection:{}", ec.what());
    } else {
      // create session and store in our session list
      const auto sessionClosedHandlerFn{[&](const boost::asio::ip::tcp::endpoint &endpoint) {
        // cleanup session details, should be posted because the session must be alive for ping etc
        const auto removeSessionFn{[this, endpoint]() {
          CheckClosedSessions(endpoint);
          // notify of closed connection
          SslSocketClientServer_t::m_SessionCallback.OnSessionClosed(endpoint);
        }};

        SslSocketClientServer_t::m_Service->GetIoService().post(removeSessionFn);
      }};

      const auto endPointKey = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());
      const auto session = std::make_shared<SslSocketSession_t>(SslSocketClientServer_t::m_Service,
                                                                SslSocketClientServer_t::m_SslContext,
                                                                SslSocketClientServer_t::m_SessionCallback,
                                                                std::move(webSocket),
                                                                sessionClosedHandlerFn);
      const auto pair{m_Sessions.emplace(endPointKey, session)};

      if (pair.second and session->Accept()) {
        LOG_INFO("Connection accepted from {}:{}", session->GetRemoteEndpoint().address().to_string(), session->GetRemoteEndpoint().port());
        // store this new session in the sessions list
      } else {
        m_Sessions.erase(pair.first);
      }
    }

    // wait for the next connection
    SslSocketServer::Accept();
  }};

  m_Acceptor.async_accept(SslSocketClientServer_t::m_Strand, acceptorFunc);
}

template <typename TSessionCallback>   //
std::size_t SslSocketServer<TSessionCallback>::CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint) noexcept
{
  const auto endpointPair{std::make_pair(remoteEndpoint.address(), remoteEndpoint.port())};
  const auto iter{m_Sessions.find(endpointPair)};

  // check if there are any dead connections that must be removed from our session list
  if (iter != m_Sessions.end()) {
    const auto &session = iter->second;
    if (not session->IsOpen()) {
      m_Sessions.erase(iter);
      LOG_TRACE("Removed 'closed' session {}:{}", remoteEndpoint.address().to_string(), remoteEndpoint.port());
    }
  }
  return m_Sessions.size();
}

template <typename TSessionCallback>   //
std::size_t SslSocketServer<TSessionCallback>::SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer,
                                                              const boost::asio::ip::tcp::endpoint &remoteEndPoint)
{
  const auto endPointKey = std::make_pair(remoteEndPoint.address(), remoteEndPoint.port());
  const auto iter = m_Sessions.find(endPointKey);

  if (iter != std::end(m_Sessions)) {
    const auto &session = iter->second;
    return session->SendData(sendBuffer);
  }

  LOG_ERROR("No endpoint not found to send data to {}:{}", remoteEndPoint.address().to_string(), remoteEndPoint.port());
  return 0;
}

template <typename TSessionCallback>   //
void SslSocketServer<TSessionCallback>::SendToAllClients(const std::vector<boost::asio::const_buffer> &sendBuffer)
{
  for (const auto &[k, v] : m_Sessions) {
    const auto &session = v;
    if (0 == session->SendData(sendBuffer)) {
      LOG_ERROR("Failed to send data to client: {}:{}", k.first.to_string(), k.second);
    }
  }
}

}   // namespace moboware::ssl_socket