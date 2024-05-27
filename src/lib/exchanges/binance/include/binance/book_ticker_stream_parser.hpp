#pragma once

#include "common/types.hpp"
#include "exchange/exchange.hpp"
#include <nlohmann/json.hpp>
#include <string>

namespace moboware::exchange::binance {

class BookTickerStreamParser {
public:
  BookTickerStreamParser() = default;
  ~BookTickerStreamParser() = default;

  inline const exchange::TopOfTheBook &GetBestBidOffer() const
  {
    return m_BestBidOffer;
  }

  [[nodiscard]] inline bool HandleBookTicker(const std::string &key, std::string &value)
  {
    char **endPtr{};
    if (key == "b")   // bid price
    {
      m_BestBidOffer.bidPrice = strtod(value.c_str(), endPtr);
    } else if (key == "B")   // bid volume
    {
      m_BestBidOffer.bidVolume = strtod(value.c_str(), endPtr);

    } else if (key == "a")   // ask price
    {
      m_BestBidOffer.askPrice = strtod(value.c_str(), endPtr);

    } else if (key == "A")   // ask volume
    {
      m_BestBidOffer.askVolume = strtod(value.c_str(), endPtr);
    }

    return TestBookTickerFields();
  }

private:
  inline bool TestBookTickerFields() const
  {
    return m_BookTickerFields == 0 or (m_BookTickerFields & BookTickerFields::BookTickerAllFields) != BookTickerFields::BookTickerAllFields;
  }

  exchange::TopOfTheBook m_BestBidOffer;

  enum BookTickerFields : std::uint16_t {
    BookTickerBidPriceField = 0b0000001,
    BookTickerBidVolumeField = 0b0000010,
    BookTickerAskPriceField = 0b0000100,
    BookTickerAskVolumeField = 0b0001000,
    BookTickerAllFields = BookTickerBidPriceField | BookTickerBidVolumeField | BookTickerAskPriceField | BookTickerAskVolumeField
  };

  std::uint16_t m_BookTickerFields{};
};
}   // namespace moboware::exchange::binance
