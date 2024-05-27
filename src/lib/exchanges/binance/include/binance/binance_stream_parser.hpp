#pragma once
#include "binance/book_ticker_stream_parser.hpp"
#include "binance/orderbook_levels_snapshot_parser.hpp"
#include "binance/trade_tick_stream_parser.hpp"
#include "common/logger.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>

namespace moboware::exchange::binance {

/**
 * @brief Binance stream parser. Uses the nlohmann sax parser to parse the json stream received from the binance websocket
 */
class BinanceStreamHandler : public nlohmann::json::json_sax_t {
public:
  enum StreamType : std::uint8_t {
    NoneStream,
    TradeTickStream = 1,
    BookTickerStream,
    Depth100msStream,
    Depth5LevelsStream,
    Depth10LevelsStream,
    Depth20LevelsStream
  };

  BinanceStreamHandler();
  virtual ~BinanceStreamHandler() = default;

  inline void Clear()
  {
    m_StreamType = NoneStream;

    m_OrderbookLevelsParser.Clear();
  }

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
    if (m_StreamType == TradeTickStream) {
      return m_TradeTickStreamParser.HandleTradeTick(m_Key, value);
    }
    return true;
  }

  inline bool number_float(number_float_t val, const string_t &s) override
  {
    return true;
  }

  inline bool string(string_t &value) override
  {
    if (m_StreamType == NoneStream and m_Key == "stream") {   // first initialization
      if (value.find("@trade") != std::string::npos) {
        m_StreamType = TradeTickStream;
      } else if (value.find("@bookTicker") != std::string::npos) {
        m_StreamType = BookTickerStream;
      } else if (value.find("@depth@100ms") != std::string::npos) {
        m_StreamType = Depth100msStream;
      } else if (value.find("@depth5@100ms") != std::string::npos) {
        m_StreamType = Depth5LevelsStream;
      } else if (value.find("@depth10@100ms") != std::string::npos) {
        m_StreamType = Depth10LevelsStream;
      } else if (value.find("@depth20@100ms") != std::string::npos) {
        m_StreamType = Depth20LevelsStream;
      }
    }

    else if (m_StreamType == TradeTickStream) {
      return m_TradeTickStreamParser.HandleTradeTick(m_Key, value);
    } else if (m_StreamType == BookTickerStream) {
      return m_BookTickerStreamParser.HandleBookTicker(m_Key, value);
    } else if (m_StreamType == Depth5LevelsStream or    //
               m_StreamType == Depth10LevelsStream or   //
               m_StreamType == Depth20LevelsStream) {
      return m_OrderbookLevelsParser.HandleOrderBookLevel(m_Key, value);
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
    if (m_StreamType == Depth5LevelsStream or    //
        m_StreamType == Depth10LevelsStream or   //
        m_StreamType == Depth20LevelsStream) {
      m_OrderbookLevelsParser.ArrayStart(m_Key);
    }
    return true;
  }

  inline bool end_array() override
  {
    if (m_StreamType == Depth5LevelsStream or    //
        m_StreamType == Depth10LevelsStream or   //
        m_StreamType == Depth20LevelsStream) {
      m_OrderbookLevelsParser.ArrayEnd(m_Key);
    }
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

  inline const exchange::TopOfTheBook &GetBestBidOffer() const
  {
    return m_BookTickerStreamParser.GetBestBidOffer();
  }

  inline const exchange::Orderbook &GetOrderbook() const
  {
    return m_OrderbookLevelsParser.GetOrderbook();
  }

  inline StreamType GetStreamType() const
  {
    return m_StreamType;
  }

private:
  std::string m_Key;

  TradeTickStreamParser m_TradeTickStreamParser;
  BookTickerStreamParser m_BookTickerStreamParser;
  OrderbookLevelsParser m_OrderbookLevelsParser;

  StreamType m_StreamType{NoneStream};
};

}   // namespace moboware::exchange::binance