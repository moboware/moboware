#pragma once

#include <cstddef>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace moboware::common {

/**
  Ring buffer for continues reading and writing of data and has the principle of a
  FIFO.
  The buffer will keep on writing until there is space available in the write
  buffer and the read buffer is always behind of the write buffer.
  In one case the data will be moved to the beginning of the buffer. This is
  when the used space is in the middle of the buffer and the requested write
  space is smaller the the total free size.
  In this case the data will be moved to the beginning of the buffer to create a
  new continues write space.
*/
template <typename BufferType, std::size_t BufferSize>   //
class RingBuffer final {
public:
  ~RingBuffer() noexcept = default;

  RingBuffer(const RingBuffer &) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;

  RingBuffer(RingBuffer &&ringBuffer) = delete;
  RingBuffer &operator=(RingBuffer &&ringBuffer) = delete;

  RingBuffer()
      : m_BufferSize(BufferSize)
  {
    m_WriteBuffer = m_Buffer.data();
    m_ReadBuffer = m_Buffer.data();
  }

  /**
  return the max free buffer size above the write buffer. Does not count in any
  freed data below the write buffer.
  */
  std::size_t GetFreeWriteBufferSize() const
  {
    return m_BufferSize - GetWriteBufferSize();
  }

  /**
  Returns the total number of bytes that write buffer contains.
  */
  std::size_t GetWriteBufferSize() const
  {
    return std::size_t(m_WriteBuffer - m_Buffer.data());
  }

  /**
   * @brief Write a data stream into the buffer and commits the buffer
   * @param data
   * @return std::size_t, return the bytes written if successful otherwise 0ul when there is no space to write
   */
  std::size_t WriteBuffer(const BufferType *data, const std::size_t dataSize)
  {
    auto *ptr{GetWriteBuffer(dataSize)};
    if (ptr) {
      // copy data into the buffer
      memcpy(ptr, data, dataSize);
      // commit
      Commit(dataSize);
      return dataSize;
    }
    return 0ul;
  }

  /**
  GetReadBufferSize returns the total number of bytes in the read buffer
  */
  std::size_t GetReadBufferSize() const
  {
    return std::size_t(m_WriteBuffer - m_ReadBuffer);
  }

  /**
   * @brief ReadBuffer, calls a read function that provides a pointer to the first element in the read buffer and the size of
   * the read buffer
   *
   * @param readFn, read function that should return a number of bytes that will be flushed, if 0ul is return the read buffer
   * will not be flushed
   * @return true, if there is data to read and a flush is successful
   * @return false, when there is no data to read or a failed flush
   */
  bool ReadBuffer(const std::function<std::size_t(const BufferType *data, const std::size_t bytesAvailable)> &readFn)
  {
    const auto readBytesAvailable{GetReadBufferSize()};
    const auto *readPtr{GetReadBuffer(readBytesAvailable)};
    if (readPtr == nullptr) {
      // no data to read!
      return false;
    }

    if (const auto bytesToFlush = readFn(readPtr, readBytesAvailable); bytesToFlush > 0) {
      Flush(bytesToFlush);
      return true;
    }
    return false;
  }

protected:
  /**
    Commit will commit written data, forwards the write pointer and increases
    write buffer size.
  */
  void Commit(const std::size_t size) const
  {
    m_WriteBuffer += size;
  }

  /**
  Flush the read bytes out of the read buffer by forwarding the readBuffer with
  the size.
  Incase the write and read buffer are empty, means pointers are equal, we can
  reset all pointer back to the beginning of the buffer
  */
  void Flush(const std::size_t size) const
  {
    if ((m_ReadBuffer + size) > m_Buffer.data() + m_BufferSize) {
      std::runtime_error("ReadBuffer flush exceeds the max buffer size");
    }

    m_ReadBuffer += size;

    if (m_WriteBuffer == m_ReadBuffer) {
      // reset to the beginning of the buffer..
      m_WriteBuffer = m_ReadBuffer = const_cast<BufferType *>(m_Buffer.data());
    }
  }

  /**
    Returns a pointer to the write buffer if enough free size is available,
    otherwise nullptr
  */
  BufferType *GetWriteBuffer(const std::size_t size)
  {
    if (std::size_t(m_WriteBuffer + size) <= std::size_t(m_Buffer.data() + m_BufferSize)) {
      return m_WriteBuffer;
    }

    const auto usedSize = std::size_t(m_WriteBuffer - m_ReadBuffer);
    const auto lowFreeSize = std::size_t(m_ReadBuffer - m_Buffer.data());
    const auto highFreeSize = std::size_t((m_Buffer.data() + m_BufferSize) - m_WriteBuffer);

    if (lowFreeSize + highFreeSize >= size) {
      // move the used part to the start of the buffer to make space
      if (m_Buffer.data() == std::memcpy(m_Buffer.data(), m_ReadBuffer, usedSize * sizeof(BufferType))) {
        m_ReadBuffer = m_Buffer.data();
        m_WriteBuffer = m_Buffer.data() + usedSize;
        return m_WriteBuffer;
      }
    }
    return nullptr;
  }

  const BufferType *GetReadBuffer(const std::size_t size) const
  {
    if (size <= std::size_t(m_WriteBuffer - m_ReadBuffer)) {
      return m_ReadBuffer;
    }
    return {};
  }

private:
  const std::size_t m_BufferSize{};
  std::array<BufferType, BufferSize> m_Buffer;
  mutable BufferType *m_WriteBuffer{};
  mutable BufferType *m_ReadBuffer{};
};

}   // namespace moboware::common
