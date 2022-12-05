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
  // afhankelijk van de lengte van de buffer wordt de memset uitgevoerd
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

  const auto threadFunction{ [&](const std::stop_token& stop_token) {
    while (not stop_token.stop_requested()) {

      if (m_ThreadWaitCondition.try_acquire_for(std::chrono::milliseconds(250))) {
        while (mLogStreamBuf.Size()) {
          FlushToStream();
        }
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
  if (mGlobalLogLevel != NONE && level >= mGlobalLogLevel) {
    // split the file name of the module
    mFunction = function;
    mFile = fileName.filename();
    mLineNumber = lineNumber;
    mLogLevel = level;
    return true;
  }

  return false;
}

void LogStream::Flush()
{
#if 0
  FlushToStream();
#else
  // ping the thread to flush
  m_ThreadWaitCondition.release();
#endif
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
  std::lock_guard<std::mutex> lck(mMutex);

  if (not mLogStreamBuf.Empty()) {
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
    { "ERROR", LEVEL::ERROR }, //
    { "FATAL", LEVEL::FATAL }  //
  };
  const auto iter{ levels.find(levelStr) };

  if (iter != std::end(levels)) {
    return iter->second;
  }
  return LEVEL::INFO;
}

const char* LogStream::GetLevelString() const
{
  switch (mLogLevel) {
    case NONE:
      return "";
      break;
    case DEBUG:
      return "DEBUG";
      break;
    case INFO:
      return "INFO ";
      break;
    case ERROR:
      return "ERROR";
      break;
    case FATAL:
      return "FATAL";
      break;
  }
  return "";
}

LogStream& operator<<(LogStream& os, const logstrm::StartOfLine&)
{
  timeval tv{};
  gettimeofday(&tv, nullptr);

  struct tm* timeinfo = localtime(&tv.tv_sec);
  os << "[" << std::setfill('0') << std::dec                 //
     << std::setw(4) << timeinfo->tm_year + 1900 << "-"      //
     << std::setw(2) << timeinfo->tm_mon + 1 << "-"          //
     << std::setw(2) << timeinfo->tm_mday << ","             //
     << std::setw(2) << timeinfo->tm_hour << ":"             //
     << std::setw(2) << timeinfo->tm_min << ":"              //
     << std::setw(2) << timeinfo->tm_sec << "."              //
     << std::setw(6) << tv.tv_usec << "]"                    //
     << "[" << os.GetLevelString() << "]"                    //
     << "[" << std::hex << std::this_thread::get_id() << "]" //
     << "[" << os.GetFile() << "," << std::dec << os.GetLineNumber() << "]";
  return os;
}

LogStream& operator<<(LogStream& os, const logstrm::EndOfLine&)
{
  // flush here....
  os << "\n";
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
