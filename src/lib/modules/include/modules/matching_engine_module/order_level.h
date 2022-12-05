#pragma once
#include "modules/matching_engine_module/i_order_handler.h"
#include <chrono>
#include <deque>
#include <map>
#include <optional>
#include <ostream>

namespace moboware::modules {

/// @brief OrderLevel class to hold orders on a price level sorted on time priority
class OrderLevel
{
public:
  explicit OrderLevel(const OrderData& orderData);
  OrderLevel(const OrderLevel&) = default;
  OrderLevel(OrderLevel&&) = default;
  OrderLevel& operator=(const OrderLevel&) = default;
  OrderLevel& operator=(OrderLevel&&) = default;
  ~OrderLevel() = default;

  void Insert(const OrderData& orderData);
  [[nodiscard]] bool CancelOrder(const Id_t& orderId);
  [[nodiscard]] bool ChangeOrderVolume(const Id_t& orderId, const VolumeType_t newVolume);

  [[nodiscard]] auto GetSize() const -> std::size_t;
  [[nodiscard]] auto IsEmpty() const -> bool;

  /// @brief Get the top level OrderData
  /// @return
  [[nodiscard]] auto GetTopLevel() const -> std::optional<OrderData>;

  [[nodiscard]] auto GetLevels(const std::function<bool(const OrderData&)>& orderLevelFunction) const -> bool;

  /// @brief Trade the top level, reduce the volume of the top level
  /// @param volume
  /// @return left volume at top level
  [[nodiscard]] auto TradeTopLevel(const VolumeType_t volume, const std::function<void(const Trade&)>& tradedFn) const -> VolumeType_t;

  [[nodiscard]] auto Find(const Id_t& id, const std::function<void(OrderData&)>&) -> bool;
  [[nodiscard]] auto MoveOrder(const Id_t& orderId, const std::function<void(OrderData&& orderData)>& moveOrderFn) -> bool;

  [[nodiscard]] friend std::ostream& operator<<(std::ostream& os, const OrderLevel& level);

private:
  using OrderLevel_t = std::deque<OrderData>;
  mutable OrderLevel_t m_TimeQueue; // order data on this level sorted on time priority
};

/// @brief
/// @param os
/// @param level
/// @return
inline std::ostream& operator<<(std::ostream& os, const OrderLevel& level)
{
  os << "{";
  for (const auto& v : level.m_TimeQueue) {
    os << "{" << v.GetVolume() << "@" << v.GetPriceAsDouble() << "};";
  }
  os << "}";
  return os;
}

}