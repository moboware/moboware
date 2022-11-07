#include "web_socket_server/web_socket_session.h"
#include "common/log.h"
#include <boost/asio/dispatch.hpp>

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware;

WebSocketSession::WebSocketSession(const std::shared_ptr<moboware::common::Service>& service,
                                   const std::shared_ptr<WebSocketCallback>& callback,
                                   tcp::socket&& webSocket)
  : m_Service(service)
  , m_DataHandlerCallback(callback)
  , m_WebSocket(std::move(webSocket))
{
}

void WebSocketSession::Start()
{
  // Get on the correct executor
  // We need to be executing within a strand to perform async operations
  // on the I/O objects in this session. Although not strictly necessary
  // for single-threaded contexts, this example code is written to be
  // thread-safe by default.

  const auto runFunc{ [this]() {
    // Set suggested timeout settings for the websocket
    m_WebSocket.set_option(websocket::stream_base::timeout::suggested(beast::role_type::server));

    // Set a decorator to change the Server of the handshake
    m_WebSocket.set_option(websocket::stream_base::decorator([](websocket::response_type& res) {
      res.set(http::field::server, std::string(BOOST_BEAST_VERSION_STRING) + " websocket-server-async");
    }));

    {
      // Accept the websocket handshake
      const auto acceptHandshakeFn{ [this](const beast::error_code& ec) {
        if (ec) {
          LOG("Failed to accept web socket handshake " << ec);
          return;
        }

        ReadData();
      } };

      m_WebSocket.async_accept(beast::bind_front_handler(acceptHandshakeFn));
    }
  } };

  asio::dispatch(m_WebSocket.get_executor(), beast::bind_front_handler(runFunc));
}

void WebSocketSession::ReadData()
{
  // clear read buffer before every read
  m_ReadBuffer.clear();

  const auto readDataFunc{ [this](const beast::error_code& ec, const std::size_t bytesTransferred) {
    boost::ignore_unused(bytesTransferred);

    // This indicates that the session was closed
    if (ec == websocket::error::closed) {
      LOG("Web socket closed");
      return;
    }

    if (ec) {
      LOG("Read error:" << ec << ", open:" << std::boolalpha << m_WebSocket.is_open());
      return;
    }

    // forward read data to channel

    m_DataHandlerCallback->OnDataRead(m_ReadBuffer, m_WebSocket.next_layer().socket().remote_endpoint());

    // initialize new read operation
    this->ReadData();
  } };

  // wait for data to read
  m_WebSocket.async_read(m_ReadBuffer, beast::bind_front_handler(readDataFunc));
}
