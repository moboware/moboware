#include "common/log_stream.h"
#include <cstdio>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <sys/time.h>
#include <thread>

LogStreamBuf::LogStreamBuf()
{
  Reset();
}

void LogStreamBuf::Reset()
{
  // afhankelijk van de lengte van de buffer wordt de memset uitgevoerd
  const char* startOfBuffer = &mLogBuffer[0];
  const size_t bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : mLogBuffersize);
  memset(mLogBuffer, 0, bufSize);

  setp(mLogBuffer, mLogBuffer + mLogBuffersize);
}

//////////////////////////////////////////

LogStream::LogStream()
  : std::ostream(&mLogStreamBuf)
  , Singleton<LogStream>()
{
  rdbuf(&mLogStreamBuf);
}

bool LogStream::SetLogFile(const std::string& logFileName)
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
  if (mOutStream.is_open()) {
    WriteToStream(mOutStream);
  } else {
    WriteToStream(std::cout);
  }
}

void LogStream::WriteToStream(std::ostream& outStream)
{
  timeval tv{};
  gettimeofday(&tv, nullptr);

  struct tm* timeinfo = localtime(&tv.tv_sec);
  outStream << "[" << std::setfill('0') << std::dec            //
            << std::setw(4) << timeinfo->tm_year + 1900 << "-" //
            << std::setw(2) << timeinfo->tm_mon + 1 << "-"     //
            << std::setw(2) << timeinfo->tm_mday << ","        //
            << std::setw(2) << timeinfo->tm_hour << ":"        //
            << std::setw(2) << timeinfo->tm_min << ":"         //
            << std::setw(2) << timeinfo->tm_sec << "."         //
            << std::setw(6) << tv.tv_usec << "]"               //
            << "[" << GetLevelString() << "]"
            << "[" << std::hex << std::this_thread::get_id() << "]"
            << "[" << mFile << "," << std::dec << mLineNumber << "]" //
            << mLogStreamBuf.GetBuffer() << std::endl;
  mLogStreamBuf.Reset();
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

//}//EON

LogStream& operator<<(LogStream& os, const logstrm::EndOfLine&)
{
  // flush here....
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
