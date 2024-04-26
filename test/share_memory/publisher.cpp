#include "shared_memory/publisher.hpp"
#include "common/logger.hpp"
#include <boost/interprocess/mapped_region.hpp>

using namespace boost;

// using namespace moboware;

SharedMemoryPublisher::SharedMemoryPublisher(const std::string &memoryName)
    : SharedMemory(memoryName, SharedMemory::Type::Publisher)
{
  // common::RingBuffer<unsigned char> rb(1024);
}

SharedMemoryPublisher::~SharedMemoryPublisher()
{
  interprocess::shared_memory_object::remove(GetSharedMemoryName().c_str());
}

void SharedMemoryPublisher::Write(const std::string_view &buffer)
{
  interprocess::mapped_region region{*GetSharedMemoryObject(), interprocess::read_write};
  MemoryMapHeader *header{static_cast<MemoryMapHeader *>(region.get_address())};

  if (header) {

    _log_debug(LOG_DETAILS,
               "Region size:{}, Buffer size:{}, Read offset:{}, Write offset:{}",
               region.get_size(),
               header->m_MaxPayloadBufferSize,
               header->m_ReadOffset,
               header->m_WriteOffset);

    // check if there is enough space for writing the msg, write prt + size < buffer size
    // otherwise reset the write prt the the start of the buffer ==> [0]
    if ((header->m_WriteOffset == header->m_ReadOffset) or   //
        (header->m_WriteOffset + sizeof(MemoryMsg) + buffer.size() > header->m_MaxPayloadBufferSize)) {
      _log_info(LOG_DETAILS, "Resetting....");
      // interprocess::scoped_lock memoryLock(m_MemoryMutex);
      header->m_WriteOffset = header->m_ReadOffset = 0;
    }

    // map the message over the start point where we need to write from
    MemoryMsg *msg{reinterpret_cast<MemoryMsg *>(&header->m_Payload[header->m_WriteOffset])};
    if (msg and buffer.size() < header->m_MaxPayloadBufferSize) {

      msg->m_Size = buffer.size();
      memcpy(msg->m_MsgData, buffer.data(), buffer.size());

      // move the write prt to end of the msg, the next free memory part.
      header->m_WriteOffset += sizeof(MemoryMsg) + buffer.size();

      _log_debug(LOG_DETAILS, "Msg size:{}, msg:{}, WriteOffset:{}", msg->m_Size, buffer, header->m_WriteOffset);
    }
  }
}
