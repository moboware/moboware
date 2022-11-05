#include "modules/tcp_client_module.h"
#include "common/log.h"

using namespace moboware::modules;
using namespace moboware::common;

TcpClientModule::TcpClientModule(const std::shared_ptr<common::Service>& service,                   //
  const std::shared_ptr<common::ChannelInterface>& channelInterface) //
  : common::IModule("TcpClientModule", service, channelInterface),                                //
  m_Timer(service),
  m_TcpClient(std::make_shared<common::TcpClient>(service))
{
}

bool TcpClientModule::LoadConfig(const Json::Value& moduleValue)
{
  LOG("Load module Config");

  m_Config.m_Address = moduleValue["Address"].asString();
  m_Config.m_Port = moduleValue["Port"].asUInt();

  return true;
}

bool TcpClientModule::Start()
{
  if (m_TcpClient->Connect(m_Config.m_Address, m_Config.m_Port))
  {
    // const common::Timer::TimerFunction timerFunc = [this](common::Timer &timer) //
    //{
    //     m_ChannelInterface->SendData("Hello timer..");
    //     timer.Restart();
    // };
    //
    // m_Timer.Start(timerFunc, std::chrono::milliseconds(1000));
    return true;
  }
  return false;
}

void TcpClientModule::OnWebSocketPayload(const uint64_t tag, const std::string& payload)
{
}