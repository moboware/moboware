#pragma once
#include <chrono>
#include <ctime>
#include <ostream>
#include <string>

namespace moboware::modules {

using PriceType_t = std::uint64_t;
using VolumeType_t = std::uint64_t;
using OrderTime_t = std::chrono::time_point<std::chrono::high_resolution_clock>;
using Id_t = std::string;
using ClientId_t = std::string;

class OrderDataBase
{
public:
  OrderDataBase() = default;

  explicit OrderDataBase(const std::string& _account,                     //
                         const std::string& _instrument,                  //
                         const PriceType_t& _price,                       //
                         const VolumeType_t& _volume,                     //
                         const std::string& _type,                        //
                         const bool _isBuySide,                           //
                         const OrderTime_t& _orderTime,                   //
                         const std::chrono::milliseconds& _orderDuration, //
                         const Id_t& _id,                                 //
                         const ClientId_t& _clientId                      //
  );

  [[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] const auto& GetAccount() const { return account; }
  void SetAccount(const std::string& _account) { account = _account; }

  [[nodiscard]] const auto& GetInstrument() const { return instrument; }
  void SetInstrument(const std::string& _instrument) { instrument = _instrument; }

  [[nodiscard]] const auto& GetPrice() const { return price; }
  void SetPrice(const PriceType_t& _price) { price = _price; }

  [[nodiscard]] auto GetPriceAsDouble() const { return static_cast<double>(GetPrice() / std::mega::num); }

  [[nodiscard]] const auto& GetVolume() const { return volume; }
  void SetVolume(const VolumeType_t& _volume) { volume = _volume; }

  [[nodiscard]] const auto& GetType() const { return type; }
  void SetType(const std::string& _type) { type = _type; }

  [[nodiscard]] auto GetIsBuySide() const { return isBuySide; }
  void SetIsBuySide(const bool _isBuySide) { isBuySide = _isBuySide; }

  [[nodiscard]] const auto& GetOrderTime() const { return orderTime; }
  void SetOrderTime(const OrderTime_t& _orderTime) { orderTime = _orderTime; }

  [[nodiscard]] const auto& GetOrderDuration() const { return orderDuration; }
  void SetOrderDuration(const std::chrono::milliseconds& _orderDuration) { orderDuration = _orderDuration; }

  [[nodiscard]] const auto& GetId() const { return id; }
  void SetId(const Id_t& _id) { id = _id; }

  [[nodiscard]] const auto& GetClientId() const { return clientId; }
  void SetClientId(const ClientId_t& _clientId) { clientId = _clientId; }

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
  bool isBuySide{ true };
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
class OrderData : public OrderDataBase
{
public:
  OrderData() = default;
  explicit OrderData(const std::string& _account,                     //
                     const std::string& _instrument,                  //
                     const PriceType_t& _price,                       //
                     const VolumeType_t& _volume,                     //
                     const std::string& _type,                        //
                     const bool _isBuySide,                           //
                     const OrderTime_t& _orderTime,                   //
                     const std::chrono::milliseconds& _orderDuration, //
                     const Id_t& _id,                                 //
                     const ClientId_t& _clientId                      //
  );
};

class OrderAmendData final : public OrderDataBase
{
public:
  OrderAmendData() = default;
  explicit OrderAmendData(const std::string& _account,                     //
                          const std::string& _instrument,                  //
                          const PriceType_t& _price,                       //
                          const PriceType_t& _newPrice,                    //
                          const VolumeType_t& _volume,                     //
                          const VolumeType_t& _newVolume,                  //
                          const std::string& _type,                        //
                          const bool _isBuySide,                           //
                          const OrderTime_t& _orderTime,                   //
                          const std::chrono::milliseconds& _orderDuration, //
                          const Id_t& _id,                                 //
                          const ClientId_t& _clientId                      //
  );

  [[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] auto GetNewPrice() const -> const PriceType_t& { return newPrice; }
  void SetNewPrice(const PriceType_t& _newPrice) { newPrice = _newPrice; }

  [[nodiscard]] auto GetNewVolume() const -> const VolumeType_t& { return newVolume; }
  void SetNewVolume(const VolumeType_t& _newVolume) { newVolume = _newVolume; }

private:
  /// @brief new price of the order
  PriceType_t newPrice{};
  /// @brief volume of the order
  VolumeType_t newVolume{};
};

/**
 * @brief Order reply struct
 */
class OrderReply final
{
public:
  OrderReply() = default;
  explicit OrderReply(const Id_t& _id,            //
                      const ClientId_t& _clientId //
  );

  [[nodiscard]] const auto& GetId() const { return id; }
  [[nodiscard]] const auto& GetClientId() const { return clientId; }

  auto operator==(const OrderReply& rhs) const -> bool;

private:
  /// @brief generated order id
  Id_t id;
  /// @brief order id assigned by the client
  ClientId_t clientId;
};

/**
 * @brief order cancel data struct
 */
class OrderCancelData final
{
public:
  OrderCancelData() = default;
  explicit OrderCancelData(const std::string& _instrument, //
                           const PriceType_t& _price,      //
                           const bool _isBuySide,          //
                           const Id_t& _id,                //
                           const ClientId_t& _clientId     //
  );

  [[nodiscard]] auto GetInstrument() const -> const std::string& { return instrument; }
  void SetInstrument(const std::string& _instrument) { instrument = _instrument; }

  [[nodiscard]] auto GetPrice() const -> const PriceType_t& { return price; }
  void SetPrice(const PriceType_t& _price) { price = _price; }

  [[nodiscard]] auto GetPriceAsDouble() const -> double { return static_cast<double>(GetPrice() / std::mega::num); }

  [[nodiscard]] auto GetIsBuySide() const { return isBuySide; }
  void SetIsBuySide(const bool _isBuySide) { isBuySide = _isBuySide; }

  [[nodiscard]] const auto& GetId() const { return id; }
  void SetId(const Id_t& _id) { id = _id; }

  [[nodiscard]] const auto& GetClientId() const { return clientId; }
  void SetClientId(const ClientId_t& _clientId) { clientId = _clientId; }

  [[nodiscard]] auto Validate() const -> bool;

private:
  /// @brief instrument
  std::string instrument;
  /// @brief prices are in 6 places precision todo add a scaling factor to the message?
  PriceType_t price{};
  /// @brief buy or sell order
  bool isBuySide{ true };
  /// @brief generated order id
  Id_t id;
  /// @brief order id assigned by the client
  ClientId_t clientId;
};

/**
 * @brief Trade object
 */
class Trade final
{
public:
  Trade() = default;
  explicit Trade(const std::string& _account,       //
                 const PriceType_t& _tradedPrice,   //
                 const VolumeType_t& _tradedvolume, //
                 const Id_t& _id,                   //
                 const ClientId_t& _clientId);

  [[nodiscard]] auto Validate() const -> bool;

  [[nodiscard]] const auto& GetAccount() const { return account; }
  void SetAccount(const std::string& _account) { account = _account; }

  [[nodiscard]] const auto& GetTradedPrice() const { return tradedPrice; }
  [[nodiscard]] auto GetTradedPriceAsDouble() const { return static_cast<double>(GetTradedPrice() / std::mega::num); }
  void SetTradedPrice(const PriceType_t& _price) { tradedPrice = _price; }

  [[nodiscard]] const auto& GetTradedVolume() const { return tradedVolume; }
  const auto& SetTradedVolume(const VolumeType_t& _volume)
  {
    tradedVolume = _volume;
    return tradedVolume;
  }

  [[nodiscard]] const auto& GetId() const { return id; }
  void SetId(const Id_t& _id) { id = _id; }

  [[nodiscard]] const auto& GetClientId() const { return clientId; }
  void SetClientId(const ClientId_t& _clientId) { clientId = _clientId; }

  auto operator==(const Trade& rhs) const -> bool;

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
class ErrorReply final
{
public:
  ErrorReply() = default;
  explicit ErrorReply(const std::string& _clientId, const std::string& _errorMessage)
    : clientId(_clientId)
    , errorMessage(_errorMessage)
  {
  }

  [[nodiscard]] const auto& GetClientId() const { return clientId; }
  void SetClientId(const ClientId_t& _clientId) { clientId = _clientId; }

  [[nodiscard]] const auto& GetErrorMessage() const { return errorMessage; }
  void SetErrorMessage(const std::string& _errorMessage) { errorMessage = _errorMessage; }

private:
  /// @brief order id assigned by the client
  ClientId_t clientId;
  //
  std::string errorMessage;
};

// ostream operators
std::ostream& operator<<(std::ostream& os, const OrderTime_t& rhs);
std::ostream& operator<<(std::ostream& os, const OrderData& rhs);
std::ostream& operator<<(std::ostream& os, const Trade& rhs);

}