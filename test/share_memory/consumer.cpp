#include "shared_memory/consumer.hpp"
#include "common/log_stream.h"
#include <boost/interprocess/mapped_region.hpp>
#include <chrono>

using namespace boost;

SharedMemoryConsumer::SharedMemoryConsumer(const std::string &memoryName)
    : SharedMemory(memoryName, SharedMemory::Type::Consumer)
{
}

std::size_t SharedMemoryConsumer::Read(char *buffer, std::size_t bufferSize, const std::chrono::milliseconds &waitTime)
{
  LOG_INFO("Waiting...")

  if (Wait(waitTime)) {
    interprocess::mapped_region region{*GetSharedMemoryObject(), interprocess::read_write};

    MemoryMapHeader *header{static_cast<MemoryMapHeader *>(region.get_address())};

    if (header) {
      const auto startTime{std::chrono::system_clock::now()};
      std::uint64_t counter{};

      LOG_INFO("Region size:" << region.get_size()                                    //
                              << ", Buffer size:" << header->m_MaxPayloadBufferSize   //
                              << ", Read offset:" << header->m_ReadOffset             //
                              << ", Write offset:" << header->m_WriteOffset);

      if (header->m_ReadOffset != header->m_WriteOffset) {   //  header msg is read prt
        std::string lastMsg;
        do {
          MemoryMsg *msg{reinterpret_cast<MemoryMsg *>(&header->m_Payload[header->m_ReadOffset])};

          if (msg and (msg->m_Size < header->m_MaxPayloadBufferSize)) {
            lastMsg = std::string((const char *)msg->m_MsgData, msg->m_Size);
            // LOG_INFO("Msg size:" << msg->m_Size << ", msg:" << lastMsg );
            //  todo copy data from process memory to internal memory
            //  memcpy(buffer, );

            // interprocess::scoped_lock memoryLock(m_MemoryMutex);
            header->m_ReadOffset += sizeof(MemoryMsg) + msg->m_Size;

          } else {
            return 0;
          }
          counter++;
        } while (header->m_WriteOffset != header->m_ReadOffset);

        const auto endTime{std::chrono::system_clock::now()};
        const auto msgPerSec{
            (double(counter) / std::chrono::duration_cast<std::chrono::nanoseconds>(endTime - startTime).count()) *
            1'000'000'000};
        LOG_INFO("Msg/sec " << std::fixed << msgPerSec << "," << lastMsg);
      }
    }
  }
  return 0;
}
