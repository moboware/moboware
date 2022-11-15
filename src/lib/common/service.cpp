#include "common/service.h"
#include "common/log.h"
#include <thread>

using namespace moboware::common;

constexpr auto numberOfServiceThreads{ 5U };

Service::Service()
  : m_IoService(numberOfServiceThreads)
{
}

int Service::Run()
{
  // Run the I/O service on the requested number of threads
  std::vector<std::thread> v;
  v.reserve(numberOfServiceThreads - 1);

  for (auto i = numberOfServiceThreads - 1; i > 0; --i) {
    v.emplace_back([this] {
      m_IoService.run();
    });
  }
  //
  m_IoService.run();

  return EXIT_SUCCESS;
}

void Service::Stop()
{
  if (!m_IoService.stopped()) {
    m_IoService.stop();
  }
}