#include "web_socket/web_socket_client.h"
#include "common/log_stream.h"
#include <boost/asio.hpp>
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::web_socket;

////////////////////////////////////////////////

WebSocketClient::WebSocketClient(const std::shared_ptr<moboware::common::Service>& service)
  : m_Service(service)
{
}

auto WebSocketClient::Start(const std::string& address, const short port) -> bool
{
  LOG_DEBUG("Connecting web socket client:" << address << ":" << port);

  boost::asio::ip::tcp::socket webSocket(m_Service->GetIoService());
  m_Session = std::make_shared<WebSocketSession>(m_Service, shared_from_this(), std::move(webSocket));

  return m_Session->Connect(address, port);
}

auto WebSocketClient::SendWebSocketData(const const_buffer& sendBuffer) -> std::size_t
{
  return m_Session->SendWebSocketData(sendBuffer);
}

void WebSocketClient::OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint)
{
  if (m_WebSocketDataReceivedFn) {
    m_WebSocketDataReceivedFn(readBuffer, remoteEndPoint);
  }
}

// called when the session is closed and the session can be cleaned up.
void WebSocketClient::OnSessionClosed() {}