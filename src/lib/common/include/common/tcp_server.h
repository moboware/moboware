#pragma once

#include "common/service.h"
#include "common/timer.h"
#include "common/session.h"
#include <memory>

namespace moboware::common
{

    /**
     * @brief TcpServer
     */
    class TcpServer
    {
    public:
        explicit TcpServer(const std::shared_ptr<Service> &io_service);
        TcpServer(const TcpServer &) = delete;
        TcpServer(TcpServer &&) = delete;
        TcpServer &operator=(const TcpServer &) = delete;
        TcpServer &operator=(const TcpServer &&) = delete;
        ~TcpServer() = default;

        [[nodiscard]] bool StartListening(const std::uint16_t port);
        void SetSessionReceiveData(const Session::ReceiveDataFunction &fn);
        std::size_t SendData(const std::string &data, const Session::Endpoint &endPoint);

    protected:
        /**
         * @brief Private session class for the tcp server
         *
         */
        class ServerSession : public Session
        {
        public:
            explicit ServerSession(const std::shared_ptr<Service> &service,
                                   const std::function<void(const Session::Endpoint &endPoint)> &removeSession)
                : Session(service),
                  _removeSession(removeSession)
            {
            }

            ServerSession(const ServerSession &) = delete;
            ServerSession(ServerSession &&) = delete;
            ServerSession &operator=(const ServerSession &) = delete;
            ServerSession &operator=(const ServerSession &&) = delete;
            virtual ~ServerSession() = default;

            std::function<void(const std::pair<std::string, std::uint16_t> &endPoint)> _removeSession;
            std::function<std::string(const std::string &query)> _queryClientFn;
        };

    private:
        void HandleAccept(const std::shared_ptr<Session> &newSession, const boost::system::error_code &error);

        void AcceptConnection();

        void AcceptSession(const std::shared_ptr<ServerSession> &session, const boost::system::error_code &errorCode);

        void SessionDisconnected(const std::shared_ptr<Session> &session, const Session::Endpoint &endPoint);
        void SetSessionHandlers(const std::shared_ptr<ServerSession> &session);

        const std::shared_ptr<Service> m_Service;
        boost::asio::ip::tcp::acceptor m_Acceptor;

        using Sessions_t = std::map<std::pair<std::string, std::uint16_t>, std::shared_ptr<ServerSession>>;
        Sessions_t m_Sessions;
        Timer m_PingTimer;

        std::function<void(const std::pair<std::string, std::uint16_t> &endPoint)> m_RemoveSession{};
        Session::ReceiveDataFunction m_ReceiveDataCallbackFunction{};

        void PostRemoveSession(const std::pair<std::string, std::uint16_t> &endPoint);
    };
}
