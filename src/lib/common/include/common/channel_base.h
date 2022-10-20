#pragma once
#include <memory>
#include <deque>
#include <jsoncpp/json/json.h>
#include "common/service.h"
#include "common/imodule.h"
#include "common/imodule_factory.h"

namespace moboware::common
{
    const std::string NAME_VALUE{"Name"};

    /**
     * @brief base class for the Channel
     */
    class ChannelBase
    {
    public:
        virtual ~ChannelBase() = default;

        ChannelBase(const ChannelBase &);
        ChannelBase(ChannelBase &&);

        ChannelBase &operator=(const ChannelBase &);
        ChannelBase &operator=(ChannelBase &&);

        bool LoadConfig(const Json::Value &channelConfig);
        [[nodiscard]] virtual bool LoadChannelConfig(const Json::Value &channelConfig) = 0;
        [[nodiscard]] virtual bool Start() = 0;
        [[nodiscard]] virtual std::shared_ptr<common::IModule> CreateModule(const std::string &moduleName, const Json::Value &module) = 0;

        virtual void Stop();

        const std::shared_ptr<Service> &GetService() const
        {
            return m_Service;
        }

    protected:
        explicit ChannelBase(const std::shared_ptr<Service> &service);
        [[nodiscard]] bool StartModules();

        using ModulePtr_t = std::shared_ptr<IModule>;
        using ModuleQueue_t = std::deque<ModulePtr_t>;
        const ModuleQueue_t &GetModules() const
        {
            return m_Modules;
        }
        const std::string &GetChannelName() const { return m_ChannelName; }

    private:

        const std::shared_ptr<Service> m_Service{};
        ModuleQueue_t m_Modules{};
        std::string m_ChannelName{};
    };
}