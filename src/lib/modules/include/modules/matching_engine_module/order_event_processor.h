#pragma once

#include "modules/matching_engine_module/i_order_handler.h"
#include "modules/matching_engine_module/order_data.h"
#include <boost/asio/ip/tcp.hpp>
#include <boost/beast/core/flat_buffer.hpp>
#include <json/json.h>

namespace moboware::modules {

/**
 * @brief Event processor class. Converts json message in to events/calls to the matching engine.
 */
class OrderEventProcessor
{
public:
  explicit OrderEventProcessor(const std::weak_ptr<IOrderHandler>& orderHandler, const boost::asio::ip::tcp::endpoint& endpoint);
  OrderEventProcessor(const OrderEventProcessor&) = delete;
  OrderEventProcessor(OrderEventProcessor&&) = delete;
  OrderEventProcessor& operator=(const OrderEventProcessor&) = delete;
  OrderEventProcessor& operator=(OrderEventProcessor&&) = delete;
  ~OrderEventProcessor() = default;

  void Process(const boost::beast::flat_buffer& readBuffer);

private:
  const std::weak_ptr<IOrderHandler> m_OrderHandler;
  void HandleOrderInsert(const Json::Value& data);
  void HandleOrderCancel(const Json::Value& data);
  void HandleOrderAmend(const Json::Value& data);
  void GetOrderBook(const Json::Value& data);

  const boost::asio::ip::tcp::endpoint m_Endpoint;
};

}