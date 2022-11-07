#pragma once

#include <boost/asio/io_service.hpp>

namespace moboware::common {
class Service
{
public:
  Service();
  ~Service() = default;
  Service(const Service&) = delete;
  Service(Service&&) = delete;
  Service& operator=(const Service&) = delete;
  Service& operator=(Service&&) = delete;

  int Run();
  void Stop();

  boost::asio::io_service& GetIoService() { return m_IoService; }

private:
  boost::asio::io_service m_IoService;
};
}