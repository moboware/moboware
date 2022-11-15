#include "modules/matching_engine_module/order_level.h"
#include "common/log.h"

using namespace moboware::modules;

OrderLevel::OrderLevel(const OrderData& orderData)
{
  // ??? what in case the order time is already available? Very small change the we have 2 order with the same insertion time, but.....
  Insert(orderData);
}

void OrderLevel::Insert(const OrderData& orderData)
{
  m_TimeMap.insert(std::make_pair(orderData.orderTime, orderData));
}

auto OrderLevel::GetSize() const -> std::size_t
{
  return m_TimeMap.size();
}

auto OrderLevel::GetTopLevel() const -> std::optional<OrderData>
{
  std::optional<OrderData> topLevel;
  if (not m_TimeMap.empty()) {
    topLevel = m_TimeMap.begin()->second;
  }
  return topLevel;
}

auto OrderLevel::TradeTopLevel(const VolumeType_t volume, const std::function<void(const Trade&)>& tradedFn) const -> VolumeType_t
{
  VolumeType_t tradedVolume{};
  if (not m_TimeMap.empty()) {
    auto& topLevel = m_TimeMap.begin()->second;
    if (topLevel.volume >= volume) {
      topLevel.volume -= volume; // partial trade
      tradedVolume = volume;
    } else if (volume > topLevel.volume) {
      tradedVolume = topLevel.volume;
      topLevel.volume = 0; // full trade!!!
    }
    // Send the Trade
    const Trade trade{ topLevel.account, topLevel.price, tradedVolume, topLevel.clientId, topLevel.id };
    tradedFn(trade);
    if (topLevel.volume == 0) {
      //  remove level
      m_TimeMap.erase(m_TimeMap.begin());
    }
  }
  return tradedVolume;
}