#include "common/channel_base.h"
#include "common/log.h"

using namespace moboware::common;

ChannelBase::ChannelBase(const std::shared_ptr<Service>& service)
  : m_Service(service)
{
}

void ChannelBase::Stop() {}

const std::string MODULES_VALUE{ "Modules" };

bool ChannelBase::LoadConfig(const Json::Value& channelConfig)
{
  if (!channelConfig.isMember(NAME_VALUE) || !channelConfig.isMember(MODULES_VALUE)) {
    return false;
  }

  m_ChannelName = channelConfig[NAME_VALUE].asString();
  if (!LoadChannelConfig(channelConfig)) {
    LOG("Failed to load channel config");
    return false;
  }

  const auto modules = channelConfig[MODULES_VALUE];
  if (modules.isArray()) {
    for (const auto& moduleValue : modules) {
      if (!moduleValue.isMember(NAME_VALUE)) {
        return false;
      }

      const auto moduleName = moduleValue[NAME_VALUE].asString();
      const auto module = CreateModule(moduleName, moduleValue);
      if (module) {
        if (module->LoadConfig(moduleValue)) {
          m_Modules.push_back(module);
        }
      }
    }
  }
  return !m_Modules.empty();
}

bool ChannelBase::StartModules()
{
  for (const auto module : m_Modules) {
    if (!module->Start()) {
      return false;
    }
  }
  return true;
}
