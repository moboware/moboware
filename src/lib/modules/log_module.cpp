#include "modules/log_module.h"
#include "common/log.h"

using namespace moboware::modules;
using namespace moboware::common;

LogModule::LogModule(const std::shared_ptr<common::Service>& service,                   //
  const std::shared_ptr<common::ChannelInterface>& channelInterface) //
  : common::IModule("LogModule", service, channelInterface)                           //
{
}

bool LogModule::LoadConfig(const Json::Value& moduleValue)
{
  LOG("Load module Config");

  return true;
}

bool LogModule::Start()
{
  return true;
}

void LogModule::OnWebSocketPayload(const uint64_t tag, const std::string& payload)
{
  GetChannelInterface()->SendData(tag, "Hello websocket " + payload);
}