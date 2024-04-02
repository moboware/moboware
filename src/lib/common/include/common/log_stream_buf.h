#pragma once
#include <array>
#include <cstring>
#include <streambuf>
#include <string>

namespace moboware::common {
template <std::size_t BufferSize = 1'024U * 1'024U>   //
class LogStreamBuf : public std::streambuf {
public:
  LogStreamBuf(const LogStreamBuf &) = delete;
  LogStreamBuf(LogStreamBuf &&) = delete;
  LogStreamBuf &operator=(const LogStreamBuf &) = delete;
  LogStreamBuf &operator=(LogStreamBuf &&) = delete;
  ~LogStreamBuf() noexcept = default;

  bool operator==(const LogStreamBuf &) const = delete;

  LogStreamBuf()
  {
    Reset();
  }

  auto Size() -> std::streamsize
  {
    const char *startOfBuffer = mLogBuffer.data();
    const auto bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : BufferSize);
    return bufSize;
  }

  void Reset()
  {
    // depending on the length of the buffer as memset is executed
    const char *startOfBuffer = mLogBuffer.data();
    const size_t bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : BufferSize);

    memset(mLogBuffer.data(), 0, bufSize);

    setp(mLogBuffer.data(), mLogBuffer.data() + BufferSize);
  }

  auto Empty() -> bool
  {
    return (Size() == 0);
  }

  inline const char *GetBuffer() const
  {
    return mLogBuffer.data();
  }

private:
  std::array<char, BufferSize> mLogBuffer;
};

}   // namespace moboware