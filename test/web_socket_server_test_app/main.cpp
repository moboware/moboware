#include "common/log.h"
#include "common/service.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_server.h"

using namespace moboware;

int main(const int, const char*[])
{
  const auto service{ std::make_shared<moboware::common::Service>() };
  const auto websocketServer{ std::make_shared<moboware::web_socket::WebSocketServer>(service) };

  const auto OnWebSocketDataReceived{ [&websocketServer](const boost::beast::flat_buffer& readBuffer,          //
                                                         const boost::asio::ip::tcp::endpoint& remoteEndPoint) //
                                      {
                                        // LOG("Read data from " << remoteEndPoint.address().to_string() << ":" << remoteEndPoint.port() //
                                        //                       << ", " << std::string((const char*)readBuffer.data().data(), readBuffer.data().size()));
                                        // send data back to the client....
                                        if (not websocketServer->SendWebSocketData(readBuffer, remoteEndPoint)) {
                                          LOG("Failed to send...");
                                        }
                                      } };

  websocketServer->SetWebSocketDataReceived(OnWebSocketDataReceived);
  if (not websocketServer->Start("0.0.0.0", 8080)) {
    return EXIT_FAILURE;
  }

  LOG("Running waiting for connections");

  service->Run();
  //
  return 0;
}