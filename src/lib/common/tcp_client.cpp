#include "common/tcp_client.h"
#include "common/log_stream.h"
#include <boost/asio.hpp>
#include <boost/bind/bind.hpp>
#include <cstdlib>
#include <vector>

using namespace boost;
using namespace boost::asio;
using namespace boost::asio::ip;
using namespace moboware::common;

TcpClient::TcpClient(const std::shared_ptr<Service>& io_service)
  : Session(io_service)
  , _service(io_service)
  , _pingTimer(io_service)
{
  SetSessionReceiveData([this](const std::shared_ptr<Session>& session, const std::array<char, maxBufferSize>& readBuffer, const std::size_t bytesRead) { //
    this->HandleReceivedData(session, readBuffer, bytesRead);
  });
}

bool TcpClient::Connect(const std::string& address, const std::uint16_t port)
{
  LOG_DEBUG("Connecting to: " << address << ":" << port);

  ip::tcp::resolver tcpResolver(_service->GetIoService());

  try { // todo change to errorCode
    const ip::tcp::endpoint endpoint(asio::ip::address::from_string(address), port);

    system::error_code errorCode;
    Session::Socket().connect(endpoint, errorCode);
    if (errorCode.failed()) {
      LOG_DEBUG("Connect failed " << errorCode);
      return false;
    }
  } catch (const std::exception& e) {
    LOG_DEBUG("Failed to resolve address:" << address << ", Error:" << e.what());
    return false;
  }
  Session::Start();

  LOG_DEBUG("Client is connected");

  // move to protocol layer
  const auto pingFunction = [this](Timer& timer) {
    const std::string payloadBuffer{ "ping" };
    if (Session::Send(asio::const_buffer(payloadBuffer.c_str(), payloadBuffer.size())) > 0) {
      timer.Restart();
    } else {
      LOG_DEBUG("Send ping failed");
    }
  };

  _pingTimer.Start(pingFunction, std::chrono::seconds(3));

  return true;
}

void TcpClient::HandleReceivedData(const std::shared_ptr<Session>& session, const std::array<char, maxBufferSize>& readBuffer, const std::size_t bytesRead)
{ // move to  protocol handler !!!!!!!!!!!!!!!!!!
  LOG_DEBUG("Handle received data, size:" << bytesRead << "," << session->GetRemoteEndpoint().first << ":" << session->GetRemoteEndpoint().second);
  const std::string payload(&readBuffer.data()[sizeof(std::uint16_t)], bytesRead - sizeof(std::uint16_t));
}
