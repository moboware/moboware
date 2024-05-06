#pragma once

#include "common/lock_less_ring_buffer.h"
#include "common/singleton.h"
#include <boost/lockfree/queue.hpp>
#include <chrono>
#include <filesystem>
#include <fmt/chrono.h>
#include <fmt/compile.h>
#include <fmt/core.h>
#include <fmt/format.h>
#include <fmt/ostream.h>
#include <iostream>
#include <map>
#include <thread>

/**
 * @brief TrivialBuffer is a simple buffer that complies with the trivial rules, has a default destructor to comply e.g.
 *          Has an array to hold a fixed size buffer
 * @tparam T
 * @tparam buffer_size, default buffer length of the array
 */
template <typename T, std::size_t buffer_size = 10 * 1024>   //
class TrivialBuffer {
public:
  TrivialBuffer() = default;
  ~TrivialBuffer() = default;

  using value_type = T;
  using reference = T &;
  using const_reference = const T &;
  using iterator = typename std::vector<T>::iterator;
  using const_iterator = typename std::vector<T>::const_iterator;

  void push_back(const T &value)
  {
    if (_index < buffer_size) {
      _array[_index++] = value;
    }
  }

  const T *data() const
  {
    return _array.data();
  }

  std::size_t size() const
  {
    return _index;
  }

  void clear()
  {
    _index = 0;
  }

private:
  std::array<T, buffer_size> _array;
  std::size_t _index{};
};

struct LogLineDetails {
  const std::filesystem::path file{};
  std::size_t line{};
  const char *function{};
};

// returns a LogLineDetails struct, used to get the current log line details
#define LOG_DETAILS                                                                                                         \
  {                                                                                                                         \
    __FILE__, __LINE__, __FUNCTION__                                                                                        \
  }

/**
 * @brief
 */
class Logger : public moboware::common::Singleton<Logger> {
public:
  static const auto MaxLogLineLength{5 * 1024};
  using LogBuffer_t = TrivialBuffer<char, MaxLogLineLength>;
  static const auto LogQueueLength{10 * 1'024u};   // max length the queue
  using LogQueue_t = boost::lockfree::queue<LogBuffer_t>;

  enum class LogLevel : std::uint8_t {
    None = 0,
    Trace,
    Debug,
    Info,
    Warning,
    Error,
    Fatal
  };

  Logger()
  {
    m_LogQueue.reserve(LogQueueLength);

    // create thread that reads the logline from the lock free queue
    const auto threadFunction{[&](const std::stop_token &stop_token) {
      while (not stop_token.stop_requested()) {
        WaitAndWrite();
      }
    }};

    m_LogConsumerThread = std::jthread(threadFunction);
  }

  ~Logger()
  {
    m_LogConsumerThread.request_stop();
  }

  Logger(const Logger &) = delete;
  Logger(Logger &&) = delete;
  Logger &operator=(const Logger &) = delete;
  Logger &operator=(Logger &&) = delete;

  void _log(const LogLevel level,                //
            const std::filesystem::path &file,   //
            const std::uint32_t lineNumber,      //
            fmt::string_view format,             //
            fmt::format_args args)               //
  {
    // format the pre log line
    const auto time{GetNowString()};
    const auto id{pthread_self()};

    LogBuffer_t buffer;
    vformat_to(std::back_insert_iterator(buffer),
               "[{}][{}][{:#x}][{}:{}]",
               fmt::make_format_args(          //
                   time,                       //
                   level,                      //
                   id,                         //
                   file.filename().native(),   //
                   lineNumber));
    // format the log line
    vformat_to(std::back_insert_iterator(buffer), format, args);
    vformat_to(std::back_insert_iterator(buffer), "{}", fmt::make_format_args("\n"));

    if (not m_LogQueue.bounded_push(buffer)) {   // failed to push due to queue full, wait and retry
      bool done{false};
      while (not done) {
        std::cout << "Log Queue full!!!" << std::endl;
        Signal(true);

        std::this_thread::sleep_for(std::chrono::microseconds(50));

        if (m_LogQueue.push(buffer)) {
          done = true;
        }
      }
    }

    Signal(false);
  }

  inline void SetLevel(const Logger::LogLevel level)
  {
    m_GlobalLogLevel = level;
  }

  inline Logger::LogLevel GetLevel(const std::string &levelStr)
  {
    static std::map<std::string, Logger::LogLevel> levels{
        {"",      LogLevel::None   }, //
        {"TRACE", LogLevel::Trace  }, //
        {"DEBUG", LogLevel::Debug  }, //
        {"INFO",  LogLevel::Info   }, //
        {"WARN",  LogLevel::Warning}, //
        {"ERROR", LogLevel::Error  }, //
        {"FATAL", LogLevel::Fatal  }  //
    };
    const auto iter{levels.find(levelStr)};

    if (iter != std::end(levels)) {
      return iter->second;
    }
    return Logger::LogLevel::Info;
  }

  bool SetLogFile(const std::filesystem::path &logFileName)
  {
    if (m_LogFileStream.is_open()) {
      m_LogFileStream.close();
    }

    m_LogFileStream.open(logFileName.c_str(), std::ios_base::out | std::ios_base::app);

    if (m_LogFileStream.is_open()) {
      return true;
    }
    return false;
  }

  [[nodiscard]] inline bool TestLevel(const Logger::LogLevel level) const
  {
    if (m_GlobalLogLevel != Logger::LogLevel::None &&   //
        level >= m_GlobalLogLevel) {
      return true;
    }

    return false;
  }

  void WaitAndWrite()
  {
    // main thread function to wait for events and write to an out stream
    if (Wait() and not m_LogQueue.empty()) {
      // read until empty
      const auto popFn{[&](const LogBuffer_t &buffer) {
        if (m_LogFileStream.is_open()) {
          m_LogFileStream.write(buffer.data(), buffer.size());
        } else {
          std::cout.write(buffer.data(), buffer.size());
        }
      }};
      m_LogQueue.consume_all(popFn);
    }
  }

private:
  void Signal(const bool tickleThread = true)
  {
    m_ConditionVariable.notify_one();
    if (tickleThread) {   // sleep for zero nano sec will 'wakeup' the consumer thread faster
      struct ::timespec ts = {0, 0};
      ::nanosleep(&ts, &ts);
    }
  }

  [[nodiscard]] bool Wait(const std::chrono::milliseconds &waitPeriod = std::chrono::milliseconds(100))
  {
    std::unique_lock lock(m_WaitMutex);
    return not(m_ConditionVariable.wait_for(lock, waitPeriod) == std::cv_status::timeout);
  }

  [[nodiscard]] inline std::string GetNowString()
  {
    const auto point{std::chrono::system_clock::now()};
    return fmt::format("{:%Y%m%d %H:%M:%S}", point);
  }

  LogQueue_t m_LogQueue;
  std::ofstream m_LogFileStream{};

  std::jthread m_LogConsumerThread;
  std::condition_variable m_ConditionVariable;
  std::mutex m_WaitMutex;

  LogLevel m_GlobalLogLevel{LogLevel::Info};
};

// user defined log level formatter
template <> struct fmt::formatter<Logger::LogLevel> : formatter<string_view> {
  // parse is inherited from formatter<string_view>.
  auto format(Logger::LogLevel l, format_context &ctx) const;
};

inline auto fmt::formatter<Logger::LogLevel>::format(Logger::LogLevel l, fmt::format_context &ctx) const
{
  string_view name = "unknown";
  switch (l) {
  case Logger::LogLevel::Info:
    name = "INFO";
    break;
  case Logger::LogLevel::Debug:
    name = "DEBUG";
    break;
  case Logger::LogLevel::Trace:
    name = "TRACE";
    break;
  case Logger::LogLevel::Error:
    name = "ERROR";
    break;
  case Logger::LogLevel::Warning:
    name = "WARN";
    break;
    case Logger::LogLevel::Fatal:
    name = "FATAL";
    break;
    case Logger::LogLevel::None:
    name = "NONE";
    break;
  }
  return formatter<string_view>::format(name, ctx);
}

// Logger _Logger;

template <typename... T>
void _log(const Logger::LogLevel level, const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(level)) {
    Logger::GetInstance()._log(level, logDetails.file, logDetails.line, format, fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_trace(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Trace)) {
    Logger::GetInstance()._log(Logger::LogLevel::Trace,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_debug(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Debug)) {
    Logger::GetInstance()._log(Logger::LogLevel::Debug,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_info(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Info)) {
    Logger::GetInstance()._log(Logger::LogLevel::Info,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_warning(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Warning)) {
    Logger::GetInstance()._log(Logger::LogLevel::Warning,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_error(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Error)) {
    Logger::GetInstance()._log(Logger::LogLevel::Error,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

template <typename... T> void _log_fatal(const LogLineDetails &logDetails, fmt::format_string<T...> format, T &&...args)
{
  if (Logger::GetInstance().TestLevel(Logger::LogLevel::Fatal)) {
    Logger::GetInstance()._log(Logger::LogLevel::Fatal,
                               logDetails.file,
                               logDetails.line,
                               format,
                               fmt::make_format_args(args...));
  }
}

//}   // namespace moboware::common::logger