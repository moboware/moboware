#include "common/lock_less_ring_buffer.h"
//#include "common/log_stream.h"
#include <atomic_queue/atomic_queue.h>
#include <chrono>
#include <cstring>
#include <iostream>
#include <thread>

using namespace moboware;

#pragma pack(push)

struct Header {
  std::uint64_t m_PayloadSize{};
  std::uint8_t m_Type{};
  std::uint64_t m_SequenceNumber{};
  std::uint64_t m_Timepoint{};
};

#pragma pack(pop)

const auto capacity{5000u};
const auto length{25u};
using ArrayBuffer_t = std::array<char, capacity>;
using LockLessRingBuffer_t = common::LockLessRingBuffer<ArrayBuffer_t, length>;
std::mutex producerMutex;
std::uint64_t producerSequenceNumber{};

void producerThreadFunction(LockLessRingBuffer_t &buffer)
{
  {
    // std::scoped_lock lock(producerMutex);

    const auto pushFn{
        [&](LockLessRingBuffer_t::Buffer_t &element, const moboware::common::QueueLengthType_t &headPosition) {   //
          const auto str{std::to_string(producerSequenceNumber)};

          Header header;
          header.m_PayloadSize = str.size();
          header.m_Type = 1;
          header.m_SequenceNumber = producerSequenceNumber;
          header.m_Timepoint = std::chrono::system_clock::now().time_since_epoch().count();

          // copy header
          std::memcpy(element.data(), &header, sizeof(header));

          // copy payload
          std::memcpy(&element.data()[sizeof(header)], str.c_str(), str.size());

          std::cout << "Pushed #seq:" << header.m_SequenceNumber << ", Head position:" << headPosition << std::endl;

          producerSequenceNumber++;
          return true;
        }};

    while (true) {
      // fill the queue
      for (int i = 0; i < std::rand() % capacity; i++) {
        if (buffer.Push(pushFn)) {
          std::cout << "Produced: " << producerSequenceNumber - 1 << "," << i << ", used:" << buffer.Size() << "/"
                    << buffer.Capacity() << std::endl;
          buffer.Signal();

        } else {
          std::cout << "Producer no space! "
                    << " #seq:" << producerSequenceNumber - 1 << " Used: " << buffer.Size() << "/" << buffer.Capacity()
                    << std::endl;
          buffer.Signal();

          std::this_thread::sleep_for(std::chrono::milliseconds(10));   // Wait for space in buffer
        }
      }
    }
  }
}

int main(int, char **)
{

  LockLessRingBuffer_t buffer;

  // producer thread
  std::vector<std::jthread> producers;
  for (int i = 0; i < 1; i++) {
    producers.emplace_back(std::jthread([&]() {
      producerThreadFunction(buffer);
    }));
  }

  // consumer thread
  std::jthread consumer([&]() {
    std::uint64_t sequenceNumber{};

    const auto popFn{[&](const LockLessRingBuffer_t::Buffer_t &element) {
      // read the header
      Header header;
      std::memcpy(&header, element.data(), sizeof(Header));
      if (header.m_PayloadSize == 0) {
        std::cout << "Payload size is zero" << std::endl;
        return;
      }
      // read the payload
      const auto str{std::string(&element.data()[sizeof(Header)], header.m_PayloadSize)};

      const auto delta{std::chrono::system_clock::now().time_since_epoch().count() - header.m_Timepoint};

      if (sequenceNumber++ != header.m_SequenceNumber) {
        std::cout << "Invalid sequence:" << sequenceNumber - 1 << ", != " << header.m_SequenceNumber << std::endl;
      } else {
        std::cout << "Consumed: " << str << ", Seq#:" << header.m_SequenceNumber << ", delta:" << delta << std::endl;
      }
    }};

    while (true) {
      std::cout << "Consumer waiting for data" << std::endl;

      if (buffer.Wait()) {
        std::cout << "Wakeup for data, queue size: " << buffer.Size() << std::endl;

        while (not buffer.Empty()) {   // read everything from the ring buffer
          std::cout << "Queue size: " << buffer.Size() << std::endl;
          buffer.Pop(popFn);
        }
        //        std::this_thread::sleep_for(std::chrono::milliseconds(5000));   // slow down reading....
      }
    }
  });

  return 0;
}
