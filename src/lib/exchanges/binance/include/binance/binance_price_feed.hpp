#pragma once

#include "binance/binance_price_session_handler.hpp"
#include "common/service.h"

namespace moboware::exchanges::binance {

class BinancePriceFeed {
public:
  explicit BinancePriceFeed(const common::ServicePtr &service, BinancePriceSessionHandler &binancePriceHandler);
  ~BinancePriceFeed() = default;
  bool Connect();

private:
  common::ServicePtr m_Service{};
  using WebSocketClient_t = web_socket::WebSocketClient<BinancePriceSessionHandler>;
  WebSocketClient_t m_WebSocketClient;
};
}   // namespace moboware::exchanges::binance