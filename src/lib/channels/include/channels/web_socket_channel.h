#pragma once
#include "common/channel_base.h"
#include "common/service.h"
#include "common/session.h"
#include "common/tcp_server.h"
#include "common/timer.h"
#include "web_socket_server/web_socket_server.h"
#include <array>
#include <memory>

namespace moboware::channels {

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

  [[nodiscard]] auto CreateModule(const std::string& moduleName, const Json::Value& module) -> std::shared_ptr<common::IModule> final;

  void SendWebSocketData(const boost::asio::const_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint) final;

  void OnWebSocketDataReceived(const boost::beast::flat_buffer& readBuffer, const boost::asio::ip::tcp::endpoint& endpoint);

private:
  const std::shared_ptr<web_socket::WebSocketServer> m_WebSocketServer;
  std::string m_Address{};
  int m_Port{};
};
} // namespace moboware::channels