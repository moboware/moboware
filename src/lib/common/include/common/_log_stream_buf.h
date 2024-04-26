#pragma once
#include <array>
#include <cstring>
#include <streambuf>
#include <string>

namespace moboware::common {

template <std::size_t BufferSize = 512U * 1'024U>   //
class LogStreamBuf : public std::streambuf {
public:
  LogStreamBuf(const LogStreamBuf &) = delete;
  LogStreamBuf(LogStreamBuf &&) = default;
  LogStreamBuf &operator=(const LogStreamBuf &) = default;
  LogStreamBuf &operator=(LogStreamBuf &&) = default;
  ~LogStreamBuf() noexcept = default;

  LogStreamBuf()
  {
    Reset();
  }

  template <std::size_t TSize> struct ArrayBuffer {
    std::size_t m_Size{};
    std::array<char, TSize> m_LogBuffer;
  };

  using ArrayBuffer_t = ArrayBuffer<BufferSize>;

  void Reset()
  {
    // depending on the length of the buffer as memset is executed
    const char *startOfBuffer = m_ArrayBuffer.m_LogBuffer.data();
    const size_t bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : BufferSize);

    memset(m_ArrayBuffer.m_LogBuffer.data(), 0, bufSize);

    setp(m_ArrayBuffer.m_LogBuffer.data(), m_ArrayBuffer.m_LogBuffer.data() + BufferSize);
    m_ArrayBuffer.m_Size = 0;
  }

  auto Empty() -> bool
  {
    return (Size() == 0);
  }

  inline const ArrayBuffer_t &GetBuffer() const
  {
    m_ArrayBuffer.m_Size = Size();
    return m_ArrayBuffer;
  }

private:
  std::size_t Size() const
  {
    const char *startOfBuffer = m_ArrayBuffer.m_LogBuffer.data();
    const auto bufSize = (pptr() ? size_t(pptr() - startOfBuffer) : BufferSize);
    return bufSize;
  }

  mutable ArrayBuffer_t m_ArrayBuffer;
};

}   // namespace moboware::common