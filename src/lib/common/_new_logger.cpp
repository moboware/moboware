#include "common/new_logger.h"

using namespace moboware::common;

NewLogger::NewLogger()
{
  // create thread that reads the logline from the lock free queue
  const auto threadFunction{[&](const std::stop_token &stop_token) {
    while (not stop_token.stop_requested()) {
      WaitAndWrite();
    }
  }};

  m_LogWriterThread = std::jthread(threadFunction);
}

bool NewLogger::SetLogFile(const std::filesystem::path &logFileName)
{
  if (m_LogFileStream.is_open()) {
    m_LogFileStream.close();
  }

  m_LogFileStream.open(logFileName.c_str(), std::ios_base::out | std::ios_base::app);

  if (m_LogFileStream.is_open()) {
    // LOG_INFO("Start new log file:" << logFileName);
    return true;
  }
  return false;
}

NewLogStream::LEVEL NewLogger::GetLevel(const std::string &levelStr)
{
  static std::map<std::string, NewLogStream::LEVEL> levels{
      {"",      NewLogStream::LEVEL::NONE }, //
      {"TRACE", NewLogStream::LEVEL::TRACE}, //
      {"DEBUG", NewLogStream::LEVEL::DEBUG}, //
      {"INFO",  NewLogStream::LEVEL::INFO }, //
      {"WARN",  NewLogStream::LEVEL::WARN }, //
      {"ERROR", NewLogStream::LEVEL::ERROR}, //
      {"FATAL", NewLogStream::LEVEL::FATAL}  //
  };
  const auto iter{levels.find(levelStr)};

  if (iter != std::end(levels)) {
    return iter->second;
  }
  return NewLogStream::LEVEL::INFO;
}

bool NewLogger::TestLevel(const NewLogStream::LEVEL level)
{
  if (m_GlobalLogLevel != NewLogStream::LEVEL::NONE &&   //
      level >= m_GlobalLogLevel) {
    return true;
  }

  return false;
}

void NewLogger::WaitAndWrite()
{
  // main thread function to wait for events and write to an out stream
  if (m_LockLessRingBuffer.Wait()) {
    while (not m_LockLessRingBuffer.Empty()) {
      // read until empty
      const auto popFn{[&](const moboware::common::NewLogStream &logStream) {
        if (m_LogFileStream.is_open()) {
          logStream.WriteToStream(m_LogFileStream);
        } else {
          logStream.WriteToStream(std::cout);
        }
      }};
      m_LockLessRingBuffer.Pop(popFn);
    }
  }
}

bool NewLogger::LogLine(
    const std::function<bool(moboware::common::NewLogStream &, const QueueLengthType_t &headPosition)> &logFn)
{
  if (m_LockLessRingBuffer.Push(logFn)) {
    m_LockLessRingBuffer.Signal(false);
    return true;
  }
  return false;
}
