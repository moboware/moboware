#include "common/logger.hpp"
#include "common/service.h"
#include "common/timer.h"
#include "socket/ssl_socket_server.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/signal_set.hpp>
#include <string>

using namespace moboware;

class SessionHandler {
public:
  SessionHandler() = default;
  ~SessionHandler() = default;

  void OnDataRead(const socket::RingBuffer_t &readBuffer,
                  const boost::asio::ip::tcp::endpoint &remoteEndPoint,
                  const common::SessionTimePoint_t &sessionTimePoint)
  {
    LOG_INFO("Received data");
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
  auto service{std::make_shared<common::Service>()};
  boost::asio::signal_set signals(service->GetIoService(), SIGTERM, SIGINT);
  signals.async_wait([&](boost::system::error_code const &, int) {
    LOG_INFO("Control-C received, stopping application");

    service->Stop();
  });

  using SslServer_t = ssl_socket::SslSocketServer<SessionHandler>;

  SessionHandler sessionHandler;
  SslServer_t sslServer(service, sessionHandler);

  const auto address{std::string{"localhost"}};
  const auto port{51374u};
  if (sslServer.Start(address, port)) {
    LOG_INFO("Server started to {}:{}", address, port);
    service->Run();
  }

  return 0;
}