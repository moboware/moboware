#include "modules/matching_engine_module/matching_engine_module.h"
#include "common/log_stream.h"
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
  LOG_DEBUG("Load matching engine module Config");
  const Json::Value instrumentsArrayValues{ moduleValue["Instruments"] };

  for (Json::ArrayIndex i{ 0 }; i < instrumentsArrayValues.size(); i++) {
    const auto instrument{ instrumentsArrayValues[i].asString() };
    LOG_DEBUG("Loading instrument " << instrument);
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
  LOG_DEBUG("Handle order insert...");
  auto iter = m_MatchingEngines.find(orderInsert.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    LOG_ERROR("Unknown instrument " << orderInsert.GetInstrument());
    return;
  }
  iter->second->OrderInsert(orderInsert, endpoint);
}

void MatchingEngineModule::HandleOrderAmend(const OrderAmendData& orderAmend, const boost::asio::ip::tcp::endpoint& endpoint)
{
  LOG_DEBUG("Handle order amend...");
  auto iter = m_MatchingEngines.find(orderAmend.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    LOG_ERROR("Unknown instrument " << orderAmend.GetInstrument());
    return;
  }
  iter->second->OrderAmend(orderAmend, endpoint);
}

void MatchingEngineModule::HandleOrderCancel(const OrderCancelData& orderCancel, const boost::asio::ip::tcp::endpoint& endpoint)
{
  LOG_DEBUG("Handle order cancel...");
  auto iter = m_MatchingEngines.find(orderCancel.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    LOG_ERROR("Unknown instrument " << orderCancel.GetInstrument());
    return;
  }
  iter->second->OrderCancel(orderCancel, endpoint);
}

void MatchingEngineModule::GetOrderBook(const std::string& instrument, const boost::asio::ip::tcp::endpoint& endpoint)
{
  LOG_DEBUG("Get Order Book:" << instrument);
  auto iter = m_MatchingEngines.find(instrument);
  if (iter == std::end(m_MatchingEngines)) {
    LOG_ERROR("Unknown instrument " << instrument);
    return;
  }

  iter->second->GetOrderBook(endpoint);
}
