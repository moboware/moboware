#pragma once
#include "modules/matching_engine_module/i_order_handler.h"
#include <chrono>
#include <deque>
#include <map>
#include <optional>
#include <ostream>

namespace moboware::modules {

/// @brief OrderLevel class to hold orders on a price level sorted on time priority
class OrderLevel {
public:
  explicit OrderLevel(OrderInsertData &&orderData);
  OrderLevel(const OrderLevel &) = default;
  OrderLevel(OrderLevel &&) = default;
  OrderLevel &operator=(const OrderLevel &) = default;
  OrderLevel &operator=(OrderLevel &&) = default;
  ~OrderLevel() = default;

  /**
   * @brief Insert an order at the end of the queue
   * @param orderData
   * @return std::optional<const OrderInsertData *>, the inserted order on success
   */
  [[nodiscard]] const OrderInsertData *Insert(OrderInsertData &&orderData) noexcept;
  [[nodiscard]] bool CancelOrder(const Id_t &orderId) noexcept;
  [[nodiscard]] bool ChangeOrderVolume(const Id_t &orderId, const VolumeType_t newVolume) noexcept;

  /**
   * @brief Get the Last Order object, returns the last order in the time queue when not emtpy
   * @return const OrderInsertData*, pointer to the last order in the queue or a nullptr
   */
  [[nodiscard]] const OrderInsertData *GetLastOrder() noexcept;
  [[nodiscard]] auto GetSize() const -> std::size_t;
  [[nodiscard]] auto IsEmpty() const -> bool;

  /// @brief Get the top level OrderInsertData
  /// @return
  [[nodiscard]] auto GetTopLevel() const -> std::optional<OrderInsertData>;

  [[nodiscard]] auto GetLevels(const std::function<bool(const OrderInsertData &)> &orderLevelFunction) const -> bool;

  /// @brief Trade the top level, reduce the volume of the top level
  /// @param volume
  /// @return left volume at top level
  [[nodiscard]] auto TradeTopLevel(const VolumeType_t volume, const std::function<void(const Trade &)> &tradedFn) const
      -> VolumeType_t;

  [[nodiscard]] auto Find(const Id_t &id, const std::function<void(OrderInsertData &)> &) -> bool;
  [[nodiscard]] auto MoveOrder(const Id_t &orderId, const std::function<void(OrderInsertData &&orderData)> &moveOrderFn)
      -> bool;

  friend std::ostream &operator<<(std::ostream &os, const OrderLevel &level);

private:
  using OrderLevel_t = std::deque<OrderInsertData>;
  mutable OrderLevel_t m_TimeQueue;   // order data on this level sorted on time priority
};

/// @brief
/// @param os
/// @param level
/// @return
inline std::ostream &operator<<(std::ostream &os, const OrderLevel &level)
{
  os << "{";
  for (const auto &v : level.m_TimeQueue) {
    os << "{" << v.GetVolume() << "@" << v.GetPriceAsDouble() << "};";
  }
  os << "}";
  return os;
}

}   // namespace moboware::modules