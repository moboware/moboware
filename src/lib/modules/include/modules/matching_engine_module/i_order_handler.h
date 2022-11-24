#pragma once
#include <boost/asio/ip/tcp.hpp>
#include <chrono>
#include <json/json.h>

namespace moboware::modules {

using PriceType_t = std::uint64_t;
using VolumeType_t = std::uint64_t;
using OrderTime_t = std::chrono::time_point<std::chrono::high_resolution_clock>;

struct OrderData
{
  /// @brief user account the order is traded on
  std::string account;
  /// @brief instrument
  std::string instrument;
  /// @brief prices are in 6 places precision todo add a scaling factor to the message?
  PriceType_t price{};
  /// @brief volume of the order
  VolumeType_t volume{};
  /// @brief order type, like: Limit, Stop, Market, etc...
  std::string type;
  /// @brief buy or sell order
  bool IsBuySide{ true };
  /// @brief creation time of the order
  OrderTime_t orderTime{};
  /// @brief  time that this order can live in milli seconds
  std::chrono::milliseconds orderDuration{};
  /// @brief generated order id
  std::string id;
  /// @brief order id assigned by the client
  std::string clientId;

  auto Validate() const -> auto
  {
    return price > 0 &&                                //
           volume > 0 &&                               //
           orderTime.time_since_epoch().count() > 0 && //
           not id.empty() &&                           //
           not clientId.empty() &&                     //
           not account.empty() &&                      //
           not instrument.empty() &&                   //
           (IsBuySide == true || IsBuySide == false);
  }
};

struct OrderInsertReply
{
  /// @brief generated order id
  std::string id;
  /// @brief order id assigned by the client
  std::string clientId;
  bool operator==(const OrderInsertReply& rhs) const { return id == rhs.id && clientId == clientId; }
};

struct Trade
{
  std::string account;
  PriceType_t tradedPrice{};
  VolumeType_t tradedVolume{};
  std::string clientId;
  std::string id;
  bool operator==(const Trade& rhs) const
  {
    return account == rhs.account && tradedPrice == rhs.tradedPrice && tradedVolume == rhs.tradedVolume && clientId == rhs.clientId && id == rhs.id;
  }
};

struct ErrorReply
{
  /// @brief order id assigned by the client
  std::string clientId;
  //
  std::string errorMessage;
};

class IOrderHandler
{
public:
  IOrderHandler() = default;
  virtual ~IOrderHandler() = default;

  virtual void HandleOrderInsert(const OrderData& orderInsert, const boost::asio::ip::tcp::endpoint& endpoint) = 0;
};
}
