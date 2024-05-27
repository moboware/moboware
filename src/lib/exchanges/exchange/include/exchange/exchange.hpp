#pragma once
#include "common/types.hpp"
#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace moboware::exchange {

struct Instrument {
  std::string exchange{};
  std::string exchangeSymbol{};
  std::string symbol{};
};

struct TopOfTheBook {   // bbo feed
  double bidPrice{};
  double bidVolume{};
  double askPrice{};
  double askVolume{};
};

struct TradeTick {
  std::string tradeId;
  double tradePrice{};
  double tradeVolume{};
  common::SystemTimePoint_t tradeTime;
};

struct MarketSubscription {
  Instrument instrument;

  bool bboFeed{false};               // bbo feed
  bool orderBookDepthFeed{false};    // order book depth diff snapshot
  bool orderBook5DepthFeed{false};   // order book 5 levels depth snapshot
  bool tradeTickFeed{false};         // trade tick feed
};

struct OrderbookLevel {
  double price{};    // price on the price level
  double volume{};   // total volume on the price level
};

struct Orderbook {
  using Bids = std::array<OrderbookLevel, 20u>;
  using Asks = std::array<OrderbookLevel, 20u>;

  Bids m_Bids;
  std::size_t numberOfBidLevels{};

  Asks m_Asks;
  std::size_t numberOfAskLevels{};
};
}   // namespace moboware::exchange