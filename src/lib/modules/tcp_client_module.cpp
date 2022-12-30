#include "modules/tcp_client_module.h"
#include "common/log_stream.h"

using namespace moboware::modules;
using namespace moboware::common;

TcpClientModule::TcpClientModule(const std::shared_ptr<common::Service>& service, //
                                 const std::shared_ptr<common::ChannelInterface>& channelInterface)
  : common::IModule("TcpClientModule", service, channelInterface)
  , //
  m_Timer(service)
  , m_TcpClient(std::make_shared<common::TcpClient>(service))
{
}

bool TcpClientModule::LoadConfig(const boost::json::value& moduleValue)
{
  LOG_DEBUG("Load module Config");

  m_Config.m_Address = moduleValue.at("Address").as_string().c_str();
  m_Config.m_Port = moduleValue.at("Port").as_int64();

  return true;
}

bool TcpClientModule::Start()
{
  if (m_TcpClient->Connect(m_Config.m_Address, m_Config.m_Port)) {
    // const common::Timer::TimerFunction timerFunc = [this](common::Timer &timer) //
    //{
    //     m_ChannelInterface->SendWebSocketData("Hello timer..");
    //     timer.Restart();
    // };
    //
    // m_Timer.Start(timerFunc, std::chrono::milliseconds(1000));
    return true;
  }
  return false;
}

void TcpClientModule::OnWebSocketDataReceived(const boost::beast::flat_buffer& sendBuffer, const boost::asio::ip::tcp::endpoint& endpoint) {}