#include "common/channel_base.h"
#include "common/logger.hpp"
#include <boost/json/src.hpp>

using namespace moboware::common;

ChannelBase::ChannelBase(const std::shared_ptr<Service> &service)
  : m_Service(service)
{
}

void ChannelBase::Stop()
{
}

const std::string MODULES_VALUE{"Modules"};

bool ChannelBase::LoadConfig(const boost::json::value &channelConfig)
{
  if (not channelConfig.as_object().contains(NAME_VALUE) ||   //
      not channelConfig.as_object().contains(MODULES_VALUE)) {
    return false;
  }

  m_ChannelName = channelConfig.at(NAME_VALUE).as_string().c_str();
  if (not LoadChannelConfig(channelConfig)) {
    LOG_DEBUG("Failed to load channel config");
    return false;
  }

  const auto modules = channelConfig.at(MODULES_VALUE);
  if (modules.is_array()) {
    for (const auto &moduleValue : modules.as_array()) {
      if (not moduleValue.as_object().contains(NAME_VALUE)) {
        return false;
      }

      const auto moduleName = moduleValue.at(NAME_VALUE).as_string().c_str();
      const auto module = CreateModule(moduleName, moduleValue);
      if (module) {
        if (module->LoadConfig(moduleValue)) {
          m_Modules.push_back(module);
        }
      }
    }
  }
  return not m_Modules.empty();
}

bool ChannelBase::StartModules()
{
  for (const auto module : m_Modules) {
    if (not module->Start()) {
      return false;
    }
  }
  return true;
}
