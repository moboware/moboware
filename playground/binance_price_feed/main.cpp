#include "binance/binance_market_data_feed.hpp"

using namespace moboware;
using namespace moboware::exchange;
using namespace moboware::exchange::binance;

class BinanceMarketDataFeed {
public:
  BinanceMarketDataFeed(const common::ServicePtr &service)
    : m_Service(service)
  {
  }

  void OnSessionDisconnect(const Instrument &instrument)
  {
    m_Service->Stop();   // stop for now!!!
  }

  void OnTradeTick(const Instrument &instrument, const TradeTick &tradeTick, const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = common::SessionTime_t::now() - sessionTimePoint;

    LOG_INFO("TradeTick {}@{}, {}, {}", tradeTick.tradePrice, tradeTick.tradeVolume, tradeTick.tradeTime, dtime);
  }

  void OnTopOfTheBook(const Instrument &instrument, const TopOfTheBook &bbo, const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = common::SessionTime_t::now() - sessionTimePoint;

    LOG_INFO("Best bid/offer, instrument {}, bid:{}@{} --- {}@{}:ask, session time:{}",
             instrument.symbol,
             bbo.bidPrice,
             bbo.bidVolume,
             bbo.askPrice,
             bbo.askVolume,
             dtime);
  }

  void
  OnOrderbook(const Instrument &instrument, const exchange::Orderbook &orderbook, const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = common::SessionTime_t::now() - sessionTimePoint;
    LOG_INFO("Orderbook, instrument {}, bid:{}@{} --- {}@{}:ask, session time:{}",
             instrument.symbol,
             orderbook.m_Bids[0].price,
             orderbook.m_Bids[0].volume,
             orderbook.m_Asks[0].price,
             orderbook.m_Asks[0].volume,
             dtime);
  }

private:
  common::ServicePtr m_Service;
};

using BinanceMarketDataFeed_t = moboware::exchange::binance::BinanceMarketDataSessionHandler<BinanceMarketDataFeed>;

int main(int, char **)
{
  Logger::GetInstance().SetLogFile("./binance_feed.log");

  common::ServicePtr service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    LOG_INFO("Control-C received, stopping application");
    service->Stop();
  });

  const MarketSubscription marketSubscription{
    {"binance", "btcusdt", "btc_usd_bbo"},
    true,
    false,
    true,
    true
  };

  BinanceMarketDataFeed_t binancePriceSessionHandler(service, marketSubscription);
  BinancePriceFeed<BinanceMarketDataFeed_t> binancePriceFeed(service, binancePriceSessionHandler, marketSubscription);

  if (binancePriceFeed.Connect()) {
    service->Run();
  }
  return 0;
}