#include "modules/matching_engine_module/order_level.h"
#include "common/logger.hpp"

using namespace moboware::modules;

OrderLevel::OrderLevel(OrderInsertData &&orderData)
{
  const auto insertedOrder{Insert(std::forward<OrderInsertData>(orderData))};
}

auto OrderLevel::GetSize() const -> std::size_t
{
  return m_TimeQueue.size();
}

auto OrderLevel::IsEmpty() const -> bool
{
  return m_TimeQueue.empty();
}

auto OrderLevel::GetTopLevel() const -> std::optional<OrderInsertData>
{
  std::optional<OrderInsertData> topLevel;
  if (not m_TimeQueue.empty()) {
    topLevel = *m_TimeQueue.begin();
  }
  return topLevel;
}

auto OrderLevel::TradeTopLevel(const VolumeType_t volume, const std::function<void(const Trade &)> &tradedFn) const -> VolumeType_t
{
  VolumeType_t tradedVolume{};
  if (not m_TimeQueue.empty()) {
    auto &topLevel = *m_TimeQueue.begin();
    if (topLevel.GetVolume() >= volume) {
      topLevel.SetVolume(topLevel.GetVolume() - volume);   // partial trade
      tradedVolume = volume;
    } else if (volume > topLevel.GetVolume()) {
      tradedVolume = topLevel.GetVolume();
      topLevel.SetVolume(0);   // full trade!!!
    }
    // Send the Trade
    const Trade trade{topLevel.GetAccount(), topLevel.GetPrice(), tradedVolume, topLevel.GetId(), topLevel.GetClientId()};
    tradedFn(trade);
    if (topLevel.GetVolume() == 0) {
      //  remove level
      m_TimeQueue.erase(m_TimeQueue.begin());
    }
  }
  return tradedVolume;
}

auto OrderLevel::GetLevels(const std::function<bool(const OrderInsertData &)> &orderLevelFunction) const -> bool
{
  for (const auto &orderData : m_TimeQueue) {
    if (not orderLevelFunction(orderData)) {
      return false;
    }
  }
  return true;
}

const OrderInsertData *OrderLevel::Insert(OrderInsertData &&orderData) noexcept
{
  const auto iter{m_TimeQueue.insert(m_TimeQueue.end(), orderData)};

  return (iter != m_TimeQueue.end()) ? &(*iter) : nullptr;
}

bool OrderLevel::CancelOrder(const Id_t &orderId) noexcept
{
  LOG_DEBUG("Cancel order id {}", orderId);
  for (auto iter{m_TimeQueue.cbegin()}; iter != std::end(m_TimeQueue); ++iter) {
    const auto &orderData{*iter};
    if (orderData.GetId() == orderId) {
      m_TimeQueue.erase(iter);
      return true;
    }
  }
  return false;
}

bool OrderLevel::ChangeOrderVolume(const Id_t &orderId, const VolumeType_t newVolume) noexcept
{
  /*const auto p{ [&orderId](const OrderInsertData& orderData) {
    return orderId == orderData.GetId();
  } };

  auto result = std::find_if(std::begin(m_TimeQueue),
                             std::end(m_TimeQueue), //
                             p);

  if (result != std::end(m_TimeQueue)) {
    auto& orderData = *result;
    orderData.SetVolume(newVolume);
    return true;
  }

  return false;
*/
  const auto orderFoundFn{[newVolume](OrderInsertData &orderData) {
    orderData.SetVolume(newVolume);
  }};
  return Find(orderId, orderFoundFn);
}

auto OrderLevel::Find(const Id_t &orderId, const std::function<void(OrderInsertData &)> &orderFoundFn) -> bool
{
  const auto p{[&orderId](const OrderInsertData &orderData) {
    return orderId == orderData.GetId();
  }};

  auto result = std::find_if(std::begin(m_TimeQueue),
                             std::end(m_TimeQueue),   //
                             p);

  if (result != std::end(m_TimeQueue)) {
    auto &orderData = *result;

    orderFoundFn(orderData);
    return true;
  }
  return false;
}

auto OrderLevel::MoveOrder(const Id_t &orderId, const std::function<void(OrderInsertData &&orderData)> &moveOrderFn) -> bool
{
  // find the order in this level and move it to the other level
  const auto p{[&orderId](const OrderInsertData &orderData) {
    return orderId == orderData.GetId();
  }};

  auto result = std::find_if(std::begin(m_TimeQueue),
                             std::end(m_TimeQueue),   //
                             p);

  if (result != std::end(m_TimeQueue)) {

    moveOrderFn(std::move(*result));
    // erase the order from this level. Remember that this level could be empty now.
    m_TimeQueue.erase(result);
    return true;
  }
  return false;
}

const OrderInsertData *OrderLevel::GetLastOrder() noexcept
{
  if (not m_TimeQueue.empty()) {
    return &m_TimeQueue.back();
  }

  return nullptr;
}
