#pragma once
#include "order_data.h"
#include <boost/asio/ip/tcp.hpp>

namespace moboware::modules {

/**
 * @brief OrderHandler interface
 */
class IOrderHandler {
public:
  IOrderHandler() = default;
  virtual ~IOrderHandler() = default;

  virtual void HandleOrderInsert(OrderInsertData &&orderInsert, const boost::asio::ip::tcp::endpoint &endpoint) = 0;

  virtual void HandleOrderAmend(const OrderAmendData &orderInsert, const boost::asio::ip::tcp::endpoint &endpoint) = 0;

  virtual void HandleOrderCancel(const OrderCancelData &orderCancel, const boost::asio::ip::tcp::endpoint &endpoint) = 0;

  virtual void GetOrderBook(const std::string &instrument, const boost::asio::ip::tcp::endpoint &endpoint) = 0;
};
}   // namespace moboware::modules
