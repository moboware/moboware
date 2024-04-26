#include "web_socket/web_socket_session.h"
#include "common/logger.hpp"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/dispatch.hpp>

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::web_socket;

WebSocketSession::WebSocketSession(const std::shared_ptr<moboware::common::Service> &service,
                                   const std::shared_ptr<WebSocketSessionCallback> &callback,
                                   tcp::socket &&webSocket)
    : m_Service(service)
    , m_DataHandlerCallback(callback)
    , m_WebSocket(std::move(webSocket))
{
}

void WebSocketSession::Accept()
{
  // Get on the correct executor
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.

  const auto runFunc{[this]() {
    // Set suggested timeout settings for the websocket
    m_WebSocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    m_WebSocket.set_option(websocket::stream_base::decorator([](websocket::response_type &res) {
      res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
    }));

    {
      // Accept the websocket handshake
      const auto acceptHandshakeFn{[this](const beast::error_code &ec) {
        if (ec) {
          _log_debug(LOG_DETAILS, "Failed to accept web socket handshake ", ec.to_string());
          return;
        }

        ReadData();
      }};

      m_WebSocket.async_accept(beast::bind_front_handler(acceptHandshakeFn));
    }
  }};

  asio::dispatch(m_WebSocket.get_executor(), beast::bind_front_handler(runFunc));
}

void WebSocketSession::ReadData()
{
  // clear read buffer before every read
  m_ReadBuffer.clear();

  const auto readDataFunc{[this](const beast::error_code &ec, const std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed) {
      _log_info(LOG_DETAILS, "Web socket closed");
      m_DataHandlerCallback->OnSessionClosed();
      return;
    }

    if (ec) {
      _log_error(LOG_DETAILS, "Read error: {}, open:{}", ec.to_string(), m_WebSocket.is_open());
      return;
    }

    // forward read data to channel

    m_DataHandlerCallback->OnDataRead(m_ReadBuffer, m_WebSocket.next_layer().socket().remote_endpoint());

    // initialize new read operation
    this->ReadData();
  }};

  // wait for data to read
  m_WebSocket.async_read(m_ReadBuffer, beast::bind_front_handler(readDataFunc));
}

auto WebSocketSession::SendWebSocketData(const boost::asio::const_buffer &sendBuffer) -> bool
{
  beast::error_code ec;

  const auto n = m_WebSocket.write(sendBuffer, ec);

  return n != 0 && !ec;
}

auto WebSocketSession::Connect(const std::string &address, const short port) -> bool
{
  // Set the timeout for the operation. Look up the domain name
  tcp::resolver resolver(m_Service->GetIoService());
  system::error_code ec;
  const auto results{resolver.resolve(address, std::to_string(port), ec)};
  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Resolving address failed:{}:{}, {}", address, port, ec.to_string());
    return false;
  }

  // Make the connection on the IP address we get from a lookup
  const ip::tcp::endpoint endpoint(ip::make_address(address, ec), port);

  m_WebSocket.next_layer().connect(endpoint, ec);

  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Connect to Web socket failed: {}:{}, {}", address, port, ec.to_string());
    return false;
  }
  // Update the host_ string. This will provide the value of the
  // Host HTTP header during the WebSocket handshake.
  // See https://tools.ietf.org/html/rfc7230#section-5.4
  const auto host{address + ':' + std::to_string(port)};

  // Set a decorator to change the User-Agent of the handshake
  m_WebSocket.set_option(websocket::stream_base::decorator([](websocket::request_type &req) {
    req.set(http::field::user_agent, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-client");
  }));

  // Perform the websocket handshake
  m_WebSocket.handshake(host, "/", ec);

  if (ec.failed()) {
    _log_error(LOG_DETAILS, "Web socket handshake failed: {}:{}, {}", address, port, ec.to_string());
    return false;
  }

  ReadData();
  return true;
}
