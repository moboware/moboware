#pragma once

#include <cstddef>
#include <cstring>
#include <memory>
#include <stdexcept>

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
template <typename BufferType>   //
class RingBuffer final {
public:
  RingBuffer() = delete;
  ~RingBuffer() noexcept = default;

  RingBuffer(const RingBuffer &) = delete;
  RingBuffer &operator=(const RingBuffer &) = delete;

  RingBuffer(RingBuffer &&ringBuffer) = delete;
  RingBuffer &operator=(RingBuffer &&ringBuffer) = delete;

  explicit RingBuffer(const std::size_t bufferSize)
      : mBufferSize(bufferSize)
      , mBuffer(std::make_unique<BufferType>(bufferSize))
  {
    mWriteBuffer = mBuffer.get();
    mReadBuffer = mBuffer.get();
  }

  /**
    Returns a pointer to the write buffer if enough free size is available,
    otherwise nullptr
  */
  BufferType *GetWriteBuffer(const std::size_t size)
  {
    if (std::size_t(mWriteBuffer + size) <= size_t(mBuffer.get() + mBufferSize)) {
      return mWriteBuffer;
    }

    const std::size_t usedSize = std::size_t(mWriteBuffer - mReadBuffer);
    const std::size_t lowFreeSize = std::size_t(mReadBuffer - mBuffer.get());
    const std::size_t highFreeSize = std::size_t((mBuffer.get() + mBufferSize) - mWriteBuffer);
    if (lowFreeSize + highFreeSize >= size) {
      // move the used part to the start of the buffer to make space
      if (mBuffer.get() == std::memcpy(mBuffer.get(), mReadBuffer, usedSize * sizeof(BufferType))) {
        mReadBuffer = mBuffer.get();
        mWriteBuffer = mBuffer.get() + usedSize;
        return mWriteBuffer;
      }
    }
    return nullptr;
  }

  /**
  return the max free buffer size above the write buffer. Does not count in any
  freed data below the write buffer.
  */
  std::size_t GetFreeWriteBufferSize() const
  {
    return mBufferSize - GetWriteBufferSize();
  }

  /**
  Returns the total number of bytes that write buffer contains.
  */
  std::size_t GetWriteBufferSize() const
  {
    return std::size_t(mWriteBuffer - mBuffer.get());
  }

  /**
    Commit will commit written data, forwards the write pointer and increases
    write buffer size.
  */
  void Commit(const std::size_t size)
  {
    mWriteBuffer += size;
  }

  const BufferType *GetReadBuffer(const std::size_t size)
  {
    if (size <= std::size_t(mWriteBuffer - mReadBuffer)) {
      return mReadBuffer;
    }
    return {};
  }

  /**
  GetReadBufferSize returns the total number of bytes in the read buffer
  */
  std::size_t GetReadBufferSize()
  {
    return std::size_t(mWriteBuffer - mReadBuffer);
  }

  /**
  Flush the read bytes out of the read buffer by forwarding the readBuffer with
  the size.
  Incase the write and read buffer are empty, means pointers are equal, we can
  reset all pointer back to the beginning of the buffer
  */
  void Flush(const std::size_t size)
  {
    if ((mReadBuffer + size) > mBuffer.get() + mBufferSize) {
      std::runtime_error("ReadBuffer flush exceeds the max buffer size");
    }

    mReadBuffer += size;

    if (mWriteBuffer == mReadBuffer) {
      // reset to the beginning of the buffer..
      mWriteBuffer = mReadBuffer = mBuffer.get();
    }
  }

private:
  const std::size_t mBufferSize{};
  std::unique_ptr<BufferType> mBuffer;
  BufferType *mWriteBuffer{};
  BufferType *mReadBuffer{};
};

}   // namespace moboware::common
