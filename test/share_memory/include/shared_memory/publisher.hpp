#pragma once
#include "shared_memory.hpp"

class SharedMemoryPublisher : public SharedMemory {
public:
  explicit SharedMemoryPublisher(const std::string &memoryName);
  ~SharedMemoryPublisher();

  void Write(const std::string_view &buffer);
};