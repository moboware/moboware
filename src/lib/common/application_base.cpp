#include "common/application_base.h"
#include "common/log_stream.h"
#include <json/json.h>

using namespace moboware::common;

ApplicationBase::ApplicationBase(const std::shared_ptr<Service>& service)
  : m_Service(service) // application and channel share same service
{
}
