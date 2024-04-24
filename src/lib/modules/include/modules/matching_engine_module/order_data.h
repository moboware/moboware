#pragma once
#include <boost/json.hpp>
#include <chrono>
#include <ctime>
#include <ostream>
#include <sstream>
#include <string>

namespace moboware::modules {

using PriceType_t = std::uint64_t;
using VolumeType_t = std::uint64_t;
using OrderTime_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Id_t = std::string;
using ClientId_t = std::string;

namespace Fields {
static const std::string Action{"Action"};
static const std::string Data{"Data"};
static const std::string Insert{"Insert"};
static const std::string Cancel{"Cancel"};
static const std::string Amend{"Amend"};
static const std::string GetBook{"GetBook"};
static const std::string Account{"Account"};
static const std::string Price{"Price"};
static const std::string NewPrice{"NewPrice"};
static const std::string TradedPrice{"TradedPrice"};
static const std::string Volume{"Volume"};
static const std::string NewVolume{"NewVolume"};
static const std::string TradedVolume{"TradedVolume"};
static const std::string Type{"Type"};
static const std::string IsBuy{"IsBuy"};
static const std::string ClientId{"ClientId"};
static const std::string Id{"Id"};
static const std::string Instrument{"Instrument"};
static const std::string Error{"Error"};
static const std::string Trade{"Trade"};
static const std::string Time{"Time"};
static const std::string Side{"Side"};
static const std::string OrderReply{"OrderReply"};
static const std::string ErrorReply{"ErrorReply"};

}   // namespace Fields

/**
 * @brief base class for order data
 */
class OrderDataBase {
public:
  OrderDataBase() = default;

  explicit OrderDataBase(const std::string &_account,                       //
                         const std::string &_instrument,                    //
                         const PriceType_t &_price,                         //
                         const VolumeType_t &_volume,                       //
                         const std::string &_type,                          //
                         const bool _isBuySide,                             //
                         const OrderTime_t &_orderTime,                     //
                         const std::chrono::milliseconds &_orderDuration,   //
                         const Id_t &_id,                                   //
                         const ClientId_t &_clientId                        //
  );

  [[nodiscard]] auto SetData(const boost::json::value &data) -> bool;

  [[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] const auto &GetAccount() const
  {
    return account;
  }

  void SetAccount(const std::string &_account)
  {
    account = _account;
  }

  [[nodiscard]] const auto &GetInstrument() const
  {
    return instrument;
  }

  void SetInstrument(const std::string &_instrument)
  {
    instrument = _instrument;
  }

  [[nodiscard]] inline const auto &GetPrice() const noexcept
  {
    return price;
  }

  inline void SetPrice(const PriceType_t &_price) noexcept
  {
    price = _price;
  }

  [[nodiscard]] inline auto GetPriceAsDouble() const noexcept
  {
    return static_cast<double>(GetPrice() / std::mega::num);
  }

  [[nodiscard]] inline const auto &GetVolume() const noexcept
  {
    return volume;
  }

  inline void SetVolume(const VolumeType_t &_volume) noexcept
  {
    volume = _volume;
  }

  [[nodiscard]] inline const auto &GetType() const noexcept
  {
    return type;
  }

  inline void SetType(const std::string &_type) noexcept
  {
    type = _type;
  }

  [[nodiscard]] inline auto GetIsBuySide() const noexcept
  {
    return isBuySide;
  }

  inline void SetIsBuySide(const bool _isBuySide) noexcept
  {
    isBuySide = _isBuySide;
  }

  [[nodiscard]] inline const auto &GetOrderTime() const noexcept
  {
    return orderTime;
  }

  inline void SetOrderTime(const OrderTime_t &_orderTime) noexcept
  {
    orderTime = _orderTime;
  }

  [[nodiscard]] inline const auto &GetOrderDuration() const noexcept
  {
    return orderDuration;
  }

  inline void SetOrderDuration(const std::chrono::milliseconds &_orderDuration) noexcept
  {
    orderDuration = _orderDuration;
  }

  [[nodiscard]] inline const auto &GetId() const noexcept
  {
    return id;
  }

  inline void SetId(const Id_t &_id) noexcept
  {
    id = _id;
  }

  [[nodiscard]] inline const auto &GetClientId() const noexcept
  {
    return clientId;
  }

  inline void SetClientId(const ClientId_t &_clientId) noexcept
  {
    clientId = _clientId;
  }

private:
  /// @brief user account the order is traded on
  std::string account{};
  /// @brief instrument
  std::string instrument{};
  /// @brief prices are in 6 places precision todo add a scaling factor to the message?
  PriceType_t price{};
  /// @brief volume of the order
  VolumeType_t volume{};
  /// @brief order type, like: Limit, Stop, Market, etc...
  std::string type{};
  /// @brief buy or sell order
  bool isBuySide{true};
  /// @brief creation time of the order
  OrderTime_t orderTime{};
  /// @brief  time that this order can live in milli seconds
  std::chrono::milliseconds orderDuration{};
  /// @brief generated order id
  Id_t id{};
  /// @brief order id assigned by the client
  ClientId_t clientId{};
};

/**
 * @brief Order data struct used for insert and amend orders
 */
class OrderInsertData final : public OrderDataBase {
public:
  OrderInsertData() = default;
  explicit OrderInsertData(const std::string &_account,                       //
                           const std::string &_instrument,                    //
                           const PriceType_t &_price,                         //
                           const VolumeType_t &_volume,                       //
                           const std::string &_type,                          //
                           const bool _isBuySide,                             //
                           const OrderTime_t &_orderTime,                     //
                           const std::chrono::milliseconds &_orderDuration,   //
                           const Id_t &_id,                                   //
                           const ClientId_t &_clientId                        //
  );
};

class OrderAmendData final : public OrderDataBase {
public:
  OrderAmendData() = default;
  explicit OrderAmendData(const std::string &_account,                       //
                          const std::string &_instrument,                    //
                          const PriceType_t &_price,                         //
                          const PriceType_t &_newPrice,                      //
                          const VolumeType_t &_volume,                       //
                          const VolumeType_t &_newVolume,                    //
                          const std::string &_type,                          //
                          const bool _isBuySide,                             //
                          const OrderTime_t &_orderTime,                     //
                          const std::chrono::milliseconds &_orderDuration,   //
                          const Id_t &_id,                                   //
                          const ClientId_t &_clientId                        //
  );

  [[nodiscard]] auto SetData(const boost::json::value &data) -> bool;

  [[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] inline auto GetNewPrice() const -> const PriceType_t &
  {
    return newPrice;
  }

  inline void SetNewPrice(const PriceType_t &_newPrice) noexcept
  {
    newPrice = _newPrice;
  }

  [[nodiscard]] inline auto GetNewVolume() const -> const VolumeType_t &
  {
    return newVolume;
  }

  inline void SetNewVolume(const VolumeType_t &_newVolume) noexcept
  {
    newVolume = _newVolume;
  }

private:
  /// @brief new price of the order
  PriceType_t newPrice{};
  /// @brief volume of the order
  VolumeType_t newVolume{};
};

/**
 * @brief Order reply struct
 */
class OrderReply final {
public:
  OrderReply() = default;
  explicit OrderReply(const Id_t &_id,              //
                      const ClientId_t &_clientId   //
  );

  [[nodiscard]] const auto &GetId() const noexcept
  {
    return id;
  }

  [[nodiscard]] const auto &GetClientId() const noexcept
  {
    return clientId;
  }

  [[nodiscard]] inline auto operator==(const OrderReply &rhs) const noexcept -> bool
  {
    return id == rhs.id and clientId == rhs.clientId;
  };

private:
  /// @brief generated order id
  Id_t id{};
  /// @brief order id assigned by the client
  ClientId_t clientId{};
};

/**
 * @brief order cancel data struct
 */
class OrderCancelData final {
public:
  OrderCancelData() = default;
  explicit OrderCancelData(const std::string &_instrument,   //
                           const PriceType_t &_price,        //
                           const bool _isBuySide,            //
                           const Id_t &_id,                  //
                           const ClientId_t &_clientId       //
  );

  [[nodiscard]] inline auto SetData(const boost::json::value &data) -> bool
  {
    SetInstrument(data.at(Fields::Instrument).as_string().c_str());
    SetPrice(data.at(Fields::Price).as_int64());
    SetIsBuySide(data.at(Fields::IsBuy).as_bool());
    SetId(data.at(Fields::Id).as_string().c_str());
    SetClientId(data.at(Fields::ClientId).as_string().c_str());

    return Validate();
  }

  [[nodiscard]] inline auto GetInstrument() const -> const std::string &
  {
    return instrument;
  }

  inline void SetInstrument(const std::string &_instrument)
  {
    instrument = _instrument;
  }

  [[nodiscard]] inline auto GetPrice() const -> const PriceType_t &
  {
    return price;
  }

  inline void SetPrice(const PriceType_t &_price)
  {
    price = _price;
  }

  [[nodiscard]] inline auto GetPriceAsDouble() const -> double
  {
    return static_cast<double>(GetPrice() / std::mega::num);
  }

  [[nodiscard]] inline auto GetIsBuySide() const
  {
    return isBuySide;
  }

  inline void SetIsBuySide(const bool _isBuySide)
  {
    isBuySide = _isBuySide;
  }

  [[nodiscard]] inline const auto &GetId() const
  {
    return id;
  }

  inline void SetId(const Id_t &_id)
  {
    id = _id;
  }

  [[nodiscard]] inline const auto &GetClientId() const
  {
    return clientId;
  }

  inline void SetClientId(const ClientId_t &_clientId)
  {
    clientId = _clientId;
  }

  [[nodiscard]] inline auto Validate() const -> bool
  {
    return (not instrument.empty()) and   //
           price > 0 and                  //
           (not id.empty()) and           //
           (not clientId.empty()) and     //
           (isBuySide == true or isBuySide == false);
  }

private:
  /// @brief instrument
  std::string instrument;
  /// @brief prices are in 6 places precision todo add a scaling factor to the message?
  PriceType_t price{};
  /// @brief buy or sell order
  bool isBuySide{true};
  /// @brief generated order id
  Id_t id;
  /// @brief order id assigned by the client
  ClientId_t clientId;
};

/**
 * @brief Trade object
 */
class Trade final {
public:
  Trade() = default;
  explicit Trade(const std::string &_account,         //
                 const PriceType_t &_tradedPrice,     //
                 const VolumeType_t &_tradedVolume,   //
                 const Id_t &_id,                     //
                 const ClientId_t &_clientId);

  //[[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] inline const auto &GetAccount() const
  {
    return account;
  }

  inline void SetAccount(const std::string &_account)
  {
    account = _account;
  }

  [[nodiscard]] inline const auto &GetTradedPrice() const
  {
    return tradedPrice;
  }

  [[nodiscard]] inline auto GetTradedPriceAsDouble() const
  {
    return static_cast<double>(GetTradedPrice() / std::mega::num);
  }

  inline void SetTradedPrice(const PriceType_t &_price)
  {
    tradedPrice = _price;
  }

  [[nodiscard]] inline const auto &GetTradedVolume() const
  {
    return tradedVolume;
  }

  inline const auto &SetTradedVolume(const VolumeType_t &_volume)
  {
    tradedVolume = _volume;
    return tradedVolume;
  }

  [[nodiscard]] inline const auto &GetId() const
  {
    return id;
  }

  inline void SetId(const Id_t &_id)
  {
    id = _id;
  }

  [[nodiscard]] inline const auto &GetClientId() const
  {
    return clientId;
  }

  inline void SetClientId(const ClientId_t &_clientId)
  {
    clientId = _clientId;
  }

  inline auto operator==(const Trade &rhs) const -> bool
  {
    return account == rhs.account and             //
           tradedPrice == rhs.tradedPrice and     //
           tradedVolume == rhs.tradedVolume and   //
           clientId == rhs.clientId and           //
           id == rhs.id;
  }

private:
  std::string account{};
  PriceType_t tradedPrice{};
  VolumeType_t tradedVolume{};
  Id_t id{};
  ClientId_t clientId{};
};

/**
 * @brief ErroReply object
 */
class ErrorReply final {
public:
  ErrorReply() = default;

  explicit ErrorReply(const std::string &_clientId, const std::string &_errorMessage)
      : clientId(_clientId)
      , errorMessage(_errorMessage)
  {
  }

  [[nodiscard]] inline const auto &GetClientId() const
  {
    return clientId;
  }

  inline void SetClientId(const ClientId_t &_clientId)
  {
    clientId = _clientId;
  }

  [[nodiscard]] inline const auto &GetErrorMessage() const
  {
    return errorMessage;
  }

  inline void SetErrorMessage(const std::string &_errorMessage)
  {
    errorMessage = _errorMessage;
  }

private:
  /// @brief order id assigned by the client
  ClientId_t clientId;
  //
  std::string errorMessage;
};

// ostream operators
std::ostream &operator<<(std::ostream &os, const OrderTime_t &rhs);
std::ostream &operator<<(std::ostream &os, const OrderInsertData &rhs);
std::ostream &operator<<(std::ostream &os, const OrderReply &rhs);
std::ostream &operator<<(std::ostream &os, const Trade &rhs);
std::ostream &operator<<(std::ostream &os, const OrderCancelData &rhs);
std::ostream &operator<<(std::ostream &os, const OrderAmendData &rhs);

// message construction operators
std::ostringstream &operator<<(std::ostringstream &os, const OrderReply &orderReply);
std::ostringstream &operator<<(std::ostringstream &os, const ErrorReply &errorReply);
std::ostringstream &operator<<(std::ostringstream &os, const Trade &orderReply);

}   // namespace moboware::modules