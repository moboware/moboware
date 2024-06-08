#pragma once

#include "bitstamp/bitstamp_stream_parser.hpp"
#include "common/service.h"
#include "exchange/exchange.hpp"
#include "socket/socket_session_base.hpp"
#include "socket/web_socket_client.hpp"

namespace moboware::exchange::bitstamp {

/**
 * @brief handles a bitstamp market stream
 */
template <typename TDataHandler>   //
class BitstampMarketDataSessionHandler : public TDataHandler {
public:
  explicit BitstampMarketDataSessionHandler(const common::ServicePtr &service, const MarketSubscription &instrument);
  ~BitstampMarketDataSessionHandler() = default;
  BitstampMarketDataSessionHandler(const BitstampMarketDataSessionHandler &) = delete;
  BitstampMarketDataSessionHandler(BitstampMarketDataSessionHandler &&) = delete;
  BitstampMarketDataSessionHandler &operator=(const BitstampMarketDataSessionHandler &) = delete;
  BitstampMarketDataSessionHandler &operator=(BitstampMarketDataSessionHandler &&) = delete;

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
BitstampMarketDataSessionHandler<TDataHandler>::BitstampMarketDataSessionHandler(const common::ServicePtr &service,
                                                                                 const MarketSubscription &marketSubscription)
  : TDataHandler(service)
  , m_MarketSubscription(marketSubscription)
{
}

// handle bitstamp feed messages:
// @bookTicker
// @trade
template <typename TDataHandler>   //
void BitstampMarketDataSessionHandler<TDataHandler>::OnDataRead(const boost::beast::flat_buffer &readBuffer,
                                                                const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                                                                const common::SessionTimePoint_t &sessionTimePoint)
{
  const std::string_view msg{(const char *)readBuffer.data().data(), readBuffer.data().size()};

  BitstampStreamParser bitstampStreamParser(msg);

  // parse JSON with sax parser
  bool result{nlohmann::json::sax_parse(msg.begin(), msg.end(), &bitstampStreamParser, nlohmann::json::input_format_t::json, false, true)};

  switch (bitstampStreamParser.GetStreamType()) {
  case MarketDataStreamType::TradeTickStream:
    TDataHandler::OnTradeTick(m_MarketSubscription.instrument, bitstampStreamParser.GetTradeTick(), sessionTimePoint);
    break;
  case MarketDataStreamType::Depth100LevelsStream:
    TDataHandler::OnOrderbook(m_MarketSubscription.instrument, bitstampStreamParser.GetOrderbook(), sessionTimePoint);
    break;
  }
}

template <typename TDataHandler>   //
void BitstampMarketDataSessionHandler<TDataHandler>::OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint)
{
  LOG_INFO("Session connected to {}:{}", endpoint.address().to_string(), endpoint.port());

  TDataHandler::OnSessionConnected(m_MarketSubscription.instrument);
}

template <typename TDataHandler>   //
void BitstampMarketDataSessionHandler<TDataHandler>::OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
{
  LOG_INFO("Session closed from {}:{}", endpoint.address().to_string(), endpoint.port());

  TDataHandler::OnSessionDisconnect(m_MarketSubscription.instrument);
}
}   // namespace moboware::exchange::bitstamp