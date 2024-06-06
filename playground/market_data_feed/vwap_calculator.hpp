#pragma once

#include "common/circular_buffer.hpp"
#include "common/logger.hpp"
#include "exchange/exchange.hpp"

namespace moboware {

class VwapCalculator {
  // vwap calculated over the last x number of trade ticks
public:
  VwapCalculator() = default;
  ~VwapCalculator() = default;

  void OnTradeTick(const moboware::exchange::Instrument &instrument,
                   const moboware::exchange::TradeTick &tradeTick,
                   const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    if (tradeTick.tradePrice > 0.0) {
      m_TradeTicks.Add({tradeTick.tradePrice, tradeTick.tradeVolume});

      // calculate vwap
      double totalVolPrice{};
      double totalVolume{};

      const auto fn{[&](const TradeTickVolPrice &tick) {
        totalVolPrice += tick.price * tick.volume;
        totalVolume += tick.volume;
        return true;
      }};

      m_TradeTicks.Loop(fn);
      const auto vwap{totalVolPrice / totalVolume};

      const auto dtime = moboware::common::SessionTime_t::now() - sessionTimePoint;
      LOG_INFO("Vwap, instrument:{}, {}, {} {}", instrument.exchangeSymbol, vwap, dtime, m_TradeTicks.Size());
    }
  }

private:
  struct TradeTickVolPrice {
    double price{};
    double volume{};
  };

  moboware::common::CircularBuffer<TradeTickVolPrice, 500> m_TradeTicks;
};
}   // namespace moboware