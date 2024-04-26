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
  static constexpr auto LogLineLength{24 * 1'024U};
  using LogStreamLine_t = LogStreamBuf<LogLineLength>;

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

  void WriteToStream(std::ostream &outStream) const;

  static const std::string_view &GetLevelString(const NewLogStream::LEVEL level);

private:
  std::string GetTimeString();
  mutable LogStreamLine_t m_LogStreamLine;
};

}   // namespace moboware::common
