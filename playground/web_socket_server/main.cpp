#include "common/logger.hpp"
#include "socket/web_socket_server.hpp"

using namespace moboware;

class SessionHandler {
public:
  SessionHandler() = default;
  ~SessionHandler() = default;

  void OnDataRead(const boost::beast::flat_buffer &readBuffer,
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

  using WebServer_t = web_socket::WebSocketServer<SessionHandler>;

  SessionHandler sessionHandler;
  WebServer_t webServer(service, sessionHandler);

  const auto address{std::string{"127.0.0.1"}};
  const auto port{6543u};
  if (webServer.Start(address, port)) {
    _log_info(LOG_DETAILS, "Server started to {}:{}", address, port);
    service->Run();
  }

  return 0;
}