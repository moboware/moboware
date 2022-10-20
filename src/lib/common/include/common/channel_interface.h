#pragma once
#include <string>
#include <memory>
#include "common/session.h"

namespace moboware::common
{
    class ChannelInterface
        : public std::enable_shared_from_this<ChannelInterface>
    {
    public:
        virtual void SendData(const uint64_t tag, const std::string &payload) = 0;
    };
}