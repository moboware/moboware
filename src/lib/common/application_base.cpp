#include "common/application_base.h"
#include <json/json.h>
#include "common/log.h"

using namespace moboware::common;

ApplicationBase::ApplicationBase(const std::shared_ptr<Service>& service)
    : m_Service(service) // application and channel share same service
{
}
