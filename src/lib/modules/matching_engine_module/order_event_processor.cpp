#include "modules/matching_engine_module/order_event_processor.h"
#include "common/log_stream.h"

using namespace moboware::modules;
using namespace boost;

OrderEventProcessor::OrderEventProcessor(const std::weak_ptr<IOrderHandler> &orderHandler,
                                         const boost::asio::ip::tcp::endpoint &endpoint)
    : m_OrderHandler(orderHandler)
    , m_Endpoint(endpoint)
{
}

void OrderEventProcessor::Process(const boost::beast::flat_buffer &readBuffer)
{
  json::stream_parser parser;
  system::error_code ec;
  if (0 == parser.write((const char *)readBuffer.data().data(),   //
                        readBuffer.data().size(),                 //
                        ec)) {
    LOG_ERROR("Failed to parse message " << ec);
    return;
  }

  const json::value &rootDocument{parser.release()};
  LOG_DEBUG("Root doc:" << rootDocument);

  const auto &action{rootDocument.at(Fields::Action)};
  const auto &data{rootDocument.at(Fields::Data)};
  LOG_DEBUG(Fields::Action << ":" << action << ":" << data);
  if (action.as_string() == Fields::Insert) {
    HandleOrderInsert(data);
  } else if (action.as_string() == Fields::Cancel) {
    HandleOrderCancel(data);
  } else if (action.as_string() == Fields::Amend) {
    HandleOrderAmend(data);
  } else if (action.as_string() == Fields::GetBook) {
    GetOrderBook(data);
  }
}

void OrderEventProcessor::HandleOrderInsert(const boost::json::value &data)
{
  OrderInsertData orderInsert;

  if (orderInsert.SetData(data)) {   // forward the converted order insert message
    m_OrderHandler.lock()->HandleOrderInsert(std::forward<OrderInsertData>(orderInsert), m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderCancel(const boost::json::value &data)
{
  OrderCancelData orderCancel;

  if (orderCancel.SetData(data)) {
    m_OrderHandler.lock()->HandleOrderCancel(orderCancel, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::HandleOrderAmend(const boost::json::value &data)
{
  OrderAmendData orderAmend;

  if (orderAmend.SetData(data)) {   // forward the converted order amend message
    m_OrderHandler.lock()->HandleOrderAmend(orderAmend, m_Endpoint);
  } else {
    LOG_ERROR("Order data validation failed"
              << " to do add order error reply");
  }
}

void OrderEventProcessor::GetOrderBook(const boost::json::value &data)
{
  const auto instrument{data.at(Fields::Instrument).as_string().c_str()};

  m_OrderHandler.lock()->GetOrderBook(instrument, m_Endpoint);
}