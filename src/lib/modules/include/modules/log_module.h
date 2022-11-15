#pragma once
#include "common/imodule_factory.h"
#include "common/service.h"
#include <memory>

namespace moboware::modules {

class LogModule : public common::IModule
{
public:
  explicit LogModule(const std::shared_ptr<common::Service>& service, //
                     const std::shared_ptr<common::ChannelInterface>& channelInterface);
  virtual ~LogModule() = default;
  LogModule(const LogModule&) = delete;
  LogModule(LogModule&&) = delete;
  LogModule& operator=(const LogModule&) = delete;
  LogModule& operator=(LogModule&&) = delete;

  bool LoadConfig(const Json::Value& moduleValue) final;
  bool Start() final;
  void OnWebSocketDataReceived(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) final;

private:
};

class LogModuleFactory : public common::IModuleFactory
{
public:
  const std::shared_ptr<common::IModule> CreateModule(const std::shared_ptr<common::Service>& service, //
                                                      const std::shared_ptr<common::ChannelInterface>& channelInterface)
  {
    return std::make_shared<LogModule>(service, channelInterface);
  }
};
}