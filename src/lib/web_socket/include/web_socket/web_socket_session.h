#pragma once
#include "common/service.h"
#include "web_socket/web_socket_session_callback.h"
#include <boost/asio/buffer.hpp>
#include <boost/beast/core/tcp_stream.hpp>
#include <boost/beast/websocket.hpp>

namespace moboware::web_socket {

class WebSocketSession
{
public:
  explicit WebSocketSession(const std::shared_ptr<moboware::common::Service>& service,
                            const std::shared_ptr<WebSocketSessionCallback>& callback,
                            boost::asio::ip::tcp::socket&& webSocket);
  /**
   * @brief Accept incoming client connection and start reading data
   */
  void Accept();

  /**
   * @brief Connect a web socket client to a web socket server an start reading data
   * @param address
   * @param port
   * @return true
   * @return false
   */
  auto Connect(const std::string& address, const short port) -> bool;

  /**
   * @brief Check if the web socket is open
   * @return true if open
   * @return false if closed
   */
  inline auto IsOpen() const -> bool { return m_WebSocket.is_open(); }

  [[nodiscard]] auto SendWebSocketData(const boost::asio::const_buffer& sendBuffer) -> bool;

private:
  void ReadData();
  const std::shared_ptr<common::Service> m_Service;
  const std::shared_ptr<WebSocketSessionCallback> m_DataHandlerCallback;

  boost::beast::websocket::stream<boost::beast::tcp_stream> m_WebSocket;
  boost::beast::flat_buffer m_ReadBuffer;
};
} // namespace moboware