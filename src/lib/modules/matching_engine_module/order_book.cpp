#include "modules/matching_engine_module/order_book.h"
#include "common/log.h"

using namespace moboware::modules;

template<typename TCompare>
bool OrderBook<TCompare>::Insert(const OrderData& orderInsert)
{
  auto iter{ m_OrderBookMap.find(orderInsert.GetPrice()) };
  if (iter != std::end(m_OrderBookMap)) {
    /// already more orders on this price level, add this one to it.
    auto& orderPriceLevel{ iter->second };
    orderPriceLevel.Insert(orderInsert);

    LOG_DEBUG("Order added at price level:" << orderInsert.GetPriceAsDouble() << ", id:" << orderInsert.GetId());

    return true;
  } else {
    /// add new order to the price level
    const auto pair{ m_OrderBookMap.emplace(std::make_pair(orderInsert.GetPrice(), OrderLevel(orderInsert))) };
    if (pair.second) {

      LOG_DEBUG("Order added at price level:" << orderInsert.GetVolume() << "@" << orderInsert.GetPriceAsDouble() << ", id:" << orderInsert.GetId());

      return true;
    }
  }
  return false;
}

template<typename TCompare>
bool OrderBook<TCompare>::Amend(const OrderAmendData& orderAmend)
{
  auto iter{ m_OrderBookMap.find(orderAmend.GetPrice()) };
  if (iter == std::end(m_OrderBookMap)) {
    LOG_ERROR("Price level not found at price " << orderAmend.GetPrice());
    return false;
  }

  auto& orderPriceLevel{ iter->second };

  if (orderAmend.GetNewVolume() == 0) { // cancel order when  volume is zero
    if (orderPriceLevel.CancelOrder(orderAmend.GetId())) {
      // cancel the order when new volume is zero
      LOG_DEBUG("Order amend volume is zero, order is cancelled at price level:" << orderAmend.GetPriceAsDouble() << ", id:" << orderAmend.GetId());
      if (orderPriceLevel.IsEmpty()) {
        RemoveLevelAtPrice(orderAmend.GetPrice());
      }
      return true;
    }
  } else if (orderAmend.GetPrice() == orderAmend.GetNewPrice()) {
    /// price is not changed on the price level, so can only change volume or time duration
    /// change the order volume of the order at price level

    if (orderPriceLevel.ChangeOrderVolume(orderAmend.GetId(), orderAmend.GetNewVolume())) {
      LOG_DEBUG("Order volume changed to " << orderAmend.GetNewVolume());
      return true;
    }
  } else {
    //// The change is on a other price level. Means we need to move the order from the original price level and insert a new order in the new price level with
    ///a / original order id
    /// move the order from one price level to the new price level

    const auto moveOrderFn{ [&](OrderData&& orderData) {
      // set new price and volume
      orderData.SetPrice(orderAmend.GetNewPrice());
      orderData.SetVolume(orderAmend.GetNewVolume());

      const auto pair{ m_OrderBookMap.emplace(std::make_pair(orderAmend.GetNewPrice(), OrderLevel(orderData))) };
      if (pair.second) {
        LOG_DEBUG("Order moved to new price level:" << orderData);
      }
    } };

    if (orderPriceLevel.MoveOrder(orderAmend.GetId(), moveOrderFn)) {
      // check if the order level is empty and needs to be removed
      if (orderPriceLevel.IsEmpty()) {
        // remove the price level
        m_OrderBookMap.erase(iter);
      }
      return true;
    }
  }
  return false;
}

template<typename TCompare>
bool OrderBook<TCompare>::Cancel(const OrderCancelData& orderCancel)
{
  auto iter{ m_OrderBookMap.find(orderCancel.GetPrice()) };
  if (iter != std::end(m_OrderBookMap)) {
    /// cancel the order at price level
    auto& orderPriceLevel{ iter->second };
    if (orderPriceLevel.CancelOrder(orderCancel.GetId())) {
      LOG_DEBUG("Order cancelled at price level:" << orderCancel.GetPriceAsDouble() << ", id:" << orderCancel.GetId());

      return true;
    }
  }
  return false;
}

template<typename TCompare>
void OrderBook<TCompare>::GetBook(const std::function<bool(const OrderLevel&)>& orderBookFunction)
{
  for (const auto& [k, v] : m_OrderBookMap) {
    if (not orderBookFunction(v)) {
      return;
    }
  }
}

template<typename TCompare>
std::optional<const OrderLevel*> OrderBook<TCompare>::GetLevelAtPrice(const PriceType_t& price)
{
  const auto iter{ m_OrderBookMap.find(price) };
  if (iter != std::end(m_OrderBookMap)) {
    return &iter->second;
  }

  return nullptr;
}

template<typename TCompare>
void OrderBook<TCompare>::RemoveLevelAtPrice(const PriceType_t& price)
{
  const auto iter{ m_OrderBookMap.find(price) };
  if (iter != std::end(m_OrderBookMap)) {
    m_OrderBookMap.erase(iter);
  }
}
