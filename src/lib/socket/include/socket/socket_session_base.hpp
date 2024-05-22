#pragma once

#include "common/logger.hpp"
#include "common/ring_buffer.hpp"
#include "common/service.h"
#include "common/types.hpp"
#include <boost/asio/ip/tcp.hpp>

namespace moboware::socket {

const std::size_t RingBufferSize{1024 * 512};
using RingBuffer_t = common::RingBuffer<char, RingBufferSize>;

template <typename TSessionCallback>   //
class SocketSessionBase {
public:
  using SessionClosedCleanupHandlerFn = std::function<void(const boost::asio::ip::tcp::endpoint &)>;

  explicit SocketSessionBase(const std::shared_ptr<moboware::common::Service> &service,
                             TSessionCallback &callback,
                             const SessionClosedCleanupHandlerFn &sessionClosedCleanupHandler);
  virtual ~SocketSessionBase() = default;
  SocketSessionBase(const SocketSessionBase &) = default;
  SocketSessionBase(SocketSessionBase &&) = default;
  SocketSessionBase &operator=(const SocketSessionBase &) = default;
  SocketSessionBase &operator=(SocketSessionBase &&) = default;

  boost::asio::ip::tcp::endpoint GetRemoteEndpoint() const;

  [[nodiscard]] virtual bool Connect(const std::string &address, const std::uint16_t port) = 0;
  [[nodiscard]] virtual bool IsOpen() const = 0;

protected:
  void SetRemoteEndpoint(const boost::asio::ip::tcp::socket &socket);
  void SetTcpBufferSizes(boost::asio::ip::tcp::socket &socket);

  [[nodiscard]] std::size_t GetReceiveBufferSize(const boost::asio::ip::tcp::socket &socket) const;
  [[nodiscard]] bool SetReceiveBufferSize(boost::asio::ip::tcp::socket &socket, const std::size_t size);

  [[nodiscard]] std::size_t GetSendBufferSize(const boost::asio::ip::tcp::socket &socket) const;
  [[nodiscard]] bool SetSendBufferSize(boost::asio::ip::tcp::socket &socket, const std::size_t size);

  [[nodiscard]] std::size_t SendData(boost::asio::ip::tcp::socket &socket,
                                     const std::vector<boost::asio::const_buffer> &sendBuffers);
  void HandleClosedSocket(boost::asio::ip::tcp::socket &socket);
  void ReadData(boost::asio::ip::tcp::socket &socket);

  socket::RingBuffer_t m_ReceiveBuffer;

  const std::shared_ptr<common::Service> m_Service{};
  TSessionCallback &m_DataHandlerCallback{};
  // lambda function to be called when the session is closed, and must be handled in the higher layers
  const SessionClosedCleanupHandlerFn m_SessionClosedCleanupHandler;

private:
  boost::asio::ip::tcp::endpoint m_RemoteEndpoint{};

  [[nodiscard]] virtual std::size_t ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer,
                                               const std::size_t requestedBufferSize) = 0;
  [[nodiscard]] virtual std::size_t WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers,
                                                boost::system::error_code &ec) = 0;
};

template <typename TSessionCallback>   //
SocketSessionBase<TSessionCallback>::SocketSessionBase(const std::shared_ptr<moboware::common::Service> &service,
                                                       TSessionCallback &callback,
                                                       const SessionClosedCleanupHandlerFn &sessionClosedCleanupHandler)
    : m_Service(service)
    , m_DataHandlerCallback(callback)
    , m_SessionClosedCleanupHandler(sessionClosedCleanupHandler)
{
}

template <typename TSessionCallback>   //
boost::asio::ip::tcp::endpoint SocketSessionBase<TSessionCallback>::GetRemoteEndpoint() const
{
  return m_RemoteEndpoint;
}

template <typename TSessionCallback>   //
void SocketSessionBase<TSessionCallback>::SetRemoteEndpoint(const boost::asio::ip::tcp::socket &socketStream)
{
  m_RemoteEndpoint = socketStream.remote_endpoint();
}

template <typename TSessionCallback>   //
std::size_t SocketSessionBase<TSessionCallback>::GetReceiveBufferSize(const boost::asio::ip::tcp::socket &socket) const
{
  boost::asio::socket_base::receive_buffer_size receiveBufferSizeOption{};
  boost::system::error_code ec{};

  socket.get_option(receiveBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Get tcp receive buffer size failed:{}", ec.what());
    return 0;
  }
  return receiveBufferSizeOption.value();
}

template <typename TSessionCallback>   //
bool SocketSessionBase<TSessionCallback>::SetReceiveBufferSize(boost::asio::ip::tcp::socket &socket, const std::size_t size)
{
  boost::asio::socket_base::receive_buffer_size receiveBufferSizeOption(size);
  boost::system::error_code ec{};

  socket.set_option(receiveBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Set tcp receive buffer size failed:{}", ec.what());
    return false;
  }

  _log_info(LOG_DETAILS, "Tcp receive buffer set to:{}", size);

  return true;
}

template <typename TSessionCallback>   //
std::size_t SocketSessionBase<TSessionCallback>::GetSendBufferSize(const boost::asio::ip::tcp::socket &socket) const
{
  boost::system::error_code ec;
  boost::asio::socket_base::send_buffer_size sendBufferSizeOption{};
  socket.get_option(sendBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Get tcp send buffer size failed:{}", ec.what());
    return 0;
  }
  return sendBufferSizeOption.value();
}

template <typename TSessionCallback>   //
bool SocketSessionBase<TSessionCallback>::SetSendBufferSize(boost::asio::ip::tcp::socket &socket, const std::size_t size)
{
  boost::system::error_code ec;
  boost::asio::socket_base::send_buffer_size sendBufferSizeOption(size);
  socket.set_option(sendBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Set tcp send buffer size failed:{}", ec.what());
    return false;
  }
  _log_info(LOG_DETAILS, "Tcp send buffer set to:{}", size);
  return true;
}

template <typename TSessionCallback>   //
void SocketSessionBase<TSessionCallback>::SetTcpBufferSizes(boost::asio::ip::tcp::socket &socket)
{
  if (not SetReceiveBufferSize(socket, socket::RingBufferSize)) {
    _log_info(LOG_DETAILS, "Receive tcp buffer size:{}", GetReceiveBufferSize(socket));
  }

  if (not SetSendBufferSize(socket, socket::RingBufferSize)) {
    _log_info(LOG_DETAILS, "Send tcp buffer size:{}", GetSendBufferSize(socket));
  }
}

template <typename TSessionCallback>   //
std::size_t SocketSessionBase<TSessionCallback>::SendData(boost::asio::ip::tcp::socket &socket,
                                                          const std::vector<boost::asio::const_buffer> &sendBuffers)
{
  if (IsOpen()) {

    boost::system::error_code ec;
    const auto sendBytes{WriteSocket(sendBuffers, ec)};

    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Write failed: {}", ec.what());
      HandleClosedSocket(socket);
      return 0;
    }
    return sendBytes;
  }
  return 0;
}

template <typename TSessionCallback>   //
void SocketSessionBase<TSessionCallback>::HandleClosedSocket(boost::asio::ip::tcp::socket &socket)
{
  _log_trace(LOG_DETAILS, "Handle closed socket");

  if (m_SessionClosedCleanupHandler) {
    // notify of a closed session
    m_SessionClosedCleanupHandler(GetRemoteEndpoint());
  }

  boost::system::error_code ec{};
  // cancel any pending async operation

  socket.cancel(ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Cancel operation on socket failed:{}", ec.what());
  }

  if (socket.is_open()) {
    _log_info(LOG_DETAILS, "Shutting down and closing socket");

    ec.clear();
    socket.shutdown(boost::asio::socket_base::shutdown_both, ec);
    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Socket shutdown failed:{}", ec.what());
      ec.clear();
    }

    ec.clear();
    socket.close(ec);
    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Socket close failed:{}", ec.what());
    }
  }
}

template <typename TSessionCallback>   //
void SocketSessionBase<TSessionCallback>::ReadData(boost::asio::ip::tcp::socket &socket)
{
  _log_debug(LOG_DETAILS, "Waiting to read data");
  //
  const auto readDataFunc{[&](const boost::system::error_code &ec) {
    // This indicates that the session was closed
    // if (ec == asio::error::connection_reset ||     //
    //    ec == asio::error::connection_aborted ||   //
    //    ec == asio::error::broken_pipe ||          //
    //    ec == asio::error::network_reset ||        //
    //    ec == asio::error::network_down) {
    //
    //  _log_error(LOG_DETAILS,"Read error, closed Web socket, {}", ec.what());
    //  m_DataHandlerCallback->OnSessionClosed(GetRemoteEndpoint());
    //  return;
    //} else
    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Read error: {}", ec.what());
      HandleClosedSocket(socket);
      return;
    } else {
      // no errors, read data from socket
      const auto bytesAvailable{socket.available()};
      _log_debug(LOG_DETAILS, "Bytes available:{}", bytesAvailable);
      if (bytesAvailable > 0 and m_ReceiveBuffer.WriteBuffer(
                                     [&](socket::RingBuffer_t::BufferType_t *dataBuffer,
                                         const std::size_t requestedBufferSize) {   //
                                       return ReadBuffer(dataBuffer, requestedBufferSize);
                                     },
                                     bytesAvailable)) {
        // forward data when successful committed the data into the read buffer
        m_DataHandlerCallback.OnDataRead(m_ReceiveBuffer, this->GetRemoteEndpoint(), std::chrono::steady_clock::now());
      }
      // initialize new read operation
      this->ReadData(socket);
    }
  }};

  // start async reading
  socket.async_wait(boost::asio::ip::tcp::socket::wait_read, readDataFunc);
}

}   // namespace moboware::socket