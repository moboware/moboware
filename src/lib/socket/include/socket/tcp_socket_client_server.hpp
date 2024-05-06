#pragma once

#include "common/service.h"
#include "common/timer.h"
#include "common/types.hpp"
#include <boost/asio/strand.hpp>

namespace moboware::tcp_socket {
/**
 * @brief shared class for tcp socket client or server implementation
 */
template <typename TSessionCallback>   //
class TcpSocketClientServer {
public:
  virtual ~TcpSocketClientServer() = default;
  TcpSocketClientServer(const TcpSocketClientServer &) = delete;
  TcpSocketClientServer(TcpSocketClientServer &&) = delete;
  TcpSocketClientServer &operator=(const TcpSocketClientServer &) = delete;
  TcpSocketClientServer &operator=(TcpSocketClientServer &&) = delete;

protected:
  explicit TcpSocketClientServer(const common::ServicePtr &service, TSessionCallback &sessionCallback);

  virtual bool Start(const std::string &address, const std::uint16_t port) = 0;

  const common::ServicePtr m_Service;
  boost::asio::strand<boost::asio::io_context::executor_type> m_Strand;
  TSessionCallback &m_SessionCallback{};
};

template <typename TSessionCallback>
TcpSocketClientServer<TSessionCallback>::TcpSocketClientServer(const common::ServicePtr &service,
                                                               TSessionCallback &sessionCallback)
    : m_Service{service}
    , m_Strand(boost::asio::make_strand(service->GetIoService()))
    , m_SessionCallback(sessionCallback)
{
}
}   // namespace moboware::tcp_socket