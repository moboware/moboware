#include "channels/web_socket_channel.h"
#include "common/logger.hpp"
#include "modules/log_module.h"
#include "modules/matching_engine_module/matching_engine_module.h"
#include "modules/tcp_client_module.h"

using namespace moboware::channels;
using namespace moboware::common;
using namespace moboware::modules;

WebSocketChannel::WebSocketChannel(const std::shared_ptr<Service> &service)
  : ChannelBase(service)
  , m_WebSocketServer(std::make_shared<web_socket::WebSocketServer>(service))
{
}

bool WebSocketChannel::LoadChannelConfig(const boost::json::value &channelConfig)
{
  // load web socket server config settings
  {
    if (not channelConfig.as_object().contains("Port")) {
      LOG_DEBUG("Missing Port in channel config");
      return false;
    }

    if (not channelConfig.at("Port").is_int64()) {
      LOG_DEBUG("Port is not int value");
      return false;
    }

    m_Port = channelConfig.at("Port").as_int64();
  }

  {
    const std::string AddressKey{"Address"};
    if (not channelConfig.as_object().contains(AddressKey)) {
      LOG_DEBUG("Missing {} in channel config", AddressKey);
      return false;
    }

    if (not channelConfig.at(AddressKey).is_string()) {
      LOG_DEBUG("{} is not string value", AddressKey);
      return false;
    }

    m_Address = channelConfig.at(AddressKey).as_string().c_str();
  }
  return true;
}

std::shared_ptr<IModule> WebSocketChannel::CreateModule(const std::string &moduleName, const boost::json::value &module)
{
  // todo load modules from module unordered_map from factory
  LOG_DEBUG("Create module {}", moduleName);
  if (moduleName == "TcpClientModule") {
    TcpClientModuleFactory factory;
    return factory.CreateModule(GetService(), shared_from_this());
  } else if (moduleName == "LogModule") {
    LogModuleFactory factory;
    return factory.CreateModule(GetService(), shared_from_this());
  } else if (moduleName == "MatchingEngineModule") {
    MatchingEngineModuleFactory factory;
    return factory.CreateModule(GetService(), shared_from_this());
  } else {
    LOG_DEBUG("Module name unknown:{}", moduleName);
  }
  return nullptr;   // or throw exception
}

bool WebSocketChannel::Start()
{
  LOG_DEBUG("Starting WebSocketChannel {}", GetChannelName());

  m_WebSocketServer->SetWebSocketDataReceived(
    [this](const boost::beast::flat_buffer &readBuffer, const boost::asio::ip::tcp::endpoint &endpoint) {
      this->OnWebSocketDataReceived(readBuffer, endpoint);
    });

  if (not m_WebSocketServer->Start(m_Address, m_Port)) {
    LOG_DEBUG("Start channel failed {}", GetChannelName());
    return false;
  }
  // start modules
  return StartModules();
}

void WebSocketChannel::SendWebSocketData(const boost::asio::const_buffer &sendBuffer, const boost::asio::ip::tcp::endpoint &endpoint)
{
  // send data back from a module to the web socket server.
  LOG_DEBUG("SendWebSocketData: {}", std::string(static_cast<const char *>(sendBuffer.data()), sendBuffer.size()));
  if (not m_WebSocketServer->SendWebSocketData(sendBuffer, endpoint)) {
    LOG_DEBUG("Web Socket error");
  }
}

void WebSocketChannel::OnWebSocketDataReceived(const boost::beast::flat_buffer &readBuffer, const boost::asio::ip::tcp::endpoint &endpoint)
{
  LOG_DEBUG("Web Socket data Received, tag:{}:{},{}",
            endpoint.address().to_string(),
            endpoint.port(),
            std::string(static_cast<const char *>(readBuffer.data().data()), readBuffer.data().size()));
  // send all received web socket payload data to all modules.
  // could be more specific in sending to the modules when we have an protocol over json e.g.
  // and implement some kind of subscription mechanism from the module to the channel wo we can send specific
  // message to subscribed modules only.
  std::for_each(std::begin(GetModules()),
                std::end(GetModules()),   //
                [&readBuffer, &endpoint](const ModulePtr_t &module) {
                  module->OnWebSocketDataReceived(readBuffer, endpoint);
                });
}
