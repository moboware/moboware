#include "binance/binance_market_data_feed.hpp"
#include "binance_market_data_feed_handler.hpp"
#include "bitstamp/bitstamp_market_data_feed.hpp"
#include "bitstamp_market_data_feed_handler.hpp"
#include <variant>

using namespace moboware;
using namespace moboware::common;
using namespace moboware::exchange;
using namespace moboware::exchange::binance;
using namespace moboware::exchange::bitstamp;

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

// bitstamp
using BitstampMarketDataFeedHandler_t = moboware::exchange::bitstamp::BitstampMarketDataSessionHandler<BitstampMarketDataFeedHandler>;
using BitstampExchangeMarketDataFeed_t =
  ExchangeMarketDataFeed<BitstampMarketDataFeed<BitstampMarketDataFeedHandler_t>, BitstampMarketDataFeedHandler_t>;
using BitstampExchangeMarketDataFeedPtr_t = std::unique_ptr<BitstampExchangeMarketDataFeed_t>;

// binance
using BinanceMarketDataFeedHandler_t = moboware::exchange::binance::BinanceMarketDataSessionHandler<BinanceMarketDataFeedHandler>;
using BinanceExchangeMarketDataFeed_t =
  ExchangeMarketDataFeed<BinanceMarketDataFeed<BinanceMarketDataFeedHandler_t>, BinanceMarketDataFeedHandler_t>;
using BinanceExchangeMarketDataFeedPtr_t = std::unique_ptr<BinanceExchangeMarketDataFeed_t>;

int main(int, char **)
{
  Logger::GetInstance().SetLogFile("./market_data_feed.log");

  common::ServicePtr service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    LOG_INFO("Control-C received, stopping application");
    service->Stop();
  });

  std::vector<std::variant<BinanceExchangeMarketDataFeedPtr_t, BitstampExchangeMarketDataFeedPtr_t>> marketFeeds;

  {   // feed for binance::btcusdt
    const MarketSubscription marketSubscription{
  // instrument
      {"binance",                              "btcusdt",                                 "btc_usdt"                           },
 // subscriptions
      {MarketDataStreamType::BookTickerStream, MarketDataStreamType::Depth10LevelsStream, MarketDataStreamType::TradeTickStream}
    };

    marketFeeds.push_back(std::move(std::make_unique<BinanceExchangeMarketDataFeed_t>(service, marketSubscription)));
  }

  {   // feed for binance::ethusdt
    const MarketSubscription marketSubscription{
  // instrument
      {
       "binance",                              //
        "ethbtc",                                                        //
        "eth_btc"    //
      },
 // subscriptions
      {MarketDataStreamType::BookTickerStream, MarketDataStreamType::Depth10LevelsStream, MarketDataStreamType::TradeTickStream}
    };

    marketFeeds.push_back(std::move(std::make_unique<BinanceExchangeMarketDataFeed_t>(service, marketSubscription)));
  }

  {   // feed for bitstamp::btcusd
    const MarketSubscription marketSubscription{
  // instrument
      {
       "bitstamp ", //
        "btcusd",                //
        "btc_usd"      //
      },
 // subscriptions
      {MarketDataStreamType::Depth100LevelsStream,            MarketDataStreamType::TradeTickStream}
    };

    marketFeeds.push_back(std::move(std::make_unique<BitstampExchangeMarketDataFeed_t>(service, marketSubscription)));
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
        } else if (std::holds_alternative<BitstampExchangeMarketDataFeedPtr_t>(marketFeedVariant)) {
          // handle bitstamp feed connection
          auto &marketFeedPtr{std::get<BitstampExchangeMarketDataFeedPtr_t>(marketFeedVariant)};

          if (marketFeedPtr and not marketFeedPtr->GetMarketFeedHandler().IsConnected()) {
            // reconnect
            if (marketFeedPtr->GetMarketFeed().Connect()) {
              LOG_INFO("Connected to bitstamp market feed");
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