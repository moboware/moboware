#include "common/session.h"
#include "common/log.h"
#include <boost/asio.hpp>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::common;

Session::Session(const std::shared_ptr<Service>& service)
  : m_Socket(service->GetIoService())
{
  _readDataHandler = [this](const system::error_code& errorCode)
  {
    if (ReadData(errorCode))
    {
      AsyncReceive();
    }
  };
}

Session::~Session()
{
  CloseSocket();
}

tcp::socket& Session::Socket()
{
  return m_Socket;
}

void Session::Start()
{
  LOG("Start session");

  Session::AsyncReceive();

  const auto remoteEndPoint = m_Socket.remote_endpoint();
  m_RemoteEndpoint = std::make_pair(remoteEndPoint.address().to_string(), remoteEndPoint.port());

  asio::socket_base::linger option(true, 0);
  m_Socket.set_option(option);
}

void Session::AsyncReceive()
{
  if (m_Socket.is_open())
  {
    m_Socket.async_wait(ip::tcp::socket::wait_read, _readDataHandler);
  }
}

bool Session::ReadData(const system::error_code& errorCode)
{
  if (errorCode.failed())
  {
    LOG("Read data failed " << errorCode);
    return false;
  }

  system::error_code readError;
  const auto bytesAvailable = m_Socket.available(readError);
  if (readError.failed())
  {
    LOG("Read bytes available failed " << readError);
    return false;
  }

  if (bytesAvailable > 0)
  {
    readError.clear();
    std::array<char, maxBufferSize> readBuffer;

    // LOG(bytesAvailable << " bytes available...");
    const auto bytesRead = asio::read(m_Socket,
      asio::buffer(readBuffer.data(), readBuffer.size()),
      asio::transfer_at_least(bytesAvailable),
      readError);

    if (readError.failed())
    {
      LOG("Read data bytes failed " << readError);
      return false;
    }

    if (m_ReceiveDataCallbackFunction && shared_from_this())
    {
      m_ReceiveDataCallbackFunction(shared_from_this(), readBuffer, bytesRead);
    }
  }
  return true;
}

std::size_t Session::Send(const asio::const_buffer& sendBuffer)
{
  std::vector<asio::const_buffer> sendBuffers;
  const std::uint16_t mesgLen = sendBuffer.size();
  const asio::const_buffer header(&mesgLen, sizeof(std::uint16_t));
  sendBuffers.push_back(header);
  sendBuffers.push_back(sendBuffer);

  system::error_code errorCode;
  const auto bytesSend = asio::write(Session::Socket(), sendBuffers, errorCode);

  if (errorCode.failed())
  {
    if (errorCode == asio::error::connection_reset ||
      errorCode == asio::error::connection_aborted ||
      errorCode == asio::error::broken_pipe ||
      errorCode == asio::error::network_reset ||
      errorCode == asio::error::network_down)
    {
      if (m_SessionDisconnectedCallback && shared_from_this())
      {
        m_SessionDisconnectedCallback(shared_from_this(), GetRemoteEndpoint());
      }
      CloseSocket();
    }
    else
    {
      LOG("send failed " << errorCode);
    }
    return 0;
  }
  // LOG("Socket write " << bytesSend << " bytes");
  return bytesSend;
}

void Session::CloseSocket()
{
  if (m_Socket.is_open())
  {
    m_Socket.close();
  }
}

const Session::Endpoint& Session::GetRemoteEndpoint() const
{
  return m_RemoteEndpoint;
}

void Session::SetSessionDisconnected(const std::function<void(const std::shared_ptr<Session>&, const Endpoint&)>& fn)
{
  m_SessionDisconnectedCallback = fn;
}

void Session::SetSessionReceiveData(const ReceiveDataFunction& fn)
{
  m_ReceiveDataCallbackFunction = fn;
}
