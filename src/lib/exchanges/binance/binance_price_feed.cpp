#include "binance/binance_price_feed.hpp"
#include "common/logger.hpp"

using namespace moboware::exchanges;
using namespace moboware::exchanges::binance;

BinancePriceFeed::BinancePriceFeed(const common::ServicePtr &service, BinancePriceSessionHandler &binancePriceHandler)
    : m_Service(service)
    , m_WebSocketClient(service, binancePriceHandler)
{
}

bool BinancePriceFeed::Connect()
{
  // make subscription list of the stream that we are interested in
  m_WebSocketClient.SetTarget("/stream?streams=btcusdt@bookTicker/btcusdt@trade");
  // m_WebSocketClient.SetTarget("/stream?streams=btcusdt@depth@100ms/btcusdt@bookTicker");

  if (m_WebSocketClient.Start("stream.binance.com", 9443)) {
    return true;
  }
  return false;
}
