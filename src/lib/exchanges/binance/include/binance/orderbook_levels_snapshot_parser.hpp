#pragma once

#include "common/types.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace moboware::exchange::binance {

// parse the binance orderbook depth feed that provides the full 5, 10 or 20 levels orderbook
// e.g. 5 level deep orderbook snapshot:
// { "stream":"btcusdt@depth5@100ms",
//      "data":{"lastUpdateId":47393218092,
//         "bids":[["68790.00000000","2.08416000"],["68789.95000000","0.03166000"],["68789.79000000","0.03709000"],["68788.60000000","0.05820000"],["68788.04000000","0.01324000"]],
//         "asks":[["68790.01000000","6.81568000"],["68790.02000000","0.00436000"],["68790.03000000","0.00436000"],["68790.04000000","0.00436000"],["68790.05000000","0.00436000"]]
// }}
class OrderbookLevelsParser {
public:
  OrderbookLevelsParser() = default;
  ~OrderbookLevelsParser() = default;

  void Clear()
  {
    m_Orderbook.numberOfBidLevels = 0;
    m_Orderbook.numberOfAskLevels = 0;
    m_ArrayIndentation = 0;
    m_PriceVolumeFieldsIndex = 0;
  }

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
}   // namespace moboware::exchange::binance