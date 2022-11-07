#pragma once

#include "common/channel_interface.h"
#include "common/imodule.h"

namespace moboware::common {
/**
 * @brief Abstract class for module factory template
 */
class IModuleFactory
{
public:
  IModuleFactory() = default;
  virtual ~IModuleFactory() = default;

  IModuleFactory(const IModuleFactory&) = delete;
  IModuleFactory(IModuleFactory&&) = delete;
  IModuleFactory& operator=(const IModuleFactory&) = delete;
  IModuleFactory& operator=(IModuleFactory&&) = delete;

  virtual const std::shared_ptr<IModule> CreateModule(const std::shared_ptr<Service>& service, const std::shared_ptr<ChannelInterface>& channelInterface) = 0;
};
}