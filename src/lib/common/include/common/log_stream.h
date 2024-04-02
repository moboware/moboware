#pragma once

#include "common/new_logger.h"
#include <iomanip>
#include <mutex>
#include <semaphore>

using LogStream = moboware::common::NewLogger;

#define LOG_STREAM(level, function, file, lineNumber, logLine)                                                              \
  {                                                                                                                         \
    moboware::common::NewLogger &newLogger{moboware::common::NewLogger::GetInstance()};                                     \
                                                                                                                            \
    if (newLogger.TestLevel(level)) {                                                                                       \
                                                                                                                            \
      const auto logFn{                                                                                                     \
          [&](moboware::common::NewLogStream &logStream, const moboware::common::QueueLengthType_t &headPosition) -> bool { \
            logStream.StartLine(level, function, file, lineNumber);                                                         \
            logStream << logLine << ", head pos:" << headPosition << std::endl;                                             \
            return true;                                                                                                    \
          }};                                                                                                               \
                                                                                                                            \
      newLogger.LogLine(logFn);                                                                                             \
    }                                                                                                                       \
  }

// define __NO_CLOG_TRACE__ switch to TRACE logging on/off
#ifdef __NO_LOG_TRACE__
// no cout log
#define LOG_TRACE(log_line) {   // empty
}
#else
#define LOG_TRACE(log_line)                                                                                                 \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::TRACE, __FUNCTION__, __FILE__, __LINE__, log_line);
#endif

#define LOG(level, log_line) LOG_STREAM(level, __FUNCTION__, __FILE__, __LINE__, log_line);
#define LOG_DEBUG(log_line)                                                                                                 \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::DEBUG, __FUNCTION__, __FILE__, __LINE__, log_line);

#define LOG_INFO(log_line)                                                                                                  \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::INFO, __FUNCTION__, __FILE__, __LINE__, log_line);
#define LOG_WARN(log_line)                                                                                                  \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::WARN, __FUNCTION__, __FILE__, __LINE__, log_line);
#define LOG_ERROR(log_line)                                                                                                 \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::ERROR, __FUNCTION__, __FILE__, __LINE__, log_line);
#define LOG_FATAL(log_line)                                                                                                 \
  LOG_STREAM(moboware::common::NewLogStream::LEVEL::FATAL, __FUNCTION__, __FILE__, __LINE__, log_line);
