#pragma once

#include "common/logger.hpp"
#include "common/service.h"
#include "common/timer.h"
#include "common/types.hpp"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

namespace moboware::ssl_socket {
/**
 * @brief shared class for ssl socket client or server implementation
 */
template <typename TSessionCallback>   //
class SslSocketClientServer {
public:
  virtual ~SslSocketClientServer() = default;
  SslSocketClientServer(const SslSocketClientServer &) = delete;
  SslSocketClientServer(SslSocketClientServer &&) = delete;
  SslSocketClientServer &operator=(const SslSocketClientServer &) = delete;
  SslSocketClientServer &operator=(SslSocketClientServer &&) = delete;

  bool LoadCertificateAndKeyFile(const std::string &certificateFile, const std::string &keyFile);

protected:
  explicit SslSocketClientServer(const common::ServicePtr &service, TSessionCallback &sessionCallback);

  virtual bool Start(const std::string &address, const std::uint16_t port) = 0;

  const common::ServicePtr m_Service;
  boost::asio::strand<boost::asio::io_context::executor_type> m_Strand;
  TSessionCallback &m_SessionCallback{};
  boost::asio::ssl::context m_SslContext{boost::asio::ssl::context::tlsv13};
};

template <typename TSessionCallback>   //
SslSocketClientServer<TSessionCallback>::SslSocketClientServer(const common::ServicePtr &service,
                                                               TSessionCallback &sessionCallback)
    : m_Service{service}
    , m_Strand(boost::asio::make_strand(service->GetIoService()))
    , m_SessionCallback(sessionCallback)
{
}

template <typename TSessionCallback>   //
bool SslSocketClientServer<TSessionCallback>::LoadCertificateAndKeyFile(const std::string &certificateFile,
                                                                        const std::string &keyFile)
{
  boost::system::error_code ec;
  m_SslContext.use_certificate_file(certificateFile, boost::asio::ssl::context::file_format::pem, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Unable to load cert file:{}, {}", certificateFile, ec.what());
    return false;
  }

  m_SslContext.use_rsa_private_key_file(keyFile, boost::asio::ssl::context::file_format::pem, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Unable to load key file:{}, {}", certificateFile, ec.what());
    return false;
  }

  return true;
}

}   // namespace moboware::ssl_socket