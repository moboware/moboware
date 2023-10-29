#include "shared_memory/shared_memory.hpp"
#include "common/log_stream.h"
#include <boost/interprocess/mapped_region.hpp>

using namespace boost;

SharedMemory::SharedMemory(const std::string &memoryName, const Type type, const boost::interprocess::offset_t size)
    : m_SharedMemoryName(memoryName)
    , m_SemaphoreName(memoryName + std::string("_Semaphore"))
    //, m_MemoryMutex(interprocess::open_or_create, std::string(memoryName + std::string("_Mutex")).c_str())
    , m_InterprocessNamedSemaphore(interprocess::open_or_create, m_SemaphoreName.c_str(), 1)
{
  if (type == Type::Publisher) {
    interprocess::shared_memory_object::remove(m_SharedMemoryName.c_str());
  }

  m_SharedMemory = std::make_unique<SharedMemoryObject_t>(interprocess::open_or_create,
                                                          m_SharedMemoryName.c_str(),
                                                          interprocess::read_write);

  // create new mem buffer
  m_SharedMemory->truncate(size);
  if (type == Type::Publisher) {
    // create region
    interprocess::mapped_region region{*GetSharedMemoryObject(), interprocess::read_write};
    MemoryMapHeader *header = static_cast<MemoryMapHeader *>(region.get_address());
    // set the size of the buffer in the shared memory

    header->m_MaxPayloadBufferSize = size - sizeof(MemoryMapHeader);
    header->m_ReadOffset = header->m_WriteOffset = 0;
  }

  LOG_INFO("Created shared memory object:" << m_SharedMemory->get_name() << ", size:" << GetSize());
}

SharedMemory::~SharedMemory()
{
  m_InterprocessNamedSemaphore.remove(m_SemaphoreName.c_str());
}

bool SharedMemory::Wait(const std::chrono::nanoseconds &waitTime)
{
  return m_InterprocessNamedSemaphore.timed_wait(std::chrono::system_clock::now() + waitTime);
}

void SharedMemory::Notify()
{
  m_InterprocessNamedSemaphore.post();
}
