#include "binance/binance_market_data_feed.hpp"
#include "common/circular_buffer.hpp"
#include <variant>

using namespace moboware;
using namespace moboware::common;
using namespace moboware::exchange;
using namespace moboware::exchange::binance;

class VwapCalculator {
  // vwap calculated over the last x number of trade ticks
public:
  explicit VwapCalculator(const std::size_t tradeTickDepth = 100)
    : m_TradeTickDepth(tradeTickDepth)
  {
  }

  void OnTradeTick(const Instrument &instrument, const TradeTick &tradeTick, const moboware::common::SessionTimePoint_t &sessionTimePoint)
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

      const auto dtime = common::SessionTime_t::now() - sessionTimePoint;
      LOG_INFO("instrument {}, Vwap {}, {} {}", instrument.exchangeSymbol, vwap, dtime, m_TradeTicks.Size());
    }
  }

private:
  std::size_t m_TradeTickDepth{};

  struct TradeTickVolPrice {
    double price{};
    double volume{};
  };

  CircularBuffer<TradeTickVolPrice, 100> m_TradeTicks;
};

class BinanceMarketDataFeedHandler {
public:
  BinanceMarketDataFeedHandler(const common::ServicePtr &service)
    : m_Service(service)
  {
  }

  void OnSessionConnected(const Instrument &instrument)
  {
    m_IsConnected = true;
    LOG_INFO("Feed connected for instrument: {} {}", instrument.exchange, instrument.exchangeSymbol);
  }

  void OnSessionDisconnect(const Instrument &instrument)
  {
    m_IsConnected = false;
    LOG_INFO("Feed disconnected from instrument: {} {}", instrument.exchange, instrument.exchangeSymbol);
  }

  void OnTradeTick(const Instrument &instrument, const TradeTick &tradeTick, const moboware::common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto dtime = common::SessionTime_t::now() - sessionTimePoint;
    LOG_INFO("instrument:{}, TradeTick {}@{}, {}, {}",
             instrument.exchangeSymbol,
             tradeTick.tradePrice,
             tradeTick.tradeVolume,
             tradeTick.tradeTime,
             dtime);

    m_VwapCalculator.OnTradeTick(instrument, tradeTick, sessionTimePoint);
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

  [[nodiscard]] inline bool IsConnected() const
  {
    return m_IsConnected;
  }

private:
  common::ServicePtr m_Service;
  bool m_IsConnected{false};
  VwapCalculator m_VwapCalculator;
};

/**
 * @brief Exchange market data feed, class to contain 3 part of the market data interface.
 *  - the market subscriptions for an exchange
 *  - the market feed handler, which holds  the callback implementation
 *  - the market feed it self to setup the connection to the exchange market data streams
 * This container class can be stored into a std container and holds a data stream for one instrument in general
 * @tparam TMarketFeed
 * @tparam TMarketFeedHandler
 */
template <typename TMarketFeed, typename TMarketFeedHandler>   //
class ExchangeMarketDataFeed {
public:
  ExchangeMarketDataFeed(const common::ServicePtr &service, const MarketSubscription &marketSubscription)
    : m_Service(service)
    , m_MarketSubscription(marketSubscription)
    , m_MarketFeedHandler(service, marketSubscription)
    , m_MarketFeed(service, m_MarketFeedHandler, marketSubscription)
  {
  }

  TMarketFeedHandler &GetMarketFeedHandler()
  {
    return m_MarketFeedHandler;
  }

  TMarketFeed &GetMarketFeed()
  {
    return m_MarketFeed;
  }

private:
  const common::ServicePtr m_Service;
  const MarketSubscription m_MarketSubscription;

  TMarketFeedHandler m_MarketFeedHandler;
  TMarketFeed m_MarketFeed;
};

using BinanceMarketDataFeedHandler_t = moboware::exchange::binance::BinanceMarketDataSessionHandler<BinanceMarketDataFeedHandler>;
using BinanceExchangeMarketDataFeed_t =
  ExchangeMarketDataFeed<BinanceMarketDataFeed<BinanceMarketDataFeedHandler_t>, BinanceMarketDataFeedHandler_t>;
using BinanceExchangeMarketDataFeedPtr_t = std::unique_ptr<BinanceExchangeMarketDataFeed_t>;

int main(int, char **)
{
  Logger::GetInstance().SetLogFile("./binance_feed.log");

  common::ServicePtr service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    LOG_INFO("Control-C received, stopping application");
    service->Stop();
  });

  std::vector<std::variant<BinanceExchangeMarketDataFeedPtr_t>> marketFeeds;

  {   // feed for btcusdt
    const MarketSubscription marketSubscription{
  // instrument
      {"binance",                              "btcusdt",                                 "btc_usdt"                           },
 // subscriptions
      {MarketDataStreamType::BookTickerStream, MarketDataStreamType::Depth10LevelsStream, MarketDataStreamType::TradeTickStream}
    };

    marketFeeds.push_back(std::move(std::make_unique<BinanceExchangeMarketDataFeed_t>(service, marketSubscription)));
  }

  {   // feed for ethusdt
    const MarketSubscription marketSubscription{
  // instrument
      {"binance",                              "ethbtc",                                  "eth_btc"                            },
 // subscriptions
      {MarketDataStreamType::BookTickerStream, MarketDataStreamType::Depth10LevelsStream, MarketDataStreamType::TradeTickStream}
    };

    marketFeeds.push_back(std::move(std::make_unique<BinanceExchangeMarketDataFeed_t>(service, marketSubscription)));
  }

  common::Timer connectionTimer(service);
  connectionTimer.Start(
    [&](common::Timer &timer) {
      for (auto &marketFeedVariant : marketFeeds) {

        if (std::holds_alternative<BinanceExchangeMarketDataFeedPtr_t>(marketFeedVariant)) {
          // handle binance feed connection
          auto &marketFeedPtr{std::get<BinanceExchangeMarketDataFeedPtr_t>(marketFeedVariant)};

          if (marketFeedPtr and not marketFeedPtr->GetMarketFeedHandler().IsConnected()) {
            // reconnect
            if (marketFeedPtr->GetMarketFeed().Connect()) {
              LOG_INFO("Connected to binance market feed");
            }
          }
        }
      }
      timer.Restart();
    },
    std::chrono::seconds(1));

  service->Run();

  return 0;
}