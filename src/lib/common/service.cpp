#include "common/service.h"
#include "common/logger.hpp"

using namespace moboware::common;

Service::Service(const std::uint8_t numberOfCpuThreads)
    : m_IoService(numberOfCpuThreads)
    , m_NumberOfServiceThreads(numberOfCpuThreads)
{
}

int Service::Run()
{
  // Run the I/O service on the requested number of threads
  for (int cpu = 1; cpu < m_NumberOfServiceThreads; cpu++) {
    m_ServiceThreads.emplace_back([&, cpu](const std::stop_token & /*stoken*/) {
      try {
        // SetThreadAffinity(cpu);

        m_IoService.run();
      } catch (const std::exception &e) {
        _log_error(LOG_DETAILS, "Fatal exception {}", e.what());
        this->Stop();
      } catch (...) {
        _log_error(LOG_DETAILS, "Unhandled exception");
        this->Stop();
      }
    });
  }

  //
  auto work{boost::asio::make_work_guard(m_IoService.get_executor())};

  m_IoService.run();

  _log_info(LOG_DETAILS, "Stopped service, exiting");

  return EXIT_SUCCESS;
}

void Service::Stop()
{
  _log_info(LOG_DETAILS, "Stopping services...");

  for (auto cpu = 1u; cpu < m_NumberOfServiceThreads; cpu++) {
    m_IoService.stop();
  }

  if (not m_IoService.stopped()) {
    m_IoService.stop();
  }
}