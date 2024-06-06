#pragma once

#include "common/types.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace moboware::exchange::bitstamp {

//
// parse the bitstamp orderbook depth feed that provides the 100 levels orderbook
//
class OrderbookLevelsParser {
public:
  OrderbookLevelsParser()
  {
    m_Orderbook.Init(100u);   // max 100 levels deep
  }

  ~OrderbookLevelsParser() = default;

  inline const exchange::Orderbook &GetOrderbook() const
  {
    return m_Orderbook;
  }

  [[nodiscard]] inline bool HandleOrderBookLevel(const std::string &key, std::string &value)
  {
    if (m_ArrayIndentation == 2) {
      m_PriceVolumeFieldsIndex++;

      char **endptr{};
      if (key == "bids") {
        if (m_PriceVolumeFieldsIndex % 2 == 1) {
          m_Orderbook.m_Bids[m_Orderbook.numberOfBidLevels].price = strtod(value.c_str(), endptr);
        } else if (m_PriceVolumeFieldsIndex % 2 == 0) {
          m_Orderbook.m_Bids[m_Orderbook.numberOfBidLevels++].volume = strtod(value.c_str(), endptr);
        }
      } else if (key == "asks") {
        if (m_PriceVolumeFieldsIndex % 2 == 1) {
          m_Orderbook.m_Asks[m_Orderbook.numberOfAskLevels].price = strtod(value.c_str(), endptr);
        } else if (m_PriceVolumeFieldsIndex % 2 == 0) {
          m_Orderbook.m_Asks[m_Orderbook.numberOfAskLevels++].volume = strtod(value.c_str(), endptr);
        }
      }
    }
    return true;
  }

  inline void ArrayStart(const std::string &key)
  {
    if (key == "bids" or key == "asks") {
      m_ArrayIndentation++;
    }
  }

  inline void ArrayEnd(const std::string &key)
  {
    if (key == "bids" or key == "asks") {
      m_ArrayIndentation--;
    }
  }

private:
  exchange::Orderbook m_Orderbook;
  // Identifies the indentation level of the bid/ask array.
  // When indentation == 1 we are in the bids or asks array. When indentation == 2 we are in a bid or ask price/volume array
  std::size_t m_ArrayIndentation{};
  std::size_t m_PriceVolumeFieldsIndex{};
};
}   // namespace moboware::exchange::bitstamp