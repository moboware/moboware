#include "modules/matching_engine_module/order_data.h"

namespace moboware::modules {

static timeval ConvertToTimeval(const OrderTime_t& orderTime)
{
  timeval tv{};

  const auto usec = std::chrono::duration_cast<std::chrono::microseconds>(orderTime.time_since_epoch());
  if (usec > std::chrono::microseconds(0)) {
    tv.tv_sec = usec.count() / 1'000'000U;
    tv.tv_usec = usec.count() % 1'000'000U;
  }
  return tv;
}

std::ostream& operator<<(std::ostream& os, const Trade& rhs)
{
  return os << "{"
            << "Account:" << rhs.GetAccount()             //
            << ", TradedPrice:" << rhs.GetTradedPrice()   //
            << ", TradedVolume:" << rhs.GetTradedVolume() //
            << ", ClientId:" << rhs.GetClientId()         //
            << ", OrderId:" << rhs.GetId()                //
            << "}";
}

std::ostream& operator<<(std::ostream& os, const OrderTime_t& rhs)
{
  timeval tv{ ConvertToTimeval(rhs) };

  const auto orderTime_t{ std::chrono::system_clock::to_time_t(rhs) };
  const auto gmTime{ gmtime(&tv.tv_sec) };

  return os << gmTime->tm_hour << ":" << gmTime->tm_min << ":" << gmTime->tm_sec << "." << tv.tv_usec;
}

std::ostream& operator<<(std::ostream& os, const OrderData& rhs)
{
  return os << "Instrument:" << rhs.GetInstrument()             //
            << ",Price:" << rhs.GetPrice()                      //
            << ",Volume:" << rhs.GetVolume()                    //
            << ",Side:" << (rhs.GetIsBuySide() ? "Bid" : "Ask") //
            << ",Id:" << rhs.GetId()                            //
            << ",ClientId:" << rhs.GetClientId()                //
            << ",Account:" << rhs.GetAccount()                  //
            << ",Type:" << rhs.GetType()                        //
            << ",Time:" << rhs.GetOrderTime()                   //
            << "";
}

}
using namespace moboware::modules;

auto OrderReply::operator==(const OrderReply& rhs) const -> bool
{
  return id == rhs.id && //
         clientId == rhs.clientId;
}

Trade::Trade(const std::string& _account,       //
             const PriceType_t& _tradedPrice,   //
             const VolumeType_t& _tradedvolume, //
             const Id_t& _id,                   //
             const ClientId_t& _clientId)
  : account(_account)
  , tradedPrice(_tradedPrice)
  , tradedVolume(_tradedvolume)
  , id(_id)
  , clientId(_clientId)
{
}

auto Trade::operator==(const Trade& rhs) const -> bool
{
  return account == rhs.account &&           //
         tradedPrice == rhs.tradedPrice &&   //
         tradedVolume == rhs.tradedVolume && //
         clientId == rhs.clientId &&         //
         id == rhs.id;
}

auto OrderDataBase::Validate() const -> bool
{
  return price > 0 &&                                //
         volume > 0 &&                               //
         orderTime.time_since_epoch().count() > 0 && //
         not id.empty() &&                           //
         not clientId.empty() &&                     //
         not account.empty() &&                      //
         not instrument.empty() &&                   //
         (isBuySide == true || isBuySide == false);
}

auto OrderAmendData::Validate() const -> bool
{
  return OrderDataBase::Validate() && //
         newPrice > 0 &&              //
         newVolume > 0;
}

OrderCancelData::OrderCancelData(const std::string& _instrument, //
                                 const PriceType_t& _price,      //
                                 const bool _isBuySide,          //
                                 const Id_t& _id,                //
                                 const ClientId_t& _clientId     //
                                 )
  : instrument(_instrument)
  , price(_price)
  , isBuySide(_isBuySide)
  , id(_id)
  , clientId(_clientId)
{
}

auto OrderCancelData::Validate() const -> bool
{
  return not instrument.empty() && //
         price > 0 &&              //
         not id.empty() &&         //
         not clientId.empty() &&   //
         (isBuySide == true || isBuySide == false);
}
OrderAmendData::OrderAmendData(const std::string& _account,                     //
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
                               )
  : OrderDataBase(_account, _instrument, _price, _volume, _type, _isBuySide, _orderTime, _orderDuration, _id, _clientId)
  , newPrice(_newPrice)
  , newVolume(_newVolume)
{
}

OrderData::OrderData(const std::string& _account,                     //
                     const std::string& _instrument,                  //
                     const PriceType_t& _price,                       //
                     const VolumeType_t& _volume,                     //
                     const std::string& _type,                        //
                     const bool _isBuySide,                           //
                     const OrderTime_t& _orderTime,                   //
                     const std::chrono::milliseconds& _orderDuration, //
                     const Id_t& _id,                                 //
                     const ClientId_t& _clientId                      //
                     )
  : OrderDataBase(_account, _instrument, _price, _volume, _type, _isBuySide, _orderTime, _orderDuration, _id, _clientId)
{
}

OrderDataBase::OrderDataBase(const std::string& _account,                     //
                             const std::string& _instrument,                  //
                             const PriceType_t& _price,                       //
                             const VolumeType_t& _volume,                     //
                             const std::string& _type,                        //
                             const bool _isBuySide,                           //
                             const OrderTime_t& _orderTime,                   //
                             const std::chrono::milliseconds& _orderDuration, //
                             const Id_t& _id,                                 //
                             const ClientId_t& _clientId                      //
                             )
  : account(_account)
  , instrument(_instrument)
  , price(_price)
  , volume(_volume)
  , type(_type)
  , isBuySide(_isBuySide)
  , orderTime(_orderTime)
  , orderDuration(_orderDuration)
  , id(_id)
  , clientId(_clientId)
{
}

OrderReply::OrderReply(const Id_t& _id, //
                       const ClientId_t& _clientId)
  : id(_id)
  , clientId(_clientId)
{
}