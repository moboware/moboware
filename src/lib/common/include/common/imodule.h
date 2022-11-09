#pragma once
#include "common/channel_interface.h"
#include "common/service.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <json/json.h>
#include <string>

namespace moboware::common {

class IModule
{
public:
  explicit IModule(const std::string& moduleName,                                     //
                   const std::shared_ptr<Service>& service,                           //
                   const std::shared_ptr<common::ChannelInterface>& channelInterface) //
    : m_ModuleName(moduleName)
    , m_Service(service)
    , m_ChannelInterface(channelInterface)
  {
  }

  virtual ~IModule() = default;
  IModule(const IModule&) = delete;
  IModule(IModule&&) = delete;
  IModule& operator=(const IModule&) = delete;
  IModule& operator=(IModule&&) = delete;

  virtual bool Start() = 0;

  virtual bool LoadConfig(const Json::Value& moduleValue) = 0;
  virtual void OnWebSocketPayload(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) = 0;

protected:
  const std::shared_ptr<common::ChannelInterface>& GetChannelInterface() const { return m_ChannelInterface; }

  const std::string& GetModuleName() const { return m_ModuleName; }

private:
  const std::string m_ModuleName;
  const std::shared_ptr<Service> m_Service;
  const std::shared_ptr<common::ChannelInterface> m_ChannelInterface;
};
}
