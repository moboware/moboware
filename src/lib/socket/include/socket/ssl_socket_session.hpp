#pragma once

#include "common/logger.hpp"
#include "common/timer.h"
#include "socket/socket_session_base.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl.hpp>

namespace moboware::ssl_socket {

/**
 * @brief We socket session class, used by the web socket server and web socket client
 * Handles session connect, data reads, web socket protocol ping and pong messages on the control layer
 * todo: implement ssl
 */
template <typename TSessionCallback>   //
class SslSocketSession : public moboware::socket::SocketSessionBase<TSessionCallback> {
public:
  using Strand_t = boost::asio::strand<boost::asio::io_context::executor_type>;
  using SessionBase_t = socket::SocketSessionBase<TSessionCallback>;

  explicit SslSocketSession(const std::shared_ptr<moboware::common::Service> &service,
                            boost::asio::ssl::context &ssl_ctx,
                            TSessionCallback &sessionCallback,
                            boost::asio::ip::tcp::socket &&sslSocket,
                            const SessionBase_t::SessionClosedCleanupHandlerFn &);
  /**
   * @brief Performs a server handshake and accept on an incoming client connection and start reading data
   */
  [[nodiscard]] bool Accept();

  /**
   * @brief Connect a web socket client to a web socket server an start reading data
   * @param address
   * @param port
   * @return true
   * @return false
   */
  bool Connect(const std::string &address, const std::uint16_t port) final;

  /**
   * @brief Check if the web socket is open
   * @return true if open
   * @return false if closed
   */
  bool IsOpen() const final;
  [[nodiscard]] std::size_t SendData(const boost::asio::const_buffer &sendBuffer);
  [[nodiscard]] std::size_t SendData(const std::vector<boost::asio::const_buffer> &sendBuffers);

private:
  void SetRemoteEndpoint();
  std::size_t ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer, const std::size_t requestedBufferSize) final;
  std::size_t WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers, boost::system::error_code &ec) final;

  using SslSocket_t = boost::asio::ssl::stream<boost::asio::ip::tcp::socket>;
  SslSocket_t m_SslSocketStream;
};

template <typename TSessionCallback>   //
SslSocketSession<TSessionCallback>::SslSocketSession(
    const common::ServicePtr &service,
    boost::asio::ssl::context &ssl_ctx,
    TSessionCallback &callback,
    boost::asio::ip::tcp::socket &&socket,
    const SessionBase_t::SessionClosedCleanupHandlerFn &sessionClosedHandlerFn)
    : moboware::socket::SocketSessionBase<TSessionCallback>(service, callback, sessionClosedHandlerFn)
    , m_SslSocketStream(std::move(socket), ssl_ctx)
{
}

template <typename TSessionCallback>   //
bool SslSocketSession<TSessionCallback>::Accept()
{
  // sslserver accept

  SetRemoteEndpoint();

  // Set suggested timeout settings for the websocket
  boost::system::error_code ec;
  m_SslSocketStream.handshake(boost::asio::ssl::stream_base::server, ec);

  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Handshake failed:{}", ec.what());
    return false;
  }

  // inform about new connection
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  SessionBase_t::SetTcpBufferSizes(boost::beast::get_lowest_layer(m_SslSocketStream));

  // start reading data
  SessionBase_t::ReadData(boost::beast::get_lowest_layer(m_SslSocketStream));

  return true;
}

template <typename TSessionCallback>   //
bool SslSocketSession<TSessionCallback>::Connect(const std::string &address, const std::uint16_t port)
{
  //////////////////////////////////////////////////////////////////////////
  // Resolve, look up the domain name
  //////////////////////////////////////////////////////////////////////////
  boost::asio::ip::tcp::resolver resolver(m_SslSocketStream.get_executor());
  boost::system::error_code ec;

  const auto resolveResults{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Resolving address failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // Make the connection on the IP address we get from a lookup
  //////////////////////////////////////////////////////////////////////////
  const boost::asio::ip::tcp::endpoint &endpoint{resolveResults.begin()->endpoint()};
  boost::beast::get_lowest_layer(m_SslSocketStream).connect(endpoint, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Connect to Web socket failed, {}:{} {}", address, port, ec.what());
    return false;
  }

  SetRemoteEndpoint();

  //////////////////////////////////////////////////////////////////////////
  // set delay and linger options
  //////////////////////////////////////////////////////////////////////////
  boost::beast::get_lowest_layer(m_SslSocketStream).set_option(boost::asio::ip::tcp::no_delay(true));
  boost::beast::get_lowest_layer(m_SslSocketStream).set_option(boost::asio::socket_base::linger(true, 0));

  // Perform SSL handshake and verify the remote host's certificate.
  m_SslSocketStream.set_verify_mode(boost::asio::ssl::verify_peer);
  m_SslSocketStream.set_verify_callback(boost::asio::ssl::host_name_verification(address));

  m_SslSocketStream.handshake(boost::asio::ssl::stream_base::client, ec);

  if (ec.failed()) {
    _log_fatal(LOG_DETAILS, "Failed to handshake to ssl socket, {}:{} {}", address, port, ec.what());
    return false;
  }

  // inform about connected client/server
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  SessionBase_t::SetTcpBufferSizes(boost::beast::get_lowest_layer(m_SslSocketStream));

  _log_info(LOG_DETAILS, "Ready for data reading/writing");

  // start reading data
  SessionBase_t::ReadData(boost::beast::get_lowest_layer(m_SslSocketStream));

  return true;
}

template <typename TSessionCallback>   //
std::size_t SslSocketSession<TSessionCallback>::ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer,
                                                           const std::size_t requestedBufferSize)
{
  boost::system::error_code readError;

  const auto bytesRead{m_SslSocketStream.read_some(boost::asio::buffer(dataBuffer, requestedBufferSize), readError)};
  if (not readError.failed()) {
    _log_debug(LOG_DETAILS, "Read {} data:{}", bytesRead, std::string(dataBuffer, bytesRead));
    return bytesRead;
  }
  return 0ul;
}

template <typename TSessionCallback>   //
std::size_t SslSocketSession<TSessionCallback>::SendData(const boost::asio::const_buffer &sendBuffer)
{
  const std::vector<boost::asio::const_buffer> sendBuffers{sendBuffer};
  return SessionBase_t::SendData(boost::beast::get_lowest_layer(m_SslSocketStream), sendBuffers);
}

template <typename TSessionCallback>   //
std::size_t SslSocketSession<TSessionCallback>::SendData(const std::vector<boost::asio::const_buffer> &sendBuffers)
{
  return SessionBase_t::SendData(boost::beast::get_lowest_layer(m_SslSocketStream), sendBuffers);
}

template <typename TSessionCallback>   //
std::size_t SslSocketSession<TSessionCallback>::WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers,
                                                            boost::system::error_code &ec)
{
  // because this is a ssl layer we need to write directly to the ssl write otherwise the data is not encrypted
  return boost::asio::write(m_SslSocketStream, sendBuffers, ec);
}

template <typename TSessionCallback>   //
bool SslSocketSession<TSessionCallback>::IsOpen() const
{
  return boost::beast::get_lowest_layer(m_SslSocketStream).is_open();
}

template <typename TSessionCallback>   //
void SslSocketSession<TSessionCallback>::SetRemoteEndpoint()
{
  SessionBase_t::SetRemoteEndpoint(boost::beast::get_lowest_layer(m_SslSocketStream));
}

}   // namespace moboware::ssl_socket