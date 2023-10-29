#pragma once
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/sync/named_mutex.hpp>
#include <boost/interprocess/sync/named_semaphore.hpp>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <chrono>
#include <memory>

class SharedMemory {
public:
  enum Type : std::uint8_t {
    Publisher,
    Consumer
  };

#pragma pack(push, 1)

  // struct to map a message
  struct MemoryMsg {
    std::uint64_t m_Size{};
    std::byte m_MsgData[];
  };

  struct MemoryMapHeader {
    std::uint64_t m_MaxPayloadBufferSize{};
    std::uint64_t m_WriteOffset{};
    std::uint64_t m_ReadOffset{};
    std::byte m_Payload[];
  };

#pragma pack(pop)

  using SharedMemoryObject_t = boost::interprocess::shared_memory_object;
  using SharedMemoryObjectPtr = std::unique_ptr<SharedMemoryObject_t>;

  explicit SharedMemory(const std::string &memoryName,
                        const Type type,
                        const boost::interprocess::offset_t size = 10 * 1024 * 1024);   // 10 MByte default memory
  ~SharedMemory();

  inline boost::interprocess::offset_t GetSize() const
  {
    boost::interprocess::offset_t size{};
    m_SharedMemory->get_size(size);
    return size;
  }

  const SharedMemoryObjectPtr &GetSharedMemoryObject() const
  {
    return m_SharedMemory;
  }

  const std::string &GetSharedMemoryName() const
  {
    return m_SharedMemoryName;
  }

  bool Wait(const std::chrono::nanoseconds &waitTime);
  void Notify();

protected:
  //  boost::interprocess::named_mutex m_MemoryMutex;

private:
  const std::string m_SharedMemoryName;
  const std::string m_SemaphoreName;
  SharedMemoryObjectPtr m_SharedMemory;

  boost::interprocess::named_semaphore m_InterprocessNamedSemaphore;   // named mutex, used to notify consumers
};
