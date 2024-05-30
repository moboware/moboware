#pragma once
#include <array>

namespace moboware::common {

/**
 * @brief Circular buffer stores sequently a value in an array at the end of the queue,
 * once the max is reached the the first value is overwritten by each new value that is added.
 * When you loop over the array you it will start from the current index on until the max queue size
 * @tparam TData
 * @tparam maxQueueSize
 */
template <typename TData, std::size_t maxQueueSize = 100>   //
class CircularBuffer {
public:
  inline std::size_t Add(TData &&data)
  {
    const auto currentIndex{endIndex++ % maxQueueSize};
    m_RingQueue[currentIndex] = data;

    // buffer if full so startIndex moves over the end +1
    if (endIndex > maxQueueSize) {
      startIndex = (startIndex + 1) % maxQueueSize;
    }

    return currentIndex;
  }

  void Loop(const std::function<bool(const TData &data)> &loopFunction)
  {
    const auto size{Size()};
    for (std::size_t i = startIndex; i < (startIndex + size); i++) {
      const auto index{i % maxQueueSize};
      if (not loopFunction(m_RingQueue[index])) {
        return;
      }
    }
  }

  inline std::size_t Size() const
  {
    if (endIndex > maxQueueSize) {
      return maxQueueSize;
    }

    return endIndex - startIndex;
  }

private:
  std::size_t startIndex{};
  std::size_t endIndex{};
  std::array<TData, maxQueueSize> m_RingQueue;
};

}   // namespace moboware::common