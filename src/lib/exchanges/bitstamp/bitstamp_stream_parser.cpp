#include "bitstamp/bitstamp_stream_parser.hpp"
#include "common/logger.hpp"

using namespace moboware;
using namespace moboware::exchange;
using namespace moboware::exchange::bitstamp;

BitstampStreamParser::BitstampStreamParser()
{
  m_Key.reserve(64u);
}