#include "common/logger.hpp"
#include "common/service.h"
#include "common/timer.h"
#include "socket/ssl_socket_client.hpp"
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

  using SslClient_t = ssl_socket::SslSocketClient<SessionHandler>;

  SessionHandler sessionHandler;
  SslClient_t sslClient(service, sessionHandler);

  const auto address{std::string{"localhost"}};
  const auto port{51374u};
  if (sslClient.Start(address, port)) {

    LOG_INFO("Client connected to {}:{}", address, port);

    common::Timer timer(service);
    const auto timerFn{[&](common::Timer &timer)   //
                       {
                         const std::string str{"blablalbalbalg"};
                         const boost::asio::const_buffer buffer{str.data(), str.size()};
                         if (sslClient.SendSocketData({buffer})) {
                           timer.Restart();
                         }
                       }};
    timer.Start(timerFn, std::chrono::milliseconds(1));

    service->Run();
  }

  return 0;
}