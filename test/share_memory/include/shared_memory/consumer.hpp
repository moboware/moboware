#pragma once

#include "shared_memory.hpp"

class SharedMemoryConsumer : public SharedMemory{
public:
  explicit SharedMemoryConsumer(const std::string& memoryName);
  ~SharedMemoryConsumer() = default;

  std::size_t Read(char *buffer, std::size_t bufferSize, const std::chrono::milliseconds&waitTime);
};