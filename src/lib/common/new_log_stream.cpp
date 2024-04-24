#include "common/new_log_stream.h"

using namespace moboware::common;

NewLogStream::NewLogStream()
    : std::ostream(&m_LogStreamLine)
{
}

std::string NewLogStream::GetTimeString()
{
  const auto point{std::chrono::system_clock::now()};
  const auto time = std::chrono::system_clock::to_time_t(point);
  const std::tm *tm = std::localtime(&time);

  char buffer[52];
  std::strftime(buffer, sizeof(buffer), "%Y-%m-%d %H:%M:%S.", tm);

  const auto duration = point.time_since_epoch();
  const auto totalSeconds = std::chrono::duration_cast<std::chrono::seconds>(duration);
  const auto remainingNanoSeconds = std::chrono::duration_cast<std::chrono::nanoseconds>(duration - totalSeconds);

  std::stringstream stream;
  stream << buffer << std::setfill('0') << std::dec << std::setw(9) << remainingNanoSeconds.count();

  return stream.str();
}

NewLogStream::LogStreamLine_t &NewLogStream::GetLogStreamLine() const
{
  return m_LogStreamLine;
}

void NewLogStream::StartLine(const NewLogStream::LEVEL level,
                             const std::string &function,
                             const std::filesystem::path &fileName,
                             const size_t lineNumber)
{
  m_LogStreamLine.Reset();

  auto &os{*this};
  os << "[" << GetTimeString()                                                              //
     << "][" << NewLogStream::GetLevelString(level)                                         //
     << "][" << std::hex << std::this_thread::get_id()                                      //
     << "][" << fileName.filename() << "," << std::setfill('0') << std::dec << lineNumber   //
     << "]";
}

const std::string_view &NewLogStream::GetLevelString(const NewLogStream::LEVEL level)
{
  static const std::string_view _TRACE{"TRACE"};
  static const std::string_view _DEBUG{"DEBUG"};
  static const std::string_view _INFO{"INFO"};
  static const std::string_view _WARN{"WARN"};
  static const std::string_view _ERROR{"ERROR"};
  static const std::string_view _FATAL{"FATAL"};
  static const std::string_view _NONE{""};

  switch (level) {
  case LEVEL::NONE:
    return _NONE;
    break;
  case LEVEL::TRACE:
    return _TRACE;
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

void NewLogStream::WriteToStream(std::ostream &outStream) const
{
  outStream.write(GetLogStreamLine().GetBuffer().m_LogBuffer.data(), GetLogStreamLine().GetBuffer().m_Size);
}
