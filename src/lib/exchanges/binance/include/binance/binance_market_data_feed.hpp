#pragma once

#include "binance/binance_market_data_session_handler.hpp"
#include "common/service.h"
#include "exchange/exchange.hpp"

namespace moboware::exchange::binance {

template <typename TMarketFeedSessionHandler>   //
class BinancePriceFeed {
public:
  explicit BinancePriceFeed(const common::ServicePtr &service,
                            TMarketFeedSessionHandler &binancePriceHandler,
                            const MarketSubscription &marketSubscription);
  ~BinancePriceFeed() = default;

  bool Connect();

private:
  common::ServicePtr m_Service{};

  using WebSocketClient_t = web_socket::WebSocketClient<TMarketFeedSessionHandler>;
  WebSocketClient_t m_WebSocketClient;
  const MarketSubscription &m_MarketSubscription;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TMarketFeedSessionHandler>   //
BinancePriceFeed<TMarketFeedSessionHandler>::BinancePriceFeed(const common::ServicePtr &service,
                                                              TMarketFeedSessionHandler &binancePriceHandler,
                                                              const MarketSubscription &marketSubscription)
  : m_Service(service)
  , m_WebSocketClient(service, binancePriceHandler)
  , m_MarketSubscription(marketSubscription)

{
}

template <typename TMarketFeedSessionHandler>   //
bool BinancePriceFeed<TMarketFeedSessionHandler>::Connect()
{
  // make subscription list of the stream that we are interested in
  //"/stream?streams=btcusdt@bookTicker/btcusdt@trade/btcusdt@depth@100ms"
  std::vector<std::string> feeds;
  if (m_MarketSubscription.bboFeed) {
    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@bookTicker");
  }

  if (m_MarketSubscription.orderBookDepthFeed) {
    // orderbook depth snapshot update period 100ms
    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth@100ms");
  }

  if (m_MarketSubscription.orderBook5DepthFeed) {
    // orderbook depth snapshot update period 100ms 5 levels deep
    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth5@100ms");
  }

  if (m_MarketSubscription.tradeTickFeed) {
    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@trade");
  }

  std::string stream{"/stream?streams="};
  for (std::size_t i = 0; i < feeds.size(); i++) {
    stream += feeds[i];
    if (i < feeds.size() - 1) {
      stream += "/";
    }
  }
  LOG_INFO("subscription:{}", stream);

  m_WebSocketClient.SetTarget(stream);

  return m_WebSocketClient.Start("stream.binance.com", 9443);
}

}   // namespace moboware::exchange::binance