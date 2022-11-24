#include "modules/matching_engine_module/order_book.h"
#include "common/log.h"

using namespace moboware::modules;

template<typename TCompare>
bool OrderBook<TCompare>::Insert(const OrderData& orderInsert)
{

  auto iter{ m_OrderBookMap.find(orderInsert.price) };
  if (iter != m_OrderBookMap.end()) {
    /// already more orders on this price level, add this one to it.
    auto& orderPriceLevel{ iter->second };
    orderPriceLevel.Insert(orderInsert);

    LOG_DEBUG("Order added at price level:" << static_cast<double>(orderInsert.price / std::mega::num) << ", id:" << orderInsert.id);

    return true;
  } else {
    /// add new order to the price level
    const auto pair{ m_OrderBookMap.emplace(std::make_pair(orderInsert.price, OrderLevel(orderInsert))) };
    if (pair.second) {

      LOG_DEBUG("Order added at price level:" << static_cast<double>(orderInsert.price / std::mega::num) << ", id:" << orderInsert.id);

      return true;
    }
  }
  return false;
}

template<typename TCompare>
void OrderBook<TCompare>::Amend(const OrderData& orderInsert)
{
}

template<typename TCompare>
void OrderBook<TCompare>::Cancel(const std::string& orderId)
{
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
  if (iter != m_OrderBookMap.end()) {
    return &iter->second;
  }

  return nullptr;
}

template<typename TCompare>
void OrderBook<TCompare>::RemoveLevelAtPrice(const PriceType_t& price)
{
  const auto iter{ m_OrderBookMap.find(price) };
  if (iter != m_OrderBookMap.end()) {
    m_OrderBookMap.erase(iter);
  }
}
