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
  bool SubscribeLiveTradeFeed(const exchange::Instrument &instrument);

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
  LOG_INFO("Connecting to Bitstamp");

  if (m_WebSocketClient.Start("ws.bitstamp.net", 9443)) {
    // subscribe to channels
    LOG_INFO("Connected to Bitstamp!");

    for (const auto streamSubscription : m_MarketSubscription.streamSubscriptions) {
      switch (streamSubscription) {
      case MarketDataStreamType::TradeTickStream:

        if (not SubscribeLiveTradeFeed(m_MarketSubscription.instrument)) {
          break;
        }
        break;
      case MarketDataStreamType::Depth100LevelsStream:
        // orderbook depth snapshot update period 100ms 20 levels deep
        // feeds.push_back(m_MarketSubscription.instrument.exchangeSymbol + "@depth20@100ms");
        break;
      }
    }
    return true;
  }
  return false;
}

template <typename TMarketFeedSessionHandler>   //
bool BitstampMarketDataFeed<TMarketFeedSessionHandler>::SubscribeLiveTradeFeed(const exchange::Instrument &instrument)
{
  LOG_INFO("subscription to live trade feed:{}::{}", instrument.exchange, instrument.exchangeSymbol);

  std::stringstream ss;
  // exchange symbol should be in lower case
  ss << "{\"event\":\"bts:subscribe\",\"data\":{\"channel\":\"live_trades_" << instrument.exchangeSymbol << "\"}}";

  return m_WebSocketClient.SendWebSocketData({ss.view().data(), ss.view().size()});
}

}   // namespace moboware::exchange::bitstamp