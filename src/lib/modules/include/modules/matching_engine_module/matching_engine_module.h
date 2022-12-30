#pragma once

#include "common/imodule.h"
#include "common/imodule_factory.h"
#include "common/service.h"
#include "modules/matching_engine_module/matching_engine.h"

namespace moboware::modules {

class MatchingEngineModule
  : public common::IModule
  , public std::enable_shared_from_this<MatchingEngineModule>
  , public IOrderHandler
{
public:
  explicit MatchingEngineModule(const std::shared_ptr<common::Service>& service, //
                                const std::shared_ptr<common::ChannelInterface>& channelInterface);
  MatchingEngineModule(const MatchingEngineModule&) = delete;
  MatchingEngineModule(MatchingEngineModule&&) = delete;
  MatchingEngineModule& operator=(const MatchingEngineModule&) = delete;
  MatchingEngineModule& operator=(MatchingEngineModule&&) = delete;
  virtual ~MatchingEngineModule() = default;

  bool LoadConfig(const boost::json::value& moduleValue) final;
  bool Start() final;
  void OnWebSocketDataReceived(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) final;

private:
  void HandleOrderInsert(const OrderInsertData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint) final;

  void HandleOrderAmend(const OrderAmendData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint) final;

  void HandleOrderCancel(const OrderCancelData& orderCancel, const boost::asio::ip::tcp::endpoint& endpoint) final;

  void GetOrderBook(const std::string& instrument, const boost::asio::ip::tcp::endpoint& endpoint) final;

  /// @brief map of matching engines per instrument
  std::map<std::string, std::shared_ptr<MatchingEngine>> m_MatchingEngines;
};

class MatchingEngineModuleFactory : public common::IModuleFactory
{
public:
  const std::shared_ptr<common::IModule> CreateModule(const std::shared_ptr<common::Service>& service, //
                                                      const std::shared_ptr<common::ChannelInterface>& channelInterface)
  {
    return std::make_shared<MatchingEngineModule>(service, channelInterface);
  }
};
}