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
using namespace moboware;

////////////////////////////////////////////////
WebSocketServer::WebSocketServer(const std::shared_ptr<moboware::common::Service>& service, //
                                 const std::shared_ptr<WebSocketCallback>& callback)
  : m_Service(service)
  , m_WebSocketChannelCallback(callback)
  , m_Acceptor(service->GetIoService())
  , m_sessionTimer(service)
{
}

[[nodiscard]] auto WebSocketServer::Start(const std::string address, const short port) -> bool
{
  // create end point
  const ip::tcp::endpoint endpoint(ip::make_address(address), port);
  //
  beast::error_code ec;

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

  Accept();

  StartSessionTimer();
  return true;
}

void WebSocketServer::StartSessionTimer()
{
  const auto timerFunc = [this](common::Timer& timer) {
    if (const auto removedSessions = this->CheckClosedSessions()) {
      LOG("Removed sessions:" << removedSessions);
    }
    timer.Restart();
  };

  m_sessionTimer.Start(timerFunc, std::chrono::seconds(5));
}

auto WebSocketServer::CheckClosedSessions() -> std::size_t
{
  int numberOfSessions{};

  auto iter = m_Sessions.begin();
  while (iter != m_Sessions.end()) {
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
      const auto key = std::make_pair(webSocket.remote_endpoint().address(), webSocket.remote_endpoint().port());
      const auto session = std::make_shared<WebSocketSession>(this->m_Service, m_WebSocketChannelCallback, std::move(webSocket));

      m_Sessions[key] = session;

      session->Start();
    }
    Accept();
  };

  // The new connection gets its own strand
  m_Acceptor.async_accept(asio::make_strand(m_Service->GetIoService()), beast::bind_front_handler(acceptorFunc));
}
