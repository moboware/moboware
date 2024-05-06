#include "common/logger.hpp"
#include "socket/tcp_socket_server.hpp"

using namespace moboware;

class SessionHandler {
public:
  SessionHandler() = default;
  ~SessionHandler() = default;

  void OnDataRead(const socket::RingBuffer_t &readBuffer,
                  const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                  const common::SystemTimePoint_t &sessionTimePoint)
  {
    _log_info(LOG_DETAILS, "Received data");
  }

  void OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint)
  {
    _log_info(LOG_DETAILS, "Session connected");
  }

  // called when the session is closed and the session can be cleaned up.
  void OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
  {
    _log_info(LOG_DETAILS, "Session closed");
  }
};

int main(int, char **)
{
  auto service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    _log_info(LOG_DETAILS, "Control-C received, stopping application");

    service->Stop();
  });

  using TcpServer_t = tcp_socket::TcpSocketServer<SessionHandler>;

  SessionHandler sessionHandler;
  TcpServer_t tcpServer(service, sessionHandler);

  const auto address{std::string{"127.0.0.1"}};
  const auto port{6543u};
  if (tcpServer.Start(address, port)) {
    _log_info(LOG_DETAILS, "Server started to {}:{}", address, port);
    service->Run();
  }

  return 0;
}