#pragma once

#include "common/singleton.h"
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <semaphore>
#include <streambuf>
#include <thread>

namespace logstrm {
struct EndOfLine
{};
const EndOfLine endl;

struct StartOfLine
{};
const StartOfLine startl;

struct Hex
{};
const Hex hex;

struct Oct
{};
const Oct oct;

struct Dec
{};
const Dec dec;

struct Fixed
{};
const Fixed fixed;

struct SetPrecision
{
  SetPrecision(std::streamsize _n)
    : mPrecision(_n)
  {
  }

  std::streamsize mPrecision;
};

inline SetPrecision setprecision(std::streamsize _n)
{
  return SetPrecision(_n);
}

} // namespace logstrm

// @brief LogStreamBuf is een implementatie class voor de std::streambuf en
// heeft een log buffer waarin alle log messages worden gestreamed
class LogStreamBuf : public std::streambuf
{
public:
  static constexpr auto LogBuffersize{ 1'024U * 1'024U };
  LogStreamBuf();
  LogStreamBuf(const LogStreamBuf&) = delete;
  LogStreamBuf(LogStreamBuf&&) = delete;
  LogStreamBuf& operator=(const LogStreamBuf&) = delete;
  LogStreamBuf& operator=(LogStreamBuf&&) = delete;
  ~LogStreamBuf() noexcept = default;

  bool operator==(const LogStreamBuf&) const = delete;

  void Reset();
  auto Size() -> std::streamsize;
  auto Empty() -> bool { return (Size() == 0); }

  inline const char* GetBuffer() const { return mLogBuffer; }

private:
  char mLogBuffer[LogBuffersize]{};
};

// @brief
class LogStream
  : public std::ostream
  , public Singleton<LogStream>
{
public:
  LogStream();
  LogStream(const LogStream&) = delete;
  LogStream(LogStream&&) = delete;
  LogStream& operator=(const LogStream&) const = delete;
  LogStream& operator=(LogStream&&) const = delete;
  ~LogStream() noexcept = default;

  bool SetLogFile(const std::filesystem::path& logFileName);

  enum LEVEL
  {
    NONE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
  };

  // @brief TestLevel geeft true terug als het globale log level niet NONE
  //        is en globale log level groter is dan het level van de log regel.
  //        E.g. indien het globale log level DEBUG is zal alle log meldingen
  //        die
  //        groter zijn worden gelogged. Log meldingen met log level COUT worden
  //        niet gelogged.
  bool TestLevel(const LEVEL level, const std::string& module, const std::filesystem::path& file, const size_t lineNumber);

  // het log level waar tegen wordt ge-logged
  inline void SetLevel(LEVEL level) { mGlobalLogLevel = level; }
  LogStream::LEVEL GetLevel(const std::string& levelStr);
  const std::string_view& GetLevelString() const;
  auto GetLineNumber() const -> std::size_t { return mLogInfo.mLineNumber; }
  auto GetFile() const -> const std::string& { return mLogInfo.mFile; }
  inline std::mutex& GetMutex() { return mMutex; };

  template<typename TObject>
  friend LogStream& operator<<(LogStream& os, const TObject& obj);

  friend LogStream& operator<<(LogStream& os, const logstrm::StartOfLine& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::EndOfLine& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Hex& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Dec& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Oct& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::SetPrecision prec);
  void Flush();

private:
  void FlushToStream();

  void WriteToStream(std::ostream& outStream);

  std::mutex mMutex{};
  LogStreamBuf mLogStreamBuf{};
  LEVEL mGlobalLogLevel{ DEBUG };

  struct LogInfo
  {
    LEVEL mLogLevel{ DEBUG };
    std::string mFunction{};
    std::string mFile{};
    std::size_t mLineNumber{};
  };

  LogInfo mLogInfo;
  std::ofstream mOutStream{};

  std::jthread m_Thread;
  std::binary_semaphore m_ThreadWaitCondition{ 0 };
};

inline LogStream& operator<<(LogStream& os, const std::string& obj)
{
  static_cast<std::ostream&>(os) << obj;
  return os;
}

template<typename TObject>
LogStream& operator<<(LogStream& os, const TObject& obj)
{
  static_cast<std::ostream&>(os) << obj;
  return os;
}

// clang-format off
#define LOG_STREAM(level, function, file, line, log_line)        \
{                                                                \
  LogStream& logStream = LogStream::GetInstance();               \
  const std::lock_guard<std::mutex> lck(logStream.GetMutex());   \
  if(logStream.TestLevel(level, function, file, line))           \
  {                                                              \
      logStream << logstrm::startl << log_line << logstrm::endl; \
  }                                                              \
}
// clang-format on

// compiler switch __NO_CLOG_DEBUG__ om DEBUG log meldingen uit of aan te
// schakelen.
#ifdef __NO_LOG_DEBUG__
// no cout log
#define LOG_DEBUG(log_line) { // empty
}
#else
#define LOG_DEBUG(log_line) LOG_STREAM(LogStream::DEBUG, __FUNCTION__, __FILE__, __LINE__, log_line)
#endif

#define LOG_INFO(log_line) LOG_STREAM(LogStream::INFO, __FUNCTION__, __FILE__, __LINE__, log_line)
#define LOG_WARN(log_line) LOG_STREAM(LogStream::WARN, __FUNCTION__, __FILE__, __LINE__, log_line)
#define LOG_ERROR(log_line) LOG_STREAM(LogStream::ERROR, __FUNCTION__, __FILE__, __LINE__, log_line)
#define LOG_FATAL(log_line) LOG_STREAM(LogStream::FATAL, __FUNCTION__, __FILE__, __LINE__, log_line)