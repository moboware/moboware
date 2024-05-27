#pragma once

#include "common/timer.h"
#include "socket/socket_session_base.hpp"
#include <boost/asio.hpp>
#include <boost/asio/buffer.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/system/error_code.hpp>

namespace moboware::tcp_socket {

/**
 * @brief We socket session class, used by the web socket server and web socket client
 * Handles session connect, data reads, web socket protocol ping and pong messages on the control layer
 */
template <typename TSessionCallback>   //
class TcpSocketSession : public moboware::socket::SocketSessionBase<TSessionCallback> {
public:
  using Strand_t = boost::asio::strand<boost::asio::io_context::executor_type>;
  using SessionBase_t = socket::SocketSessionBase<TSessionCallback>;

  explicit TcpSocketSession(const std::shared_ptr<moboware::common::Service> &service,
                            TSessionCallback &callback,
                            boost::asio::ip::tcp::socket &&tcpSocket,
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

protected:
private:
  void SetRemoteEndpoint();

  std::size_t ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer, const std::size_t requestedBufferSize) final;
  std::size_t WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers, boost::system::error_code &ec) final;

  using TcpSocket_t = boost::asio::ip::tcp::socket;
  TcpSocket_t m_TcpSocketStream;
  //
  boost::asio::ip::tcp::endpoint m_RemoteEndpoint;
};

template <typename TSessionCallback>   //
TcpSocketSession<TSessionCallback>::TcpSocketSession(const common::ServicePtr &service,
                                                     TSessionCallback &callback,
                                                     boost::asio::ip::tcp::socket &&socket,
                                                     const SessionBase_t::SessionClosedCleanupHandlerFn &sessionClosedHandlerFn)
  : moboware::socket::SocketSessionBase<TSessionCallback>(service, callback, sessionClosedHandlerFn)
  , m_TcpSocketStream(std::move(socket))
{
}

template <typename TSessionCallback>   //
bool TcpSocketSession<TSessionCallback>::Accept()
{
  SetRemoteEndpoint();
  // inform about new connection
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  SessionBase_t::SetTcpBufferSizes(m_TcpSocketStream);

  // start reading data
  SessionBase_t::ReadData(m_TcpSocketStream);

  return true;
}

template <typename TSessionCallback>   //
bool TcpSocketSession<TSessionCallback>::Connect(const std::string &address, const std::uint16_t port)
{
  // Resolve, look up the domain name
  boost::asio::ip::tcp::resolver resolver(m_TcpSocketStream.get_executor());
  boost::system::error_code ec;
  const auto resolveResults{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    LOG_ERROR("Resolving address failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  // Make the connection on the IP address we get from a lookup
  const boost::asio::ip::tcp::endpoint &endpoint{resolveResults.begin()->endpoint()};
  m_TcpSocketStream.connect(endpoint, ec);
  if (ec.failed()) {
    LOG_ERROR("Connect to Web socket failed, {}:{} {}", address, port, ec.what());
    return false;
  }

  SetRemoteEndpoint();

  m_TcpSocketStream.set_option(boost::asio::ip::tcp::no_delay(true));
  m_TcpSocketStream.set_option(boost::asio::socket_base::linger(true, 0));

  // inform about connected client/server
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  SessionBase_t::SetTcpBufferSizes(m_TcpSocketStream);

  LOG_INFO("Ready for data reading/writing");

  // start reading data
  SessionBase_t::ReadData(m_TcpSocketStream);

  return true;
}

template <typename TSessionCallback>   //
std::size_t TcpSocketSession<TSessionCallback>::ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer, const std::size_t requestedBufferSize)
{
  boost::system::error_code readError;
  const auto bytesRead{m_TcpSocketStream.read_some(boost::asio::buffer(dataBuffer, requestedBufferSize), readError)};
  if (not readError.failed()) {
    LOG_DEBUG("Read {} data:{}", bytesRead, std::string(dataBuffer, bytesRead));
    return bytesRead;
  }
  return 0ul;
}

template <typename TSessionCallback>   //
std::size_t TcpSocketSession<TSessionCallback>::SendData(const boost::asio::const_buffer &sendBuffer)
{
  // todo if a header must be added this must be calculated here and added at the front
  const std::vector<boost::asio::const_buffer> sendBuffers{sendBuffer};
  return SessionBase_t::SendData(m_TcpSocketStream, sendBuffers);
}

template <typename TSessionCallback>   //
std::size_t TcpSocketSession<TSessionCallback>::SendData(const std::vector<boost::asio::const_buffer> &sendBuffers)
{
  return SessionBase_t::SendData(m_TcpSocketStream, sendBuffers);
}

template <typename TSessionCallback>   //
std::size_t TcpSocketSession<TSessionCallback>::WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers,
                                                            boost::system::error_code &ec)
{
  return boost::asio::write(m_TcpSocketStream, sendBuffers, ec);
}

template <typename TSessionCallback>   //
bool TcpSocketSession<TSessionCallback>::IsOpen() const
{
  return m_TcpSocketStream.is_open();
}

template <typename TSessionCallback>   //
void TcpSocketSession<TSessionCallback>::SetRemoteEndpoint()
{
  SessionBase_t::SetRemoteEndpoint(m_TcpSocketStream);
}

}   // namespace moboware::tcp_socket