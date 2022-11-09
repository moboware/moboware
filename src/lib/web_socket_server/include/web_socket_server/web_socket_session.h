#pragma once
#include "common/service.h"
#include "web_socket_server/web_socket_session_callback.h"
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>

namespace moboware::web_socket {

class WebSocketSession
{
public:
  explicit WebSocketSession(const std::shared_ptr<moboware::common::Service>& service,
                            const std::shared_ptr<WebSocketSessionCallback>& callback,
                            boost::asio::ip::tcp::socket&& webSocket);
  void Start();

  /**
   * @brief Check if the web socket is open
   * @return true if open
   * @return false if closed
   */
  inline auto IsOpen() const -> bool { return m_WebSocket.is_open(); }
  [[nodiscard]] auto SendWebSocketData(const boost::beast::flat_buffer& sendBuffer) -> bool;

private:
  void ReadData();
  const std::shared_ptr<moboware::common::Service> m_Service;
  const std::shared_ptr<WebSocketSessionCallback> m_DataHandlerCallback;

  boost::beast::websocket::stream<boost::beast::tcp_stream> m_WebSocket;
  boost::beast::flat_buffer m_ReadBuffer;
};
} // namespace moboware