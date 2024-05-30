#pragma once
#include "common/types.hpp"
#include <array>
#include <chrono>
#include <string>
#include <vector>

namespace moboware::exchange {

enum MarketDataStreamType : std::uint8_t {
  NoneStream,
  TradeTickStream = 1,
  BookTickerStream,
  Depth100msStream,
  Depth5LevelsStream,
  Depth10LevelsStream,
  Depth20LevelsStream
};

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
  std::vector<MarketDataStreamType> streamSubscriptions;
};

struct OrderbookLevel {
  double price{};    // price on the price level
  double volume{};   // total volume on the price level
};

struct Orderbook {
  static const std::size_t MaxOrderbookDepth{20u};
  using Bids = std::array<OrderbookLevel, MaxOrderbookDepth>;
  using Asks = std::array<OrderbookLevel, MaxOrderbookDepth>;

  Bids m_Bids;
  std::size_t numberOfBidLevels{};

  Asks m_Asks;
  std::size_t numberOfAskLevels{};
};
}   // namespace moboware::exchange