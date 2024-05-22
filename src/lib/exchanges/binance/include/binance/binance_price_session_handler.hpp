#pragma once

#include "common/service.h"
#include "socket/socket_session_base.hpp"
#include "socket/web_socket_client.hpp"
#include <rapidjson/document.h>
#include <string>

namespace moboware::exchanges::binance {
class BinancePriceSessionHandler {
public:
  BinancePriceSessionHandler() = default;
  ~BinancePriceSessionHandler() = default;
  BinancePriceSessionHandler(const BinancePriceSessionHandler &) = delete;
  BinancePriceSessionHandler(BinancePriceSessionHandler &&) = delete;
  BinancePriceSessionHandler &operator=(const BinancePriceSessionHandler &) = delete;
  BinancePriceSessionHandler &operator=(BinancePriceSessionHandler &&) = delete;

  void OnDataRead(const boost::beast::flat_buffer &readBuffer,
                  const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                  const common::SystemTimePoint_t &sessionTimePoint);

  void OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint);

  // called when the session is closed and the session can be cleaned up.
  void OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint);

private:
  rapidjson::Document m_Document;
};
}   // namespace moboware::exchanges::binance