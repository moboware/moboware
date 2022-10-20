#include "common/service.h"
#include "common/log.h"

using namespace moboware::common;

int Service::Run()
{
    m_IoService.run();
    return EXIT_SUCCESS;
}

void Service::Stop()
{
    if (!m_IoService.stopped())
    {
        m_IoService.stop();
    }
}