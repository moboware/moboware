#pragma once

#include "common/service.h"
#include "exchange/exchange.hpp"
#include "exchange/market_data_feed.hpp"

namespace moboware::exchange {

class MarketDataFeed {
public:
  virtual bool Connect() = 0;

protected:
  explicit MarketDataFeed(const common::ServicePtr &service, const MarketSubscription &marketSubscription)
    : m_Service(service)
    , m_MarketSubscription(marketSubscription)
  {
  }

  virtual ~MarketDataFeed() = default;

  common::ServicePtr m_Service{};
  const MarketSubscription m_MarketSubscription;
};
}   // namespace moboware::exchange