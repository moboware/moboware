#pragma once
#include "common/imodule_factory.h"
#include "common/service.h"
#include "common/tcp_client.h"
#include "common/timer.h"
#include <memory>

namespace moboware::modules {
class TcpClientModule : public common::IModule
{
public:
  TcpClientModule(const std::shared_ptr<common::Service>& service, //
                  const std::shared_ptr<common::ChannelInterface>& channelInterface);
  virtual ~TcpClientModule() = default;

  TcpClientModule(const TcpClientModule&) = delete;
  TcpClientModule(TcpClientModule&&) = delete;
  TcpClientModule& operator=(const TcpClientModule&) = delete;
  TcpClientModule& operator=(TcpClientModule&&) = delete;

  bool LoadConfig(const boost::json::value& moduleValue) final;
  bool Start() final;
  void OnWebSocketDataReceived(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) final;

private:
  common::Timer m_Timer; /// reconnect timer
  std::shared_ptr<common::TcpClient> m_TcpClient;

  struct Config
  {
    std::string m_Address;
    int m_Port{};
  };

  Config m_Config;
};

class TcpClientModuleFactory : public common::IModuleFactory
{
public:
  const std::shared_ptr<common::IModule> CreateModule(const std::shared_ptr<common::Service>& service, //
                                                      const std::shared_ptr<common::ChannelInterface>& channelInterface)
  {
    return std::make_shared<TcpClientModule>(service, channelInterface);
  }
};
}