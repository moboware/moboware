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
    HandleOrderCancel(data);
  } else if (action == "Amend") {
    HandleOrderAmend(data);
  } else if (action == "GetBook") {
    GetOrderBook(data);
  }
}

void OrderEventProcessor::HandleOrderInsert(const Json::Value& data)
{
  OrderData orderInsert;

  orderInsert.SetAccount(data["Account"].asString());
  orderInsert.SetPrice(data["Price"].asUInt64());
  orderInsert.SetVolume(data["Volume"].asUInt64());
  orderInsert.SetType(data["Type"].asString());
  orderInsert.SetIsBuySide(data["IsBuy"].asBool());
  orderInsert.SetOrderTime(std::chrono::high_resolution_clock::now());
  orderInsert.SetClientId(data["ClientId"].asString());
  orderInsert.SetInstrument(data["Instrument"].asString());

  std::stringstream strm;
  strm << orderInsert.GetOrderTime().time_since_epoch().count();
  orderInsert.SetId(strm.str());
  if (orderInsert.Validate()) { // forward the converted order insert message
    m_OrderHandler.lock()->HandleOrderInsert(orderInsert, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderCancel(const Json::Value& data)
{
  const auto instrument = data["Instrument"].asString();

  OrderCancelData orderCancel;

  orderCancel.SetInstrument(data["Instrument"].asString());
  orderCancel.SetPrice(data["Price"].asDouble());
  orderCancel.SetIsBuySide(data["IsBuy"].asBool());
  orderCancel.SetId(data["Id"].asString());
  orderCancel.SetClientId(data["ClientId"].asString());
  if (orderCancel.Validate()) {
    m_OrderHandler.lock()->HandleOrderCancel(orderCancel, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderAmend(const Json::Value& data)
{
  OrderAmendData orderAmend;

  orderAmend.SetAccount(data["Account"].asString());
  orderAmend.SetPrice(data["Price"].asUInt64());
  orderAmend.SetNewPrice(data["NewPrice"].asUInt64());
  orderAmend.SetVolume(data["Volume"].asUInt64());
  orderAmend.SetNewVolume(data["NewVolume"].asUInt64());
  orderAmend.SetType(data["Type"].asString());
  orderAmend.SetIsBuySide(data["IsBuy"].asBool());
  orderAmend.SetOrderTime(std::chrono::high_resolution_clock::now());
  orderAmend.SetClientId(data["ClientId"].asString());
  orderAmend.SetInstrument(data["Instrument"].asString());

  std::stringstream strm;
  strm << orderAmend.GetOrderTime().time_since_epoch().count();
  orderAmend.SetId(strm.str());
  if (orderAmend.Validate()) { // forward the converted order amend message
    m_OrderHandler.lock()->HandleOrderAmend(orderAmend, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::GetOrderBook(const Json::Value& data)
{
  const auto instrument = data["Instrument"].asString();

  m_OrderHandler.lock()->GetOrderBook(instrument, m_Endpoint);
}