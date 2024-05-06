#pragma once

#include <boost/asio/io_service.hpp>
#include <boost/asio/signal_set.hpp>
#include <boost/asio/thread_pool.hpp>
#include <thread>

namespace moboware::common {

class Service {
public:
  Service(const std::uint8_t numberOfCpuThreads = 2u);
  ~Service() = default;
  Service(const Service &) = delete;
  Service(Service &&) = delete;
  Service &operator=(const Service &) = delete;
  Service &operator=(Service &&) = delete;

  int Run();
  void Stop();

  boost::asio::io_service &GetIoService()
  {
    return m_IoService;
  }

private:
  boost::asio::io_service m_IoService;
  const std::uint8_t m_NumberOfServiceThreads{2u};
  std::vector<std::jthread> m_ServiceThreads;
};

}   // namespace moboware::common