#pragma once
#include "common/logger.hpp"
#include "common/service.h"
#include "exchange/exchange.hpp"
#include "vwap_calculator.hpp"

namespace moboware::exchange::binance {

class BinanceMarketDataFeedHandler {
public:
  BinanceMarketDataFeedHandler(const moboware::common::ServicePtr &service)
    : m_Service(service)
  {
  }

  void OnSessionConnected(const moboware::exchange::Instrument &instrument)
  {
    m_IsConnected = true;
    LOG_INFO("Feed connected for instrument:{} {}", instrument.exchange, instrument.exchangeSymbol);
  }

  void OnSessionDisconnect(const moboware::exchange::Instrument &instrument)
  {
    m_IsConnected = false;
    LOG_INFO("Feed disconnected from instrument:{} {}", instrument.exchange, instrument.exchangeSymbol);
  }

  void OnTradeTick(const moboware::exchange::Instrument &instrument,   //
                   const moboware::exchange::TradeTick &tradeTick,     //
                   const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = moboware::common::SessionTime_t::now() - sessionTimePoint;
    LOG_INFO("TradeTick, instrument:{}, {}@{}, {}, {}",
             instrument.exchangeSymbol,
             tradeTick.tradePrice,
             tradeTick.tradeVolume,
             tradeTick.tradeTime,
             dtime);

    m_VwapCalculator.OnTradeTick(instrument, tradeTick, sessionTimePoint);
  }

  void OnTopOfTheBook(const moboware::exchange::Instrument &instrument,
                      const moboware::exchange::TopOfTheBook &bbo,
                      const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = moboware::common::SessionTime_t::now() - sessionTimePoint;

    LOG_INFO("BBO, instrument:{}, bid:{}@{} --- {}@{}:ask, session time:{}",
             instrument.symbol,
             bbo.bidPrice,
             bbo.bidVolume,
             bbo.askPrice,
             bbo.askVolume,
             dtime);
  }

  void OnOrderbook(const exchange::Instrument &instrument,
                   const exchange::Orderbook &orderbook,
                   const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = common::SessionTime_t::now() - sessionTimePoint;
    LOG_INFO("Orderbook, instrument:{}, bid:{}@{} --- {}@{}:ask, session time:{}",
             instrument.symbol,
             orderbook.m_Bids[0].price,
             orderbook.m_Bids[0].volume,
             orderbook.m_Asks[0].price,
             orderbook.m_Asks[0].volume,
             dtime);
  }

  [[nodiscard]] inline bool IsConnected() const
  {
    return m_IsConnected;
  }

private:
  moboware::common::ServicePtr m_Service;
  bool m_IsConnected{false};
  VwapCalculator m_VwapCalculator;
};

}   // namespace moboware::exchange::binance