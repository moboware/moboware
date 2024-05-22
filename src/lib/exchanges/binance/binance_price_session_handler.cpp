#include "binance/binance_price_session_handler.hpp"
#include "common/logger.hpp"

using namespace moboware::exchanges;
using namespace moboware::exchanges::binance;

void BinancePriceSessionHandler::OnDataRead(const boost::beast::flat_buffer &readBuffer,
                                            const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                                            const common::SystemTimePoint_t &sessionTimePoint)
{
  const std::string_view msg{(const char *)readBuffer.data().data(), readBuffer.data().size()};

  m_Document.Clear();
  m_Document.Parse(msg.data(), msg.size());

  if (m_Document.HasMember("stream") && m_Document.HasMember("data")) {

    _log_info(LOG_DETAILS, "Received data {}", msg);
  }
}

void BinancePriceSessionHandler::OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_info(LOG_DETAILS, "Session connected to {}:{}", endpoint.address().to_string(), endpoint.port());
}

void BinancePriceSessionHandler::OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
{
  _log_info(LOG_DETAILS, "Session closed from {}:{}", endpoint.address().to_string(), endpoint.port());
  // todo let application handle the disconnect!
}