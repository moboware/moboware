#pragma once

#include <map>
#include <optional>
#include <cstdint>
#include <websocketpp/common/connection_hdl.hpp>

namespace moboware::web_socket_server
{
  /**
   * @brief Manage 2 maps to 'map' a connection hdl to  tag and tags to connection  handle
   */
  class TagMap final
  {
  public:
    TagMap() = default;
    virtual ~TagMap() = default;
    TagMap(const TagMap&) = delete;
    TagMap(TagMap&&) = delete;
    TagMap& operator=(const TagMap&) = delete;
    TagMap& operator=(TagMap&&) = delete;

    std::uint64_t Erase(const websocketpp::connection_hdl& hdl);
    std::uint64_t Insert(const websocketpp::connection_hdl& hdl);
    std::optional<websocketpp::connection_hdl> Find(const std::uint64_t tag);

  private:
    std::map<websocketpp::connection_hdl, std::uint64_t, std::owner_less<websocketpp::connection_hdl>> m_WebSocketHandleToTag;
    std::map<std::uint64_t, websocketpp::connection_hdl> m_TagToWebSocketHandle;
    std::uint64_t m_LastTag{}; /// last used tag
  };
}