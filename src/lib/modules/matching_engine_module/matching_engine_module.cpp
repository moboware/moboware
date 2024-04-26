#include "modules/matching_engine_module/matching_engine_module.h"
#include "common/logger.hpp"
#include "modules/matching_engine_module/order_event_processor.h"

using namespace moboware::modules;
using namespace moboware::common;

MatchingEngineModule::MatchingEngineModule(const std::shared_ptr<common::Service> &service,                     //
                                           const std::shared_ptr<common::ChannelInterface> &channelInterface)   //
    : common::IModule("MatchingEngineModule", service, channelInterface)
{
}

bool MatchingEngineModule::LoadConfig(const boost::json::value &moduleValue)
{
  _log_debug(LOG_DETAILS, "Load matching engine module Config");
  const auto &instrumentsArrayValues{moduleValue.at("Instruments").as_array()};

  for (const auto &instrumentValue : instrumentsArrayValues) {
    const auto instrument{instrumentValue.as_string().c_str()};

    _log_debug(LOG_DETAILS, "Loading instrument {}", instrument);
    m_MatchingEngines[instrument] = std::make_shared<MatchingEngine>(GetChannelInterface());
  }

  return true;
}

bool MatchingEngineModule::Start()
{
  return true;
}

void MatchingEngineModule::OnWebSocketDataReceived(const boost::beast::flat_buffer &readBuffer,
                                                   const boost::asio::ip::tcp::endpoint &endpoint)
{
  OrderEventProcessor orderEventProcessor(weak_from_this(), endpoint);

  orderEventProcessor.Process(readBuffer);
}

void MatchingEngineModule::HandleOrderInsert(OrderInsertData &&orderInsert, const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_debug(LOG_DETAILS, "Handle order insert...");
  auto iter = m_MatchingEngines.find(orderInsert.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    _log_error(LOG_DETAILS, "Unknown instrument {}", orderInsert.GetInstrument());
    return;
  }
  iter->second->OrderInsert(std::forward<OrderInsertData>(orderInsert), endpoint);
}

void MatchingEngineModule::HandleOrderAmend(const OrderAmendData &orderAmend, const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_debug(LOG_DETAILS, "Handle order amend...");
  auto iter = m_MatchingEngines.find(orderAmend.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    _log_error(LOG_DETAILS, "Unknown instrument {}", orderAmend.GetInstrument());
    return;
  }
  iter->second->OrderAmend(orderAmend, endpoint);
}

void MatchingEngineModule::HandleOrderCancel(const OrderCancelData &orderCancel,
                                             const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_debug(LOG_DETAILS, "Handle order cancel...");
  auto iter = m_MatchingEngines.find(orderCancel.GetInstrument());
  if (iter == std::end(m_MatchingEngines)) {
    _log_error(LOG_DETAILS, "Unknown instrument {}", orderCancel.GetInstrument());
    return;
  }
  iter->second->OrderCancel(orderCancel, endpoint);
}

void MatchingEngineModule::GetOrderBook(const std::string &instrument, const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_debug(LOG_DETAILS, "GetOrderBook:{}", instrument);
  auto iter = m_MatchingEngines.find(instrument);
  if (iter == std::end(m_MatchingEngines)) {
    _log_warning(LOG_DETAILS, "GetOrderBook, unknown instrument {}", instrument);
    return;
  }

  iter->second->GetOrderBook(endpoint);
}
