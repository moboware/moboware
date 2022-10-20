#pragma once
#include "common/channel_base.h"
#include <boost/asio/io_service.hpp>
#include <jsoncpp/json/json.h>
#include <vector>

namespace moboware::common
{
    /**
     * @brief base class for the Application
     */
    class ApplicationBase
    {
    public:
        virtual ~ApplicationBase() = default;

        ApplicationBase(const ApplicationBase &) = delete;
        ApplicationBase(ApplicationBase &&) = delete;

        ApplicationBase &operator=(const ApplicationBase &) = delete;
        ApplicationBase &operator=(ApplicationBase &&) = delete;

        [[nodiscard]] virtual int Run(const int argc, const char *argv[]) = 0;

    protected:
        explicit ApplicationBase(const std::shared_ptr<Service> &service);

        [[nodiscard]] virtual bool LoadConfig(const std::string &configFile) = 0;

        [[nodiscard]] virtual bool ReadCommandline(const int argc, const char *argv[]) = 0;

        const std::shared_ptr<Service> &GetService() const { return m_Service; }
        virtual void Stop() = 0;

    private:
        const std::shared_ptr<Service> m_Service;
    };

}