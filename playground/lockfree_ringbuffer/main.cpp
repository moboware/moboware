#include <array>
#include <atomic>
#include <chrono>
#include <condition_variable>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

template <typename T, size_t Capacity> class LockFreeRingBuffer {
public:
  LockFreeRingBuffer()
  {
  }

  bool enqueue(T &&item)
  {
    // push element on the queue
    const size_t head_snapshot{head.load(std::memory_order_acquire)};
    const size_t next_head{(head_snapshot + 1) % Capacity};

    if (next_head == tail.load(std::memory_order_acquire)) {
      return false;   // Buffer full
    }

    buffer[head_snapshot] = std::move(item);
    // update the head pointer
    head.store(next_head, std::memory_order_release);

    return true;
  }

  bool dequeue(T &item)
  {
    // pop/pull element from the queue
    const size_t tail_snapshot{tail.load(std::memory_order_acquire)};

    if (tail_snapshot == head.load(std::memory_order_acquire)) {
      return false;   // Buffer empty
    }

    item = buffer[tail_snapshot];
    // update the tail pointer
    tail.store((tail_snapshot + 1) % Capacity, std::memory_order_release);

    return true;
  }

  std::size_t size() const
  {
    const auto head_snapshot = head.load(std::memory_order_acquire);
    const auto tail_snapshot = tail.load(std::memory_order_acquire);
    return (head_snapshot - tail_snapshot + Capacity) % Capacity;
  }

  void signal()
  {
    cv.notify_all();
  }

  bool wait()
  {
    std::unique_lock lock(mutex);
    return not(cv.wait_for(lock, std::chrono::seconds(20)) == std::cv_status::timeout);
  }

private:
  std::array<T, Capacity> buffer{};
  std::atomic<size_t> head{};
  std::atomic<size_t> tail{};
  std::condition_variable cv;
  std::mutex mutex;
};

int main(int, char **)
{
  const auto capacity{5000u};
  LockFreeRingBuffer<int, capacity> buffer;

  // producer thread
  std::thread producer([&]() {
    int n{};
    while (true) {
      // fill the queue
      for (int i = 0; i < std::rand() % capacity; i++) {
        if (buffer.enqueue(n++)) {

          std::cout << "Produced: " << n << ", size:" << buffer.size() << std::endl;
        } else {
          std::cout << "Produced waiting for space " << std::endl;
          std::this_thread::sleep_for(std::chrono::milliseconds(80));   // Wait for space in buffer
        }
        buffer.signal();
      }
    }
  });

  // consumer thread
  std::thread consumer([&]() {
    while (true) {
      std::cout << "Consumer waiting for data size:" << buffer.size() << std::endl;
      buffer.wait();

      int item{};
      while (buffer.dequeue(item)) {
        std::cout << "Consumed: " << item << ", size:" << buffer.size() << std::endl;
      }
    }
  });

  producer.join();
  consumer.join();

  return 0;
}
