#include "common/log_stream.h"
#include "common/service.h"
#include "common/timer.h"
#include "web_socket/web_socket_client.h"
#include <boost/asio.hpp>

using namespace moboware;

int main(const int, const char *[])
{
  LogStream::GetInstance().SetLevel(moboware::common::NewLogStream::LEVEL::DEBUG);
  const auto service{std::make_shared<moboware::common::Service>()};
  const auto websocketClient{std::make_shared<moboware::web_socket::WebSocketClient>(service)};

  const auto OnWebSocketDataReceived{
      [](const boost::beast::flat_buffer &readBuffer,            //
         const boost::asio::ip::tcp::endpoint &remoteEndPoint)   //
      {
        LOG_DEBUG("Read data from " << remoteEndPoint.address().to_string()   //
                                    << ":" << remoteEndPoint.port() << ", "
                                    << std::string((const char *)readBuffer.data().data(), readBuffer.data().size()));
      }};

  websocketClient->SetWebSocketDataReceived(OnWebSocketDataReceived);

  if (not websocketClient->Start("localhost", 8080)) {
    LOG_FATAL("Failed to connect client to server");
    return EXIT_FAILURE;
  }

  LOG_DEBUG("Running...");
  common::Timer timer(service);

  common::Timer::TimerFunction tmf{[&websocketClient, service](common::Timer &timer) {
    const std::string sendString{"3984trhjnkfrzvkfd.jhghfliowueryht9o 4h"};
    const boost::asio::const_buffer sendBuffer(sendString.c_str(), sendString.size());
    const std::size_t size = websocketClient->SendWebSocketData(sendBuffer);
    if (size > 0) {
      LOG_DEBUG("Wrote " << size << " bytes...");
      timer.Restart();
    } else {
      service->Stop();
    }
  }};

  timer.Start(tmf, std::chrono::milliseconds(100));

  service->Run();
  //
  return 0;
}