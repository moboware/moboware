#include "modules/log_module.h"
#include "common/logger.hpp"

using namespace moboware::modules;
using namespace moboware::common;

LogModule::LogModule(const std::shared_ptr<common::Service> &service,                     //
                     const std::shared_ptr<common::ChannelInterface> &channelInterface)   //
    : common::IModule("LogModule", service, channelInterface)                             //
{
}

bool LogModule::LoadConfig(const boost::json::value &moduleValue)
{
  _log_debug( LOG_DETAILS,"Load module Config");

  return true;
}

bool LogModule::Start()
{
  return true;
}

void LogModule::OnWebSocketDataReceived(const boost::beast::flat_buffer &readBuffer,
                                        const boost::asio::ip::tcp::endpoint &endpoint)
{
  // boost::beast::flat_buffer sendBuffer;
  // sendBuffer.prepare()
  //
  // sendBuffer = "Hello websocket ";

  // GetChannelInterface()->SendWebSocketData(sendBuffer, endpoint);
  // GetChannelInterface()->SendWebSocketData(readBuffer, endpoint);
}