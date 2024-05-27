#include "common/logger.hpp"
#include <array>
#include <atomic_queue/atomic_queue.h>
#include <chrono>
#include <iomanip>
#include <iostream>
#include <thread>
#include <vector>

// try out of the atomic_queue library, see : https://github.com/max0x7ba/atomic_queue
// this example will use multiple producers threads and single consumer thread. The producers pushes data on the lockless
// queue that is read and popped from the queue by the consumer thread.
// To synchronize the sequence number used by the producer the producer thread is protected by a mutex, otherwise the risk is
// there that the sequence number is incremented but, one producer thread is faster that the other producer thread.
// The transfer of data to the consumer thread is lock less. In an other application the sequence number mutex is not
// necessary.

using namespace moboware::common;

#pragma pack(push, 1)

struct Header {
  std::uint64_t m_PayloadSize{};
  std::uint8_t m_Type{};
  std::uint64_t m_SequenceNumber{};
  std::uint64_t m_TimePoint{};
};

#pragma pack(pop)

const auto capacity{5000u};
const auto length{500u};
using ArrayBuffer_t = std::array<char, capacity>;
using LockLessQueue = atomic_queue::AtomicQueue2<ArrayBuffer_t, 512>;   // Use heap-allocated buffer.

std::condition_variable m_ConditionVariable;
std::mutex m_WaitMutex;
std::uint64_t writeSequenceNumber{};

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

std::mutex _producerMutex;   /// only one producer can be active at the time, otherwise it is possible that the one producer
                             /// is faster than the other and this will show in the invalid sequence in the consumer, a
                             /// swapped sequence number!

void ProducerThread(LockLessQueue &lockLessQueue)
{

  while (true) {
    std::this_thread::sleep_for(std::chrono::microseconds(1));   // Wait for space in buffer

    // fill the queue
    std::srand(std::time(nullptr));
    const auto maxElements{std::rand() % capacity};
    LOG_INFO("Producing {}  elements", maxElements);
    for (int i = 0; i < maxElements; i++) {
      {
        std::scoped_lock lock(_producerMutex);

        const auto sequenceNumber{writeSequenceNumber++};

        ArrayBuffer_t array;

        const auto str{std::to_string(sequenceNumber)};

        Header header;
        header.m_PayloadSize = str.size();
        header.m_Type = 1;
        header.m_SequenceNumber = sequenceNumber;
        header.m_TimePoint = std::chrono::system_clock::now().time_since_epoch().count();

        // copy header
        std::memcpy(array.data(), &header, sizeof(header));

        // copy payload
        std::memcpy(&array.data()[sizeof(header)], str.c_str(), str.size());

        LOG_INFO("Pushed #seq: {}", header.m_SequenceNumber);

        lockLessQueue.push(array);
        Signal();
      }
    }
  }
}

void ConsumerThread(LockLessQueue &lockLessQueue, const std::function<void(const ArrayBuffer_t &element)> &popFn)
{
  while (true) {
    LOG_INFO("Consumer waiting for data");

    if (Wait()) {
      LOG_INFO("Wakeup for data, queue size:{}", lockLessQueue.was_size());

      while (not lockLessQueue.was_empty()) {   // read everything from the ring buffer
        const ArrayBuffer_t &&array{lockLessQueue.pop()};
        popFn(array);
      }
    }
  }
}

int main(int argc, char **argv)
{
  Logger::GetInstance().SetLogFile("./atomic_queue.log");
  LockLessQueue lockLessQueue;

  LOG_INFO("Lock less queue capacity:{}", lockLessQueue.capacity());

  std::vector<std::jthread> producers;
  for (int i = 0; i < 4; i++) {
    producers.emplace_back(std::jthread([&]() {
      ProducerThread(lockLessQueue);
    }));
  }

  std::jthread consumer{[&]() {
    std::uint64_t readSequenceNumber{};

    const auto popFn{[&](const ArrayBuffer_t &element) {
      // read the header
      Header header;
      std::memcpy(&header, element.data(), sizeof(Header));
      if (header.m_PayloadSize == 0) {
        LOG_ERROR("Payload size is zero");
        return;
      }
      // read the payload
      const auto str{std::string(&element.data()[sizeof(Header)], header.m_PayloadSize)};

      const auto delta{std::chrono::system_clock::now().time_since_epoch().count() - header.m_TimePoint};

      if (readSequenceNumber != header.m_SequenceNumber) {
        LOG_FATAL("Invalid sequence: {} , != {}", readSequenceNumber, header.m_SequenceNumber);
      } else {
        LOG_INFO("Consumed:{}, Seq#:{}, delta:{}", str, header.m_SequenceNumber, delta);
      }
      readSequenceNumber++;
    }};

    ConsumerThread(lockLessQueue, popFn);
  }};

  return 0;
}