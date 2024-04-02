#pragma once

#include "common/log_stream_buf.h"
#include <filesystem>
#include <mutex>
#include <ostream>
#include <string>
#include <thread>

namespace moboware::common {

class NewLogStream : public std::ostream {
public:
  using LogStreamLine_t = LogStreamBuf<24 * 1'024U>;

  enum class LEVEL : std::int8_t {
    NONE = 0,
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL
  };

  NewLogStream();

  NewLogStream(const NewLogStream &) = delete;
  NewLogStream(NewLogStream &&) = delete;
  NewLogStream &operator=(const NewLogStream &) = delete;
  NewLogStream &operator=(NewLogStream &&) = delete;

  ~NewLogStream() = default;

  LogStreamLine_t &GetLogStreamLine() const;

  void StartLine(const NewLogStream::LEVEL level,
                 const std::string &function,
                 const std::filesystem::path &fileName,
                 const size_t lineNumber);

  template <typename... TLogArgs>
  void LogLine(const NewLogStream::LEVEL level,
               const std::filesystem::path &fileName,
               const size_t lineNumber,
               const TLogArgs... logArgs)
  {
    m_LogStreamLine.Reset();

    auto &os{*this};
    os << "[" << std::setfill('0') << std::dec                           //
       << GetTimeString()                                                //
       << "][" << NewLogStream::GetLevelString(level)                    //
       << "][" << std::hex << std::this_thread::get_id()                 //
       << "][" << fileName.filename() << "," << std::dec << lineNumber   //
       << "]";
    (os << ... << logArgs);
  }

  void WriteToStream(std::ostream &outStream) const;

  static const std::string_view &GetLevelString(const NewLogStream::LEVEL level);

private:
  std::string GetTimeString();
  mutable LogStreamLine_t m_LogStreamLine;
};

}   // namespace moboware::common
