#pragma once

#include "common/service.h"
#include <boost/asio/steady_timer.hpp>
#include <chrono>

namespace moboware::common
{
    class Timer
    {
    public:
        using TimerFunction = std::function<void(Timer &)>;

        Timer(const std::shared_ptr<common::Service> &service);
        Timer(const Timer &) = delete;
        Timer(Timer &&) = delete;
        Timer &operator=(const Timer &) = delete;
        Timer &operator=(Timer &&) = delete;
        
        void Start(const TimerFunction &timerFunc, const std::chrono::microseconds timeout);

        void Restart();

    private:
        void Start(const std::chrono::microseconds timeout);

        TimerFunction m_TimerFunction;
        std::function<void(const boost::system::error_code &)> m_TimerHandler;
        boost::asio::steady_timer m_Timer;
        std::chrono::microseconds m_Timeout;
    };
}
