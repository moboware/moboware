#include "channels/web_socket_channel.h"
#include "modules/tcp_client_module.h"
#include "modules/log_module.h"

using namespace moboware::channels;
using namespace moboware::common;
using namespace moboware::modules;

WebSocketChannel::WebSocketChannel(const std::shared_ptr<Service>& service)
  : ChannelBase(service),
  m_WebSocketServer(service)
{
}

bool WebSocketChannel::LoadChannelConfig(const Json::Value& channelConfig)
{
  // load web socket server config settings
  if (!channelConfig.isMember("Port"))
  {
    LOG("Missing Port in channel config");
    return false;
  }

  if (!channelConfig["Port"].isInt())
  {
    LOG("Port is not int value");
    return false;
  }

  m_Port = channelConfig["Port"].asInt();
  return true;
}

std::shared_ptr<IModule> WebSocketChannel::CreateModule(const std::string& moduleName, const Json::Value& module)
{
  // todo load modules from module map from factory
  LOG("Create module " << moduleName);
  if (moduleName == "TcpClientModule")
  {
    TcpClientModuleFactory factory;
    return factory.CreateModule(GetService(), shared_from_this());
  }
  else if (moduleName == "LogModule")
  {
    LogModuleFactory factory;
    return factory.CreateModule(GetService(), shared_from_this());
  }
  else
  {
    LOG("Module name unknown:" << moduleName);
  }
  return nullptr; // or throw exception
}

bool WebSocketChannel::Start()
{
  LOG("Starting WebSocketChannel " << GetChannelName());

  m_WebSocketServer.SetWebSocketDataReceived([this](const uint64_t tag, const std::string& payload) //
    {                                                      //
      this->HandleWebSocketData(tag, payload);
    });
  if (!m_WebSocketServer.Start(m_Port))
  {
    LOG("Start channel failed " << GetChannelName());
    return false;
  }
  // start modules
  return StartModules();
}

void WebSocketChannel::SendData(const uint64_t tag, const std::string& payload)
{
  // send data back from a module to the web socket server.
  LOG("WebSocketChannel SendData:" << payload);
  if (!m_WebSocketServer.SendData(tag, payload))
  {
    LOG("Web Socket error");
  }
}

void WebSocketChannel::HandleWebSocketData(const uint64_t tag, const std::string& payload)
{
  LOG("Web Socket data Received, tag:" << tag << ":" << payload);
  // send all received web socket payload data to all modules.
  // could be more specific in sending to the modules when we have an protocol over json e.g.
  // and implememnt some kind of subscribtion mechanism from the module to the channel wo we can send specific
  // message to subscribed modules only.
  std::for_each(GetModules().begin(), GetModules().end(),   //
    [&tag, &payload](const ModulePtr_t& module) //
    {                                           //
      module->OnWebSocketPayload(tag, payload);
    });
}
