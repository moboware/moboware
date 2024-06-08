#pragma once

#include "bitstamp/orderbook_levels_snapshot_parser.hpp"
#include "bitstamp/trade_tick_stream_parser.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>

namespace moboware::exchange::bitstamp {
/**
 * @brief Bitstamp stream parser. Uses the nlohmann sax parser to parse the json
 *        stream received from the bitstamp websocket
 *
 * channel subscriptions: "{"event":"bts:subscription_succeeded","channel":"live_trades_btcusd","data":{}}" *
 * trade tick event :
 *  "{
 *     "data":{"id":343272513,"timestamp":"1717840425","amount":0.00051,"amount_str":"0.00051000","price":69431,"price_str":"69431","type":0,"microtimestamp":"1717840425928000","buy_order_id":1757206328406016,"sell_order_id":1757206107889664},
 *     "channel":"live_trades_btcusd","event":"trade"
 *  }"
 */
class BitstampStreamParser : public nlohmann::json::json_sax_t {
public:
  explicit BitstampStreamParser(const std::string_view &msg);

  virtual ~BitstampStreamParser() = default;

  inline bool null() override
  {
    return true;
  }

  inline bool boolean(bool value) override
  {
    return true;
  }

  inline bool number_integer(number_integer_t val) override
  {
    return true;
  }

  inline bool number_unsigned(number_unsigned_t value) override
  {
    // if (m_StreamType == MarketDataStreamType::TradeTickStream) {
    //   return m_TradeTickStreamParser.HandleTradeTick(m_Key, value);
    // }
    return true;
  }

  inline bool number_float(number_float_t val, const string_t &s) override
  {
    return true;
  }

  inline bool string(string_t &value) override
  {
    if (m_StreamType == MarketDataStreamType::TradeTickStream) {
      return m_TradeTickStreamParser.HandleTradeTick(m_Key, value);
      //    } else if (m_StreamType == MarketDataStreamType::BookTickerStream) {
      //      return m_BookTickerStreamParser.HandleBookTicker(m_Key, value);
      //    } else if (m_StreamType == MarketDataStreamType::Depth5LevelsStream or    //
      //               m_StreamType == MarketDataStreamType::Depth10LevelsStream or   //
      //               m_StreamType == MarketDataStreamType::Depth20LevelsStream) {
      //      return m_OrderbookLevelsParser.HandleOrderBookLevel(m_Key, value);
    }

    return true;
  }

  inline bool start_object(std::size_t elements) override
  {
    return true;
  }

  inline bool end_object() override
  {
    return true;
  }

  inline bool start_array(std::size_t elements) override
  {
    // if (m_StreamType == MarketDataStreamType::Depth5LevelsStream or    //
    //     m_StreamType == MarketDataStreamType::Depth10LevelsStream or   //
    //     m_StreamType == MarketDataStreamType::Depth20LevelsStream) {
    //   m_OrderbookLevelsParser.ArrayStart(m_Key);
    // }
    return true;
  }

  inline bool end_array() override
  {
    // if (m_StreamType == MarketDataStreamType::Depth5LevelsStream or    //
    //     m_StreamType == MarketDataStreamType::Depth10LevelsStream or   //
    //     m_StreamType == MarketDataStreamType::Depth20LevelsStream) {
    //   m_OrderbookLevelsParser.ArrayEnd(m_Key);
    //}

    return true;
  }

  inline bool key(string_t &value) override
  {
    m_Key = std::move(value);
    return true;
  }

  inline bool binary(nlohmann::json::binary_t &val) override
  {
    return true;
  }

  inline bool parse_error(std::size_t position, const std::string &last_token, const nlohmann::json::exception &ex) override
  {
    return false;
  }

  inline const exchange::TradeTick &GetTradeTick() const
  {
    return m_TradeTickStreamParser.GetTradeTick();
  }

  inline const exchange::Orderbook &GetOrderbook() const
  {
    return m_OrderbookLevelsParser.GetOrderbook();
  }

  inline MarketDataStreamType GetStreamType() const
  {
    return m_StreamType;
  }

private:
  std::string m_Key;

  TradeTickStreamParser m_TradeTickStreamParser;
  // BookTickerStreamParser m_BookTickerStreamParser;
  OrderbookLevelsParser m_OrderbookLevelsParser;

  MarketDataStreamType m_StreamType{NoneStream};
};
}   // namespace moboware::exchange::bitstamp