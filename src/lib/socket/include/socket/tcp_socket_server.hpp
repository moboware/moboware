#pragma once

#include "socket/tcp_socket_client_server.hpp"
#include "socket/tcp_socket_session.hpp"
#include <boost/asio/strand.hpp>
#include <map>
#include <memory>

namespace moboware::tcp_socket {

/**
 * @brief Web socket server, must be created with a shared_ptr.
 */
template <typename TSessionCallback>   //
class TcpSocketServer : public TcpSocketClientServer<TSessionCallback> {
public:
  explicit TcpSocketServer(const std::shared_ptr<moboware::common::Service> &service, TSessionCallback &sessionCallback);
  TcpSocketServer(const TcpSocketServer &) = delete;
  TcpSocketServer(TcpSocketServer &&) = delete;
  TcpSocketServer &operator=(const TcpSocketServer &) = delete;
  TcpSocketServer &operator=(TcpSocketServer &&) = delete;
  virtual ~TcpSocketServer() = default;

  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) final;
  [[nodiscard]] std::size_t SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer,
                                           const boost::asio::ip::tcp::endpoint &remoteEndPoint);
  void SendToAllClients(const std::vector<boost::asio::const_buffer> &sendBuffer);

  bool HasConnectedClients() const
  {
    return not m_Sessions.empty();
  }

private:
  using TcpSocketClientServer_t = TcpSocketClientServer<TSessionCallback>;

  void Accept();
  std::size_t CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint);

  boost::asio::ip::tcp::acceptor m_Acceptor;

  using endpointPair_t = std::pair<boost::asio::ip::address, boost::asio::ip::port_type>;
  using Sessions_t = std::map<endpointPair_t, std::shared_ptr<tcp_socket::TcpSocketSession<TSessionCallback>>>;
  Sessions_t m_Sessions;

  TSessionCallback &m_SessionCallback{};
};

template <typename TSessionCallback>   //
TcpSocketServer<TSessionCallback>::TcpSocketServer(const std::shared_ptr<moboware::common::Service> &service, TSessionCallback &sessionCallback)
  : TcpSocketClientServer_t(service, sessionCallback)
  , m_Acceptor(TcpSocketClientServer_t::m_Strand)
  , m_SessionCallback(sessionCallback)
{
}

template <typename TSessionCallback>   //
bool TcpSocketServer<TSessionCallback>::Start(const std::string &address, const std::uint16_t port)
{
  LOG_INFO("Start server on {}:{}", address, port);

  boost::asio::ip::tcp::resolver resolver(TcpSocketClientServer_t::m_Strand.get_inner_executor());
  boost::system::error_code ec;

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

  LOG_INFO("TcpSocket server started");

  // start ping timer
  // StartPingTimer(std::chrono::milliseconds(3000));

  return true;
}

template <typename TSessionCallback>   //
std::size_t TcpSocketServer<TSessionCallback>::CheckClosedSessions(const boost::asio::ip::tcp::endpoint &remoteEndpoint)
{
  const auto endpointPair{std::make_pair(remoteEndpoint.address(), remoteEndpoint.port())};
  const auto iter{m_Sessions.find(endpointPair)};
  if (iter != m_Sessions.end()) {

    const auto session = iter->second;
    if (not session->IsOpen()) {
      m_Sessions.erase(iter);
      LOG_TRACE("Removed 'closed' session {}:{}", remoteEndpoint.address().to_string(), remoteEndpoint.port());
    }
  }
  return m_Sessions.size();
}

template <typename TSessionCallback>   //
void TcpSocketServer<TSessionCallback>::Accept()
{
  const auto acceptorFunc = [&](const boost::system::error_code &ec, boost::asio::ip::tcp::socket webSocket) {
    if (ec.failed()) {
      LOG_ERROR("Failed to accept connection:{}", ec.what());
    } else {
      // create session and store in our session list
      const auto endPointKey = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());

      const auto session = std::make_shared<TcpSocketSession<TSessionCallback>>(TcpSocketClientServer_t::m_Service,
                                                                                TcpSocketClientServer_t::m_SessionCallback,
                                                                                std::move(webSocket),
                                                                                [](const boost::asio::ip::tcp::endpoint &) {
                                                                                });

      const auto pair{m_Sessions.emplace(endPointKey, session)};

      if (pair.second and session->Accept()) {
        LOG_INFO("Connection accepted from {}:{}", session->GetRemoteEndpoint().address().to_string(), session->GetRemoteEndpoint().port());
        // store this new session in the sessions list
      } else {
        m_Sessions.erase(pair.first);
      }
    }

    // wait for the next connection
    TcpSocketServer<TSessionCallback>::Accept();
  };

  m_Acceptor.async_accept(TcpSocketClientServer_t::m_Strand, acceptorFunc);
}

template <typename TSessionCallback>   //
std::size_t TcpSocketServer<TSessionCallback>::SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer,
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
void TcpSocketServer<TSessionCallback>::SendToAllClients(const std::vector<boost::asio::const_buffer> &sendBuffer)
{
  for (const auto &[k, v] : m_Sessions) {
    const auto &session = v;
    if (0 == session->SendData(sendBuffer)) {
      LOG_ERROR("Failed to send data to client: {}:{}", k.first.to_string(), k.second);
    }
  }
}

}   // namespace moboware::tcp_socket