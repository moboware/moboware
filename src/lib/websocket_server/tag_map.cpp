#include "websocket_server/tag_map.h"

using namespace moboware::web_socket_server;

std::uint64_t TagMap::Insert(const websocketpp::connection_hdl& hdl)
{
  std::uint64_t tag{};
  // store the websocket handle and provide a private handle for callbacks
  const auto iter = m_WebSocketHandleToTag.find(hdl);
  if (iter == m_WebSocketHandleToTag.end())
  {
    tag = ++m_LastTag;

    m_WebSocketHandleToTag[hdl] = tag;
    m_TagToWebSocketHandle[tag] = hdl;
  }
  else
  {
    tag = iter->second;
  }
  return tag;
}

std::uint64_t TagMap::Erase(const websocketpp::connection_hdl& hdl)
{
  const auto iter = m_WebSocketHandleToTag.find(hdl);
  if (iter != m_WebSocketHandleToTag.end())
  {
    const auto tag = iter->second;
    m_WebSocketHandleToTag.erase(iter);
    m_TagToWebSocketHandle.erase(tag);
    return tag;
  }
  return 0;
}

std::optional<websocketpp::connection_hdl> TagMap::Find(const std::uint64_t tag)
{
  const auto iter = m_TagToWebSocketHandle.find(tag);
  if (iter != m_TagToWebSocketHandle.end())
  {
    const auto hdl = iter->second;
    return hdl;
  }
  return {};
}
