#pragma once
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>
#include "common/service.h"
#include "web_socket_server/web_socket_callback.h"

namespace moboware {

class WebSocketSession {
 public:
  explicit WebSocketSession(const std::shared_ptr<moboware::common::Service>& service, const std::shared_ptr<WebSocketCallback>& callback,
                            boost::asio::ip::tcp::socket&& webSocket);
  void Start();
  inline bool IsOpen() const { return m_WebSocket.is_open(); }

 private:
  void ReadData();
  const std::shared_ptr<moboware::common::Service> m_Service;
  const std::shared_ptr<WebSocketCallback> m_DataHandlerCallback;

  boost::beast::websocket::stream<boost::beast::tcp_stream> m_WebSocket;
  boost::beast::flat_buffer m_ReadBuffer;
};
}  // namespace moboware