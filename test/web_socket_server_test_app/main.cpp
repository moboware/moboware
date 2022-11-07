#include "common/log.h"
#include "common/service.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_server.h"

using namespace moboware;

class WebSocketChannelCallback : public WebSocketCallback
{
public:
  WebSocketChannelCallback() = default;
  void OnDataRead(const boost::beast::flat_buffer& readBuffer, //
                  const boost::asio::ip::tcp::endpoint& remoteEndPoint) final
  {
    LOG("Read data from " << remoteEndPoint.address().to_string() << ":" << remoteEndPoint.port() //
                          << ", " << std::string((const char*)readBuffer.data().data(), readBuffer.data().size()));
  }
};

int main(const int, const char*[])
{
  const auto service = std::make_shared<moboware::common::Service>();
  const auto webSocketChannelCallback = std::make_shared<WebSocketChannelCallback>();
  moboware::WebSocketServer websocketServer(service, webSocketChannelCallback);

  if (not websocketServer.Start("0.0.0.0", 8080)) {
    return EXIT_FAILURE;
  }

  LOG("Running waiting for connections");

  service->Run();
  //
  return 0;
}