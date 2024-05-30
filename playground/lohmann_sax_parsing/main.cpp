#include "binance/binance_stream_parser.hpp"
#include "common/logger.hpp"
#include "exchange/exchange.hpp"
#include <chrono>
#include <nlohmann/json.hpp>
#include <sstream>

// demo of a lohmann SAX parser of binance trade tick object

using json = nlohmann::json;
using namespace std;
using namespace moboware;
using namespace moboware::exchange;
using namespace moboware::exchange::binance;

int main(int, char **)
{
  for (int i; i < 25; i++) {
    const std::string_view trade_json{R"({"stream":"btcusdt@trade",
                                      "data":{
                                      "e":"trade",
                                      "E":1672515782136,
                                      "s":"BTCUSDT",
                                      "t":12345,
                                      "p":"0.001",
                                      "q":"100",
                                      "b":88,
                                      "a":50,
                                      "T":1672515782136,
                                      "m":true,
                                      "M":true
                                       }})"};

    // create a SAX event consumer object
    BinanceStreamHandler binanceStreamHandler;

    // parse JSON
    const auto t1{common::SessionTime_t::now()};
    bool result{json::sax_parse(trade_json.begin(), trade_json.end(), &binanceStreamHandler, json::input_format_t::json, false, true)};
    const auto d{common::SessionTime_t::now() - t1};

    if (binanceStreamHandler.GetStreamType() == MarketDataStreamType::TradeTickStream) {
      LOG_INFO("parse time:{}, result:{}, TradeTick, tradeId={}, tradePrice={}, tradeVolume={}, tradeTime={}",
               d,
               result,
               binanceStreamHandler.GetTradeTick().tradeId,       //
               binanceStreamHandler.GetTradeTick().tradePrice,    //
               binanceStreamHandler.GetTradeTick().tradeVolume,   //
               binanceStreamHandler.GetTradeTick().tradeTime);    //
    }
  }

  for (int i = 0; i < 25; i++) {
    const std::string_view bookTickerStreamJson{R"(
      {"stream":"btcusdt@bookTicker",
      "data":{
        "u":47335700876,
        "s":"BTCUSDT",
        "b":"68325.09000000",
        "B":"3.51123000",
        "a":"68325.10000000",
        "A":"6.82643000"
        }})"};
    BinanceStreamHandler binanceStreamHandler;

    // parse JSON
    const auto t1{common::SessionTime_t::now()};
    bool result{
      json::sax_parse(bookTickerStreamJson.begin(), bookTickerStreamJson.end(), &binanceStreamHandler, json::input_format_t::json, false, true)};
    const auto d{common::SessionTime_t::now() - t1};

    if (binanceStreamHandler.GetStreamType() == MarketDataStreamType::BookTickerStream) {
      LOG_INFO("parse time:{}, result:{}, BookTicker, bid:{}@{} -- ask:{}@{}",
               d,
               result,
               binanceStreamHandler.GetBestBidOffer().bidPrice,     //
               binanceStreamHandler.GetBestBidOffer().bidVolume,    //
               binanceStreamHandler.GetBestBidOffer().askPrice,     //
               binanceStreamHandler.GetBestBidOffer().askVolume);   //
    }
  }

  {
    const std::string_view orderbookDepth5LevelSnapshot{
      R"({
        "stream":"btcusdt@depth5@100ms",
          "data":{"lastUpdateId":47393218092,
            "bids":[["68790.00000000","2.08416000"],["68789.95000000","0.03166000"],["68789.79000000","0.03709000"],["68788.60000000","0.05820000"],["68788.04000000","0.01324000"]],
            "asks":[["68790.01000000","6.81568000"],["68790.02000000","0.00436000"],["68790.03000000","0.00436000"],["68790.04000000","0.00436000"],["68790.05000000","0.00436000"]]
      }})"};

    BinanceStreamHandler binanceStreamHandler;

    // parse JSON
    const auto t1{common::SessionTime_t::now()};
    bool result{json::sax_parse(orderbookDepth5LevelSnapshot.begin(),
                                orderbookDepth5LevelSnapshot.end(),
                                &binanceStreamHandler,
                                json::input_format_t::json,
                                false,
                                true)};
    const auto d{common::SessionTime_t::now() - t1};

    if (binanceStreamHandler.GetStreamType() == MarketDataStreamType::Depth5LevelsStream) {
      LOG_INFO("parse time:{}, result:{}, OrderbookDepth, bid[0] {}@{}, ask[0] {}@{}",
               d,
               result,
               binanceStreamHandler.GetOrderbook().m_Bids[0].price,    //
               binanceStreamHandler.GetOrderbook().m_Bids[0].volume,   //
               binanceStreamHandler.GetOrderbook().m_Asks[0].price,    //
               binanceStreamHandler.GetOrderbook().m_Asks[0].volume    //
      );                                                               //
    }
  }
  return 0;
}
