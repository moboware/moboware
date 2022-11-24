#pragma once
#include "modules/matching_engine_module/i_order_handler.h"
#include <chrono>
#include <map>
#include <optional>
#include <ostream>

namespace moboware::modules {

/// @brief OrderLevel class to hold orders on a price level sorted on time priority
class OrderLevel
{
public:
  explicit OrderLevel(const OrderData& orderInsert);
  OrderLevel(const OrderLevel&) = default;
  OrderLevel(OrderLevel&&) = default;
  OrderLevel& operator=(const OrderLevel&) = default;
  OrderLevel& operator=(OrderLevel&&) = default;
  ~OrderLevel() = default;

  void Insert(const OrderData& orderData);

  auto GetSize() const -> std::size_t;
  struct TopVolumePrice
  {
    VolumeType_t volume{};
    PriceType_t price{};
  };
  /// @brief Get the top level OrderData
  /// @return
  auto GetTopLevel() const -> std::optional<OrderData>;

  /// @brief Trade the top level, reduce the volume of the top level
  /// @param volume
  /// @return left volume at top level
  auto TradeTopLevel(const VolumeType_t volume, const std::function<void(const Trade&)>& tradedFn) const -> VolumeType_t;

  friend std::ostream& operator<<(std::ostream& os, const OrderLevel& level);

private:
  mutable std::map<OrderTime_t, OrderData> m_TimeMap; // order data on this level sorted on time priority
};

/// @brief
/// @param os
/// @param level
/// @return
inline std::ostream& operator<<(std::ostream& os, const OrderLevel& level)
{
  os << "{";
  for (const auto [k, v] : level.m_TimeMap) {
    os << "{" << v.volume << "@" << static_cast<double>(v.price / std::mega::num) << "};";
  }
  os << "}";
  return os;
}

}