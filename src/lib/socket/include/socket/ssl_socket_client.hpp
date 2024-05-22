#pragma once

#include "common/logger.hpp"
#include "socket/ssl_socket_client_server.hpp"
#include "socket/ssl_socket_session.hpp"
#include <boost/beast/websocket.hpp>

namespace moboware::ssl_socket {

template <typename TSessionCallback>   //
class SslSocketClient : public SslSocketClientServer<TSessionCallback> {
public:
  explicit SslSocketClient(const moboware::common::ServicePtr &service, TSessionCallback &sessionCallback);
  SslSocketClient(const SslSocketClient &) = delete;
  SslSocketClient(SslSocketClient &&) = delete;
  SslSocketClient &operator=(const SslSocketClient &) = delete;
  SslSocketClient &operator=(SslSocketClient &&) = delete;
  virtual ~SslSocketClient() = default;

  [[nodiscard]] std::size_t SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer);
  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) override;

private:
  using SslSocketSession_t = moboware::ssl_socket::SslSocketSession<TSessionCallback>;
  using SslSocketClientServer_t = SslSocketClientServer<TSessionCallback>;

  std::shared_ptr<SslSocketSession_t> m_Session;
};
/**
 * @brief Construct a new Ssl Socket Client< T Session Callback>:: Ssl Socket Client object
 *
 * @tparam TSessionCallback
 * @param service
 * @param sessionCallback
 */
template <typename TSessionCallback>
SslSocketClient<TSessionCallback>::SslSocketClient(const moboware::common::ServicePtr &service,
                                                   TSessionCallback &sessionCallback)
    : SslSocketClientServer_t(service, sessionCallback)
{
  SslSocketClientServer_t::m_SslContext.set_default_verify_paths();
}

template <typename TSessionCallback>
auto SslSocketClient<TSessionCallback>::Start(const std::string &address, const std::uint16_t port) -> bool
{
  _log_trace(LOG_DETAILS, "Connecting ssl socket client, {}:{}", address, port);

  boost::asio::ip::tcp::socket sslSocket(SslSocketClientServer_t::m_Strand.get_inner_executor());
  m_Session = std::make_shared<SslSocketSession_t>(SslSocketClientServer_t::m_Service,
                                                   SslSocketClientServer_t::m_SslContext,
                                                   SslSocketClientServer_t::m_SessionCallback,
                                                   std::move(sslSocket),
                                                   [&](const boost::asio::ip::tcp::endpoint &endpoint) {
                                                     // report closed session at the client
                                                     SslSocketClientServer_t::m_SessionCallback.OnSessionClosed(endpoint);
                                                   });

  return m_Session->Connect(address, port);
}

template <typename TSessionCallback>
auto SslSocketClient<TSessionCallback>::SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer)
    -> std::size_t
{
  return m_Session->SendData(sendBuffer);
}

}   // namespace moboware::ssl_socket