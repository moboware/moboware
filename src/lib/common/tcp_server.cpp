#include "common/tcp_server.h"
#include "common/log_stream.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <cstdlib>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::common;

TcpServer::TcpServer(const std::shared_ptr<Service>& service)
  : m_Service(service)
  , m_Acceptor(service->GetIoService())
  , m_PingTimer(service)
{

  m_RemoveSession = [this](const Session::Endpoint& endPoint) {
    // need to post because this is called from the session that we are going to delete.
    PostRemoveSession(endPoint);
  };
}

void TcpServer::PostRemoveSession(const Session::Endpoint& endPoint)
{
  asio::post(m_Service->GetIoService(), [this, endPoint]() {
    m_Sessions.erase(endPoint);
  });
}

bool TcpServer::StartListening(const std::uint16_t port)
{
  {
    system::error_code errorCode;

    m_Acceptor.open(asio::ip::tcp::v4(), errorCode);
    if (errorCode.failed()) {
      LOG_DEBUG("Open acceptor failed");
      return false;
    }
  }

  m_Acceptor.set_option(tcp::acceptor::reuse_address(true));
  const tcp::endpoint endPoint(tcp::v4(), port);

  {
    // bind address and port
    system::error_code errorCode;
    m_Acceptor.bind(endPoint, errorCode);
    if (errorCode.failed()) {
      LOG_DEBUG("Bind failed");
      return false;
    }
  }

  {
    // setup listen
    system::error_code errorCode;
    m_Acceptor.listen(socket_base::max_listen_connections, errorCode);
    if (errorCode.failed()) {
      LOG_DEBUG("Setup listener failed");
      return false;
    }
  }

  AcceptConnection();

  LOG_DEBUG("Starting listener on port:" << port);

  const auto pingSessionsFunc = [this](Timer& timer) {
    for (const auto& [k, session] : m_Sessions) {
      const std::string payloadBuffer{ "ping" };
      session->Send(asio::const_buffer(payloadBuffer.c_str(), payloadBuffer.size()));
      LOG_DEBUG("Send ping to " << session->GetRemoteEndpoint().first << ":" << session->GetRemoteEndpoint().second);
    }

    timer.Restart();
  };

  m_PingTimer.Start(pingSessionsFunc, std::chrono::seconds(2));
  return true;
}

void TcpServer::AcceptSession(const std::shared_ptr<ServerSession>& session, const system::error_code& errorCode)
{
  if (errorCode.failed()) {
    LOG_DEBUG("Accept Error " << errorCode);
    return;
  }

  session->Start();

  m_Sessions[session->GetRemoteEndpoint()] = session;
  LOG_DEBUG("Accepted new session, #sessions:" << m_Sessions.size());
}

void TcpServer::SetSessionHandlers(const std::shared_ptr<ServerSession>& session)
{
  session->SetSessionDisconnected([this](const std::shared_ptr<Session>& session, const Session::Endpoint& endPoint) {
    this->SessionDisconnected(session, endPoint);
  });

  session->SetSessionReceiveData(m_ReceiveDataCallbackFunction);
}

void TcpServer::AcceptConnection()
{
  LOG_DEBUG("Accept connection");

  const auto session = std::make_shared<ServerSession>(m_Service, m_RemoveSession);

  const auto acceptFn = [this, session](const system::error_code& errorCode) //
  {
    AcceptSession(session, errorCode);
    SetSessionHandlers(session);

    AcceptConnection();
  };

  m_Acceptor.async_accept(session->Socket(), acceptFn);
}

void TcpServer::HandleAccept(const std::shared_ptr<Session>& session, const system::error_code& error)
{
  if (!error) {
    LOG_DEBUG("Accept session");

    session->Start();

    {
      const auto newSession = std::make_shared<ServerSession>(m_Service, m_RemoveSession);
      SetSessionHandlers(newSession);

      m_Acceptor.async_accept(newSession->Socket(), boost::bind(&TcpServer::HandleAccept, this, newSession, asio::placeholders::error));
    }
  }
}

void TcpServer::SetSessionReceiveData(const Session::ReceiveDataFunction& fn)
{
  m_ReceiveDataCallbackFunction = fn;
}

void TcpServer::SessionDisconnected(const std::shared_ptr<Session>& /*session*/, const Session::Endpoint& endPoint)
{
  if (m_RemoveSession) {
    LOG_DEBUG("Session is disconnecting end point :" << endPoint.first << ":" << endPoint.second);
    m_RemoveSession(endPoint);
  }
}

std::size_t TcpServer::SendWebSocketData(const std::string& data, const Session::Endpoint& endPoint)
{
  const auto iter = m_Sessions.find(endPoint);
  if (iter != std::end(m_Sessions)) {
    const auto session = iter->second;
    const_buffer buffer(data.c_str(), data.length()); // make part of the sesion
    return session->Send(buffer);
  }
  return 0;
}
