#pragma once
#include <array>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <iostream>
#include <mutex>

// reads with memory_order_acquire
// writes with memory_order_release

namespace moboware::common {

using QueueLengthType_t = std::uint16_t;

template <typename TBufferType, QueueLengthType_t queueLength = 1024, bool UsePushPopLock = false>   //
class LockLessRingBuffer {
public:
  using Buffer_t = TBufferType;
  static constexpr int CACHE_LINE_SIZE = 64;

  LockLessRingBuffer() = default;
  LockLessRingBuffer(const LockLessRingBuffer &) = delete;
  LockLessRingBuffer(LockLessRingBuffer &&) = delete;
  LockLessRingBuffer &operator=(const LockLessRingBuffer &) = delete;
  LockLessRingBuffer &operator=(LockLessRingBuffer &&) = delete;
  ~LockLessRingBuffer() = default;

#if 1
  // multiple producer/consumer version
  bool Push(const std::function<bool(Buffer_t &slot, const QueueLengthType_t &headPosition)> &pushFn)
  {
    // if (not Empty()) {
    //   return false;   // The buffer is full!!!
    // }

    const auto headPosition{m_Head.fetch_add(1, std::memory_order_seq_cst) % queueLength};

    if constexpr (m_UsePushPopLock) {
      const std::scoped_lock lock(m_PushPopMutex);
      return pushFn(m_Queue[headPosition], headPosition);   // push failed  by user
    } else {
      return pushFn(m_Queue[headPosition], headPosition);   // push failed  by user
    }
  }

  bool Pop(const std::function<void(const Buffer_t &)> &popFn) noexcept
  {
    // if (Empty()) {
    //   return false;   // buffer is empty;
    // }

    const auto tailPosition{m_Tail.fetch_add(1, std::memory_order_seq_cst) % queueLength};

    if constexpr (m_UsePushPopLock) {
      const std::scoped_lock lock(m_PushPopMutex);
      popFn(m_Queue[tailPosition]);
    } else {
      popFn(m_Queue[tailPosition]);
    }

    return true;
  }
#else
  // push/pop single producer/ single consumer an element from the queue
  bool Push(const std::function<bool(Buffer_t &)> &pushFn)
  {
    const std::scoped_lock lock(m_PushPopMutex);

    const auto headPosition{m_Head.load(std::memory_order_relaxed)};
    const auto nextHeadPosition{headPosition + 1};
    // const auto tailPosition{m_Tail.load(std::memory_order_acquire)};

    if (not HasSpace()) {
      return false;   // The buffer is full!!!
    }

    // std::cout << std::this_thread::get_id() << ", Head position:" << headPosition % queueLength
    //           << ", next:" << nextHeadPosition % queueLength << ", tail:" << tailPosition % queueLength << std::endl;

    if (pushFn(m_Queue[headPosition % queueLength])) {
      m_Head.store(nextHeadPosition, std::memory_order_release);
      return true;
    }
    return false;   // push failed  by user
  }

  /**
   * @brief pop an element from the queue
   * @param popFn
   * @return true
   * @return false
   */
  bool Pop(const std::function<void(const Buffer_t &)> &popFn) noexcept
  {
    const std::scoped_lock lock(m_PushPopMutex);

    const auto tailPosition{m_Tail.load(std::memory_order_relaxed)};
    // const auto headPosition{m_Head.load(std::memory_order_acquire)};

    if (Empty()) {
      return false;   // buffer is empty;
    }

    // std::cout << std::this_thread::get_id() << ", Tail position:" << tailPosition % queueLength
    //           << ", head pos:" << headPosition % queueLength << std::endl;

    popFn(m_Queue[tailPosition % queueLength]);

    m_Tail.store(((tailPosition + 1)), std::memory_order_release);
    return true;
  }
#endif

  [[nodiscard]] QueueLengthType_t Size() const noexcept
  {
    const auto headPosition{m_Head.load(std::memory_order_relaxed)};
    const auto tailPosition{m_Tail.load(std::memory_order_relaxed)};
    const auto headTailPosition{headPosition - tailPosition};

    return headTailPosition;
  }

  QueueLengthType_t Capacity() const noexcept
  {
    return queueLength;
  }

  bool HasSpace() const noexcept
  {
    return Size() < Capacity();
  }

  [[nodiscard]] bool Empty() const
  {
    const auto headPosition{m_Head.load(std::memory_order_relaxed) % queueLength};
    const auto tailPosition{m_Tail.load(std::memory_order_relaxed) % queueLength};
    auto ret{headPosition > tailPosition};
    return not ret;
  }

  void Signal(const bool tickleThread = true)
  {
    m_ConditionVariable.notify_all();
    if (tickleThread) {   // sleep for zero nano sec will 'wakeup' the consumer thread faster
      struct ::timespec ts = {0, 0};
      ::nanosleep(&ts, &ts);
    }
  }

  [[nodiscard]] bool Wait(const std::chrono::milliseconds &waitPeriod = std::chrono::milliseconds(100))
  {
    std::unique_lock lock(m_WaitMutex);
    return not(m_ConditionVariable.wait_for(lock, waitPeriod) == std::cv_status::timeout);
  }

private:
  alignas(CACHE_LINE_SIZE) std::atomic<QueueLengthType_t> m_Head;
  alignas(CACHE_LINE_SIZE) std::atomic<QueueLengthType_t> m_Tail;

  alignas(CACHE_LINE_SIZE) std::condition_variable m_ConditionVariable;
  alignas(CACHE_LINE_SIZE) std::mutex m_WaitMutex;
  // this mutex is currently only needed to protect from mangling the 'head' in a multi producer thread situation
  // the cause is probably in reordering  of the L1/L2 cache lines.
  alignas(CACHE_LINE_SIZE) std::recursive_mutex m_PushPopMutex{};

  alignas(CACHE_LINE_SIZE) std::array<TBufferType, queueLength> m_Queue;
  static constexpr bool m_UsePushPopLock = UsePushPopLock;
};
}   // namespace moboware::common
