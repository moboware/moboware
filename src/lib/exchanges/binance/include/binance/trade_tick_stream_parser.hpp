#pragma once

#include "common/types.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace moboware::exchange::binance {
class TradeTickStreamParser {
public:
  TradeTickStreamParser() = default;
  ~TradeTickStreamParser() = default;

  inline const exchange::TradeTick &GetTradeTick() const
  {
    return m_TradeTick;
  }

  [[nodiscard]] inline bool HandleTradeTick(const std::string &key, nlohmann::json::json_sax_t::number_unsigned_t value)
  {
    if (key == "T") {
      m_TradeTick.tradeTime = common::SystemTimePoint_t(std::chrono::milliseconds(value));   // trade time
      m_TradeTickFields |= TradeTickFields::TradeTickTimeField;

    } else if (key == "t") {
      m_TradeTick.tradeId = std::move(std::to_string(value));   // trade id
      m_TradeTickFields |= TradeTickFields::TradeTickIdField;
    }
    return TestTradeTickFields();
  }

  [[nodiscard]] inline bool HandleTradeTick(const std::string &key, std::string &value)
  {
    char **endptr{};
    if (key == "p") {
      m_TradeTick.tradePrice = strtod(value.c_str(), endptr);   // trade price
      m_TradeTickFields |= TradeTickFields::TradeTickPriceField;
    } else if (key == "q") {
      m_TradeTick.tradeVolume = strtod(value.c_str(), endptr);   // trade volume
      m_TradeTickFields |= TradeTickFields::TradeTickVolumeField;
    }
    return TestTradeTickFields();
  }

private:
  inline bool TestTradeTickFields() const
  {
    return m_TradeTickFields == 0 or (m_TradeTickFields & TradeTickFields::TradeTickAllFields) != TradeTickFields::TradeTickAllFields;
  }

  exchange::TradeTick m_TradeTick;

  // there are 4 fields in the trade tick that we need to parse, the bit value of each field will
  // be set to define and test that we have all fields and can stop parsing
  enum TradeTickFields : std::uint16_t {
    TradeTickIdField = 0b0000001,
    TradeTickPriceField = 0b0000010,
    TradeTickVolumeField = 0b0000100,
    TradeTickTimeField = 0b0001000,
    TradeTickAllFields = TradeTickIdField | TradeTickPriceField | TradeTickVolumeField | TradeTickTimeField
  };

  std::uint16_t m_TradeTickFields{};
};
}   // namespace moboware::exchange::binance