#include "common/application_base.h"
#include "common/logger.hpp"

using namespace moboware::common;

ApplicationBase::ApplicationBase(const std::shared_ptr<Service> &service)
    : m_Service(service)   // application and channel share same service
{
}
