#include "modules/matching_engine_module/order_event_processor.h"
#include "common/log_stream.h"

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

  const auto action{ rootDocument[Fields::Action].asString() };
  const auto data{ rootDocument[Fields::Data] };
  LOG_DEBUG(Fields::Action << ":" << action << ":" << data);
  if (action == Fields::Insert) {
    HandleOrderInsert(data);
  } else if (action == Fields::Cancel) {
    HandleOrderCancel(data);
  } else if (action == Fields::Amend) {
    HandleOrderAmend(data);
  } else if (action == Fields::GetBook) {
    GetOrderBook(data);
  }
}

void OrderEventProcessor::HandleOrderInsert(const Json::Value& data)
{
  OrderInsertData orderInsert;

  if (orderInsert.SetData(data)) { // forward the converted order insert message
    m_OrderHandler.lock()->HandleOrderInsert(orderInsert, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderCancel(const Json::Value& data)
{

  OrderCancelData orderCancel;

  if (orderCancel.SetData(data)) {
    m_OrderHandler.lock()->HandleOrderCancel(orderCancel, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderAmend(const Json::Value& data)
{
  OrderAmendData orderAmend;

  if (orderAmend.SetData(data)) { // forward the converted order amend message
    m_OrderHandler.lock()->HandleOrderAmend(orderAmend, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::GetOrderBook(const Json::Value& data)
{
  const auto instrument{ data[Fields::Instrument].asString() };

  m_OrderHandler.lock()->GetOrderBook(instrument, m_Endpoint);
}