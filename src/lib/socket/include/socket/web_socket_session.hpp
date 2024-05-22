#pragma once

#include "common/service.h"
#include "common/timer.h"
#include "common/types.hpp"
#include "socket/socket_session_base.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/ssl.hpp>
#include <boost/beast/websocket.hpp>
#include <boost/beast/websocket/ssl.hpp>
#include <deque>

namespace moboware::web_socket {

/**
 * @brief We socket session class, used by the web socket server and web socket client
 * Handles session connect, data reads, web socket protocol ping and pong messages on the control layer
 * todo: implement ssl
 */
template <typename TSessionCallback>   //
class WebSocketSession : public socket::SocketSessionBase<TSessionCallback> {
public:
  using Strand_t = boost::asio::strand<boost::asio::io_context::executor_type>;
  using SessionBase_t = socket::SocketSessionBase<TSessionCallback>;
  using WebSocketSession_t = WebSocketSession<TSessionCallback>;

  explicit WebSocketSession(const std::shared_ptr<moboware::common::Service> &service,
                            boost::asio::ssl::context &ssl_ctx,
                            TSessionCallback &sessionCallback,
                            boost::asio::ip::tcp::socket &&webSocket,
                            const SessionBase_t::SessionClosedCleanupHandlerFn &,
                            const std::string &target = "/");
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
  [[nodiscard]] bool IsOpen() const final;

  [[nodiscard]] auto SendWebSocketData(const boost::asio::const_buffer &sendBuffer) -> bool;

  [[nodiscard]] bool SendPongReply(const boost::asio::const_buffer &pongBuffer);
  [[nodiscard]] bool SendPingRequest();

  std::size_t GetReceiveBufferSize() const;
  bool SetReceiveBufferSize(const std::size_t size);

  std::size_t GetSendBufferSize() const;
  bool SetSendBufferSize(const std::size_t size);

private:
  void ReadData();
  void HandleControlCallback(const boost::beast::websocket::frame_type kind, const boost::beast::string_view &payload);

  std::size_t ReadBuffer(socket::RingBuffer_t::BufferType_t *dataBuffer, const std::size_t requestedBufferSize) final
  {
    return 0;
  };

  std::size_t WriteSocket(const std::vector<boost::asio::const_buffer> &sendBuffers, boost::system::error_code &ec) final
  {
    return 0;
  };

  const std::shared_ptr<common::Service> m_Service;
  using SslWebSocket_t = boost::beast::websocket::stream<boost::beast::ssl_stream<boost::beast::tcp_stream>>;

  SslWebSocket_t m_WebSocketStream;
  boost::beast::flat_buffer m_ReadBuffer;
  //
  // std::atomic<bool> m_IsSending{};
  std::string m_Target;
};

template <typename TSessionCallback>   //
WebSocketSession<TSessionCallback>::WebSocketSession(
    const common::ServicePtr &service,
    boost::asio::ssl::context &ssl_ctx,
    TSessionCallback &callback,
    boost::asio::ip::tcp::socket &&webSocket,
    const SessionBase_t::SessionClosedCleanupHandlerFn &sessionClosedHandlerFn,
    const std::string &target)
    : SessionBase_t(service, callback, sessionClosedHandlerFn)
    , m_Service(service)
    , m_WebSocketStream(std::move(webSocket), ssl_ctx)
    , m_Target(target)
{
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::Accept()
{
  // webserver accept

  SessionBase_t::SetRemoteEndpoint(boost::beast::get_lowest_layer(m_WebSocketStream).socket());

  // Set suggested timeout settings for the websocket
  boost::system::error_code ec;
  m_WebSocketStream.next_layer().handshake(boost::asio::ssl::stream_base::server, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Handshake failed:{}", ec.what());
    return false;
  }

  // Set a decorator to change the Server of the handshake
  m_WebSocketStream.set_option(
      boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::response_type &res) {
        res.set(boost::beast::http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-sync-ssl");
      }));

  m_WebSocketStream.accept(ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Accept failed:{}", ec.what());
    return false;
  }

  _log_info(LOG_DETAILS, "Start websocket ping timer....");
  m_WebSocketStream.control_callback(
      [&](const boost::beast::websocket::frame_type kind, const boost::beast::string_view &payload) {
        HandleControlCallback(kind, payload);
      });

  // inform about new connection
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  _log_info(LOG_DETAILS, "Receive tcp buffer size:{}", GetReceiveBufferSize());
  _log_info(LOG_DETAILS, "Send tcp buffer size:{}", GetSendBufferSize());

  WebSocketSession_t::SetReceiveBufferSize(1024 * 512);
  WebSocketSession_t::SetSendBufferSize(1024 * 512);

  // start reading data
  WebSocketSession_t::ReadData();

  return true;
}

template <typename TSessionCallback>   // web socket client connect
bool WebSocketSession<TSessionCallback>::Connect(const std::string &address, const std::uint16_t port)
{
  // websocket client connect

  // resolver
  // connect
  // set SNI hostname
  // SSL handshake
  // set handshake decorator
  // websocket handshake

  m_WebSocketStream.auto_fragment(false);

  // Turn off the timeout on the tcp_stream, because
  // the websocket stream has its own timeout system.
  boost::beast::get_lowest_layer(m_WebSocketStream).expires_never();

  //////////////////////////////////////////////////////////////////////////
  // Resolve, look up the domain name
  //////////////////////////////////////////////////////////////////////////
  boost::asio::ip::tcp::resolver resolver(m_WebSocketStream.get_executor());
  boost::system::error_code ec{};
  const auto resolveResults{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Resolving address failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // Make the connection on the IP address we get from a lookup
  //////////////////////////////////////////////////////////////////////////
  const boost::asio::ip::tcp::endpoint &endpoint{resolveResults.begin()->endpoint()};
  boost::beast::get_lowest_layer(m_WebSocketStream).connect(endpoint, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Connect to Web socket failed, {}:{} {}", address, port, ec.what());
    return false;
  }

  SessionBase_t::SetRemoteEndpoint(boost::beast::get_lowest_layer(m_WebSocketStream).socket());

  //////////////////////////////////////////////////////////////////////////
  // set SNI hostname on tls extention
  //////////////////////////////////////////////////////////////////////////
  // todo check if the ssl socket implementation can be used here and not use native ssl calls!
  //  if (not SSL_set_tlsext_host_name(m_WebSocketStream.next_layer().native_handle(), address.c_str())) {
  //    _log_error(LOG_DETAILS, "Failed to set tls extention");
  //    return false;
  //  }

  //////////////////////////////////////////////////////////////////////////
  // Perform the SSL handshake
  //////////////////////////////////////////////////////////////////////////
  m_WebSocketStream.next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Handshake failed:{}:{} {}", address, port, ec.what());
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // Set a decorator to change the User-Agent of the handshake
  //////////////////////////////////////////////////////////////////////////
  m_WebSocketStream.set_option(
      boost::beast::websocket::stream_base::decorator([](boost::beast::websocket::request_type &req) {
        req.set(boost::beast::http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
      }));

  m_WebSocketStream.set_option(boost::beast::websocket::stream_base::timeout::suggested(boost::beast::role_type::client));

  //////////////////////////////////////////////////////////////////////////
  // Perform the SSL and websocket handshake
  //////////////////////////////////////////////////////////////////////////
  // perform ssl handshake first, after that the wss handshake
  m_WebSocketStream.next_layer().handshake(boost::asio::ssl::stream_base::client, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "SSL socket handshake failed, {}:{}, {}", address, port, ec.what());
    return false;
  }

  // Update the host_ string. This will provide the value of the
  // Host HTTP header during the WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  const auto host{address + ':' + std::to_string(port)};

  m_WebSocketStream.handshake(host, m_Target, ec);
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Web socket handshake failed, {}:{}, {}", address, port, ec.what());
    return false;
  }

  //////////////////////////////////////////////////////////////////////////
  // set control layer handling like, ping, pong and close
  //////////////////////////////////////////////////////////////////////////
  _log_info(LOG_DETAILS, "Start websocket ping timer....");
  m_WebSocketStream.control_callback(
      [&](const boost::beast::websocket::frame_type kind, const boost::beast::string_view &payload) {
        HandleControlCallback(kind, payload);
      });

  // inform about connected client/server
  SessionBase_t::m_DataHandlerCallback.OnSessionConnected(SessionBase_t::GetRemoteEndpoint());

  // get/set tcp send/receive buffer sizes
  _log_info(LOG_DETAILS, "Receive tcp buffer size:{}", GetReceiveBufferSize());
  _log_info(LOG_DETAILS, "Send tcp buffer size:{}", GetSendBufferSize());

  // get/set tcp send/receive buffer sizes
  WebSocketSession_t::SetReceiveBufferSize(1024 * 1024);
  WebSocketSession_t::SetSendBufferSize(1024 * 512);

  _log_info(LOG_DETAILS, "Ready for data reading/writing");
  //////////////////////////////////////////////////////////////////////////
  // start reading data
  //////////////////////////////////////////////////////////////////////////
  WebSocketSession_t::ReadData();

  return true;
}

template <typename TSessionCallback>   //
void WebSocketSession<TSessionCallback>::ReadData()
{
  // clear read buffer before every read
  m_ReadBuffer.clear();

  boost::asio::dispatch(boost::asio::bind_executor(m_WebSocketStream.get_executor(), [&]() {
    //
    const auto readDataFunc{[this](const boost::beast::error_code &ec, const std::size_t /*bytesTransferred*/) {
      // This indicates that the session was closed
      if (ec == boost::beast::websocket::error::closed) {
        _log_error(LOG_DETAILS, "Read error, closed Web socket, {}", ec.what());

        SessionBase_t::m_SessionClosedCleanupHandler(SessionBase_t::GetRemoteEndpoint());
        return;
      } else if (ec.failed()) {
        _log_error(LOG_DETAILS,
                   "Read error: {}, endpoint:{}@{}",
                   ec.what(),
                   SessionBase_t::GetRemoteEndpoint().port(),
                   SessionBase_t::GetRemoteEndpoint().address().to_string());

        SessionBase_t::m_SessionClosedCleanupHandler(SessionBase_t::GetRemoteEndpoint());

        return;
      } else {
        // forward read data to channel
        SessionBase_t::m_DataHandlerCallback.OnDataRead(m_ReadBuffer,
                                                        SessionBase_t::GetRemoteEndpoint(),
                                                        std::chrono::steady_clock::now());

        // initialize new read operation
        this->ReadData();
      }
    }};
    // start async reading
    m_WebSocketStream.async_read(m_ReadBuffer, boost::beast::bind_front_handler(readDataFunc));
  }));
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::SendPongReply(const boost::asio::const_buffer &pongBuffer)
{
  const auto pongData = std::string((const char *)pongBuffer.data(), pongBuffer.size());

  boost::beast::error_code ec{};
  m_WebSocketStream.pong(pongData.c_str(), ec);
  // handle ping request with a pong reply
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Failed to send pong reply {}", ec.what());

    SessionBase_t::m_SessionClosedCleanupHandler(SessionBase_t::GetRemoteEndpoint());
    return false;
  }
  return true;
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::SendPingRequest()
{
  const auto pingMsg{"ping@" + std::to_string(std::chrono::system_clock::now().time_since_epoch().count())};
  _log_trace(LOG_DETAILS,
             "Send ping request {} {}@{}",
             pingMsg,
             SessionBase_t::GetRemoteEndpoint().port(),
             SessionBase_t::GetRemoteEndpoint().address().to_string());

  boost::beast::error_code ec{};
  m_WebSocketStream.ping(pingMsg.c_str(), ec);
  // handle on_ping
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Failed to send ping request {}", ec.what());

    SessionBase_t::m_SessionClosedCleanupHandler(SessionBase_t::GetRemoteEndpoint());
    return false;
  }
  return true;
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::SendWebSocketData(const boost::asio::const_buffer &sendBuffer)
{
  if (m_WebSocketStream.is_open()) {
    boost::system::error_code ec{};

    m_WebSocketStream.write(sendBuffer, ec);
    if (ec.failed()) {
      _log_error(LOG_DETAILS, "Write failed: {}", ec.what());
      return false;
    }
    return true;
  }
  return false;
}

template <typename TSessionCallback>   //
void WebSocketSession<TSessionCallback>::HandleControlCallback(const boost::beast::websocket::frame_type kind,
                                                               const boost::beast::string_view &payload)
{
  switch (kind) {
  case boost::beast::websocket::frame_type::close: {
    _log_warning(LOG_DETAILS,
                 "Receive session close {} {}:{}",
                 std::string_view(payload.data(), payload.size()),
                 SessionBase_t::GetRemoteEndpoint().address().to_string(),
                 SessionBase_t::GetRemoteEndpoint().port());

    SessionBase_t::m_SessionClosedCleanupHandler(SessionBase_t::GetRemoteEndpoint());

  } break;
  case boost::beast::websocket::frame_type::ping: {
    _log_info(LOG_DETAILS,
              "Received ping {} {}:{}",
              std::string_view(payload.data(), payload.size()),
              SessionBase_t::GetRemoteEndpoint().address().to_string(),
              SessionBase_t::GetRemoteEndpoint().port());
    // send pong reply
    const boost::asio::const_buffer pong_buffer{payload.data(), payload.size()};
    if (not SendPongReply(pong_buffer)) {
      _log_error(LOG_DETAILS, "Failed to send pong {}", std::string_view(payload.data(), payload.size()));
    }
  } break;
  case boost::beast::websocket::frame_type::pong: {
    _log_info(LOG_DETAILS,
              "Received pong {} {}:{}",
              std::string_view(payload.data(), payload.size()),
              SessionBase_t::GetRemoteEndpoint().address().to_string(),
              SessionBase_t::GetRemoteEndpoint().port());

  } break;
  }
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::IsOpen() const
{
  return m_WebSocketStream.is_open();
}

// template <typename TSessionCallback>   //
// boost::beast::tcp_stream::endpoint_type WebSocketSession<TSessionCallback>::GetRemoteEndpoint() const
//{
//   return m_RemoteEndpoint;
// }

// template <typename TSessionCallback>   //
// void WebSocketSession<TSessionCallback>::SetRemoteEndpoint()
//{
//   m_RemoteEndpoint = boost::beast::get_lowest_layer(m_WebSocketStream).socket().remote_endpoint();
// }

template <typename TSessionCallback>   //
std::size_t WebSocketSession<TSessionCallback>::GetReceiveBufferSize() const
{
  boost::system::error_code ec;
  boost::asio::socket_base::receive_buffer_size receiveBufferSizeOption{};
  boost::beast::get_lowest_layer(m_WebSocketStream).socket().get_option(receiveBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Get tcp receive size failed:{}", ec.what());
    return 0;
  }
  return receiveBufferSizeOption.value();
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::SetReceiveBufferSize(const std::size_t size)
{
  boost::system::error_code ec;
  boost::asio::socket_base::receive_buffer_size receiveBufferSizeOption(size);
  boost::beast::get_lowest_layer(m_WebSocketStream).socket().set_option(receiveBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Set tcp receive size failed:{}", ec.what());
    return false;
  }

  _log_info(LOG_DETAILS, "Tcp receive buffer set to:{}", size);

  return true;
}

template <typename TSessionCallback>   //
std::size_t WebSocketSession<TSessionCallback>::GetSendBufferSize() const
{
  boost::system::error_code ec{};
  boost::asio::socket_base::send_buffer_size sendBufferSizeOption{};
  boost::beast::get_lowest_layer(m_WebSocketStream).socket().get_option(sendBufferSizeOption, ec);

  if (ec) {
    _log_error(LOG_DETAILS, "Get tcp send size failed:{}", ec.what());
    return 0;
  }
  return sendBufferSizeOption.value();
}

template <typename TSessionCallback>   //
bool WebSocketSession<TSessionCallback>::SetSendBufferSize(const std::size_t size)
{
  boost::system::error_code ec{};
  boost::asio::socket_base::send_buffer_size sendBufferSizeOption(size);
  boost::beast::get_lowest_layer(m_WebSocketStream).socket().set_option(sendBufferSizeOption, ec);
  if (ec) {
    _log_error(LOG_DETAILS, "Set tcp receive size failed:{}", ec.what());
    return false;
  }
  _log_info(LOG_DETAILS, "Tcp send buffer set to:{}", size);
  return true;
}

}   // namespace moboware::web_socket