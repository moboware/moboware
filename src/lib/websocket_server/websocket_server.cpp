#include "websocket_server/websocket_server.h"
#include "common/log.h"

using namespace moboware::common;
using namespace moboware::web_socket_server;
//
using websocketpp::lib::bind;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;

WebSocketServer::WebSocketServer(const std::shared_ptr<common::Service>& service)
  : m_Service(service)
{
  m_WsppServer.set_access_channels(websocketpp::log::alevel::none);
  m_WsppServer.clear_access_channels(websocketpp::log::alevel::frame_payload);

  // The only difference in this code between an internal and external
  // io_service is the different constructor to init_asio
  m_WsppServer.init_asio(&service->GetIoService());

  // Register our message handler
  auto onMessageFunction{ [this](websocketpp::connection_hdl hdl, WsppServer_t::message_ptr msg) //
                         {                                                                      //
                             OnWebSocketMessage(hdl, msg);
                         } };
  m_WsppServer.set_message_handler(onMessageFunction);

  // set open handler
  const auto onOpenFunction{ [this](websocketpp::connection_hdl hdl)
                            {
                                const auto tag{m_TagMap.Insert(hdl)};
                                LOG("Open connection, tag=" << tag);
                            } };
  m_WsppServer.set_open_handler(onOpenFunction);

  // set close handler
  const auto onCloseFunction{ [this](websocketpp::connection_hdl hdl) //
                             {
                                 const auto tag{m_TagMap.Erase(hdl)};
                                 LOG("Close connection, tag=" << tag);
                             } };
  m_WsppServer.set_close_handler(onCloseFunction);
}

void WebSocketServer::OnWebSocketMessage(websocketpp::connection_hdl hdl, WsppServer_t::message_ptr msg)
{
  LOG("OnWebSocket Message " << msg->get_payload());

  // pass on to the channel
  if (m_WebSocketDataReceivedFn)
  {
    const auto tag{ m_TagMap.Insert(hdl) };

    m_WebSocketDataReceivedFn(tag, msg->get_payload());
  }
}

bool WebSocketServer::Start(const int port)
{
  m_WsppServer.set_reuse_addr(true);
  m_WsppServer.listen(port);

  websocketpp::lib::error_code ec{};
  m_WsppServer.start_accept(ec);
  if (ec)
  {
    LOG(ec.message());
    return false;
  }
  return true;
}

bool WebSocketServer::SendData(const std::uint64_t tag, const std::string& payload)
{
  const auto hdl{ m_TagMap.Find(tag) };
  if (hdl)
  {
    websocketpp::lib::error_code ec{};
    m_WsppServer.send(hdl.value(), payload, websocketpp::frame::opcode::text, ec);
    if (ec)
    {
      LOG("Failed to send web socket:" << ec.message());
      return false;
    }
    return true;
  }
  return false;
}

void WebSocketServer::SetWebSocketDataReceived(const WebSocketDataReceivedFn& fn)
{
  m_WebSocketDataReceivedFn = fn;
}