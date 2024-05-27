#include "common/logger.hpp"
#include "socket/web_socket_server.hpp"
#include <string>

using namespace moboware;

class SessionHandler {
public:
  SessionHandler() = default;
  ~SessionHandler() = default;

  void OnDataRead(const boost::beast::flat_buffer &readBuffer,
                  const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                  const common::SessionTimePoint_t &sessionTimePoint)
  {
    const auto buffer{readBuffer.cdata()};

    const std::string_view s{(const char *)buffer.data(), buffer.size()};
    //
    LOG_INFO("Received data {} {}", s, std::chrono::steady_clock::now().time_since_epoch().count());
  }

  void OnSessionConnected(const boost::asio::ip::tcp::endpoint &endpoint)
  {
    LOG_INFO("Session connected");
  }

  // called when the session is closed and the session can be cleaned up.
  void OnSessionClosed(const boost::asio::ip::tcp::endpoint &endpoint)
  {
    LOG_INFO("Session closed");
  }
};

int main(int, char **)
{
  ::Logger::GetInstance().SetLevel(Logger::LogLevel::Trace);

  auto service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    LOG_INFO("Control-C received, stopping application");

    service->Stop();
  });

  using WebServer_t = web_socket::WebSocketServer<SessionHandler>;

  SessionHandler sessionHandler;
  WebServer_t webServer(service, sessionHandler);

  const auto address{std::string{"127.0.0.1"}};
  const auto port{6543u};
  if (webServer.Start(address, port)) {
    LOG_INFO("Server started to {}:{}", address, port);
    service->Run();
  }

  return 0;
}