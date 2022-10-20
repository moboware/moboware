#pragma once

#include "common/log.h"
#include "common/session.h"
#include <array>
#include <boost/asio.hpp>
#include <functional>
#include <memory>

namespace moboware::common
{

  class TcpClient : public Session
  {
  public:
    explicit TcpClient(const std::shared_ptr<Service> &io_service);

    TcpClient(const TcpClient &) = delete;
    TcpClient(TcpClient &&) = delete;
    TcpClient &operator=(const TcpClient &) = delete;
    TcpClient &operator=(const TcpClient &&) = delete;
    virtual ~TcpClient() = default;

    [[nodiscard]] bool Connect(const std::string &address, const std::uint16_t port);

  private:
    void HandleReceivedData(const std::shared_ptr<Session> &session, const std::array<char, maxBufferSize> &readBuffer, const std::size_t bytesRead);

    const std::shared_ptr<Service> _service;
    Timer _pingTimer;
  };
}
