#pragma once

#include "common/logger.hpp"
#include "socket/tcp_socket_client_server.hpp"
#include "socket/tcp_socket_session.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/strand.hpp>

namespace moboware::tcp_socket {

template <typename TSessionCallback>   //
class TcpSocketClient : public TcpSocketClientServer<TSessionCallback> {
public:
  explicit TcpSocketClient(const moboware::common::ServicePtr &service, TSessionCallback &sessionCallback);
  TcpSocketClient(const TcpSocketClient &) = delete;
  TcpSocketClient(TcpSocketClient &&) = delete;
  TcpSocketClient &operator=(const TcpSocketClient &) = delete;
  TcpSocketClient &operator=(TcpSocketClient &&) = delete;
  virtual ~TcpSocketClient() = default;

  [[nodiscard]] bool Start(const std::string &address, const std::uint16_t port) final;
  [[nodiscard]] auto SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer) -> std::size_t;

private:
  using TcpSocketClientServer_t = TcpSocketClientServer<TSessionCallback>;

  std::shared_ptr<moboware::tcp_socket::TcpSocketSession<TSessionCallback>> m_Session;
};

template <typename TSessionCallback>   //
TcpSocketClient<TSessionCallback>::TcpSocketClient(const moboware::common::ServicePtr &service,
                                                   TSessionCallback &sessionCallback)
    : TcpSocketClientServer<TSessionCallback>(service, sessionCallback)
{
}

template <typename TSessionCallback>   //
auto TcpSocketClient<TSessionCallback>::Start(const std::string &address, const std::uint16_t port) -> bool
{
  _log_trace(LOG_DETAILS, "Connecting tcp socket client, {}:{}", address, port);

  boost::asio::ip::tcp::socket tcpSocket(TcpSocketClientServer_t::m_Strand.get_inner_executor());

  m_Session = std::make_shared<TcpSocketSession<TSessionCallback>>(TcpSocketClientServer_t::m_Service,
                                                                   TcpSocketClientServer_t::m_SessionCallback,
                                                                   std::move(tcpSocket),
                                                                   [](const boost::asio::ip::tcp::endpoint &) {
                                                                   });

  return m_Session->Connect(address, port);
}

template <typename TSessionCallback>   //
auto TcpSocketClient<TSessionCallback>::SendSocketData(const std::vector<boost::asio::const_buffer> &sendBuffer)
    -> std::size_t
{
  if (m_Session) {
    return m_Session->SendData(sendBuffer);
  }
  return 0;
}

}   // namespace moboware::tcp_socket