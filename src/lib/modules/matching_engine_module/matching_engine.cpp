#include "modules/matching_engine_module/matching_engine.h"
#include "common/logger.hpp"

using namespace moboware::modules;

MatchingEngine::MatchingEngine(const std::shared_ptr<common::ChannelInterface> &channelInterface)
  : m_ChannelInterface(channelInterface)
{
}

void MatchingEngine::CreateAndSendMessage(const OrderReply &orderInsertReply, const boost::asio::ip::tcp::endpoint &endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << orderInsertReply;

  const auto s{strm.view()};
  const boost::asio::const_buffer sendBuffer(s.data(), s.size());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

void MatchingEngine::CreateAndSendMessage(const Trade &trade, const boost::asio::ip::tcp::endpoint &endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << trade;

  const auto s{strm.view()};
  const boost::asio::const_buffer sendBuffer(s.data(), s.size());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

void MatchingEngine::CreateAndSendMessage(const ErrorReply &errorReply, const boost::asio::ip::tcp::endpoint &endpoint)
{
  // sendBuffer
  std::ostringstream strm;
  strm << errorReply;

  const auto s{strm.view()};
  const boost::asio::const_buffer sendBuffer(s.data(), s.size());

  m_ChannelInterface->SendWebSocketData(sendBuffer, endpoint);
}

void MatchingEngine::OrderInsert(OrderInsertData &&orderInsert, const boost::asio::ip::tcp::endpoint &endpoint)
{
  std::scoped_lock lock(m_Mutex);

  LOG_INFO("OrderInsert:{}", orderInsert);

  const auto InsertOrderFn{[&](OrderInsertData &&orderInsert) {
    return (orderInsert.GetIsBuySide() ? m_Bids.Insert(std::forward<OrderInsertData>(orderInsert))
                                       : m_Asks.Insert(std::forward<OrderInsertData>(orderInsert)));
  }};

  const auto insertedOrder{InsertOrderFn(std::forward<OrderInsertData>(orderInsert))};   // order is moved and after this not valid anymore, you
                                                                                         // should use the returned inserted order pointer
  if (insertedOrder) {
    // send order insert reply
    const OrderReply orderInsertReply{insertedOrder->GetId(), insertedOrder->GetClientId()};
    CreateAndSendMessage(orderInsertReply, endpoint);
    LOG_INFO("OrderReply:{}", orderInsertReply);

    // check if this order has matches
    const auto CheckMatch{[&](const OrderDataBase &newOrder, const boost::asio::ip::tcp::endpoint &endpoint) {
      newOrder.GetIsBuySide() ?   // matches to the ask side
        ExecuteOrder(m_Asks, m_Bids, endpoint)
                              :   // matches to the bid side
        ExecuteOrder(m_Bids, m_Asks, endpoint);
    }};

    CheckMatch(orderInsert, endpoint);
  } else {
    // send error back
    const ErrorReply errorReply{orderInsert.GetClientId(), "Failed to insert order"};
    CreateAndSendMessage(errorReply, endpoint);
  }
}

void MatchingEngine::OrderAmend(const OrderAmendData &orderAmend, const boost::asio::ip::tcp::endpoint &endpoint)
{
  std::scoped_lock lock(m_Mutex);

  LOG_INFO("OrderAmend: {}", orderAmend);

  const auto Amend{[&](const OrderAmendData &orderAmend) {
    return (orderAmend.GetIsBuySide() ? m_Bids.Amend(orderAmend) : m_Asks.Amend(orderAmend));
  }};

  if (Amend(orderAmend)) {
    // send order insert reply
    const OrderReply orderAmendReply{orderAmend.GetId(), orderAmend.GetClientId()};
    CreateAndSendMessage(orderAmendReply, endpoint);

    // check if this order has matches
    const auto CheckMatch{[&](const OrderDataBase &newOrder, const boost::asio::ip::tcp::endpoint &endpoint) {
      newOrder.GetIsBuySide() ?   // matches to the ask side
        ExecuteOrder(m_Asks, m_Bids, endpoint)
                              :   // matches to the bid side
        ExecuteOrder(m_Bids, m_Asks, endpoint);
    }};

    CheckMatch(orderAmend, endpoint);
  } else {
    // send error back
    const ErrorReply errorReply{orderAmend.GetClientId(), "Failed to amend order"};
    CreateAndSendMessage(errorReply, endpoint);
  }
}

void MatchingEngine::OrderCancel(const OrderCancelData &orderCancel, const boost::asio::ip::tcp::endpoint &endpoint)
{
  std::scoped_lock lock(m_Mutex);

  LOG_INFO("OrderCancel:{}", orderCancel);

  const auto Cancel{[&](const OrderCancelData &orderCancel) {
    return orderCancel.GetIsBuySide() ? m_Bids.Cancel(orderCancel) : m_Asks.Cancel(orderCancel);
  }};

  if (Cancel(orderCancel)) {
    // send order insert reply
    const OrderReply orderCancelReply{orderCancel.GetId(), orderCancel.GetClientId()};
    CreateAndSendMessage(orderCancelReply, endpoint);
  } else {
    // send error back
    const ErrorReply errorReply{orderCancel.GetClientId(), "Failed to cancel order"};
    CreateAndSendMessage(errorReply, endpoint);
  }
}

template <typename TOrderBook1, typename TOrderBook2>
void MatchingEngine::ExecuteOrder(TOrderBook1 &oppositeSideOrderBook,
                                  TOrderBook2 &mySideOrderBook,
                                  const boost::asio::ip::tcp::endpoint &endpoint)
{
  while (true) {
    auto &mySideOrderBookMap = mySideOrderBook.GetOrderBookMap();
    if (mySideOrderBookMap.empty()) {
      // should never happen if we just inserted an new order
      LOG_DEBUG("Other sides order book is empty");
      return;
    }

    const auto &mySideBestOrderLevel = std::begin(mySideOrderBookMap)->second;
    if (mySideBestOrderLevel.IsEmpty()) {
      LOG_DEBUG("Other side top level is empty");
      return;
    }

    const auto mySideBestTopLevel = mySideBestOrderLevel.GetTopLevel();
    if (not mySideBestTopLevel) {
      LOG_DEBUG("No best price on other side PriceLevel found");
      return;
    }

    const auto &myBestOrderData = mySideBestTopLevel.value();

    // get data of the opposite side
    auto &oppositeSideOrderBookMap = oppositeSideOrderBook.GetOrderBookMap();
    if (oppositeSideOrderBookMap.empty()) {
      return;
    }

    const auto &oppositeBestOrderLevel = std::begin(oppositeSideOrderBookMap)->second;
    if (oppositeBestOrderLevel.IsEmpty()) {
      LOG_DEBUG("Best top level is empty");
      return;
    }

    const auto oppositeBestTopLevel = oppositeBestOrderLevel.GetTopLevel();
    if (not oppositeBestTopLevel) {
      LOG_DEBUG("No best price on PriceLevel found");
      return;
    }

    const auto &oppositeBestOrderData = oppositeBestTopLevel.value();
    const auto matchPricePredicate{[](const OrderInsertData &newOrder, const OrderInsertData &bestOrder) -> bool {
      return (newOrder.GetIsBuySide() ? (newOrder.GetPrice() >= bestOrder.GetPrice()) : (bestOrder.GetPrice() >= newOrder.GetPrice()));
    }};

    if (not matchPricePredicate(myBestOrderData, oppositeBestOrderData)) {
      return;
    }
    // we have a match
    LOG_DEBUG("Match {}@{}:{}@{}",
              myBestOrderData.GetVolume(),
              myBestOrderData.GetPrice(),
              oppositeBestOrderData.GetVolume(),
              oppositeBestOrderData.GetPrice());
    // trade execution callback function
    const auto sendTradeFn{[this, &endpoint](const Trade &trade) {
      LOG_INFO("Trade:{}", trade);
      CreateAndSendMessage(trade, endpoint);
    }};

    const auto tradedVolume{oppositeBestOrderLevel.TradeTopLevel(myBestOrderData.GetVolume(), sendTradeFn)};
    //  reduce volume on the other side top level
    const auto otherSideTradedVolume{mySideBestOrderLevel.TradeTopLevel(tradedVolume, sendTradeFn)};

    /// check if the bid/ask levels are empty and remove the top level if so
    if (oppositeBestOrderLevel.IsEmpty()) {
      oppositeSideOrderBook.RemoveLevelAtPrice(oppositeBestOrderData.GetPrice());
    }
    if (mySideBestOrderLevel.IsEmpty()) {
      mySideOrderBook.RemoveLevelAtPrice(myBestOrderData.GetPrice());
    }
  }
}

void MatchingEngine::GetOrderBook(const boost::asio::ip::tcp::endpoint &endpoint)
{
  const auto printOrderLevel{[](const OrderLevel &orderLevel) {
    const auto printOrderData{[](const OrderInsertData &orderData) {
      // LOG_DEBUG("OrderInsertData:" << orderData);
      return true;
    }};

    return orderLevel.GetLevels(printOrderData);
  }};

  m_Asks.GetBook(printOrderLevel);
  m_Bids.GetBook(printOrderLevel);
}