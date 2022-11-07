#pragma once
#include "common/service.h"
#include "web_socket_server/tag_map.h"
#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>

namespace moboware::web_socket_server {
class WebSocketServer
{
public:
  explicit WebSocketServer(const std::shared_ptr<common::Service>& service);

  virtual ~WebSocketServer() = default;
  WebSocketServer(const WebSocketServer&) = delete;
  WebSocketServer(WebSocketServer&&) = delete;
  WebSocketServer& operator=(const WebSocketServer&) = delete;
  WebSocketServer& operator=(WebSocketServer&&) = delete;

  [[nodiscard]] auto Start(const int port) -> bool;
  [[nodiscard]] auto SendData(const std::uint64_t tag, const std::string& payload) -> bool;
  using WebSocketDataReceivedFn = std::function<void(const std::uint64_t tag, const std::string& payload)>;
  void SetWebSocketDataReceived(const WebSocketDataReceivedFn& fn);

private:
  using WsppServer_t = websocketpp::server<websocketpp::config::asio>;
  void OnWebSocketMessage(websocketpp::connection_hdl hdl, WsppServer_t::message_ptr msg);
  const std::shared_ptr<common::Service> m_Service;
  WsppServer_t m_WsppServer;
  TagMap m_TagMap;
  std::map<websocketpp::connection_hdl, uint64_t, std::owner_less<websocketpp::connection_hdl>> m_WebSocketHandleToTag;
  std::map<uint64_t, websocketpp::connection_hdl> m_TagToWebSocketHandle;

  WebSocketDataReceivedFn m_WebSocketDataReceivedFn;
};
} // namespace moboware::web_socket_server