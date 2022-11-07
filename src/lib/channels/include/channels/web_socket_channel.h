#pragma once
#include <array>
#include <memory>
#include "common/channel_base.h"
#include "common/service.h"
#include "common/session.h"
#include "common/tcp_server.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_server.h"

namespace moboware::channels
{

class WebSocketChannel
  : public common::ChannelBase
  , public common::ChannelInterface
{
 public:
  WebSocketChannel(const std::shared_ptr<common::Service>& service);
  WebSocketChannel(const WebSocketChannel&);
  WebSocketChannel(WebSocketChannel&&);
  WebSocketChannel& operator=(const WebSocketChannel&);
  WebSocketChannel& operator=(WebSocketChannel&&);
  virtual ~WebSocketChannel() = default;

  bool LoadChannelConfig(const Json::Value& channelConfig) final;

  bool Start() final;

  std::shared_ptr<common::IModule> CreateModule(const std::string& moduleName, const Json::Value& module) final;

  void SendData(const uint64_t tag, const std::string& payload) final;

  void HandleWebSocketData(const uint64_t tag, const std::string& payload);

 private:
  web_socket_server::WebSocketServer m_WebSocketServer;
  int m_Port{};
};
}  // namespace moboware::channels