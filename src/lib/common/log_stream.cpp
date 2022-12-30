#include "common/log_stream.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <map>
#include <sys/time.h>

LogStreamBuf::LogStreamBuf()
{
  Reset();
}

auto LogStreamBuf::Size() -> std::streamsize
{
  const char* startOfBuffer = &mLogBuffer[0];
  const auto bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : LogBuffersize);
  return bufSize;
}

void LogStreamBuf::Reset()
{
  // depending on the length of the buffer as memset is executed
  const char* startOfBuffer = &mLogBuffer[0];
  const size_t bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : LogBuffersize);
  memset(mLogBuffer, 0, bufSize);

  setp(mLogBuffer, mLogBuffer + LogBuffersize);
}

//////////////////////////////////////////

LogStream::LogStream()
  : std::ostream(&mLogStreamBuf)
  , Singleton<LogStream>()
{
  rdbuf(&mLogStreamBuf);
  // log stream thread setup...
  const auto threadFunction{ [&](const std::stop_token& stop_token) {
    while (not stop_token.stop_requested()) {

      if (m_ThreadWaitCondition.try_acquire_for(std::chrono::milliseconds(250))) {
        while (mLogStreamBuf.Size()) {
          FlushToStream();
        }
        mOutStream.flush();
      }
    }
  } };

  m_Thread = std::jthread(threadFunction);
}

bool LogStream::SetLogFile(const std::filesystem::path& logFileName)
{
  if (mOutStream.is_open()) {
    mOutStream.close();
  }

  mOutStream.open(logFileName.c_str(), std::ios_base::out | std::ios_base::app);
  if (mOutStream.is_open()) {
    LOG_INFO("Start new log file:" << logFileName);
    return true;
  }
  return false;
}

bool LogStream::TestLevel(const LEVEL level, const std::string& function, const std::filesystem::path& fileName, const size_t lineNumber)
{
  if (mGlobalLogLevel != LEVEL::NONE && level >= mGlobalLogLevel) {
    // split the file name of the module
    mLogInfo.mFunction = function;
    mLogInfo.mFile = fileName.filename();
    mLogInfo.mLineNumber = lineNumber;
    mLogInfo.mLogLevel = level;
    return true;
  }

  return false;
}

void LogStream::Flush()
{
  // ping the log thread to flush
  m_ThreadWaitCondition.release();
}

void LogStream::FlushToStream()
{
  if (mOutStream.is_open()) {
    WriteToStream(mOutStream);
  } else {
    WriteToStream(std::cout);
  }
}

void LogStream::WriteToStream(std::ostream& outStream)
{
  // this is called from the thread to write to the logStream
  // lock here to prevent raise conditions with the calling thread.

  if (not mLogStreamBuf.Empty()) {
    std::lock_guard<std::mutex> lck(mMutex);
    outStream << mLogStreamBuf.GetBuffer();

    mLogStreamBuf.Reset();
  }
}

LogStream::LEVEL LogStream::GetLevel(const std::string& levelStr)
{
  static std::map<std::string, LEVEL> levels{
    { "", LEVEL::NONE },       //
    { "DEBUG", LEVEL::DEBUG }, //
    { "INFO", LEVEL::INFO },   //
    { "WARN", LEVEL::WARN },   //
    { "ERROR", LEVEL::ERROR }, //
    { "FATAL", LEVEL::FATAL }  //
  };
  const auto iter{ levels.find(levelStr) };

  if (iter != std::end(levels)) {
    return iter->second;
  }
  return LEVEL::INFO;
}

const std::string_view& LogStream::GetLevelString() const
{
  static const std::string_view _DEBUG{ "DEBUG" };
  static const std::string_view _INFO{ "INFO" };
  static const std::string_view _WARN{ "WARN" };
  static const std::string_view _ERROR{ "ERROR" };
  static const std::string_view _FATAL{ "FATAL" };
  static const std::string_view _NONE{ "" };

  switch (mLogInfo.mLogLevel) {
    case LEVEL::NONE:
      return _NONE;
      break;
    case LEVEL::DEBUG:
      return _DEBUG;
      break;
    case LEVEL::INFO:
      return _INFO;
      break;
    case LEVEL::WARN:
      return _WARN;
      break;
    case LEVEL::ERROR:
      return _ERROR;
      break;
    case LEVEL::FATAL:
      return _FATAL;
      break;
  }
  return _NONE;
}

LogStream& operator<<(LogStream& os, const logstrm::StartOfLine&)
{
  timeval tv{};
  gettimeofday(&tv, nullptr);

  struct tm* timeinfo = localtime(&tv.tv_sec);
  os << "[" << std::setfill('0') << std::dec                          //
     << std::setw(4) << (timeinfo->tm_year + 1900) << "-"             //
     << timeinfo->tm_mon + 1 << "-"                                   //
     << timeinfo->tm_mday << ","                                      //
     << timeinfo->tm_hour << ":"                                      //
     << timeinfo->tm_min << ":"                                       //
     << timeinfo->tm_sec << "."                                       //
     << std::setw(4) << tv.tv_usec                                    //
     << "][" << os.GetLevelString()                                   //
     << "][" << std::hex << std::this_thread::get_id()                //
     << "][" << os.GetFile() << "," << std::dec << os.GetLineNumber() //
     << "]";
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::EndOfLine&)
{
  // flush here....
  os << std::endl;
  os.Flush();

  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::Hex&)
{
  os << std::hex;
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::Dec&)
{
  os << std::dec;
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::Oct&)
{
  os << std::oct;
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::Fixed&)
{
  os << os.fixed;
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::SetPrecision obj)
{
  os.precision(obj.mPrecision);
  return os;
}
