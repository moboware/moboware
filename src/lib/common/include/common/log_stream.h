#pragma once

#include "common/singleton.h"
#include <condition_variable>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <streambuf>

namespace logstrm {
struct EndOfLine
{};
const EndOfLine endl;

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
  SetPrecision(std::streamsize n)
    : mPrecision(n)
  {
  }

  std::streamsize mPrecision;
};

inline SetPrecision setprecision(std::streamsize __n)
{
  return SetPrecision(__n);
}
} // namespace logstrm

// @brief LogStreamBuf is een implementatie class voor de std::streambuf en
// heeft
//        een log buffer waarin alle log messages worden gestreamed
class LogStreamBuf : public std::streambuf
{
public:
  static const size_t mLogBuffersize = 64 * 1024;
  LogStreamBuf();
  ~LogStreamBuf() noexcept = default;
  LogStreamBuf(const LogStreamBuf&) = delete;
  LogStreamBuf& operator=(const LogStreamBuf&) = delete;

  LogStreamBuf(LogStreamBuf&&) = delete;
  LogStreamBuf& operator=(LogStreamBuf&&) = delete;
  bool operator==(const LogStreamBuf&) const = delete;

  void Reset();

  inline const char* GetBuffer() const { return mLogBuffer; }

private:
  char mLogBuffer[mLogBuffersize]{};
};

// @brief
class LogStream
  : public std::ostream
  , public Singleton<LogStream>
{
public:
  LogStream();
  ~LogStream() noexcept = default;

  LogStream(const LogStream&) = delete;
  LogStream& operator=(const LogStream&) const = delete;
  LogStream(LogStream&&) = delete;
  LogStream& operator=(LogStream&&) const = delete;

  bool SetLogFile(const std::string& logFileName);

  enum LEVEL
  {
    NONE,
    DEBUG,
    INFO,
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
  const char* GetLevelString() const;
  inline std::mutex& GetMutex() { return mMutex; };
  template<typename TObject>
  friend LogStream& operator<<(LogStream& os, const TObject& obj);

  friend LogStream& operator<<(LogStream& os, const logstrm::EndOfLine& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Hex& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Dec& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::Oct& obj);
  friend LogStream& operator<<(LogStream& os, const logstrm::SetPrecision prec);

  /*  inline std::ios_base::fmtflags setf ( std::ios_base::fmtflags fmtfl,
    std::ios_base::fmtflags mask )
    {
      return std::ostream::setf( fmtfl, mask );
    }
  */
  void Flush();

private:
  void WriteToStream(std::ostream& outStream);

  std::mutex mMutex;
  LogStreamBuf mLogStreamBuf;
  LEVEL mGlobalLogLevel = DEBUG;
  LEVEL mLogLevel = DEBUG;
  std::string mFunction;
  std::string mFile;
  size_t mLineNumber = 0;
  std::ofstream mOutStream;
};
//}//EON

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
#define LOG_STREAM(level, function, file, line, log_line)     \
{                                                             \
  LogStream& logStream = LogStream::GetInstance();            \
  std::lock_guard<std::mutex> lck(logStream.GetMutex());      \
  if(logStream.TestLevel(level, function, file, line))        \
  {                                                           \
      logStream << log_line << logstrm::endl;                 \
  }                                                           \
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
#define LOG_ERROR(log_line) LOG_STREAM(LogStream::ERROR, __FUNCTION__, __FILE__, __LINE__, log_line)
#define LOG_FATAL(log_line) LOG_STREAM(LogStream::FATAL, __FUNCTION__, __FILE__, __LINE__, log_line)