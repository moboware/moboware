#pragma once
#include <array>
#include <atomic>
#include <functional>

namespace moboware::common {

template <typename T, const std::size_t capacity, const std::size_t queueLength>   //
class LockLessRingBuffer {
public:
  using ArrayBuffer_t = std::array<T, capacity>;

  LockLessRingBuffer()
  {
    m_QueueVector.reserve(queueLength);
  }

  // push an element from the queue
  bool Push(const std::function<bool(ArrayBuffer_t &)> &pushFn)
  {
    const auto headPosition{m_Head.load(std::memory_order_relaxed)};
    const auto nextHeadPosition{(headPosition + 1) % queueLength};
    if (nextHeadPosition == m_Tail.load(std::memory_order_acquire)) {
      return false;   // The buffer is full!!!
    }

    if (pushFn(m_QueueVector[headPosition])) {
      m_Head.store(nextHeadPosition, std::memory_order_release);
      return true;
    }
    return false;   // push failed  by user
  }

  // pop an element from the queue
  bool Pop(const std::function<void(const ArrayBuffer_t &)> &popFn)
  {
    const auto tailPosition{m_Tail.load(std::memory_order_relaxed)};
    if (tailPosition == m_Head.load(std::memory_order_acquire)) {
      return false;   // buffer is empty;
    }

    popFn(m_QueueVector[tailPosition]);

    m_Tail.store(((tailPosition + 1) % queueLength), std::memory_order_release);
    return true;
  }

  std::size_t Size() const
  {
    const auto headPosition{m_Head.load(std::memory_order_acquire)};
    const auto tailPosition{m_Tail.load(std::memory_order_acquire)};
    return (headPosition - tailPosition + queueLength) % queueLength;
  }

private:
  std::vector<ArrayBuffer_t> m_QueueVector;
  alignas(64) std::atomic<std::size_t> m_Head;
  alignas(64) std::atomic<std::size_t> m_Tail;
};
}   // namespace moboware::common
