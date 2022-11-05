#include "common/timer.h"
#include <cstdlib>
#include "common/log.h"

using namespace boost;
using namespace boost::asio;
using namespace moboware::common;

Timer::Timer(const std::shared_ptr<Service>& service)
  : m_Timer(service->GetIoService())
{
  // Default timer handler lambda
  m_TimerHandler = [this](const system::error_code& error)
  {
    if (error != asio::error::operation_aborted && m_TimerFunction)
    {
      m_TimerFunction(*this);
    }
  };
}

void Timer::Restart()
{
  Start(m_Timeout);
}

void Timer::Start(const std::chrono::microseconds timeout)
{
  m_Timeout = timeout;

  m_Timer.expires_after(m_Timeout);
  m_Timer.async_wait(m_TimerHandler);
}

void Timer::Start(const TimerFunction& timerFunc, const std::chrono::microseconds timeout)
{
  m_TimerFunction = timerFunc;

  Start(timeout);
}
