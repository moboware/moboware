#pragma once

#include "binance/binance_stream_parser.hpp"
#include "common/service.h"
#include "exchange/exchange.hpp"
#include "socket/socket_session_base.hpp"
#include "socket/web_socket_client.hpp"
#include <chrono>
#include <stdlib.h>
#include <string>

namespace moboware::exchange::binance {

/**
 * @brief handles a binance market stream for one instrument
 */
template <typename TDataHandler>   //
class BinanceMarketDataSessionHandler : public TDataHandler {
public:
  explicit BinanceMarketDataSessionHandler(const common::ServicePtr &service, const MarketSubscription &instrument);
  ~BinanceMarketDataSessionHandler() = default;
  BinanceMarketDataSessionHandler(const BinanceMarketDataSessionHandler &) = delete;
  BinanceMarketDataSessionHandler(BinanceMarketDataSessionHandler &&) = delete;
  BinanceMarketDataSessionHandler &operator=(const BinanceMarketDataSessionHandler &) = delete;
  BinanceMarketDataSessionHandler &operator=(BinanceMarketDataSessionHandler &&) = delete;

  void OnDataRead(const boost::beast::flat_buffer &readBuffer,
                  const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                  const common::SessionTimePoint_t &sessionTimePoint);

  void OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint);

  // called when the session is closed and the session can be cleaned up.
  void OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint);

private:
  const MarketSubscription m_MarketSubscription;
};

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

template <typename TDataHandler>   //
BinanceMarketDataSessionHandler<TDataHandler>::BinanceMarketDataSessionHandler(const common::ServicePtr &service,
                                                                               const MarketSubscription &marketSubscription)
  : TDataHandler(service)
  , m_MarketSubscription(marketSubscription)
{
}

// handle binance feed messages:
// @bookTicker
// @trade
template <typename TDataHandler>   //
void BinanceMarketDataSessionHandler<TDataHandler>::OnDataRead(const boost::beast::flat_buffer &readBuffer,
                                                               const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                                                               const common::SessionTimePoint_t &sessionTimePoint)
{
  const std::string_view msg{(const char *)readBuffer.data().data(), readBuffer.data().size()};

  BinanceStreamParser binanceStreamParser;

  // parse JSON with sax parser
  bool result{nlohmann::json::sax_parse(msg.begin(), msg.end(), &binanceStreamParser, nlohmann::json::input_format_t::json, false, true)};

  switch (binanceStreamParser.GetStreamType()) {
  case MarketDataStreamType::TradeTickStream:
    TDataHandler::OnTradeTick(m_MarketSubscription.instrument, binanceStreamParser.GetTradeTick(), sessionTimePoint);
    break;
  case MarketDataStreamType::BookTickerStream:
    TDataHandler::OnTopOfTheBook(m_MarketSubscription.instrument, binanceStreamParser.GetBestBidOffer(), sessionTimePoint);
    break;
  case MarketDataStreamType::Depth100msStream:
    LOG_INFO("{}", msg);
    break;
  case MarketDataStreamType::Depth5LevelsStream:
  case MarketDataStreamType::Depth10LevelsStream:
  case MarketDataStreamType::Depth20LevelsStream:
    TDataHandler::OnOrderbook(m_MarketSubscription.instrument, binanceStreamParser.GetOrderbook(), sessionTimePoint);
    break;
  }
}

template <typename TDataHandler>   //
void BinanceMarketDataSessionHandler<TDataHandler>::OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint)
{
  LOG_INFO("Session connected to {}:{}", endpoint.address().to_string(), endpoint.port());

  TDataHandler::OnSessionConnected(m_MarketSubscription.instrument);
}

template <typename TDataHandler>   //
void BinanceMarketDataSessionHandler<TDataHandler>::OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
{
  LOG_INFO("Session closed from {}:{}", endpoint.address().to_string(), endpoint.port());

  TDataHandler::OnSessionDisconnect(m_MarketSubscription.instrument);
}
}   // namespace moboware::exchange::binance