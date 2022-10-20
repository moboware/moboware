#pragma once

#include "common/timer.h"
#include "common/service.h"
#include <boost/asio.hpp>
#include <chrono>
#include <memory>

namespace moboware::common
{

    const std::size_t maxBufferSize{1024};
    /**
     * @brief socket session used for server and client sessions
     *
     */
    class Session : public std::enable_shared_from_this<Session>
    {
    public:
        using Endpoint = std::pair<std::string, std::uint16_t>;
        using ReceiveDataFunction = std::function<void(const std::shared_ptr<Session> &session, const std::array<char, maxBufferSize> &readBuffer, const std::size_t bytesRead)>;

        explicit Session(const std::shared_ptr<Service> &service);
        Session(const Session &) = delete;
        Session(Session &&) = delete;
        Session &operator=(const Session &) = delete;
        Session &operator=(const Session &&) = delete;
        virtual ~Session();

        boost::asio::ip::tcp::socket &Socket();

        void Start();
        std::size_t Send(const boost::asio::const_buffer &sendBuffer);
        const Session::Endpoint &GetRemoteEndpoint() const;

        void SetSessionDisconnected(const std::function<void(const std::shared_ptr<Session> &, const Session::Endpoint &)> &fn);
        void SetSessionReceiveData(const ReceiveDataFunction &fn);

    protected:
        void CloseSocket();

        void AsyncReceive();

    private:
        boost::asio::ip::tcp::socket m_Socket;
        const std::shared_ptr<Service> _service;

        ReceiveDataFunction m_ReceiveDataCallbackFunction{};
        std::function<void(const std::shared_ptr<Session> &, const Session::Endpoint &endPoint)> m_SessionDisconnectedCallback{};

        std::function<void(const boost::system::error_code &)> _readDataHandler;
        [[nodiscard]] bool ReadData(const boost::system::error_code &errorCode);

        Endpoint m_RemoteEndpoint;
        const std::chrono::time_point<std::chrono::system_clock> _clientAliveTime{std::chrono::system_clock::now()};
    };

}
