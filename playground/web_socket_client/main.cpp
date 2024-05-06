#include "common/logger.hpp"
#include "common/service.h"
#include "common/timer.h"
#include "socket/web_socket_client.hpp"
#include <boost/asio/buffer.hpp>
#include <boost/asio/signal_set.hpp>
#include <string>

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

  using WebClient_t = web_socket::WebSocketClient<SessionHandler>;

  SessionHandler sessionHandler;
  WebClient_t webClient(service, sessionHandler);

  const auto address{std::string{"127.0.0.1"}};
  const auto port{6543u};
  if (webClient.Start(address, port)) {

    _log_info(LOG_DETAILS, "Client connected to {}:{}", address, port);

    common::Timer timer(service);
    const auto timerFn{[&](common::Timer &timer)   //
                       {
                         const std::string str{"blablalbalbalg"};
                         const boost::asio::const_buffer buffer{str.data(), str.size()};
                         if (webClient.SendWebSocketData({buffer})) {
                           timer.Restart();
                         }
                       }};
    timer.Start(timerFn, std::chrono::milliseconds(1));

    service->Run();
  }

  return 0;
}