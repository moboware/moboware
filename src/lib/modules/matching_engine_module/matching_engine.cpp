#include "modules/matching_engine_module/matching_engine.h"
#include "common/log_stream.h"

using namespace moboware::modules;

MatchingEngine::MatchingEngine(const std::shared_ptr<common::ChannelInterface>& channelInterface)
  : m_ChannelInterface(channelInterface)
{
}

void MatchingEngine::CreateAndSendMessage(const OrderInsertReply& orderInsertReply, const boost::asio::ip::tcp::endpoint& endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << "{\"OrderInsertReply\":"                                //
       << "{\"ClientId\":\"" << orderInsertReply.clientId << "\"" //
       << ",\"Id\":\"" << orderInsertReply.id << "\"}"            //
       << "}";

  const std::string s{ strm.str() };
  const boost::asio::const_buffer sendBuffer(s.c_str(), s.length());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

void MatchingEngine::CreateAndSendMessage(const Trade& trade, const boost::asio::ip::tcp::endpoint& endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << "{\"Trade\":"                                //
       << "{\"ClientId\":\"" << trade.clientId << "\"" //
       << ",\"Id\":\"" << trade.id << "\""             //
       << ",\"Account\":\"" << trade.account << "\""   //
       << ",\"TradedPrice\":" << trade.tradedPrice     //
       << ",\"TradedVolume\":" << trade.tradedVolume   //
       << "}"                                          //
       << "}";

  const std::string s{ strm.str() };
  const boost::asio::const_buffer sendBuffer(s.c_str(), s.length());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

void MatchingEngine::CreateAndSendMessage(const ErrorReply& errorReply, const boost::asio::ip::tcp::endpoint& endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << "{\"OrderInsertReply\":" //
       << "{\"ClientId\":"
       << "\"" << errorReply.clientId << "\""             //
       << ",\"error\":" << errorReply.errorMessage << "}" //
       << "}";

  const std::string s{ strm.str() };
  const boost::asio::const_buffer sendBuffer(s.c_str(), s.length());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

bool MatchingEngine::Insert(const OrderData& orderInsert)
{
  if (orderInsert.IsBuySide) {
    return m_Bids.Insert(orderInsert);
  } else {
    return m_Asks.Insert(orderInsert);
  }
  return false;
}

void MatchingEngine::OrderInsert(const OrderData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint)
{
  std::scoped_lock lock(m_Mutex);

  if (Insert(orderInsert)) {
    // send order insert reply
    const OrderInsertReply orderInsertReply{ orderInsert.id, orderInsert.clientId };
    CreateAndSendMessage(orderInsertReply, endpoint);
    // check if this order has matches
    CheckMatch(orderInsert, endpoint);
  } else {
    // send error back
    const ErrorReply errorReply{ orderInsert.clientId, "Failed to insert order" };
    CreateAndSendMessage(errorReply, endpoint);
  }
}

template<typename TOrderBook1, typename TOrderBook2>
void MatchingEngine::ExecuteOrder(TOrderBook1& oppositeSideOrderBook, TOrderBook2& mySideOrderBook, const boost::asio::ip::tcp::endpoint& endpoint)
{
  while (true) {
    auto& mySideOrderBookMap = mySideOrderBook.GetOrderBookMap();
    if (mySideOrderBookMap.empty()) {
      // should never happen if we just inserted an new order
      LOG_ERROR("Other sides order book is empty");
      return;
    }

    const auto& mySideBestOrderLevel = mySideOrderBookMap.begin()->second;
    if (mySideBestOrderLevel.GetSize() == 0) {
      LOG_ERROR("Other side top level is empty");
      return;
    }

    const auto mySideBestTopLevel = mySideBestOrderLevel.GetTopLevel();
    if (not mySideBestTopLevel) {
      LOG_ERROR("No best price on other side PriceLevel found");
      return;
    }

    const auto& mybestOrderData = mySideBestTopLevel.value();

    // get data of the opposite side
    auto& oppositeSideOrderBookMap = oppositeSideOrderBook.GetOrderBookMap();
    if (oppositeSideOrderBookMap.empty()) {
      return;
    }

    const auto& oppositeBestOrderLevel = oppositeSideOrderBookMap.begin()->second;
    if (oppositeBestOrderLevel.GetSize() == 0) {
      LOG_ERROR("Best top level is empty");
      return;
    }

    const auto oppositeBestTopLevel = oppositeBestOrderLevel.GetTopLevel();
    if (not oppositeBestTopLevel) {
      LOG_ERROR("No best price on PriceLevel found");
      return;
    }

    const auto& oppositeBestOrderData = oppositeBestTopLevel.value();
    const auto matchPricePredicate{ [](const OrderData& newOrder, const OrderData& bestOrder) -> bool {
      return (newOrder.IsBuySide ? (newOrder.price >= bestOrder.price) : (bestOrder.price >= newOrder.price));
    } };

    if (not matchPricePredicate(mybestOrderData, oppositeBestOrderData)) {
      return;
    }
    // we have a match
    LOG_DEBUG("Match " << mybestOrderData.volume << "@" << mybestOrderData.price << ":" //
                       << oppositeBestOrderData.volume << "@" << oppositeBestOrderData.price);
    // trade execution callback function
    const auto sendTradeFn{ [this, &endpoint](const Trade& trade) {
      LOG_DEBUG("Create and send trade:" << trade);
      CreateAndSendMessage(trade, endpoint);
    } };

    const auto tradedVolume{ oppositeBestOrderLevel.TradeTopLevel(mybestOrderData.volume, sendTradeFn) };
    //  reduce volume on the other side top level
    mySideBestOrderLevel.TradeTopLevel(tradedVolume, sendTradeFn);

    /// check if the bid/ask levels are empty and remove the top level if so
    if (oppositeBestOrderLevel.GetSize() == 0) {
      oppositeSideOrderBook.RemoveLevelAtPrice(oppositeBestOrderData.price);
    }
    if (mySideBestOrderLevel.GetSize() == 0) {
      mySideOrderBook.RemoveLevelAtPrice(mybestOrderData.price);
    }
  }
}

void MatchingEngine::CheckMatch(const OrderData& newOrder, const boost::asio::ip::tcp::endpoint& endpoint)
{
  if (newOrder.IsBuySide) { // matches to the ask side
    ExecuteOrder(m_Asks, m_Bids, endpoint);

  } else { // matches to the bid side
    ExecuteOrder(m_Bids, m_Asks, endpoint);
  }
  return;
  ///////
  //  const auto& bidsMap{ m_Bids.GetOrderBookMap() };
  //  const auto& asksMap{ m_Asks.GetOrderBookMap() };
  //
  //  if (not bidsMap.empty() && not asksMap.empty()) {
  //    const auto& bestBidLevel = bidsMap.begin()->second;
  //    const auto& bestAskLevel = asksMap.begin()->second;
  //
  //    if (bestBidLevel.GetSize() && bestAskLevel.GetSize()) {
  //      const auto bestBid = bestBidLevel.GetTopLevel();
  //      if (not bestBid) {
  //        LOG_ERROR("No best bid price on PriceLevel found");
  //        return;
  //      }
  //
  //      const auto bestAsk = bestAskLevel.GetTopLevel();
  //      if (not bestAsk) {
  //        LOG_ERROR("No best bid price on PriceLevel found");
  //        return;
  //      }
  //
  //      const auto& bestBidLevelData = bestBid.value();
  //      const auto& bestAskLevelData = bestAsk.value();
  //      if (bestBidLevelData.price >= bestAskLevelData.price) {
  //        // we have a match
  //        LOG_DEBUG("Match " << bestBidLevelData.volume << "@" << bestBidLevelData.price << ":" //
  //                           << bestAskLevelData.volume << "@" << bestAskLevelData.price);
  //        // trade
  //        const auto sendTradeFn{ [this, &endpoint](const Trade& trade) {
  //          CreateAndSendMessage(trade, endpoint);
  //        } };
  //
  //        if (bestBidLevelData.volume >= bestAskLevelData.volume) // full trade on ask side
  //        {
  //          const auto tradedVolume{ bestAskLevel.TradeTopLevel(bestBidLevelData.volume, sendTradeFn) };
  //          // reduce volume on the bid top level
  //          bestBidLevel.TradeTopLevel(tradedVolume, sendTradeFn);
  //        } else if (bestAskLevelData.volume >= bestBidLevelData.volume) // full trade on bid side
  //        {
  //          const auto tradedVolume{ bestAskLevel.TradeTopLevel(bestBidLevelData.volume, sendTradeFn) };
  //          // reduce volume on the ask top level
  //          bestBidLevel.TradeTopLevel(tradedVolume, sendTradeFn);
  //        }
  //
  //        { /// check if the bid/ask levels are empty and remove the top level if so
  //          if (bestAskLevel.GetSize() == 0) {
  //            m_Asks.RemoveLevelAtPrice(bestAskLevelData.price);
  //          }
  //
  //          if (bestBidLevel.GetSize() == 0) {
  //            m_Bids.RemoveLevelAtPrice(bestBidLevelData.price);
  //          }
  //        }
  //      }
  //    }
  //  }
}
