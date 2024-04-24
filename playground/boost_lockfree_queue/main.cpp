#include "common/log_stream.h"
#include "common/log_stream_buf.h"
#include <atomic>
#include <boost/lockfree/queue.hpp>
#include <chrono>
#include <condition_variable>
#include <cstring>
#include <iostream>
#include <thread>

using namespace boost;
using namespace moboware;

#pragma pack(push, 1)

struct Header {
  std::uint64_t m_PayloadSize{};
  std::uint8_t m_Type{};
  std::uint64_t m_SequenceNumber{};
  std::uint64_t m_Timepoint{};
};

#pragma pack(pop)

const auto bufferLength{5000u};

const auto queueLength{25u};

using ArrayBuffer_t = common::LogStreamBuf<common::NewLogStream::LogLineLength>::ArrayBuffer_t;
using ArrayQueue_t = boost::lockfree::queue<ArrayBuffer_t>;

std::mutex producerMutex;
std::uint64_t producerSequenceNumber{};

struct spinlock {
  std::atomic<bool> lock_ = {0};

  void lock() noexcept
  {
    for (;;) {
      // Optimistically assume the lock is free on the first try
      if (!lock_.exchange(true, std::memory_order_acquire)) {
        return;
      }
      // Wait for lock to be released without generating cache misses
      while (lock_.load(std::memory_order_relaxed)) {
        // Issue X86 PAUSE or ARM YIELD instruction to reduce contention between
        // hyper-threads
        __builtin_ia32_pause();
      }
    }
  }

  bool try_lock() noexcept
  {
    // First do a relaxed load to check if lock is free in order to prevent
    // unnecessary cache misses if someone does while(!try_lock())
    return !lock_.load(std::memory_order_relaxed) && !lock_.exchange(true, std::memory_order_acquire);
  }

  void unlock() noexcept
  {
    lock_.store(false, std::memory_order_release);
  }
};

// spinlock waitFlag;
std::condition_variable m_ConditionVariable;
std::mutex m_WaitMutex;

void Signal(const bool tickleThread = true)
{
#if 0
  waitFlag.unlock();
#else
  m_ConditionVariable.notify_all();
#endif
  if (tickleThread) {   // sleep for zero nano sec will 'wakeup' the consumer thread faster
    struct ::timespec ts = {0, 0};
    ::nanosleep(&ts, &ts);
    //
    std::this_thread::yield();
  }
}

[[nodiscard]] bool Wait(const std::chrono::milliseconds &waitPeriod = std::chrono::milliseconds(100))
{
#if 0
  waitFlag.lock();
  return true;
#else
  std::unique_lock lock(m_WaitMutex);
  return not(m_ConditionVariable.wait_for(lock, waitPeriod) == std::cv_status::timeout);
#endif
}

void producerThreadFunction(ArrayQueue_t &arrayQueue)
{
  {
    // std::scoped_lock lock(producerMutex);

    while (true) {
      std::this_thread::sleep_for(std::chrono::milliseconds(500));   // Wait for space in buffer

      // fill the queue
      for (int i = 0; i < std::rand() % queueLength; i++) {
        // fill the buffer with data
        common::NewLogStream stream;
        stream << "Log: producer seq#:" << producerSequenceNumber
               << ", timepoint: " << std::chrono::system_clock::now().time_since_epoch().count();

        // ArrayBuffer_t element;
        //  const auto str{std::to_string(producerSequenceNumber)};
        //   Header header;
        //   header.m_PayloadSize = str.size();
        //   header.m_Type = 1;
        //   header.m_SequenceNumber = producerSequenceNumber;
        //   header.m_Timepoint = std::chrono::system_clock::now().time_since_epoch().count();

        // copy header
        // std::memcpy(element.m_Array.data(), &header, sizeof(header));

        // copy payload
        // std::memcpy(&element.m_Array.data()[sizeof(header)], str.c_str(), str.size());

        // std::cout << "Pushed #seq:" << producerSequenceNumber << std::endl;

        producerSequenceNumber++;

        if (arrayQueue.push(stream.GetLogStreamLine().GetBuffer())) {
          std::cout << "Produced: " << (producerSequenceNumber - 1)   //
                    << std::endl;
          Signal();

        } else {
          std::cout << "Producer no space! "
                    << " #seq:" << producerSequenceNumber - 1 << " Used: "   //<< buffer.Size() << "/" << buffer.Capacity()
                    << std::endl;
          Signal(false);
          std::this_thread::sleep_for(std::chrono::milliseconds(10));   // Wait for space in buffer
        }
      }
    }
  }
}

int main(int, char **)
{

  ArrayQueue_t queue(queueLength);

  // producer thread
  std::vector<std::jthread> producers;
  for (int i = 0; i < 2; i++) {
    producers.emplace_back(std::jthread([&]() {
      producerThreadFunction(queue);
    }));
  }

  // consumer thread
  std::jthread consumer([&]() {
    std::uint64_t sequenceNumber{};

    const auto popFn{[&](const ArrayBuffer_t &element) {
      // read the header
      // Header header;
      // std::memcpy(&header, element.m_Array.data(), sizeof(Header));
      // if (header.m_PayloadSize == 0) {
      //        std::cout << "Payload size is zero" << std::endl;
      //      return;
      //  }
      // read the payload
      // const auto str{std::string(&element.m_Array.data()[sizeof(Header)], header.m_PayloadSize)};

      // const auto delta{std::chrono::system_clock::now().time_since_epoch().count() - header.m_Timepoint};

      // if (sequenceNumber++ != header.m_SequenceNumber) {
      //   std::cout << "Invalid sequence:" << sequenceNumber - 1 << ", != " << header.m_SequenceNumber << std::endl;
      // } else {
      //   std::cout << "Consumed: " << str << ", Seq#:" << header.m_SequenceNumber << ", delta:" << delta << std::endl;
      // }
      std::cout << std::string_view(element.m_LogBuffer.data(), element.m_Size) << std::endl;
    }};

    while (true) {
      if (Wait() and not queue.empty()) {
        std::cout << "Wakeup for data: " << std::endl;
        queue.consume_all(popFn);
      }
    }
  });

  return 0;
}
