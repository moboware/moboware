#include "bitstamp/bitstamp_stream_parser.hpp"
#include "common/logger.hpp"

using namespace moboware;
using namespace moboware::exchange;
using namespace moboware::exchange::bitstamp;

BitstampStreamParser::BitstampStreamParser(const std::string_view &msg)
{
  m_Key.reserve(64u);

  // try to find what kind type json message is received:
  // - trade tick
  // - subscription ack
  // - order book snapshot
  if (msg.find("\"event\":\"trade\"") != std::string::npos) {
    m_StreamType = MarketDataStreamType::TradeTickStream;
  }
}
