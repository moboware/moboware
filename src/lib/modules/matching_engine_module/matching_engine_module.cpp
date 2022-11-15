#include "modules/matching_engine_module/matching_engine_module.h"
#include "common/log.h"
#include "modules/matching_engine_module/order_event_processor.h"

using namespace moboware::modules;
using namespace moboware::common;

MatchingEngineModule::MatchingEngineModule(const std::shared_ptr<common::Service>& service,                   //
                                           const std::shared_ptr<common::ChannelInterface>& channelInterface) //
  : common::IModule("MatchingEngineModule", service, channelInterface)
{
}

bool MatchingEngineModule::LoadConfig(const Json::Value& moduleValue)
{
  LOG("Load matching engine module Config");
  const Json::Value instrumentsArrayValues{ moduleValue["Instruments"] };

  for (Json::ArrayIndex i{ 0 }; i < instrumentsArrayValues.size(); i++) {
    const auto instrument{ instrumentsArrayValues[i].asString() };
    LOG("Loading instrument " << instrument);
    m_MatchingEngines[instrument] = std::make_shared<MatchingEngine>(GetChannelInterface());
  }

  return true;
}

bool MatchingEngineModule::Start()
{
  return true;
}

void MatchingEngineModule::OnWebSocketDataReceived(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint)
{
  OrderEventProcessor orderEventProcessor(weak_from_this(), endpoint);

  orderEventProcessor.Process(readBuffer);
}

void MatchingEngineModule::HandleOrderInsert(const OrderData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint)
{
  LOG("Handle order insert...");
  auto iter = m_MatchingEngines.find(orderInsert.instrument);
  if (iter == m_MatchingEngines.end()) {
    LOG_ERROR("Unknown instrument " << orderInsert.instrument);
    return;
  }
  iter->second->OrderInsert(orderInsert, endpoint);
}