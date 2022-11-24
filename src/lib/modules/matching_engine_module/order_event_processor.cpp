#include "modules/matching_engine_module/order_event_processor.h"
#include "common/log.h"

using namespace moboware::modules;

OrderEventProcessor::OrderEventProcessor(const std::weak_ptr<IOrderHandler>& orderHandler, const boost::asio::ip::tcp::endpoint& endpoint)
  : m_OrderHandler(orderHandler)
  , m_Endpoint(endpoint)
{
}

void OrderEventProcessor::Process(const boost::beast::flat_buffer& readBuffer)
{
  Json::CharReaderBuilder builder{};
  const auto reader = std::unique_ptr<Json::CharReader>(builder.newCharReader());
  Json::Value rootDocument{};
  Json::String* errors{};

  if (not reader->parse((const char*)readBuffer.data().data(),                            //
                        (const char*)readBuffer.data().data() + readBuffer.data().size(), //
                        &rootDocument,
                        errors)) {
    LOG_ERROR("Failed to parse message ");
  };

  const auto action{ rootDocument["Action"].asString() };
  const auto data{ rootDocument["Data"] };
  LOG("Action:" << action << ":" << data);
  if (action == "Insert") {
    HandleOrderInsert(data);
  } else if (action == "Cancel") {
  } else if (action == "Amend") {
  }
}

void OrderEventProcessor::HandleOrderInsert(const Json::Value& data)
{
  OrderData orderInsert;

  orderInsert.account = data["Account"].asString();
  orderInsert.price = data["Price"].asUInt64();
  orderInsert.volume = data["Volume"].asUInt64();
  orderInsert.type = data["Type"].asString();
  orderInsert.IsBuySide = data["IsBuy"].asBool();
  orderInsert.orderTime = std::chrono::high_resolution_clock::now();
  orderInsert.clientId = data["ClientId"].asString();
  orderInsert.instrument = data["Instrument"].asString();

  std::stringstream strm;
  strm << orderInsert.orderTime.time_since_epoch().count();
  orderInsert.id = strm.str();
  if (orderInsert.Validate()) { // forward the converted order insert message
    m_OrderHandler.lock()->HandleOrderInsert(orderInsert, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}
