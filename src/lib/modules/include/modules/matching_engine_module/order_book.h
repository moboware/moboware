#pragma once
#include "modules/matching_engine_module/order_level.h"
#include <functional>
#include <map>
#include <optional>

namespace moboware::modules {

template<typename TCompare>
class OrderBook
{
public:
  OrderBook() = default;
  OrderBook(const OrderBook&) = delete;
  OrderBook(OrderBook&&) = delete;
  OrderBook& operator=(const OrderBook&) = delete;
  OrderBook& operator=(OrderBook&&) = delete;
  ~OrderBook() = default;

  bool Insert(const OrderData& orderInsert);

  void Amend(const OrderData& orderInsert);

  void Cancel(const std::string& orderId);

  /// @brief
  /// @param
  void GetBook(const std::function<bool(const OrderLevel&)>&);

  /// @brief get the order level at price
  /// @param price
  /// @return order level
  std::optional<const OrderLevel*> GetLevelAtPrice(const PriceType_t& price);

  void RemoveLevelAtPrice(const PriceType_t& price);

  using OrderBookMap_t = std::map<PriceType_t, OrderLevel, TCompare>;
  inline OrderBookMap_t& GetOrderBookMap() { return m_OrderBookMap; }
  inline const OrderBookMap_t& GetOrderBookMap() const { return m_OrderBookMap; }

private:
  OrderBookMap_t m_OrderBookMap;
};
}

using OrderBidBook_t = moboware::modules::OrderBook<std::greater<uint64_t>>;
template class moboware::modules::OrderBook<std::greater<uint64_t>>;

using OrderAskBook_t = moboware::modules::OrderBook<std::less<uint64_t>>;
template class moboware::modules::OrderBook<std::less<uint64_t>>;
