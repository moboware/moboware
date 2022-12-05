#include "web_socket_server/web_socket_server.h"
#include "common/log.h"
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/websocket.hpp>

using namespace boost;
using namespace boost::beast;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::web_socket;

////////////////////////////////////////////////
WebSocketServer::WebSocketServer(const std::shared_ptr<moboware::common::Service>& service)
  : m_Service(service)
  , m_Acceptor(service->GetIoService())
{
}

auto WebSocketServer::Start(const std::string address, const short port) -> bool
{
  // create end point
  beast::error_code ec;
  const ip::tcp::endpoint endpoint(ip::make_address(address, ec), port);
  //
  if (ec) {
    LOG("Make address failed:" << ec);
    return false;
  }

  // Open the acceptor
  m_Acceptor.open(endpoint.protocol(), ec);
  if (ec) {
    LOG("Open acceptor failed:" << ec);
    return false;
  }

  // Allow address reuse
  m_Acceptor.set_option(asio::socket_base::reuse_address(true), ec);
  if (ec) {
    LOG("set_option failed:" << ec);
    return false;
  }

  // Bind to the server address
  m_Acceptor.bind(endpoint, ec);
  if (ec) {
    LOG("bind failed:" << ec);
    return false;
  }

  // Start listening for connections
  m_Acceptor.listen(asio::socket_base::max_listen_connections, ec);
  if (ec) {
    LOG("start listen failed:" << ec);
    return false;
  }

  // start accepting connections
  Accept();

  return true;
}

auto WebSocketServer::CheckClosedSessions() -> std::size_t
{
  int numberOfSessions{};

  auto iter = std::begin(m_Sessions);
  while (iter != std::end(m_Sessions)) {
    const auto session = iter->second;
    if (not session->IsOpen()) {
      iter = m_Sessions.erase(iter);
      numberOfSessions++;
    } else {
      iter++;
    }
  }
  return numberOfSessions;
}

void WebSocketServer::Accept()
{
  const auto acceptorFunc = [this](beast::error_code ec, tcp::socket webSocket) {
    if (ec) {
      LOG("Failed to accept connection:" << ec);
    } else {
      LOG("Connection accepted from " << webSocket.remote_endpoint().address().to_string() << ":" << webSocket.remote_endpoint().port());

      // create session and store in our session list
      const auto endPointKey = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());

      const auto session = std::make_shared<WebSocketSession>(m_Service, shared_from_this(), std::move(webSocket));
      session->Start();
      m_Sessions[endPointKey] = session;
    }
    Accept();
  };

  // The new connection gets its own strand
  m_Acceptor.async_accept(asio::make_strand(m_Service->GetIoService()), beast::bind_front_handler(acceptorFunc));
}

auto WebSocketServer::SendWebSocketData(const boost::asio::const_buffer& sendBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint) -> bool
{
  const auto endPointKey = std::make_pair(remoteEndPoint.address(), remoteEndPoint.port());
  const auto iter = m_Sessions.find(endPointKey);
  if (iter != std::end(m_Sessions)) {

    const auto& session = iter->second;
    return session->SendWebSocketData(sendBuffer);
  }

  LOG("No endpoint not found to send data to" << remoteEndPoint.address().to_string() << remoteEndPoint.port());
  return false;
}

void WebSocketServer::SetWebSocketDataReceived(const WebSocketDataReceivedFn& fn)
{
  m_WebSocketDataReceivedFn = fn;
}

void WebSocketServer::OnDataRead(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& remoteEndPoint)
{
  if (m_WebSocketDataReceivedFn) {
    m_WebSocketDataReceivedFn(readBuffer, remoteEndPoint);
  }
}

void WebSocketServer::OnSessionClosed()
{
  const auto removeSessionFn{ [this]() {
    CheckClosedSessions();
  } };

  m_Service->GetIoService().post(removeSessionFn);
}
