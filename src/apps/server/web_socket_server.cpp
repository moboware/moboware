
#include "applications/application.h"

#include <iostream>
#include "common/log.h"

/**
 * @brief web socket Server application
 *
 */
using namespace moboware::common;
using namespace moboware::channels;
using namespace moboware::applications;

int main(const int argc, const char* argv[])
{
  const auto service = std::make_shared<Service>();
  const auto myChannel1 = std::make_shared<WebSocketChannel>(service);
  const auto myChannel2 = std::make_shared<WebSocketChannel>(service);

  Application myApp(service, { myChannel1, myChannel2 });
  return myApp.Run(argc, argv);
}