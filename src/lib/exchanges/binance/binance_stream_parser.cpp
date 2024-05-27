#include "binance/binance_stream_parser.hpp"
#include "common/logger.hpp"

using namespace std;
using namespace moboware;
using namespace moboware::exchange;
using namespace moboware::exchange::binance;

BinanceStreamHandler::BinanceStreamHandler()
{
  m_Key.reserve(64u);
}
