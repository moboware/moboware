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
        LOG_ERROR("Fatal exception {}", e.what());
        this->Stop();
      } catch (...) {
        LOG_ERROR("Unhandled exception");
        this->Stop();
      }
    });
  }

  //
  auto work{boost::asio::make_work_guard(m_IoService.get_executor())};

  m_IoService.run();

  LOG_INFO("Stopped service, exiting");

  return EXIT_SUCCESS;
}

void Service::Stop()
{
  LOG_INFO("Stopping services...");

  for (auto cpu = 1u; cpu < m_NumberOfServiceThreads; cpu++) {
    m_IoService.stop();
  }

  if (not m_IoService.stopped()) {
    m_IoService.stop();
  }
}