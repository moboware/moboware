#pragma once

#include "bitstamp/bitstamp_market_data_session_handler.hpp"
#include "common/service.h"
#include "exchange/exchange.hpp"
#include "exchange/market_data_feed.hpp"

namespace moboware::exchange::bitstamp {

/**
 * @brief Main class for initializing a market data feed to the bitstamp exchange.
 *        This class needs, next to the service and market subscriptions als a callback handler instance.
 *        All classes used, are templated based and do not use any virtual methods in the data path to keep the latency to a minimum
 *  The bitstamp market data feeds currently implement:
 *  - realtime ticker feed
 *  - orderbook snapshot of 100 levels depth
 * There is no full order book, because this needs a http rest recovery interface and that is due to the latency delay not implemented.
 *  All feeds need only the websocket interface of bitstamp.
 * @tparam TMarketFeedSessionHandler
 */
template <typename TMarketFeedSessionHandler>   //
class BitstampMarketDataFeed : public MarketDataFeed {
public:
  explicit BitstampMarketDataFeed(const common::ServicePtr &service,
                                  TMarketFeedSessionHandler &binancePriceHandler,
                                  const MarketSubscription &marketSubscription);
  virtual ~BitstampMarketDataFeed() = default;

  bool Connect() override;

private:
  using WebSocketClient_t = web_socket::WebSocketClient<TMarketFeedSessionHandler>;
  WebSocketClient_t m_WebSocketClient;
};

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////
template <typename TMarketFeedSessionHandler>   //
BitstampMarketDataFeed<TMarketFeedSessionHandler>::BitstampMarketDataFeed(const common::ServicePtr &service,
                                                                          TMarketFeedSessionHandler &binancePriceHandler,
                                                                          const MarketSubscription &marketSubscription)
  : MarketDataFeed(service, marketSubscription)
  , m_WebSocketClient(service, binancePriceHandler)
{
}

template <typename TMarketFeedSessionHandler>   //
bool BitstampMarketDataFeed<TMarketFeedSessionHandler>::Connect()
{
  // make subscription list of the stream that we are interested in
  //"/stream?streams=btcusdt@bookTicker/btcusdt@trade/btcusdt@depth@100ms"
  // std::vector<std::string> feeds;
  // for (const auto streamSubscription : m_MarketSubscription.streamSubscriptions) {
  //  switch (streamSubscription) {
  //  case MarketDataStreamType::BookTickerStream:
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@bookTicker");
  //    break;
  //  case MarketDataStreamType::TradeTickStream:
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@trade");
  //    break;
  //  case MarketDataStreamType::Depth100msStream:
  //    // orderbook depth snapshot update period 100ms
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth@100ms");
  //    break;
  //  case MarketDataStreamType::Depth5LevelsStream:
  //    // orderbook depth snapshot update period 100ms 5 levels deep
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth5@100ms");
  //    break;
  //  case MarketDataStreamType::Depth10LevelsStream:
  //    // orderbook depth snapshot update period 100ms 10 levels deep
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth10@100ms");
  //    break;
  //  case MarketDataStreamType::Depth20LevelsStream:
  //    // orderbook depth snapshot update period 100ms 20 levels deep
  //    feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth20@100ms");
  //    break;
  //  }
  //}
  //
  std::string stream{"/stream?streams="};
  // for (std::size_t i = 0; i < feeds.size(); i++) {
  //  stream += feeds[i];
  //  if (i < feeds.size() - 1) {
  //    stream += "/";
  //  }
  //}
  LOG_INFO("subscription:{}", stream);

  m_WebSocketClient.SetTarget(stream);

  return m_WebSocketClient.Start("ws.bitstamp.net", 9443);
}

}   // namespace moboware::exchange::bitstamp