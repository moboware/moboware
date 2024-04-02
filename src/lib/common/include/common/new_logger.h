#pragma once
#include "common/lock_less_ring_buffer.h"
#include "common/singleton.h"
#include "new_log_stream.h"
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>

namespace moboware::common {

/**
 * @brief
 */
class NewLogger : public moboware::common::Singleton<NewLogger> {
public:
  using LogStreamBuffer_t = moboware::common::NewLogStream;

  static const std::size_t length{512U};   // default length of the log message queue
  static constexpr bool usePushPopLock{true};
  using LockLessRingBuffer_t = common::LockLessRingBuffer<LogStreamBuffer_t, length, usePushPopLock>;

  NewLogger();
  NewLogger(const NewLogger &) = delete;
  NewLogger(NewLogger &&) = delete;
  NewLogger &operator=(const NewLogger &) = delete;
  NewLogger &operator=(NewLogger &&) = delete;
  ~NewLogger() = default;

  bool SetLogFile(const std::filesystem::path &logFileName);

  inline void SetLevel(const NewLogStream::LEVEL level)
  {
    m_GlobalLogLevel = level;
  }

  bool LogLine(const std::function<bool(NewLogStream &, const QueueLengthType_t &headPosition)> &logFn);

  bool TestLevel(const NewLogStream::LEVEL level);

  NewLogStream::LEVEL GetLevel(const std::string &levelStr);

private:
  void WaitAndWrite();

  NewLogStream::LEVEL m_GlobalLogLevel{NewLogStream::LEVEL::DEBUG};
  std::ofstream m_LogFileStream{};

  LockLessRingBuffer_t m_LockLessRingBuffer;
  std::jthread m_LogWriterThread;
};
}   // namespace moboware::common
